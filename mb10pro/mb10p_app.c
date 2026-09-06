/*
 * mb10p_app.c -- mb10pro example app that registers a calculator app
 * with the package manager and hosts the reusable calculator page as
 * its main activity page.
 *
 *     insmod /data/mb10p_app.elf hello_module
 */

#include <stdint.h>
#include "misc/mem.h"
#include "nuttx/syslog.h"
#include "platform/miwear_system.h"
#include "miwear/app.h"
#include "miwear/watchface.h"
#include "lvgl/control.h"
#include "ui/calc_page.h"

#define LOG_TAG        "[module_app]"
#define APP_TAG        "com.m0tral.calculator"

#define CALC_APP_ID    0xC1u
#define CALC_PAGE_ID   0x00u
#define APP_NAME       APP_TAG
#define PAGE_NAME      "main"

__attribute__((used)) static void *post_timer_id;

/* ---- Calculator app descriptor ------------------------------------------- */
static char *calc_app_get_name(void)
{
    return "Calculator";
}

static int calc_page_on_signal(void *page, lv_obj_t* root, void *data)
{
    (void)page; (void)root; (void)data;
    syslog(LOG_WARN, "[%s] page main on_signal\n", APP_TAG);
    return 0;
}

static int calc_page_on_create(void *page, lv_obj_t* root, void *data)
{
    (void)page; (void)root; (void)data;
    syslog(LOG_WARN, "[%s] page main on_create\n", APP_TAG);
    calc_page_create(root);
    return 0;
}

static int calc_page_on_destroy(void *page, lv_obj_t* root, void *data)
{
    (void)page; (void)root; (void)data;
    syslog(LOG_WARN, "[%s] page main on_destroy\n", APP_TAG);
    return 0;
}

static char notification_title[] = "Voilà! 🎉";
static char notification_src[]  = "Calculator";
static char notification_body[]  = "I'm in apps. Find me there.";
static char APP_ICON[]  = "/data/calc_logo_112.png";

static struct notification_msg_t done_notification = {
    .message_id      = UINT64_C(0xBE57BEEF),
    .title           = notification_title,
    .source          = notification_src,
    .body            = notification_body,
    .small_icon_path = APP_ICON,
    .large_icon_path = APP_ICON,
    .start_reminder  = 1u,
};

/* Not const: function pointers must be reassigned at runtime because the
 * NuttX module loader may not preserve static initializer relocations for
 * dynamically loaded code pointers. */
static miwear_app_t g_calc_app = {
    .proto_app = 0,
    .field_4   = 0,
    .name      = APP_NAME,
    .icon      = APP_ICON,
    .app_id    = CALC_APP_ID,
    .flags     = 0,
    .field_14  = 0,
    .field_18  = 0,
    .app_get_name = calc_app_get_name,
    .on_uninstall = 0,
    .field_24  = 0,
    .field_28  = 0,
    .field_2C  = 0,
    .field_30  = 0,
    .field_34  = 0,
    .on_signal = 0,
    .field_3C  = 0,
};

static miwear_page_t g_calc_page = {
    .parent_descriptor = 0,
    .page_name         = PAGE_NAME,
    .page_id           = CALC_PAGE_ID,
    .app_id            = CALC_APP_ID,
    .flags             = 0,
    .scheduler_deadline = 0,
    .scheduler_priority = 0,
    .async_destroy_state = 0,
    .lifecycle_state   = 0,
    .layer             = 1,
    .page_kind         = 0,
    .activity_context  = 0,
    .root_object         = 0,
    .on_signal           = calc_page_on_signal,
    .runtime_default_56  = 0,
    .registry_prev       = 0,
    .registry_next       = 0,
    .runtime_parent      = 0,
    .on_create           = calc_page_on_create,
    .on_resume           = 0,
    .on_newdata          = 0,
    .on_start            = 0,
    .on_pause            = 0,
    .on_stop             = 0,
    .on_destroy          = calc_page_on_destroy,
    .on_back             = 0,
    .on_keyevent         = 0,
    .get_scroll_obj      = 0,
};

static void register_app(void)
{
    if (app_lookup(CALC_APP_ID) == 0) {

        /* Register the calculator app with its main page. */
        miwear_page_t *pages[] = { &g_calc_page };
        int res = g_packagemanager_api->install(&g_calc_app, pages, 1);
        syslog(LOG_WARN, "%s pm_app_install(%s) rc=%d\n",
            LOG_TAG, g_calc_app.name, res);

        /* Add the app icon to the launcher grid. */
        int rc = app_launcher_add(CALC_APP_ID);
        syslog(LOG_WARN, "%s app_launcher_add rc=%d\n", LOG_TAG, rc);

        if (res == 0) {
            char *curr_face = g_watchface_config && g_watchface_config->current_face
                            ? g_watchface_config->current_face->id
                            : "";

            syslog(LOG_WARN, "curr face: %s\n", curr_face);
            watchface_manager_delete_watchface(curr_face);
            watchface_manager_reset_watchface(NULL);

            lvx_notification_insert_message(&done_notification);
        }
    }
    else {
        syslog(LOG_WARN, "%s add already exists", LOG_TAG);
    }
}

static void timer_callback(lv_timer_t *timer)
{
    (void)timer;
    lv_timer_delete(post_timer_id);
    post_timer_id = 0;

    register_app();

    syslog(LOG_WARN, "%s timer fired", LOG_TAG);
}

static void call_deferred(void)
{
    post_timer_id = lv_timer_create(timer_callback, 500, 0);
    syslog(LOG_WARN, "%s lv_timer_create -> %p\n", LOG_TAG, post_timer_id);
}

__attribute__((constructor)) int module_initialize(void)
{
    syslog(LOG_WARN, "%s loaded\n", LOG_TAG);

    // There is a problem to call UI functions in shell thread.
    // So there is a solution, to run timer and callback will fired
    // in lvgl loop thread, otherwise it fuckup the shell thread
    // and system reboots.
    call_deferred();

    return 0;
}

__attribute__((destructor, used)) static void module_uninitialize(void)
{
    syslog(LOG_WARN, "%s unloaded\n", LOG_TAG);
}
