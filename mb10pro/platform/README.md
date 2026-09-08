# mb10pro platform bindings

This directory owns every firmware-address binding used by the project's
`.c` files. The consumer TUs never see a firmware version -- they include
the single dispatcher header and it pulls in the right per-firmware file
for them.

## Layout

```
platform/
├── miwear_system.h               # dispatcher -- includes the active per-version header
├── mb10p/
│   ├── fw_3.101.043/
│   │   └── miwear_system.h       # full bindings for fw_3.101.043 (default)
│   └── fw_3.201.016/
│       └── miwear_system.h       # full bindings for fw_3.201.016
└── README.md                     # this file
```

`platform/miwear_system.h` is the *only* include site TUs need; the
per-version files under `platform/mb10p/fw_X.Y.Z/` carry every binding the
project uses (system + launcher), so all LVGL / NuttX / screen / miwear
symbols come from one include.

## Switching target firmware

The `MB10P_FW_VERSION` macro identifies which firmware a TU binds against.
The full version number (e.g. `3_101_043`) is the canonical name; the
underlying integer is written without digit separators because GCC 10.x --
the version this project ships with -- doesn't accept `3_101_043` even
under `-std=gnu2x` (C2x digit-separator support landed in GCC 12).

Build with `-DMB10P_FW_VERSION=<integer>`:

| C2x form   | -D value  | Resolves to                            |
| ---------- | --------- | -------------------------------------- |
| 3_101_043  | 3101043   | `mb10p/fw_3.101.043/` (default)        |
| 3_201_016  | 3201016   | `mb10p/fw_3.201.016/`                  |

```sh
# default (fw_3.101.043 / 3_101_043)
arm-none-eabi-gcc ... -Imb10pro mb10pro/mb10p_app.c -c -o mb10p_app.o

# explicitly target fw_3.201.016 / 3_201_016
arm-none-eabi-gcc ... -Imb10pro -DMB10P_FW_VERSION=3201016 \
    mb10pro/mb10p_app.c -c -o mb10p_app.o
```

The VS Code / Zed `tasks.json` files inherit the default; add a duplicate
task with the `-D` flag if you need to build against the newer firmware.

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
2. Create `platform/mb10p/fw_X.Y.Z/miwear_system.h` by copying the closest
   existing version's file and updating only the address macros. The
   header comment should cite the firmware build number and IDA
   instance/port.
3. Extend the `#if` ladder in the dispatcher `platform/miwear_system.h`
   to recognise the new tag (e.g.
   `MB10P_FW_VERSION == 3301017  /* 3_301_017 */`) and resolve to the new
   directory.

## What does NOT change between versions

The per-version headers are intentionally small. They only:

* `#include` the shared family headers that own typedefs (`<nuttx/...>`,
  `<lvgl/...>`, `<miwear/...>`, `<misc/print.h>`).
* `#define` symbol -> address/typedef bindings.

Anything that's not an address (struct layouts, callbacks, prototypes,
`*_t` typedefs) lives in the family header and is the same for every
firmware revision.
