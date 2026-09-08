#ifndef MB10P_MIWEAR_SYSTEM_H
#define MB10P_MIWEAR_SYSTEM_H

/* mb10pro (Xiaomi Mi Band 10 Pro) platform dispatcher.
 *
 * Included by the top-level platform/miwear_system.h when MB10P_FW_VERSION
 * is defined. Picks which per-revision header under fw_X.Y.Z/ to pull in.
 *
 * Build with -DMB10P_FW_VERSION=3101043 to target fw_3.101.043 (default)
 *      or -DMB10P_FW_VERSION=3201016 to target fw_3.201.016
 *
 * The integers are the full version numbers 3_101_043 / 3_201_016 with
 * digit separators removed (the toolchain's GCC 10.x does not accept the
 * digit-separator form `3_101_043` even under -std=gnu2x).
 *
 * Per-version header is free to redefine, add or drop macros as long as
 * the family-header typedefs in <common/nuttx/...>, <common/lvgl/...>,
 * <common/miwear/...>, <common/misc/print.h> stay in sync. Add a new
 * firmware by creating a new directory under platform/mb10p/ and
 * extending the `#if` ladder below.
 */

#if defined(MB10P_FW_VERSION) && (MB10P_FW_VERSION == 3201016)  /* 3_201_016 */
#   include "fw_3.201.016/miwear_system.h"
#elif defined(MB10P_FW_VERSION) && (MB10P_FW_VERSION == 3101043) /* 3_101_043 */
#   include "fw_3.101.043/miwear_system.h"
#else
#   include "fw_3.101.043/miwear_system.h"   /* default = 3_101_043 */
#endif

#endif /* MB10P_MIWEAR_SYSTEM_H */
