#ifndef MB10P_MIWEAR_SYSTEM_H
#define MB10P_MIWEAR_SYSTEM_H

/* mb10pro single-header platform dispatcher.
 *
 * Consumer TUs write
 *
 *     #include "platform/miwear_system.h"
 *
 * and never see a firmware version. This header is the single switch that
 * picks which revision's address bindings are active.
 *
 * Build with  -DMB10P_FW_VERSION=3101043  to target fw_3.101.043 (default)
 *          or -DMB10P_FW_VERSION=3201016  to target fw_3.201.016
 *
 * The integers are the full version numbers 3_101_043 / 3_201_016 with
 * digit separators removed (the toolchain's GCC 10.x does not accept the
 * digit-separator form `3_101_043` even under -std=gnu2x).
 *
 * The integers are the full version numbers 3_101_043 / 3_201_016 with
 * digit separators removed (the toolchain's GCC 10.x does not accept the
 * digit-separator form `3_101_043` even under -std=gnu2x).
 *
 * Switching a TU is purely a -D flag -- no #include path rewriting, no
 * symlink tricks, no per-module hacks. The chosen header forwards every
 * address-to-pointer macro (system layer: LVGL / NuttX / screen / stdio;
 * miwear layer: launcher / packagemanager dispatch) from one of:
 *
 *     platform/mb10p/fw_3.101.043/miwear_system.h
 *     platform/mb10p/fw_3.201.016/miwear_system.h
 *
 * Per-version header is free to redefine, add or drop macros as long as
 * the family-header typedefs in <nuttx/...>, <lvgl/...>, <miwear/...>,
 * <misc/print.h> stay in sync. Add a new firmware by creating a third
 * platform/mb10p/fw_X.Y.Z/miwear_system.h and extending the table below.
 */

#if defined(MB10P_FW_VERSION) && (MB10P_FW_VERSION == 3201016)  /* 3_201_016 */
#   include "mb10p/fw_3.201.016/miwear_system.h"
#elif defined(MB10P_FW_VERSION) && (MB10P_FW_VERSION == 3101043) /* 3_101_043 */
#   include "mb10p/fw_3.101.043/miwear_system.h"
#else
#   include "mb10p/fw_3.101.043/miwear_system.h"   /* default = 3_101_043 */
#endif

#endif /* MB10P_MIWEAR_SYSTEM_H */
