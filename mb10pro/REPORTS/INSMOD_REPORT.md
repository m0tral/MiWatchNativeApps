# mb10pro `insmod` Reverse-Engineering Report

Firmware analysed: `vela_ap.bin` (mb10pro fw_3.101.043), IDA database
`C:\Xiaomi\MiBand10Pro\fw_3.101.043_IDA\vela_ap.bin.i64`.

All addresses below are from the loaded IDB; cross-checked against the
Apache NuttX 11.0 source tree for the loader contract.

---

## 1. NSH command surface

| Symbol                | Address      | Size  | Notes |
|-----------------------|--------------|-------|-------|
| `nsh_cmd_insmod`      | `0x2c23ec54` | `0x38` | NSH builtin dispatcher |
| `nsh_cmd_rmmod`       | `0x2c23ec94` | `0x4c` | Calls `modhandle()` then `rmmod()` |
| `nsh_cmd_lsmod`       | `0x2c23ecec` | `0x172`| Reads `/proc/modules` |

### Loader API (renamed from `sub_xxx` in the IDB)

| Symbol                  | Address      | Role |
|-------------------------|--------------|------|
| `insmod`                | `0x2C203A50` | libc user-facing entry |
| `rmmod`                 | `0x2C203A5C` | libc user-facing entry |
| `modhandle`             | `0x2C203A68` | libc user-facing entry |
| `modlib_insert`         | `0x2C1E8650` | main loader (`modlib_insert.c:297`) |
| `modlib_load`           | `0x2C1E7BF8` | loads sections, `e_type` check (`modlib_load.c:557`) |
| `modlib_initialize`     | `0x2C1E7B18` | opens ELF, reads header |
| `modlib_uninitialize`   | `0x2C1E8564` | releases `modlib_initialize` resources |
| `modlib_bind`           | `0x2C1E70BC` | relocates + resolves imports |
| `modlib_unload`         | `0x2C1E85E0` | error-path unload |
| `modlib_undepend`       | `0x2C1E7ABC` | tears down deps on failure |
| `modlib_registry_lock`  | `0x2C1E804C` | exclusive registry access |
| `modlib_registry_unlock`| `0x2C1E805C` | release registry lock |
| `modlib_registry_add`   | `0x2C1E806C` | push module onto list |
| `modlib_registry_find`  | `0x2C1E80D0` | lookup by name |
| `modhandle_impl`        | `0x2C1E8614` | internal `modhandle()` body |
| `rmmod_impl`            | `0x2C1E88D8` | internal `rmmod()` body |

The string `"module_initialize"` does **not** appear in `vela_ap.bin`.
The loader does not resolve the entry symbol by name — it reads `e_entry`
straight from the ELF header.

---

## 2. Call chain

```
nsh_cmd_insmod(argc, argv)                          @ 0x2c23ec54
   └── insmod(filename, modname)                    @ 0x2C203A50   (libc entry)
        └── modlib_insert(...)                      @ 0x2C1E8650
              assert: "../../nuttx/libs/libc/modlib/modlib_insert.c", line 297
             ├── modlib_registry_lock()             @ 0x2C1E804C
             ├── modlib_registry_find(modname)      @ 0x2C1E80D0
             ├── modlib_initialize(filename, &li)   @ 0x2C1E7B18   (open + read ELF hdr)
             ├── modlib_load(&li)                   @ 0x2C1E7BF8   ← HARD CHECK on e_type
             ├── malloc(164)                        -> struct module_s
             ├── modlib_bind(modp, &li)             @ 0x2C1E70BC   (resolve imports)
             ├── call (modinfo via e_entry)         @ 0x2c1e8778
             └── modlib_registry_add(modp)          @ 0x2C1E806C
```

`nsh_cmd_rmmod`:
```
modhandle("name")            @ 0x2C203A68  -> modhandle_impl() @ 0x2C1E8614
rmmod(handle)                @ 0x2C203A5C  -> rmmod_impl()     @ 0x2C1E88D8
```

`nsh_cmd_lsmod` opens `/proc/modules` and parses lines split by `,\n`
into the columns `NAME, INIT, UNINIT, ARG, NEXPORTS, TEXT_SIZE,
DATA_SIZE`. The procfs file is produced by the kernel-side
`sched/module/mod_procfs.c` (not present in `vela_ap.bin`).

---

## 3. Critical contract

### 3.1 ELF type

`modlib_load` (`0x2C1E7BF8`, assert path
`libs/libc/modlib/modlib_load.c:557`) opens with:

```c
v2 = *(uint16_t *)(a1 + 60);   /* e_type */
if (v2 == 3) { /* accept */ }
```

- **`e_type` must be `ET_DYN` (3)** — shared object.
- `ET_REL` (1) is **rejected** here even though `modlib_insert`'s later
  guard (`(e_type & 0xFFFD) == 1`) would have allowed it. The `modlib_load`
  check runs first.
- `e_type = 2` (executable) and `4` (core) are also rejected.

Conclusion: the module must be built as a position-independent
shared object (`-shared -fPIC`).

### 3.2 Entry point

After successful load, `modlib_insert` (around `0x2c1e8778` in the
loaded IDB) does:

```c
if ((ehdr.e_type & 0xFFFD) == 1) {
    for (i = 0; i < v46; i++) v43[i]();     /* ctor / preinit array */
    for (i = 0; i < v44; i++) v41[i]();     /* init array */
    /* store modinfo -> {exports, nexports, arg, uninit} */
}
```

The entry function itself is invoked via the value of `e_entry`; the
loader doesn't know or care that we call it `module_initialize`.
For `modlib_load` to consider the load a success, the entry section
just needs to be marked `SHT_PROGBITS` and be reachable.

**Build requirement:** `-Wl,--entry=module_initialize`.

### 3.3 mod_info_s layout (32-bit ARM)

```
offset  field            used by insmod        used by rmmod
+0      uninitializer    —                     called at unload
+4      arg              —                     passed to uninitializer
+8      exports          exports symtab        —
+12     nexports         # of exports          —
```

The initializer must return `0` (OK) or a negated errno.

### 3.4 struct module_s size

`malloc(164)` is the exact size used by `modlib_insert` for the
registry entry. Field layout (32-bit):

```
+0     flink                          (4)
+4     modname[96]   MODLIB_NAMEMAX   (96)   ← strlcpy(modname, .., 96)
+100   initializer                    (4)   ← stored by the loader
+104   modinfo                        (16)  ← 4 * 32-bit fields
+120   textalloc   / datastart        (8)
+128   textsize    / datasize         (8)
+136   dependents                     (4)
+140   dependencies[...]              (CONFIG_MODLIB_MAXDEPEND * 4)
...                              total = 164
```

`MODLIB_NAMEMAX` in this build is **96** (not the upstream 16).

---

## 4. Symbol resolution

`modlib_bind` (`0x2C1E70BC`) walks the module's relocation table and
patches `Elf32_Rel` entries against the **process symbol table** that
`modlib_setsymtab()` registered at boot. This is the libc/user symtab
of `vela_ap.bin`, **not** the kernel symbol table.

### 4.1 Two ways for a module to reach firmware code

**(a) Loader-mediated binding (the generic path).** Declare the symbol
`extern`, leave it undefined in the module ELF, let `modlib_bind`
patch the GOT/PLT at `insmod` time. Works for any symbol the host
process exported.

**(b) Hardcoded absolute call (the path used by `mb10p_hello.c`).** Cast
the firmware address to a function pointer and call through it. The
module ELF ends up with **zero unresolved symbols** — `modlib_bind`
has nothing to do. Mirrors the existing `hello\hello_mb8p.c` /
`hello\hello_s3e.c` pattern (printf via a baked-in Thumb address).

```c
typedef int (*syslog_t)(int level, const char *fmt, ...);
#define syslog ((syslog_t)(0x2C1F0D20 | 1))   /* Thumb bit set */
```

The literal pool in the produced `.text` contains `0x2C1F0D21`
(the `|1` sets bit 0 — the Thumb indicator). The call sites emit
`blx rN` against the loaded value; `BLX` reads the LSB, switches the
core to Thumb state, clears the bit, and jumps to `0x2C1F0D20` where
`syslog` actually lives. Same `+1` convention used by
`hello\hello_mb8p.c` and `hello\hello_s3e.c` for `printf`/`puts`.

`arm-none-eabi-nm` on the final ELF shows **no `U` symbols** — the
whole binding table is empty.

### 4.2 Implications

- Choose (a) when the address isn't known, varies across firmware
  revisions, or when you want loader flexibility.
- Choose (b) when you want a fully self-contained ELF that insmods
  without any symtab requirement (and the address is fixed in the
  firmware image you're targeting).
- Do **not** link a newlib/libc archive into the module ELF either
  way; nothing in `mb10p_hello.c` needs it.

---

## 5. Verified build

```
arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -shared -fPIC -fno-plt \
    -nostdlib -nostartfiles -fno-builtin \
    "-Wl,--entry=module_initialize" \
    -o mb10p_hello.elf mb10pro/mb10p_hello.c
```

`readelf -h mb10p_hello.elf` (live output, fw_3.101.043 build):

```
Type:                              DYN (Shared object file)
Machine:                           ARM
Entry point address:               0x189
```

`nm` excerpt:

```
00000174 t hello_module_uninit
00000188 T module_initialize
         U printf
         U puts
```

Entry `0x189 = 0x188 + 1` — Thumb-bit set by the linker on the
`module_initialize` symbol. Correct for Cortex-M.

---

## 6. Deploy procedure

```
nsh> insmod /data/mb10p_hello.elf hello_module
hello_module: loaded into mb10pro userland (modinfo=0x...)
hello_module: hello from NuttX loadable module!
nsh> lsmod
NAME               INIT     UNINIT    ARG  NEXPORTS  TEXT  TEXT_SIZE  DATA  DATA_SIZE
hello_module       0x189    0x174     0x0       0  0x..  ...        0x..  ...
nsh> rmmod hello_module
```

The module is pushed onto the filesystem at `/data/`. On failure,
`nsh_cmd_insmod` prints `nsh: <argv[0]>: insmod failed: <errno>`.
