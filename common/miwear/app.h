#ifndef XIAOMI_APP_H
#define XIAOMI_APP_H

/* watchs3 packagemanager app API.
 *
 * The firmware publishes a dispatch table (struct packagemanager_api_s)
 * of Thumb function pointers at SRAM address 0x200EB660. Layout below is
 * reverse-engineered from vela_ap.bin fw_4.8.0 after the functions in
 * the table were renamed in IDA.
 *
 * Entry points (code addresses) live in the 0x2Cxxxxxx region; the SRAM
 * table stores them with the high nibble 0x0C instead of 0x2C (a quirk
 * of this firmware image). The Thumb bit (bit 0) is set on every code
 * pointer.
 *
 * Access is through the macro in platform/watchs3_platform.h:
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

typedef int (*launcher_data_load_app_info_t)(void); /* declared here for the launcher helpers listed below */

int launcher_data_load_app_info(void); /* replacement for the legacy launcher_data_reload_app_name -- declared opaque here; binding lives in platform header */

/* LVGL linked-list API. The user-side binding `lv_ll_clear` in the platform
 * header resolves to the firm's symbol at lv_ll_clear (IDA-Named). */
typedef void (*lv_ll_clear_t)(void *ll_p);
void lv_ll_clear(void *ll_p);

/* LVGL linked-list head struct (firmware-layout, see platform/miwear_system.h
 * for the global pointer). Defining the layout lets the platform-header
 * binding take an `lv_ll_t &` and let user code write `&g_appinfo_list`.
 *
 *   +0x00  uint32_t n_size         -- number of nodes
 *   +0x04  uint32_t head          -- first node (or `this` if list is empty)
 *   +0x08  uint32_t tail          -- last node (or `this` if list is empty)
 *
 * Total 12 B (4-byte packed). On firmware the struct lives as a static
 * global at G_APPINFO_LIST_ADDR (typically 0x3C204E30 on WatchS3 2.6.0).
 * Only the bindings need this; user code only ever sees `lv_ll_t *`. */
typedef struct lv_ll_s {
    uint32_t n_size;
    void    *head;
    void    *tail;
} lv_ll_t;

_Static_assert(sizeof(lv_ll_t) == 12,
               "lv_ll_t must be 12 B to match LVGL's lv_ll head");

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

typedef int (*launcher_data_load_app_info_t)(void);
typedef int (*launcher_page_main_update_layout_t)(void);

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

/* ---------------------------------------------------------------------------
 * App dispatch table -- model-specific.
 *
 * WatchS3 (vela_ap.bin fw_2.6.0 / fw_4.8.0) is sizeof=0x2C with the layout
 * shown by `struct miwear_app_t` below (size verified at compile time).
 *
 * mb10pro (vela_ap.bin fw_3.101.043 / fw_3.201.016) is sizeof=0x40 with a
 * separate layout (`miwear_app_mb10p_t` below) -- function pointers land at
 * totally different offsets, so the two structs are NOT a sub/superset pair.
 * Each model picks its own typedef based on MIWEAR_APP_USE_MB10P_T.
 *
 * Reverse-engineered offsets (WatchS3, 0x2C, 12 fields = 44 bytes):
 *   0x00  int      proto_app              -- has-been-initialized flag
 *   0x04  int      field_4               -- reserved / 0 in our sample
 *   0x08  char    *name                  -- app display name (UTF-8)
 *   0x0C  char    *icon                  -- icon path
 *   0x10  uint16_t app_id               -- tag fed into pm_app_install
 *   0x12  uint16_t field_12             -- payload size or flags (UNKNOWN; 0 in our sample)
 *   0x14  fn_ptr   get_app_name          -- char *(*)(void)
 *   0x18  fn_ptr   is_background_supported
 *                                       -- int (*)(void); 1 if bg app
 *   0x1C  int      field_1C              -- reserved
 *   0x20  int      field_20              -- reserved
 *   0x24  fn_ptr   on_app_signal         -- int (*)(void); signal handler
 *   0x28  int      field_28              -- reserved; struct ends here
 * --------------------------------------------------------------------------- */

typedef struct miwear_app_t
{
  int          proto_app;
  int          field_4;
  char        *name;
  char        *icon;
  uint16_t     app_id;
  uint16_t     field_12;
  char        *(*get_app_name)(void);
  int          (*is_background_supported)(void);
  int          field_1C;
  int          field_20;
  int          (*on_app_signal)(void);
  int          field_28;
} miwear_app_t;

/* ---------------------------------------------------------------------------
 * Page state struct -- model-specific.
 *
 * WatchS3 (vela_ap.bin fw_2.6.0 / fw_4.8.0) is the default -- see
 * `miwear_page_t` defined below. Reverse-engineered offsets (sizeof=0x70):
 *   0x00  void    *parent_descriptor
 *   0x04  pad 12
 *   0x10  void    *page_name
 *   0x14  uint16_t page_id
 *   0x16  uint16_t app_id
 *   0x18  uint16_t flags
 *   0x1A  pad 2
 *   0x1C  int32_t scheduler_deadline
 *   0x20  int32_t scheduler_priority
 *   0x24  uint8_t lifecycle_state
 *   0x25  uint8_t layer
 *   0x26  uint8_t page_kind
 *   0x27  pad 1
 *   0x28  void    *activity_context
 *   0x2C  void    *root_object
 *   0x30  void    *on_signal
 *   0x34  void    *runtime_default_56
 *   0x38  pad 4
 *   0x3C  void    *registry_prev
 *   0x40  void    *registry_next
 *   0x44  void    *runtime_parent
 *   0x48  void    *on_create
 *   0x4C  void    *on_resume
 *   0x50  void    *on_newdata
 *   0x54  void    *on_start
 *   0x58  void    *on_pause
 *   0x5C  void    *on_stop
 *   0x60  void    *on_destroy
 *   0x64  void    *on_back
 *   0x68  void    *on_keyevent
 *   0x6C  void    *get_scroll_obj
 *
 * WatchS3 firmware does NOT carry an `async_destroy_state` field at +0x24
 * (unlike mb10p). Everything from `lifecycle_state` onward shifts up by 4.
 *
 * mb10pro is sizeof=0x74 with a separate layout -- defined below as
 * `miwear_page_mb10p_t`. Function pointers land at different offsets so
 * the two structs are NOT a sub/superset pair. Selected via
 * `MIWEAR_APP_USE_MB10P_T`.
 * --------------------------------------------------------------------------- */

typedef struct miwear_page_t {
    void      *parent_descriptor;    /* +0x00 */
    uint8_t    _pad_4[0x0C];         /* +0x04 */
    void      *page_name;            /* +0x10 */
    uint16_t   page_id;              /* +0x14 */
    uint16_t   app_id;               /* +0x16 */
    uint16_t   flags;                /* +0x18 */
    uint8_t    _pad_1a[0x02];        /* +0x1A */
    int32_t    scheduler_deadline;   /* +0x1C */
    int32_t    scheduler_priority;   /* +0x20 */
    uint8_t    lifecycle_state;      /* +0x24 -- async_destroy_state NOT present on WatchS3 */
    uint8_t    layer;                /* +0x25 */
    uint8_t    page_kind;            /* +0x26 */
    uint8_t    _pad_27[0x01];       /* +0x27 */
    void      *activity_context;      /* +0x28 */
    void      *root_object;           /* +0x2C */
    void      *on_signal;             /* +0x30 */
    void      *runtime_default_56;    /* +0x34 */
    uint8_t    _pad_38[0x04];        /* +0x38 */
    void      *registry_prev;         /* +0x3C */
    void      *registry_next;         /* +0x40 */
    void      *runtime_parent;        /* +0x44 */
    void      *on_create;             /* +0x48 */
    void      *on_resume;             /* +0x4C */
    void      *on_newdata;            /* +0x50 */
    void      *on_start;              /* +0x54 */
    void      *on_pause;              /* +0x58 */
    void      *on_stop;               /* +0x5C */
    void      *on_destroy;            /* +0x60 */
    void      *on_back;               /* +0x64 */
    void      *on_keyevent;           /* +0x68 */
    void      *get_scroll_obj;        /* +0x6C */
} miwear_page_t;

_Static_assert(sizeof(miwear_page_t) == 0x70,
               "miwear_page_t must be 0x70 bytes (112) to match WatchS3 firmware");
_Static_assert(_Alignof(miwear_page_t) == 4,
               "miwear_page_t must be 4-byte aligned (WatchS3 firmware ABI)");
_Static_assert(offsetof(miwear_page_t, parent_descriptor)    == 0x00, "offsetof(miwear_page_t, parent_descriptor) must be 0x00");
_Static_assert(offsetof(miwear_page_t, page_name)            == 0x10, "offsetof(miwear_page_t, page_name) must be 0x10");
_Static_assert(offsetof(miwear_page_t, page_id)              == 0x14, "offsetof(miwear_page_t, page_id) must be 0x14");
_Static_assert(offsetof(miwear_page_t, app_id)               == 0x16, "offsetof(miwear_page_t, app_id) must be 0x16");
_Static_assert(offsetof(miwear_page_t, flags)                == 0x18, "offsetof(miwear_page_t, flags) must be 0x18");
_Static_assert(offsetof(miwear_page_t, scheduler_deadline)   == 0x1C, "offsetof(miwear_page_t, scheduler_deadline) must be 0x1C");
_Static_assert(offsetof(miwear_page_t, scheduler_priority)   == 0x20, "offsetof(miwear_page_t, scheduler_priority) must be 0x20");
_Static_assert(offsetof(miwear_page_t, lifecycle_state)     == 0x24, "offsetof(miwear_page_t, lifecycle_state) must be 0x24 -- async_destroy_state is NOT present on WatchS3");
_Static_assert(offsetof(miwear_page_t, activity_context)    == 0x28, "offsetof(miwear_page_t, activity_context) must be 0x28");
_Static_assert(offsetof(miwear_page_t, root_object)         == 0x2C, "offsetof(miwear_page_t, root_object) must be 0x2C");
_Static_assert(offsetof(miwear_page_t, on_signal)            == 0x30, "offsetof(miwear_page_t, on_signal) must be 0x30");
_Static_assert(offsetof(miwear_page_t, registry_prev)        == 0x3C, "offsetof(miwear_page_t, registry_prev) must be 0x3C");
_Static_assert(offsetof(miwear_page_t, on_create)            == 0x48, "offsetof(miwear_page_t, on_create) must be 0x48");
_Static_assert(offsetof(miwear_page_t, get_scroll_obj)       == 0x6C, "offsetof(miwear_page_t, get_scroll_obj) must be 0x6C");

_Static_assert(sizeof(miwear_app_t) == 0x2C,
               "miwear_app_t must be 0x2C bytes (44) to match WatchS3 firmware");
_Static_assert(_Alignof(miwear_app_t) == 4,
               "miwear_app_t must be 4-byte aligned (WatchS3 firmware ABI)");
_Static_assert(offsetof(miwear_app_t, proto_app)     == 0x00, "offsetof(miwear_app_t, proto_app)     must be 0x00");
_Static_assert(offsetof(miwear_app_t, name)          == 0x08, "offsetof(miwear_app_t, name)          must be 0x08");
_Static_assert(offsetof(miwear_app_t, icon)          == 0x0C, "offsetof(miwear_app_t, icon)          must be 0x0C");
_Static_assert(offsetof(miwear_app_t, app_id)        == 0x10, "offsetof(miwear_app_t, app_id)        must be 0x10");
_Static_assert(offsetof(miwear_app_t, get_app_name) == 0x14, "offsetof(miwear_app_t, get_app_name) must be 0x14");
_Static_assert(offsetof(miwear_app_t, is_background_supported)
              == 0x18, "offsetof(miwear_app_t, is_background_supported) must be 0x18");
_Static_assert(offsetof(miwear_app_t, on_app_signal) == 0x24, "offsetof(miwear_app_t, on_app_signal) must be 0x24");

/* Function-typedef aliases so consumer code can refer to them by name
 * rather than re-spelling the prototype. */
typedef int    (*miwear_on_background_supported_t)(void);
typedef int    (*miwear_on_app_signal_t)(void);

/* ---------------------------------------------------------------------------
 * mb10p layout (vela_ap.bin fw_3.101.043 / fw_3.201.016), sizeof=0x40 = 64 bytes.
 *
 * NOT a WatchS3 superset -- the function pointers land at totally different
 * offsets (e.g. miwear_app_t.on_app_signal at +0x24 here vs +0x18 there), so
 * you cannot derive one from the other.
 *
 * Consumers can pull in this layout explicitly by defining
 * MIWEAR_APP_USE_MB10P_T before including this header; otherwise the WatchS3
 * miwear_app_t is selected.
 *
 * Offsets reverse-engineered from firmware:
 *   0x00  int       proto_app
 *   0x04  int       field_4
 *   0x08  char      *name
 *   0x0C  char      *icon
 *   0x10  uint16_t  app_id
 *   0x12  uint16_t  flags                  -- real flag bitset (was 0x2C convention); different semantics from WatchS3 field_12 (which is at the same offset but carries a payload size there)
 *   0x14  int       field_14              -- reserved
 *   0x18  int       field_18              -- reserved
 *   0x1C  fn_ptr    app_get_name
 *   0x20  fn_ptr    on_uninstall          -- WatchS3 has is_background_supported here, not on_uninstall
 *   0x24  int       field_24
 *   0x28  int       field_28
 *   0x2C  int       field_2C
 *   0x30  int       field_30
 *   0x34  int       field_34
 *   0x38  fn_ptr    on_signal             -- WatchS3 has on_app_signal at +0x24 here, not +0x38
 *   0x3C  int       field_3C
 * --------------------------------------------------------------------------- */

#ifdef MIWEAR_APP_USE_MB10P_T

typedef struct miwear_app_mb10p_t
{
  int          proto_app;
  int          field_4;
  char        *name;
  char        *icon;
  uint16_t     app_id;
  uint16_t     flags;
  int          field_14;
  int          field_18;
  char        *(*app_get_name)(void);
  int          (*on_uninstall)(void);
  int          field_24;
  int          field_28;
  int          field_2C;
  int          field_30;
  int          field_34;
  int          (*on_signal)(void);
  int          field_3C;
} miwear_app_mb10p_t;

_Static_assert(sizeof(miwear_app_mb10p_t) == 0x40,
               "miwear_app_mb10p_t must be 0x40 bytes (64) to match mb10p firmware");

/* Under MIWEAR_APP_USE_MB10P_T, alias miwear_app_t to the mb10p layout so
 * legacy code that doesn't know about the two-model split still compiles. */
typedef miwear_app_mb10p_t miwear_app_t;

typedef struct miwear_page_mb10p_t {
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
} miwear_page_mb10p_t;

_Static_assert(sizeof(miwear_page_mb10p_t) == 0x74,
               "miwear_page_mb10p_t must be 0x74 bytes (116) to match mb10p firmware");

/* Under MIWEAR_APP_USE_MB10P_T, alias the per-model names so legacy code
 * that doesn't know about the two-model split still compiles against the
 * same names. */
typedef miwear_app_mb10p_t miwear_app_t;
typedef miwear_page_mb10p_t miwear_page_t;

#endif /* MIWEAR_APP_USE_MB10P_T */

#endif /* XIAOMI_APP_H */
