#ifndef WATCHS3_MIWEAR_SYSTEM_H
#define WATCHS3_MIWEAR_SYSTEM_H

/* watchs3 (Xiaomi Watch S3) platform dispatcher.
 *
 * Included by the top-level platform/miwear_system.h when WATCHS3_FW_VERSION
 * is defined. Pulls in the WatchS3-specific watchface_config_t layout
 * (v1) BEFORE the per-fw header so the per-fw miwear_system.h can bind
 * `g_watchface_config` against the right struct type.
 *
 * Build with -DWATCHS3_FW_VERSION=480 to target fw_4.8.0 (default)
 *      or -DWATCHS3_FW_VERSION=260 to target fw_2.6.0
 *
 * The integers are the full version numbers 4_8_0 / 2_6_0 with digit
 * separators removed (the toolchain's GCC 10.x does not accept the
 * digit-separator form `4_8_0` even under -std=gnu2x).
 *
 * Per-version header is free to redefine, add or drop macros as long as
 * the family-header typedefs in <common/nuttx/...>, <common/lvgl/...>,
 * <common/miwear/...>, <common/misc/print.h> stay in sync. Add a new
 * firmware by creating a new directory under platform/watchs3/ and
 * extending the `#if` ladder below.
 */

#include "platform/watchs3/watchface_layout.h"   /* watchface_config_v1_t */

#if defined(WATCHS3_FW_VERSION) && (WATCHS3_FW_VERSION == 260)    /* 2_6_0 */
#   include "fw_2.6.0/miwear_system.h"
#elif defined(WATCHS3_FW_VERSION) && (WATCHS3_FW_VERSION == 480)   /* 4_8_0 */
#   include "fw_4.8.0/miwear_system.h"
#else
#   include "fw_4.8.0/miwear_system.h"   /* default = 4_8_0 */
#endif

#endif /* WATCHS3_MIWEAR_SYSTEM_H */
