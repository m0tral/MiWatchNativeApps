#ifndef XIAOMI_MIWEAR_SYSTEM_H
#define XIAOMI_MIWEAR_SYSTEM_H

/* Xiaomi unified platform dispatcher.
 *
 * Consumer TUs write
 *
 *     #include "platform/miwear_system.h"
 *
 * and never see a firmware version. This header is the single switch that
 * picks which model's address bindings are active, then which revision of
 * that model's bindings.
 *
 * Build flags select the target model:
 *
 *   -DMB10P_FW_VERSION=<int>     # mb10pro (Xiaomi Mi Band 10 Pro)
 *   -DWATCHS3_FW_VERSION=<int>   # watchs3 (Xiaomi Watch S3)
 *
 * The model name selects a directory under platform/; the firmware integer
 * selects a per-revision header inside that directory.
 *
 * Per-version header is free to redefine, add or drop macros as long as
 * the family-header typedefs in <common/nuttx/...>, <common/lvgl/...>,
 * <common/miwear/...>, <common/misc/print.h> stay in sync. Add a new
 * firmware by creating a new directory platform/<model>/fw_X.Y.Z/ and
 * extending the matching `#if` ladder below.
 */

#if defined(MB10P_FW_VERSION)
#   include "mb10p/miwear_system.h"
#elif defined(WATCHS3_FW_VERSION)
#   include "watchs3/miwear_system.h"
#else
#   error "platform/miwear_system.h: define MB10P_FW_VERSION or WATCHS3_FW_VERSION"
#endif

#endif /* XIAOMI_MIWEAR_SYSTEM_H */
