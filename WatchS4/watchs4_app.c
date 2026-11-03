/* Xiaomi Watch S4 Global 5.4.2 native calculator; module remains resident.
 * Framework layout and entry ABI are grounded in the supplied firmware.
 */
#include "platform/miwear_system.h"
#include "ui/calc_page.c"
#include "installer/installer_finish.h"

#define APP_ID 0xc1u
#define PACKAGE "com.m0tral.calculator.s4"
static lv_timer_t *pending_timer;
static s4_app_t app;
static s4_page_t page;
static s4_page_t *pages[1];

static const char *app_name(s4_app_t *unused) {
    (void)unused;
    return "Calculator";
}

static int page_create(void *unused, lv_obj_t *root, void *data) {
    (void)unused; (void)data;
    const int rc = root ? ui_create(root) : -22;
    fw_log(4, "[s4_calculator] page create rc=%d\n", rc);
    return rc;
}

static int page_destroy(void *unused, lv_obj_t *root, void *data) {
    (void)unused; (void)root; (void)data;
    ui_reset_pointers();
    return 0;
}

static int refuse_unload(void *unused) {
    (void)unused;
    fw_log(4, "[s4_calculator] resident until reboot; unload refused\n");
    return -16;
}

static void register_calculator(lv_timer_t *timer) {
    lv_timer_delete(timer);
    pending_timer = 0;
    s4_app_t *existing = fw_app_lookup(APP_ID);
    if (existing) {
        fw_log(3, "[s4_calculator] ID 0xc1 already in use; registration skipped\n");
        return;
    }
    app.name = PACKAGE;
    app.icon = "/data/s4_calc_logo_112.png";
    app.app_id = APP_ID;
    app.flags = 4; /* Built-in uninstaller disabled for this resident demo. */
    app.get_name = app_name;
    page.name = "main";
    page.app_id = APP_ID;
    page.page_id = 0;
    page.cache_policy = 1;
    page.on_create = page_create;
    page.on_destroy = page_destroy;
    pages[0] = &page;
    fw_install(&app, pages, 1);
    s4_app_t *installed = fw_app_lookup(APP_ID);
    if (!installed || !s4_equal(installed->name, PACKAGE)) {
        fw_log(3, "[s4_calculator] package registration failed\n");
        return;
    }
    fw_launcher_add(APP_ID);
    fw_log(4, "[s4_calculator] registered; open Calculator from launcher\n");
    finish_install();
}

int module_initialize(s4_module_info_t *info) {
    if (!info || !s4_firmware_matches() || !s4_installer_firmware_matches()) return -22;
    if (fw_app_lookup(APP_ID)) return -17;
    info->uninitialize = refuse_unload;
    info->arg = 0;
    pending_timer = lv_timer_create(register_calculator, 500, 0);
    if (!pending_timer) return -12;
    fw_log(4, "[s4_calculator] loaded; registration scheduled\n");
    return 0;
}
