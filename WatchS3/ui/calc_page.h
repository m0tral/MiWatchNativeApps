#ifndef XIAOMI_UI_CALC_PAGE_H
#define XIAOMI_UI_CALC_PAGE_H

/* watchs3 calculator page component.
 *
 * Builds a full-screen LVGL calculator on a given parent. The page owns
 * the lvgl widgets; the calculator state machine (display string,
 * running total, pending op, %) lives in calc_state.c so it can be
 * exercised from host-side tests.
 *
 * Pulls in calc_state.h so callers can both build the UI and drive /
 * inspect the state through the same header.
 */

#include "common/lvgl/control.h"
#include "ui/calc_state.h"

/* Create and return the calculator root container (sized to the parent). */
lv_obj_t *calc_page_create(lv_obj_t *parent);

#endif /* XIAOMI_UI_CALC_PAGE_H */
