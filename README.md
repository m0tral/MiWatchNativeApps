# MiWatch Native Apps

Native applications for Xiaomi SmartBand Pro devices based on **NuttX + LVGL**, using dynamically loaded ELF modules.

This project demonstrates that it is possible to develop and run **native applications directly on Xiaomi's watch platform**, even on Global firmware where the original AiotJS application engine has been removed.

## Hardware

The target platform is the latest SmartBand Pro generation:

* **Bestechnic BES2700iMP**
* ARM Cortex-M33
* 16 MB internal SPI Flash
* ~2.5 MB SRAM
* 16 MB PSRAM
* 256/512 Mb external SPI NAND Flash

Software stack:

* **NuttX RTOS**
* **LVGL 9**
* **Lua** — used by watchfaces
* **AiotJS / QuickJS** — used by Xiaomi 3rd party applications

Unfortunately, Global firmware removes the AiotJS engine instead of providing applications for the international market. This project explores another approach: loading our own native applications as NuttX modules.

## Why NuttX Modules?

NuttX provides a module loader similar in concept to Linux kernel modules.

The firmware already contains the standard NuttX commands:

```text
insmod
lsmod
rmmod
```

`insmod` can load ELF modules at runtime, perform relocation, and instantiate them without modifying the firmware itself.

This has several advantages:

* No hardcoded code addresses
* No firmware patching required
* Global/static memory is handled by the module loader
* Fast development and testing
* Modules can be loaded and unloaded independently
* Much faster iteration than reflashing the application firmware
* Significantly lower risk during development, it's protected from bootloop, as system doesn't load it automatically

NuttX supports both **ELF** and **NXFLAT** modules. For this project, **ET_REL relocatable ELF** modules are used because they provide a relatively simple build pipeline.

See the [NuttX binary loader documentation](https://nuttx.apache.org/docs/latest/components/binfmt.html).

## Loading a Native Application

A Lua watchface is used as a convenient deployment mechanism.

The compiled ELF is copied to:

```text
/data/mb10p_app.elf
```

and loaded using:

```text
insmod /data/mb10p_app.elf mb10p_app
```

The module can then be inspected with:

```text
lsmod
```

and removed with:

```text
rmmod mb10p_app
```

The module constructor is used as the initialization entry point:

```c
__attribute__((constructor))
int module_initialize(void)
{
    syslog(LOG_WARN, "[hello_module] loaded");
    return 0;
}
```

The destructor is called when the module is unloaded.

## Xiaomi Application Architecture

The Xiaomi application subsystem is surprisingly well structured.

Applications are represented by two main objects:

```c
miwear_app_t
miwear_page_t
```

An application provides metadata and lifecycle callbacks, while pages represent its UI and lifecycle.

The package manager exposes an interface similar to:

```c
struct packagemanager_api_s
{
    int (*install)(miwear_app_t *app,
                   miwear_page_t *pages[],
                   int count);

    int (*recover)(miwear_app_t *app);
    int (*uninstall)(miwear_app_t *app);
    int (*unregister)(miwear_app_t *app);
    int (*update_notify)(miwear_app_t *app);

    void *(*widgets_add)(miwear_app_t *app,
                         void *widgets,
                         int count);

    void (*widgets_remove)(miwear_app_t *app);

    int (*set_hidden)(miwear_app_t *app, int hidden);
    int (*is_hidden)(miwear_app_t *app);
};
```

Registering an application is essentially:

```c
miwear_page_t *pages[] = { &g_calc_page };

g_packagemanager_api->install(
    &g_calc_app,
    pages,
    1
);
```

The application can then be added to the launcher:

```c
app_launcher_add(CALC_APP_ID);
```

This means a dynamically loaded ELF module can become a fully integrated Xiaomi application rather than simply running as an isolated process.

## LVGL Application

The first example application is a simple **calculator**.

The UI is implemented using the watch's existing **LVGL 9** environment.

The calculator contains:

* Numeric keypad
* `+`, `-`, `*`, `/`
* `%`
* Decimal point
* Backspace
* Clear
* Result display

Example UI creation:

```c
lv_obj_t *root = lv_obj_create(parent, 0);

lv_obj_set_size(root, SCREEN_W, SCREEN_H);

lv_obj_t *disp = lv_label_create(root);

lv_label_set_text(disp, calc_display);
```

Buttons are created using the normal LVGL API and connected to event callbacks:

```c
lv_obj_add_event_cb(
    btn,
    calc_on_press,
    LV_EVENT_CLICKED,
    (void *)lbl
);
```

The calculator logic uses the Cortex-M33 floating-point unit:

```text
-mfloat-abi=hard
-mfpu=fpv5-sp-d16
```

The resulting ELF was also checked to ensure that unsupported `__aeabi_*` floating-point helper imports were not generated.

## Important Threading Detail

One important discovery was that Xiaomi UI functions cannot safely be called directly from the NuttX shell thread.

Doing so can corrupt the UI state and cause a system reboot.

Instead, the module schedules the initialization through an LVGL timer:

```c
lv_timer_create(timer_callback, 500, 0);
```

The callback then executes in the LVGL/UI context:

```c
static void timer_callback(lv_timer_t *timer)
{
    lv_timer_delete(post_timer_id);
    post_timer_id = 0;

    register_app();
}
```

This allows the native application to safely interact with the Xiaomi UI subsystem.

## Additional Integration

The calculator also demonstrates access to other system APIs.

For example, the module can:

* Query the current watchface
* Remove the current watchface
* Reset the watchface UI
* Create system notifications
* Register applications in the launcher
* Access Xiaomi's internal application manager

A notification can be generated using:

```c
lvx_notification_insert_message(&done_notification);
```

This demonstrates that a native module has considerably more access to the system than Lua or JavaScript applications.

## Supported Firmware

Currently implemented for:

| Device      | Stock firmware | Mod firmware |
| ----------- | -------------- | ------------ |
| MB10 Pro CN | 3.101.043      | 3.121.043    |
| MB10 Pro GL | 3.201.016      | 3.221.016    |

The same application concept can potentially be ported to other Xiaomi devices using the same platform architecture.

## Build Environment

Current build environment:

```text
GCC 10.2.1 20201103
ARM GNU Toolchain
VS Code
```

The repository contains VS Code build tasks and example projects.

The resulting calculator module is only approximately:

```text
6.4 KB
```

For example:

```text
mb10p_app_3.101.043.elf
```

## Current Limitations

The application is currently loaded manually using `insmod`.

Therefore, it is lost after a device reboot.

For development this is acceptable because reinstalling the module takes only a few seconds.

A future firmware modification can provide an initialization script executed during boot, allowing custom applications to be automatically loaded when the system starts.

## What This Makes Possible

The calculator is only a proof of concept.

Once native modules can interact with the Xiaomi application subsystem, many other possibilities become available:

* Custom native applications
* Native games
* Custom notification managers
* Telegram/WhatsApp message interaction
* Custom schedulers
* Pomodoro timers
* Metronomes
* Water-drinking reminders
* Custom notification centers
* Network-enabled applications
* Replacement applications for existing Xiaomi apps
* Extensions to the Lua API
* Extensions to the AiotJS API
* Integration with smart-home systems

For example, Lua can already be used to execute shell commands and make network requests. This makes it possible to combine Lua, NuttX shell functionality and native modules into a much more powerful development environment.

## Example

The calculator running directly on the watch:

![Calculator running on MB10 Pro](/img/mb10p_calc_app.gif)

## References

* [NuttX Binary Loader](https://nuttx.apache.org/docs/latest/components/binfmt.html)
* [NuttX `insmod`](https://nuttx.apache.org/docs/latest/applications/nsh/commands.html#insmod-install-an-os-module)
* [NuttX Shell](https://nuttx.apache.org/docs/latest/applications/nsh/index.html)
* [MiWatchNativeApps](https://github.com/m0tral/MiWatchNativeApps)
* [Easyface](https://github.com/m0tral/Easyface)

### Community

* [m0tral Telegram News](https://t.me/mi_watch_news)
* [m0tral Telegram Chat](https://t.me/mi_watch_int)
