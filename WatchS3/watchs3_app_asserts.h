#ifndef WATCHS3_APP_ASSERTS_H
#define WATCHS3_APP_ASSERTS_H

/* Per-TU address asserts for watchs3_app.c.
 *
 * Pulls in the platform header so the typed pointer bindings are visible,
 * then declares _Static_assert blocks ONLY for the symbols this translation
 * unit actually references. Filling in the real address satisfies the
 * assert; the assert only fires for symbols consumed here.
 *
 * The mask `(ADDR & ~1u)` clears the Thumb bit so the default template
 * value `(0x00000000u | 1) == 1u` correctly fails the assert, while
 * any real Thumb function or plain data address passes.
 */

#include "platform/miwear_system.h"

/* logger / notifications / timers */
_Static_assert((SYSLOG_ADDR & ~1u) != 0u,
               "syslog address is NULL");
_Static_assert((LVX_NOTIFICATION_INSERT_MESSAGE_ADDR & ~1u) != 0u,
               "lvx_notification_insert_message address is NULL");
_Static_assert((LV_TIMER_CREATE_ADDR & ~1u) != 0u,
               "lv_timer_create address is NULL");
_Static_assert((LV_TIMER_DELETE_ADDR & ~1u) != 0u,
               "lv_timer_delete address is NULL");

/* miwear launcher / packagemanager dispatch
 * (note: APP_LAUNCHER_ADD is NOT referenced here -- watchs3_app.c uses
 *  launcher_data_load_app_info + launcher_page_main_update_layout to
 *  update the grid, since "Watch S3 has no quick app engine") */
_Static_assert((PACKAGEMANAGER_API_ADDR & ~1u) != 0u,
               "packagemanager API address is NULL");
_Static_assert((APP_LOOKUP_ADDR & ~1u) != 0u,
               "app_lookup address is NULL");

/* launcher-side helpers (used directly by watchs3_app.c, not via the
 * packagemanager_api_s dispatch table) */
_Static_assert((LAUNCHER_DATA_LOAD_APP_INFO_ADDR & ~1u) != 0u,
               "launcher_data_load_app_info address is NULL");
_Static_assert((LV_LL_CLEAR_ADDR & ~1u) != 0u,
               "lv_ll_clear address is NULL");
_Static_assert((G_APPINFO_LIST_ADDR & ~1u) != 0u,
               "g_appinfo_list address is NULL");
_Static_assert((LAUNCHER_PAGE_MAIN_UPDATE_LAYOUT_ADDR & ~1u) != 0u,
               "launcher_page_main_update_layout address is NULL");

/* watchface manager (current_face pointer read; watchface_manager_* entries) */
_Static_assert((WATCHFACE_CONFIG_ADDR & ~1u) != 0u,
               "WATCHFACE_CONFIG data pointer is NULL");
_Static_assert((WATCHFACE_MANAGER_DELETE_WATCHFACE_ADDR & ~1u) != 0u,
               "watchface_manager_delete_watchface address is NULL");
_Static_assert((WATCHFACE_MANAGER_RESET_WATCHFACE_ADDR & ~1u) != 0u,
               "watchface_manager_reset_watchface address is NULL");

#endif /* WATCHS3_APP_ASSERTS_H */
