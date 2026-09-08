#ifndef XIAOMI_UI_CALC_PAGE_H
#define XIAOMI_UI_CALC_PAGE_H

/* watchs3 calculator page component.
 *
 * Builds a full-screen LVGL calculator on a given parent. The page owns
 * its own state (display string, result flag) and exposes only a create
 * function; lifetime is managed by the caller via the returned root.
 *
 * This header only declares the public API; implementation lives in
 * ui/calc_page.c.
 */

#include "common/lvgl/control.h"

/* Create and return the calculator root container (sized to the parent). */
lv_obj_t *calc_page_create(lv_obj_t *parent);

#endif /* XIAOMI_UI_CALC_PAGE_H */
