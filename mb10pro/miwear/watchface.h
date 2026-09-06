#ifndef XIAOMI_WATCHFACE_H
#define XIAOMI_WATCHFACE_H

/* mb10pro watchface manager API.
 *
 * Layered above miwear/app.h: an installed watchface is a miwear_app_t
 * variant whose lifecycle hooks are routed through the watchface manager.
 * Removal goes through the dedicated entry below so the launcher grid,
 * persisted manifest, and live-instance tracking all stay consistent.
 *
 * Function pointer targets (code addresses) and the root data pointer
 * (`g_watchface_config`) are reverse-engineered from vela_ap.bin; the
 * matching `#define` bindings live in
 * platform/mb10p/fw_X.Y.Z/miwear_system.h (the dispatcher at
 * platform/miwear_system.h picks the right per-version file).
 */

#include <stdint.h>

/* Forward decl for the list nodes the manager walks. */
typedef struct watchface_entry_s watchface_entry_t;

/* ---------------------------------------------------------------------------
 * Name translation array element (8 bytes each).
 *
 * Reverse-engineered from parse_watchface_list_json: each entry is packed
 * `(language_id:1, _pad:3, *translation:4)`. The translated string is
 * malloc'd by the JSON loader and freed when the entry is destroyed.
 * --------------------------------------------------------------------------- */
typedef struct watchface_name_translation_s {
    uint8_t  language_id;          /* 0x00 */
    uint8_t  reserved_01[3];       /* 0x01..0x03 */
    char    *translation;          /* 0x04 */
} watchface_name_translation_t;

_Static_assert(sizeof(watchface_name_translation_t) == 8,
               "watchface_name_translation_t must be 8 bytes (1+3+4 ptr)");

/* ---------------------------------------------------------------------------
 * Theme-type info entry (65 bytes each).
 *
 * Loop body in create_watchface_list_json does
 *   v84 = (char *)(i + 65 * v81);
 *   snprintf(v84, 64, "%s", ...);   // 64-byte name
 *   v84[64] = sub_2C1EBCD8(...);    // 1-byte type
 * so the firmware treats each item as exactly 65 bytes and fills name + type
 * back-to-back, no implicit padding. The struct mirrors that -- no trailing
 * reserved bytes.
 * --------------------------------------------------------------------------- */
typedef struct watchface_theme_type_s {
    char    name[64];              /* 0x00..0x3F */
    uint8_t type;                  /* 0x40 */
} watchface_theme_type_t;

_Static_assert(sizeof(watchface_theme_type_t) == 65,
               "watchface_theme_type_t must be 65 bytes (64 char[64] + 1 type)");

/* ---------------------------------------------------------------------------
 * Watchface list entry (200 bytes).
 *
 * Heap-allocated via calloc(1, 200) inside parse_watchface_list_json for
 * every item the firmware finds in <res_dir>/watchface_list.json.
 * Linked into the manager via next/prev at offsets 0x00 / 0x04 (the
 * head/tail slots at g_watchface_config +0x70 and +0x74 are also embedded
 * next/prev nodes, not separate records, hence the empty-list init
 * `*(base + 0x70) = base + 0x70; *(base + 0x74) = base + 0x70`).
 *
 * Layout derived from the writer create_watchface_list_json and the reader
 * parse_watchface_list_json -- both touch every named field below.
 * --------------------------------------------------------------------------- */
typedef struct watchface_entry_s {
    watchface_entry_t *next;              /* 0x00 */
    watchface_entry_t *prev;              /* 0x04 */

    char                id[64];           /* 0x08 -- face_id / dial id string */
    char                name[64];         /* 0x48 -- display name */

    uint8_t             type;             /* 0x88 -- 1=builtin, 2=market, 3=invalid */
    uint8_t             reserved_89[3];    /* 0x89..0x8B */
    uint32_t            version;          /* 0x8C -- wf_version */

    uint8_t             in_use;           /* 0x90 */
    uint8_t             is_delete;        /* 0x91 */
    uint8_t             editable;         /* 0x92 */
    uint8_t             support_album;    /* 0x93 */
    uint8_t             support_AOD;      /* 0x94 */
    uint8_t             support_normal;   /* 0x95 */
    uint8_t             support_android_only; /* 0x96 */
    uint8_t             support_dark_mode;/* 0x97 */
    uint8_t             support_literal;  /* 0x98 */
    uint8_t             support_video;    /* 0x99 */
    uint8_t             support_game;     /* 0x9A */
    uint8_t             reserved_9B;      /* 0x9B */
    uint32_t            theme_count;      /* 0x9C */

    watchface_theme_type_t *theme_type_info;            /* 0xA0 (count at +0x9C, items at 65 bytes) */
    uint32_t            color_table_count;             /* 0xA4 */
    uint32_t           *color_table;                   /* 0xA8 (each item uint32) */
    uint32_t            color_group_table_count;       /* 0xAC */
    uint32_t           *color_group_table;             /* 0xB0 (each item uint32) */

    uint32_t            name_translation_count;        /* 0xB4 */
    watchface_name_translation_t *name_translation;     /* 0xB8 (items at 8 bytes) */

    uint16_t            power_consumption;             /* 0xBC */
    uint16_t            reserved_BE;                   /* 0xBE */
    uint32_t            trial_period;                  /* 0xC0 */
    void               *config_blob;                  /* 0xC4 -- 132-byte inner watchface_config_t */
} watchface_entry_t;

_Static_assert(sizeof(watchface_entry_t) == 200,
               "watchface_entry_t must be 200 bytes to match the firmware "
               "list node allocated by parse_watchface_list_json");

/* ---------------------------------------------------------------------------
 * Top-level watchface manager state. Heap-allocated (calloc(120, 1)) inside
 * watchface_config_init() so its address is the runtime pointer stored in
 * SRAM at g_watchface_config.
 *
 * Layout (reverse-engineered from vela_ap.bin fw_3.101.043, function
 * watchface_config_init at 0x2CA7CCC8 + set_use_watchface at 0x2CA7C67C):
 *
 *   0x00  char    fw_version[64]         -- last successfully-loaded fw
 *                                        -- version string; gates the
 *                                        -- reload-on-bump path
 *   0x40  int32_t sorted                 -- 0 = order_list not yet sorted,
 *                                        -- 1 = already sorted + persisted
 *   0x44  int32_t state                  -- ordering / reload state flag
 *   0x48  watchface_entry_t *current_face_id
 *                                      -- pointer to the currently-active
 *                                        -- watchface_entry_t; updated by
 *                                        -- watchface_config_set_use_watchface()
 *                                        -- when the chosen entry has a
 *                                        -- builtin (non-market) type
 *   0x4C  watchface_entry_t *default_face_id
 *                                      -- pointer to the fallback/default
 *                                        -- watchface_entry_t; updated when
 *                                        -- the chosen entry is of type 2
 *                                        -- (market)
 *   0x50..0x6F  reserved[0x20]          -- private fields not yet mapped;
 *                                        -- 32 bytes between default_face_id
 *                                        -- and head; preserved as opaque
 *                                        -- uint8_t array
 *   0x70  watchface_entry_t *head        -- doubly-linked list sentinel
 *                                        -- (next/prev are in-place at 0x00/0x04
 *                                        -- of this slot when the list is empty)
 *   0x74  watchface_entry_t *tail        -- doubly-linked list sentinel
 *                                        -- == head slot when the list is
 *                                        -- empty; otherwise the LAST entry's
 *                                        -- address. Used as the BACKWARD
 *                                        -- search starting point in
 *                                        -- watchface_config_set_use_watchface;
 *                                        -- NOT the current watchface.
 *
 * Total size 120 bytes (0x78). Note: the current/default pointers are
 * 4-byte entry-pointers (the firmware does *(_DWORD *) = ptr twice --
 * once at +0x48 and once at +0x4C), NOT a single QWORD. The 8-byte
 * zero at init (`*(_QWORD *)(base + 0x48) = 0`) is just bulk-clear of
 * both 4-byte slots.
 * --------------------------------------------------------------------------- */
typedef struct watchface_config_s {
    char                 fw_version[64];   /* 0x00 */
    int32_t              sorted;            /* 0x40 */
    int32_t              state;             /* 0x44 */
    watchface_entry_t   *current_face;   /* 0x48 */
    watchface_entry_t   *default_face;   /* 0x4C */
    uint8_t              reserved_0x50[32]; /* 0x50..0x6F */
    watchface_entry_t   *head;              /* 0x70 */
    watchface_entry_t   *tail;              /* 0x74 */
} watchface_config_t;

_Static_assert(sizeof(watchface_config_t) == 120,
               "watchface_config_t must be 120 bytes to match the "
               "firmware struct at g_watchface_config");

/* Delete a watchface by its launcher face_id.
 *
 * Returns 0 on success, non-zero on failure (face_id unknown, manager
 * busy, or storage error). Removing an in-memory watchface also
 * deregisters it from the packagemanager dispatch so subsequent launches
 * resolve to "not installed". */
typedef int (*watchface_manager_delete_watchface_t)(char* face_id);
typedef int (*watchface_manager_reset_watchface_t)(char* face_id);

#endif /* XIAOMI_WATCHFACE_H */
