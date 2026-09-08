#ifndef WATCHS3_MIWEAR_SYSTEM_FW_2_6_0_H
#define WATCHS3_MIWEAR_SYSTEM_FW_2_6_0_H

/* watchs3 (Xiaomi Watch S3) per-firmware system + miwear bindings.
 *
 * This is the SINGLE per-version platform header for the project.
 * It owns every macro that resolves a firmware symbol to a Thumb-callable
 * function pointer or to a data pointer, all reverse-engineered from
 * vela_ap.bin (fw_2.6.0) via the IDA Pro MCP tools (instance2,
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
 *   - Every binding is split into a `XXX_ADDR` unsigned address constant
 *     and a typed pointer macro `xxx` that casts that address. Code
 *     addresses must carry `| 1` (Thumb bit) in their `_ADDR` value so
 *     BLX switches state.
 *   - Typedefs live in the matching family header (common/nuttx/syslog.h,
 *     common/nuttx/reminder.h, common/lvgl/timer.h, common/lvgl/control.h,
 *     common/miwear/app.h, common/nuttx/screen.h, common/misc/print.h).
 *   - DO NOT put new prototypes here; put them in the family header.
 *   - All addresses below are currently 0 (UNKNOWN). Per-TU static
 *     asserts live in the corresponding *_asserts.h header (e.g.
 *     WatchS3/watchs3_app_asserts.h) so only the bindings actually
 *     consumed by each compilation unit gate the build.
 *
 * Recovery status (WatchS3 2.6.0, IDA Pro MCP port 13337):
 *
 *   Function                                   | Address          | Conf
 *   ------------------------------------------- + ---------------- + -----
 *   syslog                       (named)        | 0x2C349E39 (T)  | EXACT
 *   lvx_notification_insert_message (named)    | 0x2C771781 (T)  | EXACT
 *   lv_timer_create              (timer list)  | 0x2C50C2FD (T)  | HIGH
 *   lv_timer_delete              (timer list)  | 0x2C50C355 (T)  | HIGH
 *   g_packagemanager_api         (data table)  | 0x3C213BD4      | EXACT
 *   watchface_manager_delete_watchface (named) | 0x2C75A04D (T)  | EXACT
 *
 * Unresolved (left at UNRESOLVED template per
 * fw_func_search_prompt3.md -- assert will fire in *_asserts.h so
 * the build gates on each TU consuming them):
 *
 *   app_launcher_add(app_id)          -- UNRESOLVED (orig 0x2C48B62C
 *                                        points inside sub_2C48B148, an
 *                                        SSL/HTTP handler, not this fn)
 *   app_lookup(app_id)                -- UNRESOLVED (orig 0x2C7A06EC
 *                                        is valid Thumb-2 code but IDA
 *                                        did not auto-analyze this
 *                                        region; needs manual define_func
 *                                        + decompilation before sign-off)
 *   watchface_manager_reset_watchface(face_id) -- UNRESOLVED (no
 *                                        "reset watchface" string
 *                                        found anywhere in fw)
 *
 * (T) = Thumb bit set on the address; plain value = data pointer.
 */

#include "common/nuttx/syslog.h"
#include "common/nuttx/reminder.h"
#include "common/nuttx/notification.h"
#include "common/nuttx/screen.h"
#include "common/lvgl/timer.h"
#include "common/lvgl/control.h"
#include "common/miwear/app.h"      /* launcher helpers, packagemanager dispatch, lv_ll_t typedef */
#include "common/miwear/watchface.h" /* for watchface_config_t and watchface_manager_delete_watchface_t; on WatchS3 the g_watchface_config->mid bytes (offsets +0x50..+0x6F) are opaque zero-fill -- the firmware does NOT touch this region, only the per-model mb10p firmware does */
#include "common/misc/print.h" /* for the snprintf prototype */

/* ============================================================================
 * Firmware address constants (UNKNOWN -- set to 0; resolve via IDA)
 * ============================================================================ */

/* Logger */
#define SYSLOG_ADDR                          (0x2C349E38 | 1)

/* LVGL reminder subsystem (struct reminder_t in common/nuttx/reminder.h) */
#define LVX_REMINDER_SET_UID_ADDR            (0x00000000u | 1)
#define LVX_REMINDER_SET_LEVEL_ADDR          (0x00000000u | 1)
#define LVX_REMINDER_SET_VIBRATION_ADDR      (0x00000000u | 1)
#define LVX_REMINDER_IGNORE_QUIET_MODE_ADDR  (0x00000000u | 1)
#define LVX_REMINDER_IGNORE_SLEEP_MODE_ADDR  (0x00000000u | 1)
#define LVX_REMINDER_IGNORE_MUTE_MODE_ADDR   (0x00000000u | 1)
#define LVX_REMINDER_SET_FLAG_BIT7_ADDR      (0x00000000u | 1)
#define LVX_REMINDER_SET_KIND_BYTE_ADDR      (0x00000000u | 1)
#define LVX_REMINDER_SET_LIFE_TIMES_ADDR     (0x00000000u | 1)
#define LVX_REMINDER_SET_ON_CREATE_ADDR      (0x00000000u | 1)
#define LVX_REMINDER_SET_ON_DESTROY_ADDR     (0x00000000u | 1)
#define LVX_REMINDER_DISABLE_BACKSPACE_ADDR  (0x00000000u | 1)
#define LVX_REMINDER_SET_BELL_ADDR           (0x00000000u | 1)
#define LVX_REMINDER_START_ADDR              (0x00000000u | 1)
#define LVX_REMINDER_CANCEL_ADDR             (0x00000000u | 1)

/* Notifications */
#define LVX_NOTIFICATION_INSERT_MESSAGE_ADDR (0x2C771780u | 1)

/* Built-in OTA reminder */
#define REMINDER_PAGE_OTA_START_ADDR         (0x00000000u | 1)

/* LVGL timer (typedefs in common/lvgl/timer.h) */
#define LV_TIMER_CREATE_ADDR                 (0x2C50C2FCu | 1)
#define LV_TIMER_DELETE_ADDR                 (0x2C50C354u | 1)

/* LVGL control widget constructors (typedefs in common/lvgl/control.h) */
#define LV_OBJ_CREATE_ADDR                   (0x2C4DFA80 | 1)
#define LV_IMAGE_CREATE_ADDR                 (0x2C512A80 | 1)
#define LV_LABEL_CREATE_ADDR                 (0x2C513B1C | 1)
#define LV_ANALOG_TIME_CREATE_ADDR           (0x00000000u | 1)
#define LV_BTN_CREATE_ADDR                   (0x00000000u | 1)
#define LVX_BTN_CREATE_ADDR                  (0x2C731BF0 | 1)
#define LV_CANVAS_CREATE_ADDR                (0x00000000u | 1)
#define LV_CROSSVIEW_CREATE_ADDR             (0x00000000u | 1)
#define LV_SLEEPCHART_CREATE_ADDR            (0x00000000u | 1)
#define LV_CURVEDLABEL_CREATE_ADDR           (0x00000000u | 1)
#define LV_FRAMEANIM_CREATE_ADDR             (0x00000000u | 1)
#define LV_IMGLABEL_CREATE_ADDR              (0x00000000u | 1)
#define LV_INDICATOR_CREATE_ADDR             (0x00000000u | 1)
#define LV_OPTIONLIST_CREATE_ADDR            (0x00000000u | 1)
#define LV_SLIDER_CREATE_ADDR                (0x00000000u | 1)
#define LV_XVIEW_CREATE_ADDR                 (0x00000000u | 1)
#define LV_TIMEVIEW_CREATE_ADDR              (0x00000000u | 1)
#define LV_XWIN_CREATE_ADDR                  (0x00000000u | 1)
#define LV_ZOOMBTN_CREATE_ADDR               (0x00000000u | 1)
#define LV_EVENTDISPATCH_CREATE_ADDR         (0x00000000u | 1)
#define LV_RENDERDATA_CREATE_ADDR            (0x00000000u | 1)
#define LV_RENDER_FILE_CREATE_ADDR           (0x00000000u | 1)
#define LV_IMGARC_CREATE_ADDR                (0x00000000u | 1)
#define LV_SLOT_CREATE_ADDR                  (0x00000000u | 1)
#define LV_TEXT_CREATE_ADDR                  (0x00000000u | 1)
#define LV_WFWIDGET_CREATE_ADDR              (0x00000000u | 1)
#define LV_WIDGETLAYOUT_CREATE_ADDR          (0x00000000u | 1)

/* LVGL generic widget methods (typedefs in common/lvgl/control.h) */
#define LV_OBJ_SET_SIZE_ADDR                 (0x2C4E096C | 1)
#define LV_IMAGE_SET_SRC_ADDR                (0x00000000u | 1)
#define LV_OBJ_ADD_EVENT_CB_ADDR             (0x2C4E7AC4 | 1)
#define LV_LABEL_SET_TEXT_ADDR               (0x2C51496C | 1)
#define LVX_BTN_SET_TEXT_FMT_ADDR            (0x2C731E24 | 1)
#define LV_OBJ_ADD_FLAG_ADDR                 (0x2C4DFA9C | 1)
#define LV_OBJ_CLEAR_FLAG_ADDR               (0x00000000 | 1)
#define LV_OBJ_ALIGN_ADDR                    (0x2C4E0BE0 | 1)
#define LV_OBJ_ALIGN_TO_ADDR                 (0x2C4E2978 | 1)
#define LV_OBJ_SET_WIDTH_ADDR                (0x00000000 | 1)
#define LV_OBJ_SET_STYLE_TEXT_ALIGN_ADDR     (0x2C4E6F6C | 1)
#define LV_LABEL_SET_LONG_MODE_ADDR          (0x2C514A8C | 1)

/* stdio (prototype in common/misc/print.h) */
#define SNPRINTF_ADDR                        (0x2C342BF8 | 1)

/* Style setters */
#define LV_OBJ_SET_STYLE_BG_COLOR_ADDR       (0x00000000u | 1)
#define LVX_BTN_SET_STYLE_BG_COLOR_ADDR      (0x2C731C88u | 1)
#define LV_OBJ_SET_STYLE_BG_OPA_ADDR         (0x2C4E6C3A | 1)
#define LV_OBJ_SET_STYLE_PAD_LEFT_ADDR       (0x2C4E6BD4u | 1)
#define LV_OBJ_SET_STYLE_PAD_RIGHT_ADDR      (0x2C4E6BE8u | 1)
#define LV_OBJ_SET_STYLE_PAD_TOP_ADDR        (0x2C4E6BACu | 1)
#define LV_OBJ_SET_STYLE_PAD_BOTTOM_ADDR     (0x2C4E6BC0u | 1)
#define LV_OBJ_SET_STYLE_PAD_ROW_ADDR        (0x00000000u | 1)
#define LV_OBJ_SET_STYLE_PAD_COLUMN_ADDR     (0x00000000u | 1)
#define LVX_OBJ_SET_STYLE_TEXT_ADDR          (0x2C687F5C | 1)

/* Event dispatcher + animation veneer family */
#define LV_OBJ_EVENT_ADDR                    (0x00000000u | 1)
#define LV_OBJ_GET_EVENT_CODE_ADDR           (0x2C4E7A2C | 1)
#define LV_OBJ_GET_TARGET_ADDR               (0x00000000u | 1)
#define LV_OBJ_GET_PARENT_ADDR               (0x00000000u | 1)
#define LV_ANIM_DEL_ADDR                     (0x00000000u | 1)
#define LVX_LABEL_SET_TIME_TEXT_ADDR         (0x00000000u | 1)
#define LV_ANIM_SET_ADDR                     (0x00000000u | 1)
#define LV_ANIM_START_ADDR                   (0x00000000u | 1)

/* lv_event_get_* family */
#define LV_EVENT_GET_USER_DATA_ADDR          (0x2C4E7A50 | 1)
#define LV_EVENT_GET_PARAM_ADDR              (0x00000000u | 1)

/* Firmware data addresses for the label-style structs that
 * lvx_obj_set_style_text pulls from. Cast to (lv_style_t *) at call sites. */
#define LB_TEXT_STYLE_1_ADDR                 0x3CC80BA8 // 36
#define LB_TEXT_STYLE_2_ADDR                 0x3CC80BB0 // 40
#define LB_TEXT_STYLE_3_ADDR                 0x3CC80BB8 // 44

/* Screen power management (typedefs in common/nuttx/screen.h) */
#define SCREEN_TURN_ON_ADDR                  (0x00000000u | 1)
#define SCREEN_TURN_OFF_ADDR                 (0x00000000u | 1)
#define SCREEN_SET_KEEPON_ADDR               (0x00000000u | 1)
#define SCREEN_TOGGLE_KEEPON_DURATION_ADDR   (0x00000000u | 1)
#define SCREEN_SESSION_START_ADDR            (0x00000000u | 1)
#define SCREEN_SESSION_STOP_ADDR             (0x00000000u | 1)
#define SCREEN_SESSION_RESTORE_DEFAULT_ADDR  (0x00000000u | 1)
#define SCREEN_SESSION_SET_FULL_POWER_ADDR   (0x00000000u | 1)
#define SCREEN_SESSION_SET_BRIGHTNESS_ADDR   (0x00000000u | 1)
#define SCREEN_SESSION_SET_AUTO_BRIGHT_ADDR  (0x00000000u | 1)
#define SCREEN_SESSION_SET_KEEPON_ADDR       (0x00000000u | 1)
#define SCREEN_SESSION_SET_TOUCH_PALM_ADDR   (0x00000000u | 1)
#define SCREEN_SESSION_SET_WRIST_DROP_ADDR   (0x00000000u | 1)

/* miwear launcher / packagemanager dispatch (typedefs in common/miwear/app.h) */
#define PACKAGEMANAGER_API_ADDR               0x3C213BD8
#define APP_LAUNCHER_ADD_ADDR                 (0x00000000u | 1)
#define APP_LOOKUP_ADDR                       (0x2C74AD04 | 1)
#define LAUNCHER_DATA_LOAD_APP_INFO_ADDR    (0x2C5B1E7C | 1)

/* launcher-side runtime helpers used from watchs3_app.c.
 * lv_ll_clear -- nukes a LVGL linked-list's nodes without freeing entries
 *                (used right before launcher_data_load_app_info to force a
 *                 full reload). Named in IDB at 0x2C50AB50, size 4 B.
 * g_appinfo_list  -- static `lv_ll_t` holding installed app ids inside the
 *                    launcher subsystem. SRAM data pointer; NO Thumb bit.
 *                    Confirmed at 14+ xrefs in launcher region
 *                    (load_app_info, launcher_data_load_app_info, etc.). */
#define LV_LL_CLEAR_ADDR                    (0x2C50AB50u | 1)
#define G_APPINFO_LIST_ADDR                 0x3C204E30u
/*#define LAUNCHER_DATA_LOAD_APP_INFO_ADDR    (0x2C5B2220 | 1)*/
#define LAUNCHER_PAGE_MAIN_UPDATE_LAYOUT_ADDR (0x2C5B09A8 | 1)

/* Watchface manager (typedefs in common/miwear/watchface.h) */
#define WATCHFACE_CONFIG_ADDR                    0x3CCCF8D8
#define WATCHFACE_MANAGER_DELETE_WATCHFACE_ADDR  (0x2C75A04C | 1)
#define WATCHFACE_MANAGER_RESET_WATCHFACE_ADDR   (0x2C6295F0 | 1)

/* ============================================================================
 * Typed pointer bindings (cast the address constants above)
 * ============================================================================ */

/* Logger */
#define syslog                       ((syslog_t)SYSLOG_ADDR)

/* LVGL reminder subsystem (struct reminder_t in common/nuttx/reminder.h) */
#define lvx_reminder_set_uid           ((lvx_reminder_set_uid_t)         LVX_REMINDER_SET_UID_ADDR)
#define lvx_reminder_set_level         ((lvx_reminder_set_level_t)       LVX_REMINDER_SET_LEVEL_ADDR)
#define lvx_reminder_set_vibration     ((lvx_reminder_set_vibration_t)   LVX_REMINDER_SET_VIBRATION_ADDR)
#define lvx_reminder_ignore_quiet_mode ((lvx_reminder_ignore_quiet_mode_t)LVX_REMINDER_IGNORE_QUIET_MODE_ADDR)
#define lvx_reminder_ignore_sleep_mode ((lvx_reminder_ignore_sleep_mode_t)LVX_REMINDER_IGNORE_SLEEP_MODE_ADDR)
#define lvx_reminder_ignore_mute_mode  ((lvx_reminder_ignore_mute_mode_t) LVX_REMINDER_IGNORE_MUTE_MODE_ADDR)
#define lvx_reminder_set_flag_bit7     ((lvx_reminder_set_flag_bit7_t)   LVX_REMINDER_SET_FLAG_BIT7_ADDR)
#define lvx_reminder_set_kind_byte     ((lvx_reminder_set_kind_byte_t)   LVX_REMINDER_SET_KIND_BYTE_ADDR)
#define lvx_reminder_set_life_times    ((lvx_reminder_set_life_times_t)  LVX_REMINDER_SET_LIFE_TIMES_ADDR)
#define lvx_reminder_set_on_create     ((lvx_reminder_set_on_create_t)   LVX_REMINDER_SET_ON_CREATE_ADDR)
#define lvx_reminder_set_on_destroy    ((lvx_reminder_set_on_destroy_t)  LVX_REMINDER_SET_ON_DESTROY_ADDR)
#define lvx_reminder_disable_backspace ((lvx_reminder_disable_backspace_t)LVX_REMINDER_DISABLE_BACKSPACE_ADDR)
#define lvx_reminder_set_bell          ((lvx_reminder_set_bell_t)        LVX_REMINDER_SET_BELL_ADDR)
#define lvx_reminder_start             ((lvx_reminder_start_t)           LVX_REMINDER_START_ADDR)
#define lvx_reminder_cancel            ((lvx_reminder_cancel_t)          LVX_REMINDER_CANCEL_ADDR)

/* Notifications */
#define lvx_notification_insert_message  ((notification_insert_t)LVX_NOTIFICATION_INSERT_MESSAGE_ADDR)

/* Built-in OTA reminder */
#define reminder_page_ota_start       ((reminder_page_ota_start_t)REMINDER_PAGE_OTA_START_ADDR)

/* LVGL timer (typedefs in common/lvgl/timer.h) */
#define lv_timer_create               ((lv_timer_create_t)LV_TIMER_CREATE_ADDR)
#define lv_timer_delete               ((lv_timer_delete_t)LV_TIMER_DELETE_ADDR)

/* LVGL control widgets (typedefs in common/lvgl/control.h) */
#define lv_obj_create                 ((lv_obj_create_t)  LV_OBJ_CREATE_ADDR)
#define lv_image_create               ((lv_obj_create_t)  LV_IMAGE_CREATE_ADDR)
#define lv_label_create               ((lv_class_create_t)LV_LABEL_CREATE_ADDR)

#define lv_analog_time_create         ((lv_class_create_t)LV_ANALOG_TIME_CREATE_ADDR)
#define lv_btn_create                 ((lv_obj_create_t)  LV_BTN_CREATE_ADDR)
#define lvx_btn_create                ((lv_class_create_t)LVX_BTN_CREATE_ADDR)
#define lv_canvas_create              ((lv_class_create_t)LV_CANVAS_CREATE_ADDR)
#define lv_crossview_create           ((lv_class_create_t)LV_CROSSVIEW_CREATE_ADDR)
#define lv_sleepchart_create          ((lv_class_create_t)LV_SLEEPCHART_CREATE_ADDR)
#define lv_curvedlabel_create         ((lv_class_create_t)LV_CURVEDLABEL_CREATE_ADDR)
#define lv_frameanim_create           ((lv_class_create_t)LV_FRAMEANIM_CREATE_ADDR)
#define lv_imglabel_create            ((lv_class_create_t)LV_IMGLABEL_CREATE_ADDR)
#define lv_indicator_create           ((lv_class_create_t)LV_INDICATOR_CREATE_ADDR)
#define lv_optionlist_create          ((lv_class_create_t)LV_OPTIONLIST_CREATE_ADDR)
#define lv_slider_create              ((lv_class_create_t)LV_SLIDER_CREATE_ADDR)
#define lv_xview_create               ((lv_class_create_t)LV_XVIEW_CREATE_ADDR)
#define lv_timeview_create            ((lv_class_create_t)LV_TIMEVIEW_CREATE_ADDR)
#define lv_xwin_create                ((lv_class_create_t)LV_XWIN_CREATE_ADDR)
#define lv_zoombtn_create             ((lv_class_create_t)LV_ZOOMBTN_CREATE_ADDR)
#define lv_eventdispatch_create       ((lv_class_create_t)LV_EVENTDISPATCH_CREATE_ADDR)
#define lv_renderdata_create          ((lv_class_create_t)LV_RENDERDATA_CREATE_ADDR)
#define lv_render_file_create         ((lv_class_create_t)LV_RENDER_FILE_CREATE_ADDR)
#define lv_imgarc_create              ((lv_class_create_t)LV_IMGARC_CREATE_ADDR)
#define lv_slot_create                ((lv_class_create_t)LV_SLOT_CREATE_ADDR)
#define lv_text_create                ((lv_class_create_t)LV_TEXT_CREATE_ADDR)
#define lv_wfwidget_create            ((lv_class_create_t)LV_WFWIDGET_CREATE_ADDR)
#define lv_widgetlayout_create        ((lv_class_create_t)LV_WIDGETLAYOUT_CREATE_ADDR)

/* LVGL generic widget methods (typedefs in common/lvgl/control.h) */
#define lv_obj_set_size               ((lv_obj_set_size_t)  LV_OBJ_SET_SIZE_ADDR)
#define lv_image_set_src              ((lv_image_set_src_t) LV_IMAGE_SET_SRC_ADDR)
#define lv_obj_add_event_cb           ((lv_obj_add_event_cb_t)LV_OBJ_ADD_EVENT_CB_ADDR)
#define lv_label_set_text             ((lv_label_set_text_t)LV_LABEL_SET_TEXT_ADDR)
#define lvx_btn_set_text_fmt               ((lvx_btn_set_text_fmt_t)  LVX_BTN_SET_TEXT_FMT_ADDR)
#define lv_obj_add_flag               ((lv_obj_add_flag_t)  LV_OBJ_ADD_FLAG_ADDR)
#define lv_obj_clear_flag             ((lv_obj_clear_flag_t)LV_OBJ_CLEAR_FLAG_ADDR)
#define lv_obj_align                  ((lv_obj_align_t)     LV_OBJ_ALIGN_ADDR)
#define lv_obj_align_to               ((lv_obj_align_to_t)  LV_OBJ_ALIGN_TO_ADDR)
#define lv_obj_set_width              ((lv_obj_set_width_t) LV_OBJ_SET_WIDTH_ADDR)
#define lv_obj_set_style_text_align   ((lv_obj_set_style_text_align_t)LV_OBJ_SET_STYLE_TEXT_ALIGN_ADDR)
#define lv_label_set_long_mode        ((lv_label_set_long_mode_t)LV_LABEL_SET_LONG_MODE_ADDR)

/* stdio (prototype in common/misc/print.h) */
#define snprintf                    ((int (*)(char *, size_t, const char *, ...))SNPRINTF_ADDR)

/* Style setters */
#define lv_obj_set_style_bg_color     ((lv_obj_set_style_bg_color_t)LV_OBJ_SET_STYLE_BG_COLOR_ADDR)
#define lvx_btn_set_style_bg_color    ((lvx_btn_set_style_bg_color_t)LVX_BTN_SET_STYLE_BG_COLOR_ADDR)
#define lv_obj_set_style_bg_opa       ((lv_obj_set_style_bg_opa_t)  LV_OBJ_SET_STYLE_BG_OPA_ADDR)
#define lv_obj_set_style_pad_left     ((lv_obj_set_style_pad_t)    LV_OBJ_SET_STYLE_PAD_LEFT_ADDR)
#define lv_obj_set_style_pad_right    ((lv_obj_set_style_pad_t)    LV_OBJ_SET_STYLE_PAD_RIGHT_ADDR)
#define lv_obj_set_style_pad_top      ((lv_obj_set_style_pad_t)    LV_OBJ_SET_STYLE_PAD_TOP_ADDR)
#define lv_obj_set_style_pad_bottom   ((lv_obj_set_style_pad_t)    LV_OBJ_SET_STYLE_PAD_BOTTOM_ADDR)
#define lv_obj_set_style_pad_row      ((lv_obj_set_style_pad_t)    LV_OBJ_SET_STYLE_PAD_ROW_ADDR)
#define lv_obj_set_style_pad_column   ((lv_obj_set_style_pad_t)    LV_OBJ_SET_STYLE_PAD_COLUMN_ADDR)
#define lvx_obj_set_style_text        ((lvx_obj_set_style_text_t)  LVX_OBJ_SET_STYLE_TEXT_ADDR)

/* Event dispatcher + animation veneer family */
#define lv_obj_event                  ((lv_obj_event_t)           LV_OBJ_EVENT_ADDR)
#define lv_obj_get_event_code         ((lv_obj_get_event_code_t)  LV_OBJ_GET_EVENT_CODE_ADDR)
#define lv_obj_get_target             ((lv_obj_get_target_t)      LV_OBJ_GET_TARGET_ADDR)
#define lv_obj_get_parent             ((lv_obj_get_target_t)      LV_OBJ_GET_PARENT_ADDR)
#define lv_anim_del                   ((lv_anim_del_t)            LV_ANIM_DEL_ADDR)
#define lvx_label_set_time_text       ((lvx_label_set_time_text_t)LVX_LABEL_SET_TIME_TEXT_ADDR)
#define lv_anim_set                   ((lv_anim_set_t)            LV_ANIM_SET_ADDR)
#define lv_anim_start                 ((lv_anim_start_t)          LV_ANIM_START_ADDR)

/* lv_event_get_* family */
#define lv_event_get_user_data        ((lv_event_get_user_data_t) LV_EVENT_GET_USER_DATA_ADDR)
#define lv_event_get_param            ((lv_event_get_param_t)     LV_EVENT_GET_PARAM_ADDR)

#define LB_TEXT_STYLE_36              ((uintptr_t)LB_TEXT_STYLE_1_ADDR)
#define LB_TEXT_STYLE_40              ((uintptr_t)LB_TEXT_STYLE_2_ADDR)
#define LB_TEXT_STYLE_44              ((uintptr_t)LB_TEXT_STYLE_3_ADDR)

/* Screen power management (typedefs in common/nuttx/screen.h) */
#define screen_turn_on                       ((int (*)(const char *, int))                 SCREEN_TURN_ON_ADDR)
#define screen_turn_off                      ((const char *(*)(const char *))             SCREEN_TURN_OFF_ADDR)
#define screen_set_keepon                    ((const char *(*)(const char *, int))         SCREEN_SET_KEEPON_ADDR)
#define screen_toggle_keepon_duration        ((int (*)(const char *, int))                 SCREEN_TOGGLE_KEEPON_DURATION_ADDR)

#define screen_session_start                 ((int (*)(screen_session_t *, int))          SCREEN_SESSION_START_ADDR)
#define screen_session_stop                  ((int (*)(screen_session_t *))               SCREEN_SESSION_STOP_ADDR)
#define screen_session_restore_default       ((int (*)(screen_session_t *))               SCREEN_SESSION_RESTORE_DEFAULT_ADDR)
#define screen_session_set_full_power        ((int (*)(screen_session_t *, int))          SCREEN_SESSION_SET_FULL_POWER_ADDR)
#define screen_session_set_brightness_value  ((int (*)(screen_session_t *, int))          SCREEN_SESSION_SET_BRIGHTNESS_ADDR)
#define screen_session_set_auto_brightness   ((int (*)(screen_session_t *, int))          SCREEN_SESSION_SET_AUTO_BRIGHT_ADDR)
#define screen_session_set_keepon            ((int (*)(screen_session_t *, int))          SCREEN_SESSION_SET_KEEPON_ADDR)
#define screen_session_set_touch_palm        ((int (*)(screen_session_t *, int))          SCREEN_SESSION_SET_TOUCH_PALM_ADDR)
#define screen_session_set_wrist_drop        ((int (*)(screen_session_t *, int))          SCREEN_SESSION_SET_WRIST_DROP_ADDR)

/* miwear launcher / packagemanager dispatch (typedefs in common/miwear/app.h) */
#define g_packagemanager_api                ((struct packagemanager_api_s *)PACKAGEMANAGER_API_ADDR)
#define app_launcher_add                    ((app_launcher_add_t)APP_LAUNCHER_ADD_ADDR)
#define app_lookup                          ((app_lookup_t)APP_LOOKUP_ADDR)
#define launcher_data_load_app_info       ((launcher_data_load_app_info_t)LAUNCHER_DATA_LOAD_APP_INFO_ADDR)
#define launcher_page_main_update_layout    ((launcher_page_main_update_layout_t)LAUNCHER_PAGE_MAIN_UPDATE_LAYOUT_ADDR)
#define lv_ll_clear                         ((lv_ll_clear_t)LV_LL_CLEAR_ADDR)
/* `g_appinfo_list` is bound as a `lv_ll_t *` pointing at the firmware's static
 * list head at G_APPINFO_LIST_ADDR. Conventional LVGL idiom: pass it directly
 * to `lv_ll_clear`, not as `&`. The pointer cast routes through uintptr_t so
 * there's no direct integer-to-pointer cast in the macro and no UB concerns. */
#define g_appinfo_list                      ((lv_ll_t *)(uintptr_t)(G_APPINFO_LIST_ADDR))

/* Watchface manager (typedefs in common/miwear/watchface.h) */
#define g_watchface_config                      (*(watchface_config_t **)WATCHFACE_CONFIG_ADDR)
#define watchface_manager_delete_watchface      ((watchface_manager_delete_watchface_t)WATCHFACE_MANAGER_DELETE_WATCHFACE_ADDR)
#define watchface_manager_reset_watchface       ((watchface_manager_reset_watchface_t)WATCHFACE_MANAGER_RESET_WATCHFACE_ADDR)

#endif /* WATCHS3_MIWEAR_SYSTEM_FW_2_6_0_H */
