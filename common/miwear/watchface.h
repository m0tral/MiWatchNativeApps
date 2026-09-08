#ifndef XIAOMI_WATCHFACE_H
#define XIAOMI_WATCHFACE_H

/* watchface manager API.
 *
 * Layered above miwear/app.h: an installed watchface is a miwear_app_t
 * variant whose lifecycle hooks are routed through the watchface manager.
 * Removal goes through the dedicated entry below so the launcher grid,
 * persisted manifest, and live-instance tracking all stay consistent.
 *
 * Function pointer targets (code addresses) and the root data pointer
 * (`g_watchface_config`) are reverse-engineered from vela_ap.bin; the
 * matching `#define` bindings live in
 * platform/<model>/fw_X.Y.Z/miwear_system.h (the dispatcher at
 * platform/miwear_system.h picks the right per-version file).
 *
 * --------------------------------------------------------------------------
 * watchface_config_t is model-specific and lives in:
 *   - platform/watchs3/watchface_layout.h  (v1)
 *   - platform/mb10p/watchface_layout.h    (v2)
 *
 * The two layouts diverge on the active-watchface slot offset:
 *   - WatchS3 family (verified fw_2.6.0 + fw_4.8.0): current_face at +0x44
 *   - mb10p family:                                current_face at +0x48
 *
 * The layout header that gets pulled in by the platform family dispatcher
 * (`platform/<model>/miwear_system.h`) `#define`s `watchface_config_t`
 * to the right per-model `watchface_config_vN_t`, so this file only needs
 * a forward declaration.
 * --------------------------------------------------------------------------
 */

#include <stddef.h>
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
 * Identical between WatchS3 and mb10p (modulo firmware version).
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
 * Per-model overlay for the 32-byte model-specific mid region of
 * watchface_config_t (offsets 0x50..0x6F). Only valid on mb10p / mb10pro;
 * WatchS3 family never touches those offsets, so the equivalent accessor
 * there is just `&g_watchface_config->reserved_4C[4]` (see
 * platform/watchs3/watchface_layout.h).
 *
 * Use on mb10p:
 *     watchface_config_mb10p_overlay_t *ov =
 *         (watchface_config_mb10p_overlay_t *)&g_watchface_config->mid;
 * --------------------------------------------------------------------------- */
typedef struct watchface_config_mb10p_overlay_s {
    uint64_t             ts_start_a;     /* 0x50 -- start timestamp, type-A */
    uint64_t             ts_start_b;     /* 0x58 -- start timestamp, type-B */
    uint64_t             ts_last;        /* 0x60 -- last observation time  */
    watchface_entry_t   *entry_ref;      /* 0x68 -- currently-observed entry */
    uint8_t              flags_0x6C;     /* 0x6C -- level-A flag */
    uint8_t              flags_0x6D;     /* 0x6D -- level-B flag */
    uint8_t              reserved_0x6E[2];/* 0x6E..0x6F -- 2 bytes unused */
} watchface_config_mb10p_overlay_t;

_Static_assert(sizeof(watchface_config_mb10p_overlay_t) == 32,
               "watchface_config_mb10p_overlay_t must fit exactly in the "
               "0x50..0x6F region of watchface_config_t (32 bytes)");
_Static_assert(offsetof(watchface_config_mb10p_overlay_t, ts_start_a) == 0,
               "watchface_config_mb10p_overlay_t.ts_start_a must be at "
               "offset 0 of the overlay (= +0x50 of watchface_config_t)");
_Static_assert(offsetof(watchface_config_mb10p_overlay_t, entry_ref)  == 0x18,
               "watchface_config_mb10p_overlay_t.entry_ref must be at "
               "offset 0x18 of the overlay (= +0x68 of watchface_config_t)");
_Static_assert(offsetof(watchface_config_mb10p_overlay_t, flags_0x6D) == 0x1D,
               "watchface_config_mb10p_overlay_t.flags_0x6D must be at "
               "offset 0x1D of the overlay (= +0x6D of watchface_config_t)");

/* ---------------------------------------------------------------------------
 * Top-level watchface manager state. The actual layout is model-specific
 * (see platform/<model>/watchface_layout.h); `watchface_config_t` is NOT
 * defined here. It is created by the matching layout header, which the
 * platform family dispatcher (`platform/<model>/miwear_system.h`) pulls
 * in before any per-fw miwear_system.h binds `g_watchface_config`.
 *
 * Consumers that include this header directly must go through
 * platform/miwear_system.h to get the proper model-specific definition.
 * --------------------------------------------------------------------------- */

/* Delete a watchface by its launcher face_id.
 *
 * Returns 0 on success, non-zero on failure (face_id unknown, manager
 * busy, or storage error). Removing an in-memory watchface also
 * deregisters it from the packagemanager dispatch so subsequent launches
 * resolve to "not installed". */
typedef int (*watchface_manager_delete_watchface_t)(char* face_id);
typedef int (*watchface_manager_reset_watchface_t)(char* face_id);

#endif /* XIAOMI_WATCHFACE_H */
