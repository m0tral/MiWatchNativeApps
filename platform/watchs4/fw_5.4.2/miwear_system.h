/* S4 Global 5.4.2 only. Integration notes: WatchS4/README.md.
 * Firmware SHA256: 44eaac5831cbae612ead1e371e26a5c142965605a8065605ec0db701a50b3c82
 * All function-pointer constants include the ARM Thumb bit.
 */
#ifndef S4_NATIVE_PLATFORM_H
#define S4_NATIVE_PLATFORM_H
#include <stdint.h>
#include <stddef.h>
#include "../notification.h"

typedef struct lv_obj lv_obj_t;
typedef struct lv_event lv_event_t;
typedef struct lv_timer lv_timer_t;
typedef void (*timer_cb_t)(lv_timer_t *);
typedef void (*event_cb_t)(lv_event_t *);

#define FW_FN(address, result, ...) ((result (*)(__VA_ARGS__))(uintptr_t)((address) | 1u))
#define fw_log FW_FN(0x2c2159d8u, void, int, const char *, ...)
#define lv_timer_create FW_FN(0x2c53abe0u, lv_timer_t *, timer_cb_t, uint32_t, void *)
#define lv_timer_delete FW_FN(0x2c53ac54u, void, lv_timer_t *)
#define lv_obj_create FW_FN(0x2c494084u, lv_obj_t *, lv_obj_t *, void *)
#define lv_label_create FW_FN(0x2c58d4a0u, lv_obj_t *, lv_obj_t *)
#define lv_obj_set_size FW_FN(0x2c495624u, void, lv_obj_t *, int32_t, int32_t)
#define lv_obj_align FW_FN(0x2c49585cu, void, lv_obj_t *, uint32_t, int32_t, int32_t)
#define lv_obj_add_event_cb FW_FN(0x2c49519cu, void *, lv_obj_t *, event_cb_t, uint32_t, void *)
#define lv_event_get_code FW_FN(0x2c53672cu, uint32_t, lv_event_t *)
#define lv_event_get_user_data FW_FN(0x2c536750u, void *, lv_event_t *)
#define lv_label_set_text FW_FN(0x2c58e23cu, void, lv_obj_t *, const char *)
#define lv_label_set_long_mode FW_FN(0x2c58e360u, void, lv_obj_t *, uint32_t)
#define lvx_btn_create FW_FN(0x2c7fd3c4u, lv_obj_t *, lv_obj_t *)
#define lvx_btn_set_style_bg_color FW_FN(0x2c7fd460u, void, lv_obj_t *, uint32_t)
#define lvx_btn_set_text_fmt FW_FN(0x2c7fd5f0u, void, lv_obj_t *, const char *, ...)
#define lv_obj_add_style FW_FN(0x2c49ac14u, void, lv_obj_t *, const void *, uint32_t)
#define S4_FONT40_STYLE ((const void *)(uintptr_t)0x3c2cab88u)
#define lv_obj_set_style_pad_top FW_FN(0x2c49bb7cu, void, lv_obj_t *, int32_t, uint32_t)
#define lv_obj_set_style_pad_bottom FW_FN(0x2c49bb94u, void, lv_obj_t *, int32_t, uint32_t)
#define lv_obj_set_style_pad_left FW_FN(0x2c49bbacu, void, lv_obj_t *, int32_t, uint32_t)
#define lv_obj_set_style_pad_right FW_FN(0x2c49bbc4u, void, lv_obj_t *, int32_t, uint32_t)
#define lv_obj_set_style_bg_opa FW_FN(0x2c49bc98u, void, lv_obj_t *, uint32_t, uint32_t)
#define lv_obj_set_style_bg_color FW_FN(0x2c49bc6cu, void, lv_obj_t *, uint32_t, uint32_t)
#define lv_obj_set_style_text_color FW_FN(0x2c49c098u, void, lv_obj_t *, uint32_t, uint32_t)
#define lv_obj_set_style_text_font FW_FN(0x2c49c0dcu, void, lv_obj_t *, const void *, uint32_t)
#define lv_obj_set_style_text_align FW_FN(0x2c49c124u, void, lv_obj_t *, uint32_t, uint32_t)

enum { S4_CLICKED = 7, S4_TOP_LEFT = 1, S4_LONG_CLIP = 4 };

/* Only this prefix is accessed: insmod passes module+0x68 to ET_REL entry;
 * rmmod invokes (*uninitialize)(arg), honoring a negative return. */
typedef struct {
    int (*uninitialize)(void *);
    void *arg;
} s4_module_info_t;

typedef struct s4_app {
    void *runtime_prev, *runtime_next;
    const char *name, *icon;
    uint16_t app_id, flags;
    const char *optional_string_14, *optional_string_18;
    const char *(*get_name)(struct s4_app *);
    void *optional_20, *optional_24, *optional_28, *optional_2c;
    void *runtime_pages, *runtime_widgets, *on_signal;
    uint32_t runtime_flags;
} s4_app_t;

typedef int (*page_callback_t)(void *, lv_obj_t *, void *);
typedef struct {
    void *parent_descriptor;
    uint32_t reserved_04[3];
    const char *name;
    uint16_t page_id, app_id;
    uint16_t flags, reserved_1a;
    int32_t scheduler_deadline, scheduler_priority;
    uint32_t async_destroy;
    uint8_t lifecycle, cache_policy, kind, reserved_2b;
    void *context;
    lv_obj_t *root;
    void *on_signal, *runtime_default, *reserved_3c;
    void *registry_prev, *registry_next, *runtime_parent;
    page_callback_t on_create, on_resume, on_newdata, on_start;
    page_callback_t on_pause, on_stop, on_destroy, on_back;
    page_callback_t on_keyevent, get_scroll_obj;
} s4_page_t;

_Static_assert(sizeof(void *) == 4, "S4 requires 32-bit pointers");
_Static_assert(sizeof(s4_app_t) == 0x40, "S4 app ABI");
_Static_assert(offsetof(s4_app_t, get_name) == 0x1c, "S4 app name callback ABI");
_Static_assert(sizeof(s4_page_t) == 0x74, "S4 page ABI");
_Static_assert(offsetof(s4_page_t, root) == 0x30, "S4 page root ABI");
_Static_assert(offsetof(s4_page_t, on_create) == 0x4c, "S4 page create ABI");
_Static_assert(offsetof(s4_page_t, on_destroy) == 0x64, "S4 page destroy ABI");

typedef void (*install_fn_t)(s4_app_t *, s4_page_t **, int);
#define fw_install (*(install_fn_t volatile *)(uintptr_t)0x3c219eb0u)
#define fw_app_lookup FW_FN(0x2c818d34u, s4_app_t *, uint16_t)
#define fw_launcher_add FW_FN(0x2c52b90cu, void, uint16_t)

/* S4-only installer integration, traced from stock delete/notification flows. */
#define fw_watchface_delete FW_FN(0x2c8429b4u, int, const char *)
#define fw_watchface_reset FW_FN(0x2c6240b8u, void, int, int)
#define fw_watchface_lookup FW_FN(0x2c8428ccu, void *, const char *)
#define fw_notification_insert FW_FN(0x2c818100u, int, const s4_notification_t *)
#define fw_notification_present FW_FN(0x2c55a4ecu, void, uint64_t, int)

static inline const char *fw_current_face_id(void) {
    const uint8_t *config = *(const uint8_t *volatile *)(uintptr_t)0x3c2f9d80u;
    if (!config) return 0;
    const uint8_t *entry = *(const uint8_t *const *)(config + 0x44);
    return entry ? (const char *)(entry + 8) : 0;
}

_Static_assert(sizeof(s4_notification_t) == 0x58, "S4 notification ABI");
_Static_assert(offsetof(s4_notification_t, title) == 0x0c, "S4 notification title ABI");
_Static_assert(offsetof(s4_notification_t, start_reminder) == 0x50, "S4 reminder ABI");
_Static_assert(offsetof(s4_notification_t, callback_data) == 0x54, "S4 callback data ABI");

static inline int s4_installer_firmware_matches(void) {
    return *(const volatile uint32_t *)0x2c8429b4u == 0xaf00b580u &&
           *(const volatile uint32_t *)0x2c6240b8u == 0xaf00b5b0u &&
           *(const volatile uint32_t *)0x2c818100u == 0x41f0e92du &&
           *(const volatile uint32_t *)0x2c55a4ecu == 0x41f0e92du;
}

static inline int s4_equal(const char *a, const char *b) {
    if (!a || !b) return 0;
    while (*a && *a == *b) { ++a; ++b; }
    return *a == *b;
}

/* A few exact-image anchors, not an on-watch hash of the entire firmware. */
static inline int s4_firmware_matches(void) {
    const volatile uint32_t *log_code = (const volatile uint32_t *)0x2c2159d8u;
    const volatile uint32_t *install_slot = (const volatile uint32_t *)0x3c219eb0u;
    const char *prop = (const char *)0x2ca26528u;
    const char expected[] = "ro.build.version=5.4.2\n";
    for (unsigned i = 0; i < sizeof(expected)-1; ++i)
        if (prop[i] != expected[i]) return 0;
    return *log_code == 0xb580b40eu && *install_slot == 0x2c819701u &&
           s4_equal((const char *)0x2c8f150cu, "Xiaomi Watch S4");
}
#endif
