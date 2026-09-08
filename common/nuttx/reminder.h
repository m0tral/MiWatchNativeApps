#ifndef XIAOMI_REMINDER_H
#define XIAOMI_REMINDER_H

/* watchs3 LVGL reminder subsystem.
 *
 * struct reminder_t layout reverse-engineered from
 * libs/lvgl/reminder/lvx_reminder_*.c via reminder_page_ota_start
 * (firmware symbol at 0x2c5329f8) and the field setters:
 *
 *   lvx_reminder_set_uid           -> ((uint32_t*)r)[ 0]   offset 0x00
 *   lvx_reminder_set_level         -> ((uint8_t *)r)[12]   offset 0x0C
 *   sub_2CA998AC                   -> ((uint8_t *)r)[36]   offset 0x24
 *   sub_2CA98E04/1C/34, ignore_qm,
 *   lvx_reminder_disable_backspace -> ((uint8_t *)r)[37]   bitfield 0x25
 *   lvx_reminder_set_life_times    -> ((uint32_t*)r)[ 7]   offset 0x1C
 *   lvx_reminder_set_on_create     -> ((uint32_t*)r)[10]   offset 0x28
 *   lvx_reminder_set_on_destroy    -> ((uint32_t*)r)[11]   offset 0x2C
 *   v1[14] = 0xC5321B9u            -> ((uint32_t*)r)[14]   offset 0x38
 *
 * From lvx_reminder_start, v19[i] resolves to offset i*4 and is called
 * as fn(v19, v19[1], v19[2]):
 *   v19[10] = on_create            (offset 0x28)
 *   v19[12] = on_resume            (offset 0x30)
 *   v19[13] = on_pause             (offset 0x34)
 *
 * The struct is copied wholesale (memcpy of 184 bytes) into the
 * reminder registry by sub_2CA96E70, so the two pointers at +0x10 and
 * +0x14 are deep-copied (strdup'd) on insert -- no need to keep the
 * source buffers alive after lvx_reminder_start() returns.
 *
 * Total size: 184 bytes (46 x uint32_t).
 */

#include <stdint.h>
#include "common/lvgl/control.h"   /* for lv_obj_t (used in lvx_reminder_lifecycle_t below) */

struct reminder_t;

/* Reminder life-cycle callback. Called by the LVGL reminder engine with:
 *   r        : pointer to the inserted reminder_t (registry copy)
 *   session  : opaque screen-session handle (v19[1] in lvx_reminder_start)
 *   userdata : copy of reminder_t.userdata at offset +0x08
 */
typedef void (*lvx_reminder_lifecycle_t)(struct reminder_t *r,
                                         lv_obj_t *obj,
                                         unsigned int userdata);

/* Compile-time enforced offset checks against the firmware contract. */
#define REMINDER_SIZE_BYTES 184

struct reminder_t
{
    /* +0x00 */ uint32_t uid;                   /* lvx_reminder_set_uid */
    /* +0x04 */ uint32_t _reserved0;
    /* +0x08 */ uint32_t userdata;              /* passed as 3rd cb arg */
    /* +0x0C */ uint8_t  level;                 /* lvx_reminder_set_level */
    /* +0x0D */ uint8_t  vibro_type_id;
    /* +0x0E */ uint8_t  vibro_count;
    /* +0x0F */ uint8_t  _pad;
    /* +0x10 */ char    *bell_mp3_name;         /* strdup'd on insert */
    /* +0x14 */ char    *snd_channel_name;      /* strdup'd on insert */
    /* +0x18 */ uint32_t bell_attr;             // play counter may be
    /* +0x1C */ int32_t  life_times;            /* -1 = forever, else ticks */
    /* +0x20 */ uint32_t _reserved2;
    /* +0x24 */ uint8_t  kind;                  /* sub_2CA998AC, 0x1C=28 default */
    /* +0x25 */ uint8_t  flags;                 /* bitfield (see REMINDER_FLAG_*) */
    /* +0x26 */ uint8_t  flags2;                /* secondary bitfield */
    /* +0x27 */ uint8_t  _pad1;
    /* +0x28 */ lvx_reminder_lifecycle_t on_create;   /* v1[10] set_on_create */
    /* +0x2C */ lvx_reminder_lifecycle_t on_destroy;  /* v1[11] set_on_destroy */
    /* +0x30 */ lvx_reminder_lifecycle_t on_resume;   /* v1[12] */
    /* +0x34 */ lvx_reminder_lifecycle_t on_pause;    /* v1[13] */
    /* +0x38 */ lvx_reminder_lifecycle_t on_cb;       /* v1[14] */
    /* +0x3C */ uint32_t _tail[31];             /* +0x3C..+0xB7 unknown state */
};

_Static_assert(sizeof(struct reminder_t) == REMINDER_SIZE_BYTES,
               "reminder_t must be 184 bytes (firmware contract)");

/* ---- Bit positions in the byte at offset 0x25 (byte 37) ---------------- *
 * Each setter does:
 *   ((uint8_t*)r)[0x25] = (((uint8_t*)r)[0x25] & ~mask) | ((value & 1) << bit)
 */
enum {
    REMINDER_FLAG_IGNORE_QUIET_MODE  = (1u << 1), /* lvx_reminder_ignore_quiet_mode */
    REMINDER_FLAG_IGNORE_SLEEP_MODE  = (1u << 2), /* lvx_reminder_ignore_sleep_mode */
    REMINDER_FLAG_IGNORE_MUTE_MODE   = (1u << 3), /* lvx_reminder_ignore_mute_mode */
    REMINDER_FLAG_DISABLE_BACKSPACE  = (1u << 5), /* lvx_reminder_disable_backspace */
    REMINDER_FLAG_BIT7               = (1u << 7), /* lvx_reminder_set_flag_bit7 */
};

/* Bit positions in the byte at offset 0x26 (byte 38) */
enum {
    REMINDER_FLAG2_INVISIBLE = (1u << 1), /* lvx_reminder_start: +38 & 2 */
    REMINDER_FLAG2_STACK_TOP = (1u << 2), /* lvx_reminder_start: +38 & 4 */
    REMINDER_FLAG2_TOPMOST   = (1u << 7), /* lvx_reminder_start: +38 & 0x80 */
};

/* ---- Firmware entry points -------------------------------------------- *
 * Typedefs only. Thumb-bit-set address bindings (with `|1`) live in
 * watchs3_platform.h as `#define`s of the same name -- a TU must
 * include watchs3_platform.h to actually call any of these, which is
 * what gives the call site its absolute BLX target.
 */
typedef int (*lvx_reminder_set_uid_t)         (struct reminder_t *r, uint32_t uid);
typedef int (*lvx_reminder_set_level_t)       (struct reminder_t *r, uint8_t level);
typedef int (*lvx_reminder_set_vibration_t)   (struct reminder_t *r, uint8_t type_id, uint8_t count);
typedef int (*lvx_reminder_ignore_quiet_mode_t)(struct reminder_t *r, uint8_t on);
typedef int (*lvx_reminder_ignore_sleep_mode_t)(struct reminder_t *r, uint8_t on);
typedef int (*lvx_reminder_ignore_mute_mode_t)(struct reminder_t *r, uint8_t on);
typedef int (*lvx_reminder_set_flag_bit7_t)   (struct reminder_t *r, uint8_t on);
typedef int (*lvx_reminder_set_kind_byte_t)   (struct reminder_t *r, uint8_t kind);
typedef int (*lvx_reminder_set_life_times_t)  (struct reminder_t *r, int32_t ticks);
typedef int (*lvx_reminder_set_on_create_t)   (struct reminder_t *r, lvx_reminder_lifecycle_t fn);
typedef int (*lvx_reminder_set_on_destroy_t)  (struct reminder_t *r, lvx_reminder_lifecycle_t fn);
typedef int (*lvx_reminder_disable_backspace_t)(struct reminder_t *r, uint8_t on);
typedef int (*lvx_reminder_set_bell_t)        (struct reminder_t *r,
                                                const char *mp3_name,
                                                const char *channel_name,
                                                uint8_t attr);
typedef int (*lvx_reminder_start_t)           (struct reminder_t *r);
typedef int (*lvx_reminder_cancel_t)          (uint32_t uid);
typedef int (*reminder_page_ota_start_t)      (void);

#endif /* XIAOMI_REMINDER_H */
