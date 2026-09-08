#ifndef WATCHS3_WATCHFACE_LAYOUT_H
#define WATCHS3_WATCHFACE_LAYOUT_H

/* WatchS3 (Xiaomi Watch S3) -- watchface_config_t layout (v1).
 *
 * Verified against both vela_ap.bin fw_2.6.0 (g_watchface_config = 0x3CCCF8D8)
 * and vela_ap.bin fw_4.8.0 (g_watchface_config = 0x3C2BC1F0).
 *
 * Confirmed by reading the firmware:
 *   - watchface_reset_watchface: reads *(g_watchface_config + 0x44) as the
 *     canonical "current active watchface" slot. fw 2.6.0 routes this via
 *     sub_2C759F20 -> sub_2C75D184 which is literally
 *     `return *(DWORD *)(g_watchface_config + 0x44);`.
 *   - watchface_config_set_use_watchface (fw 2.6.0: 0x2C75E5B8) and
 *     watchface_manager_set_watchface_by_id (fw 4.8.0: 0x2C6DCB54) BOTH
 *     do:
 *         if (entry->type == 2) { *(base + 0x48) = entry; }
 *         else                   { *(base + 0x44) = entry; }
 *
 * Therefore: on WatchS3 the *active* watchface lives at +0x44; +0x48 is
 * populated only when the active face is a market face (type==2).
 *
 * Trailing region (0x4C..0x77): layout differs between fw_2.6.0 and
 * fw_4.8.0 (the linked-list head/tail pointers were relocated between
 * versions), so we keep that region OPAQUE here. Walk the list via the
 * firmware helpers (watchface_manager_reset_watchface, etc.) instead of
 * dereferencing offsets past 0x48 directly.
 */

#include <stddef.h>
#include <stdint.h>
#include "common/miwear/watchface.h"   /* for watchface_entry_t forward decl */

typedef struct watchface_config_v1_s {
    char                 fw_version[64];   /* 0x00 */
    int32_t              sorted;            /* 0x40 */
    watchface_entry_t   *current_face;      /* 0x44 -- active (non-market/builtin) */
    watchface_entry_t   *current_market;    /* 0x48 -- active market face (type==2) */

    /* 0x4C..0x77: opaque trailer. List head/tail moved between
     * fw_2.6.0 (+0x50/+0x54 area) and fw_4.8.0 (+0x70/+0x74). Do NOT
     * dereference -- call the firmware helpers. */
    uint8_t              reserved_4C[0x2C]; /* 0x4C..0x77 */
} watchface_config_v1_t;

_Static_assert(sizeof(watchface_config_v1_t) == 0x78,
               "watchface_config_v1_t must be 120 bytes (0x78) to match "
               "the firmware calloc(120, 1) for the manager struct");
_Static_assert(offsetof(watchface_config_v1_t, fw_version)    == 0x00,
               "watchface_config_v1_t.fw_version must be at firmware +0x00");
_Static_assert(offsetof(watchface_config_v1_t, sorted)        == 0x40,
               "watchface_config_v1_t.sorted must be at firmware +0x40");
_Static_assert(offsetof(watchface_config_v1_t, current_face)  == 0x44,
               "watchface_config_v1_t.current_face must be at firmware +0x44 "
               "(verified via sub_2C75D184 in fw_2.6.0 and "
               "watchface_reset_watchface in fw_4.8.0)");
_Static_assert(offsetof(watchface_config_v1_t, current_market) == 0x48,
               "watchface_config_v1_t.current_market must be at firmware +0x48 "
               "(set by watchface_config_set_use_watchface when type==2)");
_Static_assert(sizeof(((watchface_config_v1_t *)0)->reserved_4C) == 0x2C,
               "watchface_config_v1_t.reserved_4C must cover 0x4C..0x77 (44 bytes)");

/* Family typedef alias. Per-fw miwear_system.h references `watchface_config_t`
 * when binding `g_watchface_config`; this provides the WatchS3 view. */
#define watchface_config_t watchface_config_v1_t

#endif /* WATCHS3_WATCHFACE_LAYOUT_H */
