#ifndef MB10P_WATCHFACE_LAYOUT_H
#define MB10P_WATCHFACE_LAYOUT_H

/* mb10p / mb10pro (Xiaomi Mi Band 10 Pro) -- watchface_config_t layout (v2).
 *
 * Per the legacy RE notes that previously lived in
 * common/miwear/watchface.h: the firmware exposes current_face at +0x48
 * and default_face at +0x4C on this family. The trailing 32-byte
 * mid-region (+0x50..0x6F) is overlaid by
 * watchface_config_mb10p_overlay_t for per-firmware time-delta tracking.
 */

#include <stddef.h>
#include <stdint.h>
#include "common/miwear/watchface.h"   /* for watchface_entry_t forward decl */

typedef struct watchface_config_v2_s {
    /* 0x00..0x4F: header */
    char                 fw_version[64];   /* 0x00 */
    int32_t              sorted;            /* 0x40 */
    int32_t              state;             /* 0x44 */
    watchface_entry_t   *current_face;      /* 0x48 */
    watchface_entry_t   *default_face;      /* 0x4C */

    /* 0x50..0x6F: model-specific mid region. Cast through
     * watchface_config_mb10p_overlay_t (declared in common/miwear/watchface.h). */
    uint8_t              mid[32];           /* 0x50..0x6F */

    /* 0x70..0x77: doubly-linked list head/tail sentinels */
    watchface_entry_t   *head;              /* 0x70 */
    watchface_entry_t   *tail;              /* 0x74 */
} watchface_config_v2_t;

_Static_assert(sizeof(watchface_config_v2_t) == 120,
               "watchface_config_v2_t must be 120 bytes to match the "
               "firmware calloc(120, 1) for the manager struct");
_Static_assert(offsetof(watchface_config_v2_t, fw_version)    == 0x00,
               "watchface_config_v2_t.fw_version must be at firmware +0x00");
_Static_assert(offsetof(watchface_config_v2_t, sorted)        == 0x40,
               "watchface_config_v2_t.sorted must be at firmware +0x40");
_Static_assert(offsetof(watchface_config_v2_t, state)         == 0x44,
               "watchface_config_v2_t.state must be at firmware +0x44");
_Static_assert(offsetof(watchface_config_v2_t, current_face)  == 0x48,
               "watchface_config_v2_t.current_face must be at firmware +0x48");
_Static_assert(offsetof(watchface_config_v2_t, default_face)  == 0x4C,
               "watchface_config_v2_t.default_face must be at firmware +0x4C");
_Static_assert(offsetof(watchface_config_v2_t, mid)           == 0x50,
               "watchface_config_v2_t.mid must be at firmware +0x50");
_Static_assert(sizeof(((watchface_config_v2_t *)0)->mid)       == 32,
               "watchface_config_v2_t.mid must span firmware +0x50..+0x6F (32 bytes)");
_Static_assert(offsetof(watchface_config_v2_t, head)          == 0x70,
               "watchface_config_v2_t.head must be at firmware +0x70");
_Static_assert(offsetof(watchface_config_v2_t, tail)          == 0x74,
               "watchface_config_v2_t.tail must be at firmware +0x74");

/* Family typedef alias. Per-fw miwear_system.h references `watchface_config_t`
 * when binding `g_watchface_config`; this provides the mb10p view. */
#define watchface_config_t watchface_config_v2_t

#endif /* MB10P_WATCHFACE_LAYOUT_H */
