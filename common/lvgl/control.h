#ifndef XIAOMI_LVGL_CONTROL_H
#define XIAOMI_LVGL_CONTROL_H

/* watchs3 LVGL control-widget constructors + generic methods.
 *
 * Constructors follow the same shape (reverse-engineered from vela_ap.bin):
 *
 *     lv_<class>_create(lv_obj_t *parent)
 *     {
 *         obj = lv_obj_class_create_obj_veneer2(&lv_<class>, parent);
 *         lv_obj_class_init_obj_veneer(obj, 0);
 *         return obj;
 *     }
 *
 * Each constructor is a Thumb function whose literal pool entry loads
 * the address of the matching `lv_<class>` struct. Address bindings
 * live in platform/watchs3_platform.h.
 *
 * Generic methods (lv_obj_set_size, lv_image_set_src) are Thumb thunks
 * that JUMPOUT into the apps/graphics/lvgl library at higher addresses.
 */

#include <stdint.h>

/* Opaque widget handle. The real lvgl type is `lv_obj_t`; we don't need
 * its layout here, only to pass pointers to it. */
typedef struct lv_obj lv_obj_t;

/* Opaque event descriptor. The real lvgl type is `lv_event_dsc_t`. */
typedef struct lv_event lv_event_t;
typedef struct lv_event_dsc lv_event_dsc_t;

/* Opaque style struct -- the real lvgl type is `lv_style_t`. */
typedef struct lv_style lv_style_t;

/* lv_coord_t is `int32_t` in lvgl 9. */
typedef int32_t lv_coord_t;

/* lv_opa_t is `uint8_t` in lvgl 9 (0=transparent .. 255=opaque). */
typedef uint8_t lv_opa_t;

/* lv_color_t is a packed ARGB8888 `uint32_t` in lvgl 9. */
typedef uint32_t lv_color_t;

/* lv_style_selector_t is `uint32_t` in lvgl 9 (bitmask of LV_STATE_*
 * | LV_PART_* values). */
typedef uint32_t lv_style_selector_t;

/* lv_event_code_t is `uint8_t` in lvgl 9 (the event code enum). */
typedef uint8_t lv_event_code_t;

/* lv_align_t -- widget alignment enum. Numeric values match LVGL 9.x.
 * Validated against vela_ap.bin: callers of lv_obj_align pass values
 *   0, 1, 2, 4, 5, 6, 7, 8, 9, 20  (DEFAULT, TOP_LEFT, TOP_MID,
 *   BOTTOM_LEFT, BOTTOM_MID, BOTTOM_RIGHT, LEFT_MID, CENTER, RIGHT_MID,
 *   OUT_BOTTOM_MID).
 * Value 3 (TOP_RIGHT) is unused in the firmware; values >=16 are the
 * OUT_* family for placing children outside the parent bounds. */
typedef enum {
    LV_ALIGN_DEFAULT          = 0,
    LV_ALIGN_TOP_LEFT         = 1,
    LV_ALIGN_TOP_MID          = 2,
    LV_ALIGN_TOP_RIGHT        = 3,
    LV_ALIGN_BOTTOM_LEFT      = 4,
    LV_ALIGN_BOTTOM_MID       = 5,
    LV_ALIGN_BOTTOM_RIGHT     = 6,
    LV_ALIGN_LEFT_MID         = 7,
    LV_ALIGN_RIGHT_MID        = 8,
    LV_ALIGN_CENTER           = 9,
    LV_ALIGN_OUT_TOP_LEFT     = 16,
    LV_ALIGN_OUT_TOP_MID      = 17,
    LV_ALIGN_OUT_TOP_RIGHT    = 18,
    LV_ALIGN_OUT_BOTTOM_LEFT  = 19,
    LV_ALIGN_OUT_BOTTOM_MID   = 20,
    LV_ALIGN_OUT_BOTTOM_RIGHT = 21,
    LV_ALIGN_OUT_LEFT_TOP     = 22,
    LV_ALIGN_OUT_LEFT_MID     = 23,
    LV_ALIGN_OUT_LEFT_BOTTOM  = 24,
    LV_ALIGN_OUT_RIGHT_TOP    = 25,
    LV_ALIGN_OUT_RIGHT_MID    = 26,
    LV_ALIGN_OUT_RIGHT_BOTTOM = 27,
} lv_align_t;

/* Standard LVGL event callback. Receives an event descriptor describing
 * what happened (target object, event code, user_data, etc.). */
typedef void (*lv_event_cb_t)(lv_event_t *e);

/* Single typedef -- every constructor in this file has the same shape. */
typedef lv_obj_t *(*lv_class_create_t)(lv_obj_t *parent);

typedef lv_obj_t *(*lv_obj_create_t)(lv_obj_t *parent, void* data);

/* Generic methods. */
typedef void (*lv_obj_set_size_t)   (lv_obj_t *obj, lv_coord_t w, lv_coord_t h);
typedef void (*lv_image_set_src_t)  (lv_obj_t *obj, const void *src);
typedef void (*lv_obj_add_event_cb_t)(lv_obj_t *obj,
                                      lv_event_cb_t event_cb,
                                      lv_event_code_t filter,
                                      void *user_data);
typedef void (*lv_label_set_text_t)  (lv_obj_t *obj, const char *text);
typedef void (*lvx_btn_set_text_fmt_t)    (lv_obj_t *btn, const char *text, ...);
typedef void (*lv_obj_add_flag_t)    (lv_obj_t *obj, uint32_t flag);
typedef void (*lv_obj_clear_flag_t)  (lv_obj_t *obj, uint32_t flag);
typedef void (*lv_obj_align_t)       (lv_obj_t *obj, lv_align_t align,
                                      int32_t x_ofs, int32_t y_ofs);
typedef void (*lv_obj_align_to_t)    (lv_obj_t *obj, const lv_obj_t *target,
                                      lv_align_t align,
                                      int32_t x_ofs, int32_t y_ofs);
typedef void (*lv_obj_set_width_t)   (lv_obj_t *obj, lv_coord_t w);
typedef void (*lv_obj_set_style_text_align_t)(lv_obj_t *obj,
                                              uint8_t value,
                                              uint32_t selector);
typedef void (*lv_label_set_long_mode_t)(lv_obj_t *obj, uint8_t long_mode);
typedef void (*lv_obj_set_style_bg_color_t)(lv_obj_t *obj, lv_color_t color,
                                           lv_style_selector_t selector);
typedef void (*lv_obj_set_style_bg_opa_t)  (lv_obj_t *obj, lv_opa_t opa,
                                           lv_style_selector_t selector);
typedef void (*lv_obj_set_style_pad_t)    (lv_obj_t *obj, lv_coord_t value,
                                           lv_style_selector_t selector);
typedef int  (*lvx_obj_set_style_text_t)(lv_obj_t *obj,
                                          lv_style_t *style,
                                          lv_opa_t opa,
                                          lv_style_selector_t selector);
typedef void (*lvx_btn_set_style_bg_color_t)(lv_obj_t *obj, lv_color_t color);

/* Event-dispatch + animation veneer family -- exposed without the
 * `_veneer` suffix in user-facing code (the firmware symbols keep it). */
typedef lv_obj_t *(*lv_obj_event_t)(lv_obj_t *obj);
typedef uint8_t   (*lv_obj_get_event_code_t)(lv_obj_t *obj);
typedef uint32_t  (*lv_obj_get_target_t)(lv_obj_t *obj);
typedef void      (*lv_anim_del_t)(void);
typedef int       (*lvx_label_set_time_text_t)(lv_obj_t *parent, uint32_t value);
typedef void      (*lv_anim_set_t)(uint32_t handle, uint32_t duration_ms, uint32_t value);
typedef void      (*lv_anim_start_t)(void);

/* lv_event_get_* family -- the small 8-18 byte veneer wrappers live in
 * the low-ROM area at 0x200BFD44-0x200BFDA8, each returning a single
 * field from lv_event_t: +4=target, +8=code, +12=user_data, +16=param. */
typedef void *(*lv_event_get_user_data_t)(lv_event_t *e);
typedef void *(*lv_event_get_param_t)    (lv_event_t *e);

/* ---- Constructors ---- */
lv_obj_t *lv_obj_create             (lv_obj_t *parent, void *data);
lv_obj_t *lv_image_create           (lv_obj_t *parent, void *data);
lv_obj_t *lv_label_create           (lv_obj_t *parent);
lv_obj_t *lv_btn_create             (lv_obj_t *parent);   /* lv_btn_create_veneer in firmware (0x2C588BA0) */
lv_obj_t *lvx_btn_create            (lv_obj_t *parent);   /* miwear-specific button class (0x2CA48474) */

lv_obj_t *lv_analog_time_create     (lv_obj_t *parent);
lv_obj_t *lv_xchart_create          (lv_obj_t *parent);
lv_obj_t *lv_crossview_create       (lv_obj_t *parent);
lv_obj_t *lv_sleepchart_create      (lv_obj_t *parent);
lv_obj_t *lv_curvedlabel_create     (lv_obj_t *parent);
lv_obj_t *lv_frameanim_create       (lv_obj_t *parent);
lv_obj_t *lv_imglabel_create        (lv_obj_t *parent);
lv_obj_t *lv_indicator_create       (lv_obj_t *parent);
lv_obj_t *lv_optionlist_create      (lv_obj_t *parent);
lv_obj_t *lv_slider_create          (lv_obj_t *parent);
lv_obj_t *lv_xview_create           (lv_obj_t *parent);
lv_obj_t *lv_timeview_create        (lv_obj_t *parent);
lv_obj_t *lv_xwin_create            (lv_obj_t *parent);
lv_obj_t *lv_zoombtn_create         (lv_obj_t *parent);
lv_obj_t *lv_eventdispatch_create   (lv_obj_t *parent);
lv_obj_t *lv_renderdata_create      (lv_obj_t *parent);
lv_obj_t *lv_render_file_create     (lv_obj_t *parent);
lv_obj_t *lv_imgarc_create          (lv_obj_t *parent);
lv_obj_t *lv_slot_create            (lv_obj_t *parent);
lv_obj_t *lv_text_create            (lv_obj_t *parent);
lv_obj_t *lv_wfwidget_create        (lv_obj_t *parent);
lv_obj_t *lv_widgetlayout_create    (lv_obj_t *parent);

/* ---- Generic methods ---- */
void lv_obj_set_size               (lv_obj_t *obj, lv_coord_t w, lv_coord_t h);
void lv_image_set_src              (lv_obj_t *obj, const void *src);
void lv_obj_add_event_cb           (lv_obj_t *obj,
                                    lv_event_cb_t event_cb,
                                    lv_event_code_t filter,
                                    void *user_data);
void lv_label_set_text             (lv_obj_t *obj, const char *text);
void lvx_btn_set_text_fmt               (lv_obj_t *btn, const char *text, ...);
void lv_obj_add_flag               (lv_obj_t *obj, uint32_t flag);
void lv_obj_clear_flag             (lv_obj_t *obj, uint32_t flag);
void lv_obj_align                  (lv_obj_t *obj, lv_align_t align,
                                    int32_t x_ofs, int32_t y_ofs);
void lv_obj_align_to               (lv_obj_t *obj, const lv_obj_t *target,
                                    lv_align_t align,
                                    int32_t x_ofs, int32_t y_ofs);
void lv_obj_set_width              (lv_obj_t *obj, lv_coord_t w);
void lv_obj_set_style_text_align   (lv_obj_t *obj, uint8_t value,
                                    uint32_t selector);
void lv_label_set_long_mode       (lv_obj_t *obj, uint8_t long_mode);

void lv_obj_set_style_bg_color     (lv_obj_t *obj, lv_color_t color,
                                    lv_style_selector_t selector);
void lv_obj_set_style_bg_opa       (lv_obj_t *obj, lv_opa_t opa,
                                    lv_style_selector_t selector);
void lv_obj_set_style_pad_left     (lv_obj_t *obj, lv_coord_t value,
                                    lv_style_selector_t selector);
void lv_obj_set_style_pad_right    (lv_obj_t *obj, lv_coord_t value,
                                    lv_style_selector_t selector);
void lv_obj_set_style_pad_top      (lv_obj_t *obj, lv_coord_t value,
                                    lv_style_selector_t selector);
void lv_obj_set_style_pad_bottom   (lv_obj_t *obj, lv_coord_t value,
                                    lv_style_selector_t selector);
void lv_obj_set_style_pad_column   (lv_obj_t *obj, lv_coord_t value,
                                    lv_style_selector_t selector);

int  lvx_obj_set_style_text        (lv_obj_t *obj, lv_style_t *style,
                                    lv_opa_t opa,
                                    lv_style_selector_t selector);

void lvx_btn_set_style_bg_color    (lv_obj_t *obj, lv_color_t color);

/* Event dispatcher + animation veneer API. Names are stripped of the
 * firmware's `_veneer` suffix at the C boundary. */
lv_obj_t *lv_obj_event              (lv_obj_t *obj);
uint8_t   lv_obj_get_event_code     (lv_obj_t *obj);
uint32_t  lv_obj_get_target         (lv_obj_t *obj);
void      lv_anim_del               (void);
int       lvx_label_set_time_text   (lv_obj_t *parent, uint32_t value);
void      lv_anim_set               (uint32_t handle, uint32_t duration_ms,
                                    uint32_t value);
void      lv_anim_start             (void);

void *    lv_event_get_user_data   (lv_obj_t *obj);
void *    lv_event_get_param       (lv_obj_t *obj);

#endif /* XIAOMI_LVGL_CONTROL_H */
