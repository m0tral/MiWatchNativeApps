#ifndef XIAOMI_APP_H
#define XIAOMI_APP_H

/* mb10pro packagemanager app API.
 *
 * The firmware publishes a dispatch table (struct packagemanager_api_s)
 * of Thumb function pointers at SRAM address 0x200EB660. Layout below is
 * reverse-engineered from vela_ap.bin fw_3.101.043 after the functions in
 * the table were renamed in IDA.
 *
 * Entry points (code addresses) live in the 0x2Cxxxxxx region; the SRAM
 * table stores them with the high nibble 0x0C instead of 0x2C (a quirk
 * of this firmware image). The Thumb bit (bit 0) is set on every code
 * pointer.
 *
 * Access is through the macro in platform/mb10p_platform.h:
 *   #define packagemanager_api ((struct packagemanager_api_s *)0x200EB660)
 */

#include <stdint.h>

/* ---- Forward declarations --------------------------------------------- */
struct miwear_app_t;
struct miwear_page_t;
typedef struct miwear_app_t  miwear_app_t;
typedef struct miwear_page_t miwear_page_t;

/* ---- Public API prototypes --------------------------------------------- */

/* Install the calling module as a packagemanager app.
 * `app`    : pointer to app descriptor (struct miwear_app_t)
 * `data`   : initial data / widget list
 * `count`  : number of items in `data`
 * Returns 0 on success.
 */
int pm_app_install(miwear_app_t *app, miwear_page_t *pages[], int count);

/* Recover a previously uninstalled app. */
int pm_app_recover(miwear_app_t *app);

/* Uninstall an installed app. */
int pm_app_uninstall(miwear_app_t *app);

/* Fully unregister an app and remove its widgets. */
int pm_app_unregister(miwear_app_t *app);

/* Notify the package manager that an app has been updated. */
int pm_app_update_notify(miwear_app_t *app);

/* Add widgets to an installed app.
 * Returns pointer to the widget list, or NULL on failure.
 */
void *pm_widgets_add(miwear_app_t *app, const void *widgets, int count);

/* Empty no-op slot in the dispatch table. */
void pm_app_noop(miwear_app_t *app);

/* Set or clear the hidden flag of an installed app. */
int pm_app_set_hidden(miwear_app_t *app, int hidden);

/* Query whether the current app is hidden. */
int pm_app_is_hidden(miwear_app_t *app);

/* Add the current app's icon to the launcher grid. */
typedef int (*app_launcher_add_t)(uint16_t app_id);

// Search app in registry
typedef int (*app_lookup_t)(uint16_t app_id);

/* ---- Dispatch table ------------------------------------------------------ */
struct packagemanager_api_s
{
    /* +0x00 */ int (*install)(miwear_app_t *app, miwear_page_t *pages[], int count);      /* pm_app_install */
    /* +0x04 */ int (*recover)(miwear_app_t *app);                                /* pm_app_recover */
    /* +0x08 */ int (*uninstall)(miwear_app_t *app);                              /* pm_app_uninstall */
    /* +0x0C */ int (*unregister)(miwear_app_t *app);                             /* pm_app_unregister */
    /* +0x10 */ int (*update_notify)(miwear_app_t *app);                          /* pm_app_update_notify */
    /* +0x14 */ void *(*widgets_add)(miwear_app_t *app, void *widgets, int count); /* pm_widgets_add */
    /* +0x18 */ void (*widgets_remove)(miwear_app_t *app);                        /* pm_app_noop */
    /* +0x1C */ int (*set_hidden)(miwear_app_t *app, int hidden);                 /* pm_app_set_hidden */
    /* +0x20 */ int (*is_hidden)(miwear_app_t *app);                              /* pm_app_is_hidden */
};

enum miwear_app_id
{
  XIAOMI_APP_LAUNCHER = 0x1,
  XIAOMI_APP_WATCHFACE = 0x2,
  XIAOMI_APP_PRESSURE = 0x3,
  XIAOMI_APP_BREATH = 0x4,
  XIAOMI_APP_HEARTRATE = 0x5,
  XIAOMI_APP_SLEEP = 0xC,
  XIAOMI_APP_OXYGEN = 0xE,
  XIAOMI_APP_SPORTS = 0xF,
  XIAOMI_APP_HOME = 0x10,
  XIAOMI_APP_SETUPWIZARD = 0x11,
  XIAOMI_APP_ACTIVITIES = 0x13,
  XIAOMI_APP_NOTIFICATIONS = 0x14,
  XIAOMI_APP_MEDIA = 0x15,
  XIAOMI_APP_SETTINGS = 0x16,
  XIAOMI_APP_COMPASS = 0x18,
  XIAOMI_APP_FLASHLIGHT = 0x1A,
  XIAOMI_APP_CALENDAR = 0x1B,
  XIAOMI_APP_REMOTE_CAMERA = 0x1C,
  XIAOMI_APP_RECORD = 0x1D,
  XIAOMI_APP_ALIPAY = 0x1E,
  XIAOMI_APP_WOMENHEALTH = 0x1F,
  XIAOMI_APP_CHRONOGRAPH = 0x20,
  XIAOMI_APP_WEATHER = 0x21,
  XIAOMI_APP_SYSTEM = 0x22,
  XIAOMI_APP_PHONE = 0x23,
  XIAOMI_APP_TIMER = 0x26,
  XIAOMI_APP_FIND_PHONE = 0x27,
  XIAOMI_APP_ALARM = 0x28,
  XIAOMI_APP_RECORDER = 0x29,
  XIAOMI_APP_NFCCARD = 0x2B,
  XIAOMI_APP_VOICE = 0x2D,
  XIAOMI_APP_CONTACT = 0x2E,
  XIAOMI_APP_COURSE = 0x2F,
  XIAOMI_APP_SHARE_DEVICE = 0x31,
  XIAOMI_APP_EASTER_EGG = 0x33,
  XIAOMI_APP_VITALITY = 0x36,
  XIAOMI_APP_WORLDCLOCK = 0x3B,
  XIAOMI_APP_TODOLIST = 0x3C,
  XIAOMI_APP_MIJIA = 0x3F,
  XIAOMI_APP_CHECK_TOOL = 0x41,
  XIAOMI_APP_INTERCONNECT = 0x43,
  XIAOMI_APP_TRAINING = 0x44,
  XIAOMI_APP_PERPETUAL_CALENDAR = 0x45,
  XIAOMI_APP_TOURLING = 0x4E,
  XIAOMI_APP_CONTROL_CENTER = 0x4F,
  XIAOMI_APP_FUSION_CENTER = 0x52,
  XIAOMI_APP_CARCONTROL = 0x53,
};

typedef struct miwear_app_t
{
  int proto_app;
  int field_4;
  char *name;
  char *icon;
  uint16_t app_id;
  uint16_t flags;
  int field_14;
  int field_18;
  char *(*app_get_name)(void);
  int (*on_uninstall)(void);
  int field_24;
  int field_28;
  int field_2C;
  int field_30;
  int field_34;
  int (*on_signal)(void);
  int field_3C;
} miwear_app_t;

typedef struct miwear_page_t {
    void    *parent_descriptor;       // +0x00
    uint8_t  _pad_4[0x0C];            // +0x04
    void    *page_name;               // +0x10
    uint16_t page_id;                 // +0x14
    uint16_t app_id;                  // +0x16
    uint16_t flags;                   // +0x18
    uint8_t  _pad_1a[0x02];           // +0x1A
    int32_t  scheduler_deadline;      // +0x1C
    int32_t  scheduler_priority;      // +0x20
    uint32_t async_destroy_state;     // +0x24
    uint8_t  lifecycle_state;         // +0x28
    uint8_t  layer;                   // +0x29
    uint8_t  page_kind;               // +0x2A
    uint8_t  _pad_2b[0x01];            // +0x2B
    void    *activity_context;         // +0x2C
    void    *root_object;              // +0x30
    void    *on_signal;                // +0x34
    void    *runtime_default_56;       // +0x38
    uint8_t  _pad_3c[0x04];             // +0x3C
    void    *registry_prev;            // +0x40
    void    *registry_next;            // +0x44
    void    *runtime_parent;           // +0x48
    void    *on_create;                // +0x4C
    void    *on_resume;                // +0x50
    void    *on_newdata;               // +0x54
    void    *on_start;                 // +0x58
    void    *on_pause;                 // +0x5C
    void    *on_stop;                  // +0x60
    void    *on_destroy;               // +0x64
    void    *on_back;                  // +0x68
    void    *on_keyevent;              // +0x6C
    void    *get_scroll_obj;           // +0x70
} miwear_page_t;


#endif /* XIAOMI_APP_H */
