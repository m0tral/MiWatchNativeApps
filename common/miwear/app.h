#ifndef XIAOMI_APP_H
#define XIAOMI_APP_H

#include <stdint.h>

struct miwear_app_v1_t;
struct miwear_page_v1_t;
typedef struct miwear_app_v1_t  miwear_app_v1_t;
typedef struct miwear_page_v1_t miwear_page_v1_t;

struct miwear_app_v2_t;
struct miwear_page_v2_t;
typedef struct miwear_app_v2_t  miwear_app_v2_t;
typedef struct miwear_page_v2_t miwear_page_v2_t;

/* Default = v1 (WatchS3). Define MIWEAR_APP_USE_V2_T to swap in mb10p.
 * Implemented as a macro alias rather than a typedef so the model-specific
 * layout header can re-alias the same identifier in the same TU without
 * triggering "conflicting types for 'miwear_app_t'". The dispatch table
 * below is defined AFTER the conditional alias so it sees the right one. */
#ifdef MIWEAR_APP_USE_V2_T
#   define miwear_app_t   miwear_app_v2_t
#   define miwear_page_t  miwear_page_v2_t
#else
#   define miwear_app_t   miwear_app_v1_t
#   define miwear_page_t  miwear_page_v1_t
#endif

int pm_app_install(miwear_app_t *app, miwear_page_t *pages[], int count);

typedef int (*launcher_data_load_app_info_t)(void);
int launcher_data_load_app_info(void);

typedef void (*lv_ll_clear_t)(void *ll_p);
void lv_ll_clear(void *ll_p);

typedef struct lv_ll_s {
    uint32_t n_size;
    void    *head;
    void    *tail;
} lv_ll_t;
_Static_assert(sizeof(lv_ll_t) == 12, "lv_ll_t must be 12 B");

int pm_app_recover(miwear_app_t *app);
int pm_app_uninstall(miwear_app_t *app);
int pm_app_unregister(miwear_app_t *app);
int pm_app_update_notify(miwear_app_t *app);
void *pm_widgets_add(miwear_app_t *app, const void *widgets, int count);
void pm_app_noop(miwear_app_t *app);
int pm_app_set_hidden(miwear_app_t *app, int hidden);
int pm_app_is_hidden(miwear_app_t *app);

typedef int (*app_launcher_add_t)(uint16_t app_id);
typedef int (*app_lookup_t)(uint16_t app_id);
typedef int (*launcher_page_main_update_layout_t)(void);

struct packagemanager_api_s
{
    int (*install)(miwear_app_t *app, miwear_page_t *pages[], int count);
    int (*recover)(miwear_app_t *app);
    int (*uninstall)(miwear_app_t *app);
    int (*unregister)(miwear_app_t *app);
    int (*update_notify)(miwear_app_t *app);
    void *(*widgets_add)(miwear_app_t *app, void *widgets, int count);
    void (*widgets_remove)(miwear_app_t *app);
    int (*set_hidden)(miwear_app_t *app, int hidden);
    int (*is_hidden)(miwear_app_t *app);
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

typedef struct miwear_app_v1_t {
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
} miwear_app_v1_t;
_Static_assert(sizeof(miwear_app_v1_t) == 0x2C, "miwear_app_v1_t must be 0x2C bytes");

typedef struct miwear_page_v1_t {
    void      *parent_descriptor;
    uint8_t    _pad_4[0x0C];
    void      *page_name;
    uint16_t   page_id;
    uint16_t   app_id;
    uint16_t   flags;
    uint8_t    _pad_1a[0x02];
    int32_t    scheduler_deadline;
    int32_t    scheduler_priority;
    uint8_t    lifecycle_state;
    uint8_t    layer;
    uint8_t    page_kind;
    uint8_t    _pad_27[0x01];
    void      *activity_context;
    void      *root_object;
    void      *on_signal;
    void      *runtime_default_56;
    uint8_t    _pad_38[0x04];
    void      *registry_prev;
    void      *registry_next;
    void      *runtime_parent;
    void      *on_create;
    void      *on_resume;
    void      *on_newdata;
    void      *on_start;
    void      *on_pause;
    void      *on_stop;
    void      *on_destroy;
    void      *on_back;
    void      *on_keyevent;
    void      *get_scroll_obj;
} miwear_page_v1_t;
_Static_assert(sizeof(miwear_page_v1_t) == 0x70, "miwear_page_v1_t must be 0x70 bytes");

typedef int (*miwear_on_background_supported_t)(void);
typedef int (*miwear_on_app_signal_t)(void);

#ifdef MIWEAR_APP_USE_V2_T

typedef struct miwear_app_v2_t {
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
} miwear_app_v2_t;
_Static_assert(sizeof(miwear_app_v2_t) == 0x40, "miwear_app_v2_t must be 0x40 bytes");

typedef struct miwear_page_v2_t {
    void    *parent_descriptor;
    uint8_t  _pad_4[0x0C];
    void    *page_name;
    uint16_t page_id;
    uint16_t app_id;
    uint16_t flags;
    uint8_t  _pad_1a[0x02];
    int32_t  scheduler_deadline;
    int32_t  scheduler_priority;
    uint32_t async_destroy_state;
    uint8_t  lifecycle_state;
    uint8_t  layer;
    uint8_t  page_kind;
    uint8_t  _pad_2b[0x01];
    void    *activity_context;
    void    *root_object;
    void    *on_signal;
    void    *runtime_default_56;
    uint8_t  _pad_3c[0x04];
    void    *registry_prev;
    void    *registry_next;
    void    *runtime_parent;
    void    *on_create;
    void    *on_resume;
    void    *on_newdata;
    void    *on_start;
    void    *on_pause;
    void    *on_stop;
    void    *on_destroy;
    void    *on_back;
    void    *on_keyevent;
    void    *get_scroll_obj;
} miwear_page_v2_t;
_Static_assert(sizeof(miwear_page_v2_t) == 0x74, "miwear_page_v2_t must be 0x74 bytes");

#endif

#endif /* XIAOMI_APP_H */
