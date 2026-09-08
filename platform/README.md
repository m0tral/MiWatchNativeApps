# platform bindings

This directory owns every firmware-address binding used by the project's
`.c` files. The consumer TUs never see a firmware version -- they include
the single dispatcher header and it pulls in the right per-firmware file
for them.

## Layout

```
platform/
├── miwear_system.h               # top-level dispatcher (picks model)
├── mb10p/                        # Xiaomi Mi Band 10 Pro bindings
│   ├── miwear_system.h           # mb10p dispatcher (picks firmware)
│   ├── fw_3.101.043/
│   │   └── miwear_system.h       # full bindings for fw_3.101.043 (default)
│   ├── fw_3.201.016/
│   │   └── miwear_system.h       # full bindings for fw_3.201.016
│   └── (fw_2.1.58/, fw_3.1.175/ placeholders for mb9p legacy)
├── watchs3/                      # Xiaomi Watch S3 bindings
│   ├── miwear_system.h           # watchs3 dispatcher (picks firmware)
│   ├── fw_2.6.0/
│   │   └── miwear_system.h       # full bindings for fw_2.6.0
│   └── fw_4.8.0/
│       └── miwear_system.h       # full bindings for fw_4.8.0 (default)
├── README.md                     # this file
└── verify_no_eabi.ps1            # post-link __aeabi_* import scanner
```

`platform/miwear_system.h` is the *only* include site TUs need; it picks
the model (mb10p / watchs3) based on the `-D` macro, and the per-model
dispatcher then picks the firmware revision. All LVGL / NuttX / screen /
miwear / stdio symbols come from one include.

## Switching target model and firmware

Two `-D` flags select what gets compiled:

| -D macro                | Selects | Under           |
| ----------------------- | ------- | --------------- |
| `MB10P_FW_VERSION=N`    | mb10pro | `platform/mb10p/` |
| `WATCHS3_FW_VERSION=N`  | watchs3 | `platform/watchs3/` |

Within a model, the integer picks the firmware revision:

### mb10pro

| C2x form   | -D value  | Resolves to                            |
| ---------- | --------- | -------------------------------------- |
| 3_101_043  | 3101043   | `mb10p/fw_3.101.043/` (default)        |
| 3_201_016  | 3201016   | `mb10p/fw_3.201.016/`                  |

### watchs3

| C2x form   | -D value  | Resolves to                            |
| ---------- | --------- | -------------------------------------- |
| 4_8_0      | 480       | `watchs3/fw_4.8.0/` (default)          |
| 2_6_0      | 260       | `watchs3/fw_2.6.0/`                    |

The full version numbers (e.g. `3_101_043`) are the canonical names; the
underlying integers are written without digit separators because GCC 10.x
-- the version this project ships with -- doesn't accept `3_101_043` even
under `-std=gnu2x` (C2x digit-separator support landed in GCC 12).

```sh
# mb10pro default (fw_3.101.043 / 3_101_043)
arm-none-eabi-gcc ... -I. -Imb10pro -DMB10P_FW_VERSION=3101043 \
    mb10pro/mb10p_app.c -c -o mb10p_app.o

# mb10pro explicitly target fw_3.201.016
arm-none-eabi-gcc ... -I. -Imb10pro -DMB10P_FW_VERSION=3201016 \
    mb10pro/mb10p_app.c -c -o mb10p_app.o

# watchs3 default (fw_4.8.0 / 4_8_0)
arm-none-eabi-gcc ... -I. -Iwatchs3 -DWATCHS3_FW_VERSION=480 \
    watchs3/watchs3_app.c -c -o watchs3_app.o
```

The VS Code / Zed `tasks.json` inherits the default for each model; add a
duplicate task with the `-D` flag if you need to build against a newer
firmware.

## Usage

```c
#include "platform/miwear_system.h"
```

That's the only platform include every TU needs. The header exposes both
system bindings (`syslog`, `lv_*`, `lvx_reminder_*`, `screen_*`,
`snprintf`) and miwear bindings (`g_packagemanager_api`,
`app_launcher_add`, `app_lookup`).

## Adding a new firmware version

1. Reverse-engineer the symbols for the new `vela_ap.bin` (or pin down the
   addresses via the IDA Pro MCP tools); see `fw_func_search_prompt.md`.
2. Create `platform/<model>/fw_X.Y.Z/miwear_system.h` by copying the
   closest existing version's file and updating only the address macros.
   The header comment should cite the firmware build number and IDA
   instance/port.
3. Extend the `#if` ladder in the per-model dispatcher
   `platform/<model>/miwear_system.h` to recognise the new tag (e.g.
   `MB10P_FW_VERSION == 3301017  /* 3_301_017 */`) and resolve to the new
   directory.

## What does NOT change between versions

The per-version headers are intentionally small. They only:

* `#include` the shared family headers that own typedefs
  (`<common/nuttx/...>`, `<common/lvgl/...>`, `<common/miwear/...>`,
  `<common/misc/print.h>`).
* `#define` symbol -> address/typedef bindings.

Anything that's not an address (struct layouts, callbacks, prototypes,
`*_t` typedefs) lives in the family header and is the same for every
firmware revision.
