#ifndef WATCHS3_CALC_PAGE_ASSERTS_H
#define WATCHS3_CALC_PAGE_ASSERTS_H

/* Per-TU address asserts for ui/calc_page.c.
 *
 * Pulls in the platform header so the typed pointer bindings are visible,
 * then declares _Static_assert blocks ONLY for the symbols this translation
 * unit actually references. Filling in the real address satisfies the
 * assert; the assert only fires for symbols consumed here.
 */

#include "platform/miwear_system.h"

/* lvgl control widgets */
_Static_assert((LV_OBJ_CREATE_ADDR & ~1u) != 0u,
               "lv_obj_create address is NULL");
_Static_assert((LV_LABEL_CREATE_ADDR & ~1u) != 0u,
               "lv_label_create address is NULL");
_Static_assert((LVX_BTN_CREATE_ADDR & ~1u) != 0u,
               "lvx_btn_create address is NULL");

/* lvgl widget methods */
_Static_assert((LV_OBJ_SET_SIZE_ADDR & ~1u) != 0u,
               "lv_obj_set_size address is NULL");
_Static_assert((LV_OBJ_ALIGN_ADDR & ~1u) != 0u,
               "lv_obj_align address is NULL");
_Static_assert((LV_OBJ_ADD_EVENT_CB_ADDR & ~1u) != 0u,
               "lv_obj_add_event_cb address is NULL");
_Static_assert((LV_LABEL_SET_TEXT_ADDR & ~1u) != 0u,
               "lv_label_set_text address is NULL");
_Static_assert((LVX_BTN_SET_TEXT_FMT_ADDR & ~1u) != 0u,
               "lvx_btn_set_text_fmt address is NULL");
_Static_assert((LV_OBJ_SET_STYLE_TEXT_ALIGN_ADDR & ~1u) != 0u,
               "lv_obj_set_style_text_align address is NULL");
_Static_assert((LV_OBJ_SET_STYLE_BG_OPA_ADDR & ~1u) != 0u,
               "lv_obj_set_style_bg_opa address is NULL");
_Static_assert((LV_OBJ_SET_STYLE_PAD_LEFT_ADDR & ~1u) != 0u,
               "lv_obj_set_style_pad_left address is NULL");
_Static_assert((LV_OBJ_SET_STYLE_PAD_RIGHT_ADDR & ~1u) != 0u,
               "lv_obj_set_style_pad_right address is NULL");
_Static_assert((LV_OBJ_SET_STYLE_PAD_TOP_ADDR & ~1u) != 0u,
               "lv_obj_set_style_pad_top address is NULL");
_Static_assert((LV_OBJ_SET_STYLE_PAD_BOTTOM_ADDR & ~1u) != 0u,
               "lv_obj_set_style_pad_bottom address is NULL");
_Static_assert((LVX_BTN_SET_STYLE_BG_COLOR_ADDR & ~1u) != 0u,
               "lvx_btn_set_style_bg_color address is NULL");
_Static_assert((LVX_OBJ_SET_STYLE_TEXT_ADDR & ~1u) != 0u,
               "lvx_obj_set_style_text address is NULL");
_Static_assert((LV_OBJ_GET_EVENT_CODE_ADDR & ~1u) != 0u,
               "lv_obj_get_event_code address is NULL");
_Static_assert((LV_EVENT_GET_USER_DATA_ADDR & ~1u) != 0u,
               "lv_event_get_user_data address is NULL");

/* label-style structs */
_Static_assert((LB_TEXT_STYLE_1_ADDR & ~1u) != 0u,
               "LB_TEXT_STYLE_1 address is NULL");

/* stdio */
_Static_assert((SNPRINTF_ADDR & ~1u) != 0u,
               "snprintf address is NULL");

#endif /* WATCHS3_CALC_PAGE_ASSERTS_H */
