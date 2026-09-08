#ifndef MB10P_MIWEAR_SYSTEM_FW_3_101_043_H
#define MB10P_MIWEAR_SYSTEM_FW_3_101_043_H

/* mb10pro (Xiaomi Mi Band 10 Pro) per-firmware system + miwear bindings.
 *
 * This is the SINGLE per-version platform header for the project.
 * It owns every macro that resolves a firmware symbol to a Thumb-callable
 * function pointer or to a data pointer, all reverse-engineered from
 * vela_ap.bin (fw_3.101.043) via the IDA Pro MCP tools. Two layers
 * coexist here:
 *
 *   - the "system" layer (nuttx / lvgl / miwear_runtime / screen) -- the
 *     LVGL widget, NuttX reminder, screen session and stdio bindings
 *     every module needs.
 *   - the "miwear" layer (launcher / packagemanager dispatch) -- only
 *     used by the one TU that registers an app via the launcher.
 *
 * Both are version-specific, so they're kept in the same header to avoid
 * the dispatcher / per-version split and to keep the include surface to a
 * single line at every call site.
 *
 * Convention:
 *   - Code addresses carry `| 1` (Thumb bit) so BLX switches state.
 *   - Typedefs live in the matching family header (nuttx/syslog.h,
 *     nuttx/reminder.h, lvgl/timer.h, lvgl/control.h, miwear/app.h,
 *     nuttx/screen.h, misc/print.h).
 *   - DO NOT put new prototypes here; put them in the family header.
 */

#include "common/nuttx/syslog.h"
#include "common/nuttx/reminder.h"
#include "common/nuttx/notification.h"
#include "common/nuttx/screen.h"
#include "common/lvgl/timer.h"
#include "common/lvgl/control.h"
#include "common/miwear/app.h"
#include "common/miwear/watchface.h" /* for watchface_config_t and watchface_manager_delete_watchface_t */
#include "common/misc/print.h" /* for the snprintf prototype */

/* ============================================================================
 * Logger
 * ============================================================================ */
#define syslog                       ((syslog_t)(0x2C1F0D20 | 1))              /* Thumb bit set */

/* ============================================================================
 * LVGL reminder subsystem (struct reminder_t in nuttx/reminder.h)
 * ============================================================================ */
#define lvx_reminder_set_uid           ((lvx_reminder_set_uid_t)         (0x2CA9989C | 1))   /* Thumb bit set */
#define lvx_reminder_set_level         ((lvx_reminder_set_level_t)       (0x2CA9987C | 1))   /* Thumb bit set */
#define lvx_reminder_set_vibration     ((lvx_reminder_set_vibration_t)   (0x2CA9988C | 1))   /* Thumb bit set */
#define lvx_reminder_ignore_quiet_mode ((lvx_reminder_ignore_quiet_mode_t)(0x2CA98DEC | 1))  /* Thumb bit set */
#define lvx_reminder_ignore_sleep_mode ((lvx_reminder_ignore_sleep_mode_t)(0x2CA98E04 | 1))  /* Thumb bit set */
#define lvx_reminder_ignore_mute_mode  ((lvx_reminder_ignore_mute_mode_t) (0x2CA98E1C | 1))  /* Thumb bit set */
#define lvx_reminder_set_flag_bit7     ((lvx_reminder_set_flag_bit7_t)   (0x2CA98E34 | 1))   /* Thumb bit set */
#define lvx_reminder_set_kind_byte     ((lvx_reminder_set_kind_byte_t)   (0x2CA998AC | 1))   /* Thumb bit set */
#define lvx_reminder_set_life_times    ((lvx_reminder_set_life_times_t)  (0x2CA98DCC | 1))   /* Thumb bit set */
#define lvx_reminder_set_on_create     ((lvx_reminder_set_on_create_t)   (0x2CA98D44 | 1))   /* Thumb bit set */
#define lvx_reminder_set_on_destroy    ((lvx_reminder_set_on_destroy_t)  (0x2CA98D54 | 1))   /* Thumb bit set */
#define lvx_reminder_disable_backspace ((lvx_reminder_disable_backspace_t)(0x2CA98CFC | 1)) /* Thumb bit set */
#define lvx_reminder_set_bell          ((lvx_reminder_set_bell_t)        (0x2CA99994 | 1))   /* Thumb bit set */
#define lvx_reminder_start             ((lvx_reminder_start_t)           (0x2CA98E7C | 1))   /* Thumb bit set */
#define lvx_reminder_cancel            ((lvx_reminder_cancel_t)          (0x2CA99860 | 1))   /* Thumb bit set */

/* ============================================================================
 * Notifications
 * ============================================================================ */
#define lvx_notification_insert_message ((notification_insert_t)(0x2CA9A898 | 1))

/* ============================================================================
 * Built-in OTA reminder
 * ============================================================================ */
#define reminder_page_ota_start       ((reminder_page_ota_start_t)(0x2C5329F8 | 1))                       /* Thumb bit set */

/* ============================================================================
 * LVGL timer (typedefs in lvgl/timer.h)
 * ============================================================================ */
#define lv_timer_create               ((lv_timer_create_t)(0x2C587ED0 | 1))                                /* Thumb bit set */
#define lv_timer_delete               ((lv_timer_delete_t)(0x2C588C08 | 1))                                /* Thumb bit set */

/* ============================================================================
 * LVGL control widgets (typedefs in lvgl/control.h)
 *
 * All `_create` functions are Thumb and follow the same pattern:
 *     lv_<class>_create(parent)
 *     -> lv_obj_class_create_obj_veneer2(&lv_<class>, parent)
 *     -> lv_obj_class_init_obj_veneer(obj, 0)
 *     -> return obj
 *
 * Bindings below use the generic lv_class_create_t typedef since every
 * constructor has signature `lv_obj_t *(*)(lv_obj_t *parent)`.
 * ============================================================================ */
#define lv_obj_create                 ((lv_obj_create_t)  (0x2C5881E0 | 1))                               /* Thumb bit set */
#define lv_image_create               ((lv_obj_create_t)  (0x2C588D58 | 1))                               /* Thumb bit set (lv_image_create_veneer) */
#define lv_label_create               ((lv_class_create_t)(0x2C588F30 | 1))                               /* Thumb bit set (lv_label_create_veneer) */

#define lv_analog_time_create         ((lv_class_create_t)(0x2CA47268 | 1))                               /* Thumb bit set */
#define lv_btn_create                 ((lv_obj_create_t)  (0x2C588BA0 | 1))                               /* Thumb bit set (lv_btn_create_veneer) */
#define lvx_btn_create                ((lv_class_create_t)(0x2CA48474 | 1))                               /* Thumb bit set (lvx_btn_create in fw) */
#define lv_xchart_create              ((lv_class_create_t)(0x2CA4A9E0 | 1))                               /* Thumb bit set */
#define lv_crossview_create           ((lv_class_create_t)(0x2C50B174 | 1))                               /* Thumb bit set */
#define lv_sleepchart_create          ((lv_class_create_t)(0x2C646ED4 | 1))                               /* Thumb bit set */
#define lv_curvedlabel_create         ((lv_class_create_t)(0x2CA4EDB0 | 1))                               /* Thumb bit set */
#define lv_frameanim_create           ((lv_class_create_t)(0x2CA504D4 | 1))                               /* Thumb bit set */
#define lv_imglabel_create            ((lv_class_create_t)(0x2CA51978 | 1))                               /* Thumb bit set */
#define lv_indicator_create           ((lv_class_create_t)(0x2CA58474 | 1))                               /* Thumb bit set */
#define lv_optionlist_create          ((lv_class_create_t)(0x2CA5ED70 | 1))                               /* Thumb bit set */
#define lv_slider_create              ((lv_class_create_t)(0x2CA5FA7C | 1))                               /* Thumb bit set */
#define lv_xview_create               ((lv_class_create_t)(0x2CA60658 | 1))                               /* Thumb bit set */
#define lv_timeview_create            ((lv_class_create_t)(0x2CA65590 | 1))                               /* Thumb bit set */
#define lv_xwin_create                ((lv_class_create_t)(0x2CA67244 | 1))                               /* Thumb bit set */
#define lv_zoombtn_create             ((lv_class_create_t)(0x2CA67FE0 | 1))                               /* Thumb bit set */
#define lv_eventdispatch_create       ((lv_class_create_t)(0x2CA750E8 | 1))                               /* Thumb bit set */
#define lv_renderdata_create          ((lv_class_create_t)(0x2CA88180 | 1))                               /* Thumb bit set */
#define lv_render_file_create         ((lv_class_create_t)(0x2CA886A0 | 1))                               /* Thumb bit set */
#define lv_imgarc_create              ((lv_class_create_t)(0x2CA89C58 | 1))                               /* Thumb bit set */
#define lv_slot_create                ((lv_class_create_t)(0x2CA8BDEC | 1))                               /* Thumb bit set */
#define lv_text_create                ((lv_class_create_t)(0x2CA8CD58 | 1))                               /* Thumb bit set */
#define lv_wfwidget_create            ((lv_class_create_t)(0x2CA8F2A4 | 1))                               /* Thumb bit set */
#define lv_widgetlayout_create        ((lv_class_create_t)(0x2CA9BDEC | 1))                               /* Thumb bit set */

/* ============================================================================
 * LVGL generic widget methods (typedefs in lvgl/control.h)
 * Both are Thumb thunks that JUMPOUT into the apps/graphics/lvgl library.
 * ============================================================================ */
#define lv_obj_set_size               ((lv_obj_set_size_t)  (0x2C588628 | 1))                              /* Thumb bit set */
#define lv_image_set_src              ((lv_image_set_src_t) (0x2C587FA0 | 1))                              /* Thumb bit set */
#define lv_obj_add_event_cb           ((lv_obj_add_event_cb_t)(0x2C5883F0 | 1))                            /* Thumb bit set */
#define lv_label_set_text             ((lv_label_set_text_t)(0x2C589490 | 1))                              /* Thumb bit set */
#define lvx_btn_set_text_fmt               ((lvx_btn_set_text_fmt_t)  (0x2CA48688 | 1))                              /* Thumb bit set */
#define lv_obj_add_flag               ((lv_obj_add_flag_t)  (0x2C588960 | 1))                              /* Thumb bit set */
#define lv_obj_clear_flag             ((lv_obj_clear_flag_t)(0x2CAC0540 | 1))                              /* Thumb bit set (lv_obj_clear_flag_veneer) */
#define lv_obj_align                  ((lv_obj_align_t)     (0x2C589188 | 1))                              /* Thumb bit set */
#define lv_obj_align_to               ((lv_obj_align_to_t)  (0x2C588EF0 | 1))                              /* Thumb bit set */
#define lv_obj_set_width              ((lv_obj_set_width_t) (0x2C588420 | 1))                              /* Thumb bit set */
#define lv_obj_set_style_text_align   ((lv_obj_set_style_text_align_t)(0x2C588830 | 1))                    /* Thumb bit set */
#define lv_label_set_long_mode       ((lv_label_set_long_mode_t)(0x2C588BF8 | 1))                          /* Thumb bit set */

/* ============================================================================
 * stdio (typedefs/prototypes in misc/print.h)
 * Thumb, direct implementation -- not a thunk.
 * ============================================================================ */
#define snprintf                    ((int (*)(char *, size_t, const char *, ...))(0x2C1EA8BC | 1))               /* Thumb bit set */

/* ---- Style setters (firmware exposes _v variants; the public API strips the suffix) ---- */
#define lv_obj_set_style_bg_color     ((lv_obj_set_style_bg_color_t)(0x2CAC00A0 | 1))                        /* Thumb bit set (lv_obj_set_style_bg_color_v) */
#define lvx_btn_set_style_bg_color    ((lvx_btn_set_style_bg_color_t)(0x2CA48510 | 1))                        /* Thumb bit set */
#define lv_obj_set_style_bg_opa       ((lv_obj_set_style_bg_opa_t)  (0x2CAC0F90 | 1))                        /* Thumb bit set (lv_obj_set_style_bg_opa_v) */
#define lv_obj_set_style_pad_left     ((lv_obj_set_style_pad_t)    (0x2CAC0E48 | 1))                        /* Thumb bit set (lv_obj_set_style_pad_left_v) */
#define lv_obj_set_style_pad_right    ((lv_obj_set_style_pad_t)    (0x2CAC01E0 | 1))                        /* Thumb bit set (lv_obj_set_style_pad_right_v) */
#define lv_obj_set_style_pad_top      ((lv_obj_set_style_pad_t)    (0x2CAC0D10 | 1))                        /* Thumb bit set (lv_obj_set_style_pad_top_v) */
#define lv_obj_set_style_pad_bottom   ((lv_obj_set_style_pad_t)    (0x2CAC1198 | 1))                        /* Thumb bit set (lv_obj_set_style_pad_bottom_v) */
#define lv_obj_set_style_pad_column   ((lv_obj_set_style_pad_t)    (0x2CAC06C8 | 1))                        /* Thumb bit set (lv_obj_set_style_pad_column_v) */
#define lvx_obj_set_style_text        ((lvx_obj_set_style_text_t)  (0x2C4BF898 | 1))                        /* Thumb bit set */

/* ============================================================================
 * Event dispatcher + animation veneer family (typedefs in lvgl/control.h)
 * Names below are stripped of the firmware's `_veneer` suffix at the C
 * boundary -- the underlying symbols keep it.
 * ============================================================================ */
#define lv_obj_event                  ((lv_obj_event_t)           (0x2CA48A9C | 1))                           /* Thumb bit set (lv_obj_event_veneer) */
#define lv_obj_get_event_code         ((lv_obj_get_event_code_t)  (0x2CAC1068 | 1))                           /* Thumb bit set (lv_obj_get_event_code_veneer) */
#define lv_obj_get_target             ((lv_obj_get_target_t)      (0x2CAC04B0 | 1))                           /* Thumb bit set (lv_obj_get_target_veneer) */
#define lv_anim_del                   ((lv_anim_del_t)            (0x2CAC0990 | 1))                           /* Thumb bit set (lv_anim_del_veneer) */
#define lvx_label_set_time_text       ((lvx_label_set_time_text_t)(0x2CA4881C | 1))                           /* Thumb bit set */
#define lv_anim_set                   ((lv_anim_set_t)           (0x2CAC0608 | 1))                           /* Thumb bit set (lv_anim_set_veneer) */
#define lv_anim_start                 ((lv_anim_start_t)         (0x2CAC0A10 | 1))                           /* Thumb bit set (lv_anim_start_veneer) */

/* ============================================================================
 * lv_event_get_* family (typedefs in lvgl/control.h) -- small field readers
 * in the lv_event_t struct, exposed without the firmware's _veneer suffix.
 * ============================================================================ */
#define lv_event_get_user_data        ((lv_event_get_user_data_t) (0x2CAC01A0 | 1))                           /* Thumb bit set (lv_event_get_user_data_veneer) */
#define lv_event_get_param            ((lv_event_get_param_t)     (0x2CAC0608 | 1))                           /* Thumb bit set (lv_event_get_param_veneer) */

/* Firmware data addresses for the label-style structs that
 * lvx_obj_set_style_text pulls from. Cast to (lv_style_t *) at call sites.
 * 0x2010CF28 / 0x2010CF34 are in the firmware SRAM region (0x20000000+). */
#define LB_TEXT_STYLE_1              ((uintptr_t)0x2010CF28)
#define LB_TEXT_STYLE_2              ((uintptr_t)0x2010CF34)
#define LB_TEXT_STYLE_3              ((uintptr_t)0x2010D054)

/* ============================================================================
 * Screen power management (typedefs in nuttx/screen.h)
 * All Thumb; |1 forces BLX into Thumb state. Functions are direct ARM code
 * (not thunks), sizes range from 0x3c to 0x7c bytes.
 * ============================================================================ */
#define screen_turn_on                       ((int (*)(const char *, int))                 (0x2CA7323C | 1))     /* Thumb bit set */
#define screen_turn_off                      ((const char *(*)(const char *))             (0x2CA73290 | 1))     /* Thumb bit set */
#define screen_set_keepon                    ((const char *(*)(const char *, int))       (0x2CA73198 | 1))     /* Thumb bit set */
#define screen_toggle_keepon_duration        ((int (*)(const char *, int))                 (0x2CA7310C | 1))     /* Thumb bit set */

#define screen_session_start                 ((int (*)(screen_session_t *, int))          (0x2CA72B6C | 1))     /* Thumb bit set */
#define screen_session_stop                  ((int (*)(screen_session_t *))               (0x2CA72BFC | 1))     /* Thumb bit set */
#define screen_session_restore_default       ((int (*)(screen_session_t *))               (0x2CA72EC8 | 1))     /* Thumb bit set */
#define screen_session_set_full_power        ((int (*)(screen_session_t *, int))          (0x2CA72E74 | 1))     /* Thumb bit set */
#define screen_session_set_brightness_value  ((int (*)(screen_session_t *, int))          (0x2CA72E1C | 1))     /* Thumb bit set */
#define screen_session_set_auto_brightness   ((int (*)(screen_session_t *, int))          (0x2CA72DB8 | 1))     /* Thumb bit set */
#define screen_session_set_keepon             ((int (*)(screen_session_t *, int))          (0x2CA72D64 | 1))     /* Thumb bit set */
#define screen_session_set_touch_palm         ((int (*)(screen_session_t *, int))          (0x2CA72D10 | 1))     /* Thumb bit set */
#define screen_session_set_wrist_drop         ((int (*)(screen_session_t *, int))          (0x2CA72CBC | 1))     /* Thumb bit set */

/* ============================================================================
 * miwear launcher / packagemanager dispatch (typedefs in miwear/app.h)
 *
 * packagemanager_api->install(...) resolves to a Thumb call directly.
 * ============================================================================ */
#define g_packagemanager_api           ((struct packagemanager_api_s *)0x200EB660)
#define app_launcher_add              ((app_launcher_add_t)(0x2C513A44 | 1))    /* Thumb bit set */
#define app_lookup                    ((app_lookup_t)(0x2CA69934 | 1))          /* Thumb bit set */

/* ============================================================================
 * Watchface manager (typedefs in miwear/watchface.h)
 *
 * g_watchface_config is the heap-allocated 120-byte manager state
 * (owned by watchface_config_init, its SRAM address is fixed and stable
 * after boot). use -DMB10P_FW_VERSION=3101043 to bind v1 below.
 *
 * wf_delete_watchface is the launcher-side "delete a watchface" entry.
 * Found in vela_ap.bin via xrefs to the "[%s] %s: delete watchface(%s)
 * success" string at 0x2CCB013C. The Thumb entry is wf_delete_watchface
 * at 0x2CA7C774 (size ~0x45E bytes).
 * ============================================================================ */
#define g_watchface_config                      (*(watchface_config_t **)0x2013FE6C)
#define watchface_manager_delete_watchface      ((watchface_manager_delete_watchface_t)(0x2CA95C68 | 1))
#define watchface_manager_reset_watchface       ((watchface_manager_reset_watchface_t)(0x2C5F36C0 | 1))

#endif /* MB10P_MIWEAR_SYSTEM_FW_3_101_043_H */
