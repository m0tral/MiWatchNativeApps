#ifndef MB10P_UI_CALC_PAGE_H
#define MB10P_UI_CALC_PAGE_H

/* mb10pro calculator page component.
 *
 * Builds a full-screen LVGL calculator on a given parent. The page owns
 * its own state (display string, result flag) and exposes only a create
 * function; lifetime is managed by the caller via the returned root.
 *
 * This header only declares the public API; implementation lives in
 * ui/calc_page.c.
 */

#include "lvgl/control.h"

/* Create and return the calculator root container (sized to the parent). */
lv_obj_t *calc_page_create(lv_obj_t *parent);

#endif /* MB10P_UI_CALC_PAGE_H */
