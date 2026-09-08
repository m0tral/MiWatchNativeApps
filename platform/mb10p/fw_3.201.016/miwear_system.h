#ifndef MB10P_MIWEAR_SYSTEM_FW_3_201_016_H
#define MB10P_MIWEAR_SYSTEM_FW_3_201_016_H

/* mb10pro (Xiaomi Mi Band 10 Pro) per-firmware system + miwear bindings.
 *
 * This is the SINGLE per-version platform header for the project.
 * It owns every macro that resolves a firmware symbol to a Thumb-callable
 * function pointer or to a data pointer, all reverse-engineered from
 * vela_ap.bin (fw_3.201.016) via the IDA Pro MCP tools (instance2,
 * port 13338). Two layers coexist here:
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
#include "common/miwear/watchface.h" /* for watchface_manager_delete_watchface_t / watchface_manager_reset_watchface_t typedefs; watchface_config_t (v2) comes from platform/mb10p/watchface_layout.h via the family dispatcher */
#include "common/misc/print.h" /* for the snprintf prototype */

/* ============================================================================
 * Logger
 * ============================================================================ */
#define syslog                       ((syslog_t)(0x2C1BEA08 | 1))              /* Thumb bit set */

/* ============================================================================
 * LVGL reminder subsystem (struct reminder_t in nuttx/reminder.h)
 * ============================================================================ */
#define lvx_reminder_set_uid           ((lvx_reminder_set_uid_t)         (0x2C7D063C | 1))   /* Thumb bit set */
#define lvx_reminder_set_level         ((lvx_reminder_set_level_t)       (0x2C7D061C | 1))   /* Thumb bit set */
#define lvx_reminder_set_vibration     ((lvx_reminder_set_vibration_t)   (0x2C7D062C | 1))   /* Thumb bit set */
#define lvx_reminder_ignore_quiet_mode ((lvx_reminder_ignore_quiet_mode_t)(0x2C7CFB8C | 1))  /* Thumb bit set */
#define lvx_reminder_ignore_sleep_mode ((lvx_reminder_ignore_sleep_mode_t)(0x2C7CFBA4 | 1))  /* Thumb bit set */
#define lvx_reminder_ignore_mute_mode  ((lvx_reminder_ignore_mute_mode_t) (0x2C7CFBBC | 1))  /* Thumb bit set */
#define lvx_reminder_set_flag_bit7     ((lvx_reminder_set_flag_bit7_t)   (0x2C7CFBD4 | 1))   /* Thumb bit set */
#define lvx_reminder_set_kind_byte     ((lvx_reminder_set_kind_byte_t)   (0x2C7D064C | 1))   /* Thumb bit set */
#define lvx_reminder_set_life_times    ((lvx_reminder_set_life_times_t)  (0x2C7CFB6C | 1))   /* Thumb bit set */
#define lvx_reminder_set_on_create     ((lvx_reminder_set_on_create_t)   (0x2C7CFAE4 | 1))   /* Thumb bit set */
#define lvx_reminder_set_on_destroy    ((lvx_reminder_set_on_destroy_t)  (0x2C7CFAF4 | 1))   /* Thumb bit set */
#define lvx_reminder_disable_backspace ((lvx_reminder_disable_backspace_t)(0x2C7CFAB4 | 1)) /* Thumb bit set */
#define lvx_reminder_set_bell          ((lvx_reminder_set_bell_t)        (0x2C7D0734 | 1))   /* Thumb bit set */
#define lvx_reminder_start             ((lvx_reminder_start_t)           (0x2C7CFC1C | 1))   /* Thumb bit set */
#define lvx_reminder_cancel            ((lvx_reminder_cancel_t)          (0x2C7D0600 | 1))   /* Thumb bit set */

/* ============================================================================
 * Notifications
 * ============================================================================ */
#define lvx_notification_insert_message  ((notification_insert_t)(0x2C7D1318 | 1))

/* ============================================================================
 * Built-in OTA reminder
 * ============================================================================ */
#define reminder_page_ota_start   ((reminder_page_ota_start_t)(0x2C4A9B80 | 1))

/* ============================================================================
 * LVGL timer (typedefs in lvgl/timer.h)
 * ============================================================================ */
#define lv_timer_create               ((lv_timer_create_t)(0x2C5588C0 | 1))                                /* Thumb bit set */
#define lv_timer_delete               ((lv_timer_delete_t)(0x2C559AE8 | 1))                                /* Thumb bit set */

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
#define lv_obj_create                 ((lv_obj_create_t)  (0x2C107928 | 1))                               /* Thumb bit set */
#define lv_image_create               ((lv_obj_create_t)  (0x2C557E70 | 1))                               /* Thumb bit set (lv_image_create_veneer) */
#define lv_label_create               ((lv_class_create_t)(0x2C5588A8 | 1))                               /* Thumb bit set (lv_label_create_veneer) */

#define lv_analog_time_create         ((lv_class_create_t)(0x2C141F48 | 1))                               /* Thumb bit set */
#define lv_btn_create                 ((lv_obj_create_t)  (0x2C14DE08 | 1))                               /* Thumb bit set (lv_btn_create_veneer) */
#define lvx_btn_create                ((lv_class_create_t)(0x2C77F734 | 1))                               /* Thumb bit set (lvx_btn_create in fw) */
#define lv_canvas_create              ((lv_class_create_t)(0x2C136AC0 | 1))                               /* Thumb bit set */
#define lv_crossview_create           ((lv_class_create_t)(0x2C143B40 | 1))                               /* Thumb bit set */
#define lv_sleepchart_create          ((lv_class_create_t)(0x2C144878 | 1))                               /* Thumb bit set */
#define lv_curvedlabel_create         ((lv_class_create_t)(0x2C786070 | 1))                               /* Thumb bit set */
#define lv_frameanim_create           ((lv_class_create_t)(0x2C787794 | 1))                               /* Thumb bit set - UNRESOLVED: 0x2C145E8C has class name "image", not "frameanim" */
#define lv_imglabel_create            ((lv_class_create_t)(0x2C145DC0 | 1))                               /* Thumb bit set */
#define lv_indicator_create           ((lv_class_create_t)(0x2C147F04 | 1))                               /* Thumb bit set */
#define lv_optionlist_create          ((lv_class_create_t)(0x2C149208 | 1))                               /* Thumb bit set */
#define lv_slider_create              ((lv_class_create_t)(0x2C149868 | 1))                               /* Thumb bit set */
#define lv_xview_create               ((lv_class_create_t)(0x2C14B784 | 1))                               /* Thumb bit set */
#define lv_timeview_create            ((lv_class_create_t)(0x2C14BCB8 | 1))                               /* Thumb bit set */
#define lv_xwin_create                ((lv_class_create_t)(0x2C14D958 | 1))                               /* Thumb bit set */
#define lv_zoombtn_create             ((lv_class_create_t)(0x2C14DD88 | 1))                               /* Thumb bit set */
#define lv_eventdispatch_create       ((lv_class_create_t)(0x2C14EFE4 | 1))                               /* Thumb bit set */
#define lv_renderdata_create          ((lv_class_create_t)(0))                               /* Thumb bit set */
#define lv_render_file_create         ((lv_class_create_t)(0x2C7BF4BC | 1))                               /* Thumb bit set */
#define lv_imgarc_create              ((lv_class_create_t)(0x2C1569D8 | 1))                               /* Thumb bit set */
#define lv_slot_create                ((lv_class_create_t)(0x2C7C2BC4 | 1))                               /* Thumb bit set */
#define lv_text_create                ((lv_class_create_t)(0x2C7C3B08 | 1))                               /* Thumb bit set */
#define lv_wfwidget_create            ((lv_class_create_t)(0x2C7C6054 | 1))                               /* Thumb bit set */
#define lv_widgetlayout_create        ((lv_class_create_t)(0x2C156F0C | 1))                               /* Thumb bit set */

/* ============================================================================
 * LVGL generic widget methods (typedefs in lvgl/control.h)
 * Both are Thumb thunks that JUMPOUT into the apps/graphics/lvgl library.
 * ============================================================================ */
#define lv_obj_set_size               ((lv_obj_set_size_t)  (0x2C5590E8 | 1))                              /* Thumb bit set */
#define lv_image_set_src              ((lv_image_set_src_t) (0x2C559968 | 1))                              /* Thumb bit set */
#define lv_obj_add_event_cb           ((lv_obj_add_event_cb_t)(0x2C558028 | 1))                            /* Thumb bit set */
#define lv_label_set_text             ((lv_label_set_text_t)(0x2C559B20 | 1))                              /* Thumb bit set */
#define lvx_btn_set_text_fmt               ((lvx_btn_set_text_fmt_t)  (0x2C77F948 | 1))                              /* Thumb bit set */
#define lv_obj_add_flag               ((lv_obj_add_flag_t)  (0x2C5597F8 | 1))                              /* Thumb bit set */
#define lv_obj_clear_flag             ((lv_obj_clear_flag_t)(0x2C558688 | 1))                              /* Thumb bit set (lv_obj_clear_flag_veneer) */
#define lv_obj_align                  ((lv_obj_align_t)     (0x2C558590 | 1))                              /* Thumb bit set */
#define lv_obj_align_to               ((lv_obj_align_to_t)  (0x2C5592D0 | 1))                              /* Thumb bit set */
#define lv_obj_set_width              ((lv_obj_set_width_t) (0x2C558300 | 1))                              /* Thumb bit set */
#define lv_obj_set_style_text_align   ((lv_obj_set_style_text_align_t)(0x2C558FC0 | 1))                    /* Thumb bit set */
#define lv_label_set_long_mode       ((lv_label_set_long_mode_t)(0x2C558CE0 | 1))                          /* Thumb bit set */

/* ============================================================================
 * stdio (typedefs/prototypes in misc/print.h)
 * Thumb, direct implementation -- not a thunk.
 * ============================================================================ */
#define snprintf                    ((int (*)(char *, size_t, const char *, ...))(0x2C1B8DBC | 1))               /* Thumb bit set */

/* ---- Style setters (firmware exposes _v variants; the public API strips the suffix) ---- */
#define lv_obj_set_style_bg_color     ((lv_obj_set_style_bg_color_t)(0x2C558790 | 1))                        /* Thumb bit set (lv_obj_set_style_bg_color_v) */
#define lvx_btn_set_style_bg_color    ((lvx_btn_set_style_bg_color_t)(0x2C77F7D0 | 1))                        /* Thumb bit set */
#define lv_obj_set_style_bg_opa       ((lv_obj_set_style_bg_opa_t)  (0x2C5585E0 | 1))                        /* Thumb bit set (lv_obj_set_style_bg_opa_v) */
#define lv_obj_set_style_pad_left     ((lv_obj_set_style_pad_t)    (0x2C5583E0 | 1))                        /* Thumb bit set (lv_obj_set_style_pad_left_v) */
#define lv_obj_set_style_pad_right    ((lv_obj_set_style_pad_t)    (0x2C558CC0 | 1))                        /* Thumb bit set (lv_obj_set_style_pad_right_v) */
#define lv_obj_set_style_pad_top      ((lv_obj_set_style_pad_t)    (0x2C559140 | 1))                        /* Thumb bit set (lv_obj_set_style_pad_top_v) */
#define lv_obj_set_style_pad_bottom   ((lv_obj_set_style_pad_t)    (0x2C559548 | 1))                        /* Thumb bit set (lv_obj_set_style_pad_bottom_v) */
#define lv_obj_set_style_pad_row      ((lv_obj_set_style_pad_t)    (0x2C557E30 | 1))                        /* Thumb bit set (lv_obj_set_style_pad_column_v) */
#define lv_obj_set_style_pad_column   ((lv_obj_set_style_pad_t)    (0x2C557E68 | 1))                        /* Thumb bit set (lv_obj_set_style_pad_column_v) */
#define lvx_obj_set_style_text        ((lvx_obj_set_style_text_t)  (0x2C44996C | 1))                        /* Thumb bit set */

/* ============================================================================
 * Event dispatcher + animation veneer family (typedefs in lvgl/control.h)
 * Names below are stripped of the firmware's `_veneer` suffix at the C
 * boundary -- the underlying symbols keep it.
 * ============================================================================ */                            /* Thumb bit set */
#define lvx_label_set_time_text       ((lvx_label_set_time_text_t)(0x2C77FADC | 1))                           /* Thumb bit set */
#define lv_anim_set                   ((lv_anim_set_t)            (0))                           /* Thumb bit set (lv_anim_set_veneer) */
#define lv_anim_start                 ((lv_anim_start_t)          (0))                           /* Thumb bit set (lv_anim_start_veneer) */
#define lv_anim_del                   ((lv_anim_del_t)            (0))                           /* Thumb bit set (lv_anim_del_veneer) */

/* ============================================================================
 * lv_event_get_* family (typedefs in lvgl/control.h) -- small field readers
 * in the lv_event_t struct, exposed without the firmware's _veneer suffix.
 * ============================================================================ */
#define lv_obj_event                  ((lv_obj_event_t)           (0))                           /* Thumb bit set (lv_obj_event_veneer) */
#define lv_obj_get_event_code         ((lv_obj_get_event_code_t)  (0x2C558FA0 | 1))                           /* Thumb bit set (lv_obj_get_event_code_veneer) */
#define lv_obj_get_target             ((lv_obj_get_target_t)      (0))                           /* Thumb bit set (lv_obj_get_target_veneer) */
#define lv_obj_get_parent             ((lv_obj_get_target_t)      (0x2C558EF8 | 1))  
#define lv_event_get_user_data        ((lv_event_get_user_data_t) (0x2C557D70 | 1))                           /* Thumb bit set (lv_event_get_user_data_veneer) */
#define lv_event_get_param            ((lv_event_get_param_t)     (0))                           /* Thumb bit set (lv_event_get_param_veneer) */

/* Firmware data addresses for the label-style structs that
 * lvx_obj_set_style_text pulls from. Cast to (lv_style_t *) at call sites.
 * 0x200D66E0 / 0x200D66EC are in the firmware SRAM region (0x20000000+). */
#define LB_TEXT_STYLE_1              ((uintptr_t)0x200D66E0)
#define LB_TEXT_STYLE_2              ((uintptr_t)0x200D66EC)
#define LB_TEXT_STYLE_3              ((uintptr_t)0x200D680C)

/* ============================================================================
 * Screen power management (typedefs in nuttx/screen.h)
 * All Thumb; |1 forces BLX into Thumb state. Functions are direct ARM code
 * (not thunks), sizes range from 0x3c to 0x7c bytes.
 * ============================================================================ */
#define screen_turn_on                       ((int (*)(const char *, int))                 (0x2C7A9FFC | 1))     /* Thumb bit set */
#define screen_turn_off                      ((const char *(*)(const char *))             (0x2C7AA050 | 1))     /* Thumb bit set */
#define screen_set_keepon                    ((const char *(*)(const char *, int))       (0x2C7A9F58 | 1))     /* Thumb bit set */
#define screen_toggle_keepon_duration        ((int (*)(const char *, int))                 (0x2C7A9ECC | 1))     /* Thumb bit set */

#define screen_session_start                 ((int (*)(screen_session_t *, int))          (0x2C7A992C | 1))     /* Thumb bit set */
#define screen_session_stop                  ((int (*)(screen_session_t *))               (0x2C7A99BC | 1))     /* Thumb bit set */
#define screen_session_restore_default       ((int (*)(screen_session_t *))               (0x2C7A9C88 | 1))     /* Thumb bit set */
#define screen_session_set_full_power        ((int (*)(screen_session_t *, int))          (0x2C7A9C34 | 1))     /* Thumb bit set */
#define screen_session_set_brightness_value  ((int (*)(screen_session_t *, int))          (0x2C7A9BDC | 1))     /* Thumb bit set */
#define screen_session_set_auto_brightness   ((int (*)(screen_session_t *, int))          (0x2C7A9B78 | 1))     /* Thumb bit set */
#define screen_session_set_keepon             ((int (*)(screen_session_t *, int))          (0x2C7A9B24 | 1))     /* Thumb bit set */
#define screen_session_set_touch_palm         ((int (*)(screen_session_t *, int))          (0x2C7A9AD0 | 1))     /* Thumb bit set */
#define screen_session_set_wrist_drop         ((int (*)(screen_session_t *, int))          (0x2C7A9A7C | 1))     /* Thumb bit set */

/* ============================================================================
 * miwear launcher / packagemanager dispatch (typedefs in miwear/app.h)
 *
 * packagemanager_api->install(...) resolves to a Thumb call directly.
 * ============================================================================ */
#define g_packagemanager_api           ((struct packagemanager_api_s *)0x200B7140)
#define app_launcher_add              ((app_launcher_add_t)(0x2C48B62C | 1))    /* Thumb bit set */
#define app_lookup                    ((app_lookup_t)(0x2C7A06EC | 1))          /* Thumb bit set */

/* ============================================================================
 * Watchface manager (typedefs in miwear/watchface.h)
 *
 * Heap-allocated 120-byte manager state at the same SRAM slot as v1
 * (vela_ap.bin instance2 layout). The deletion entry is sub_2C7B3510
 * (no symbol yet in instance2; found via xrefs to the
 * "[%s] %s: delete watchface success" string at 0x2C97A760).
 * Bind when building with -DMB10P_FW_VERSION=3201016.
 * ============================================================================ */
#define g_watchface_config                      (*(watchface_config_t **)0x200FE3FC)
#define watchface_manager_delete_watchface      ((watchface_manager_delete_watchface_t)(0x2C7CC994 | 1))
#define watchface_manager_reset_watchface       ((watchface_manager_reset_watchface_t)(0x2C553B68 | 1))

#endif /* MB10P_MIWEAR_SYSTEM_FW_3_201_016_H */
