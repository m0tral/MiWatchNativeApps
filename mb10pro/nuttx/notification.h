#ifndef MB10P_NOTIFICATIONS_H
#define MB10P_NOTIFICATIONS_H

/* mb10pro (Xiaomi Mi Band 10 Pro) notification subsystem stub.
 *
 * Thin wrapper over the firmware's notification queue so modules can
 * post UI events without depending on the full miwear SDK. Addresses
 * below are placeholders -- fill in from IDA once you locate the real
 * entry points in vela_ap.bin.
 */

#include <stdint.h>
#include <stddef.h>

/* Notification category. Matches the values used by miwear's status-bar
 * subsystem on mb10pro. */
typedef enum {
    MB10P_NOTIFY_GENERIC   = 0,
    MB10P_NOTIFY_CALL      = 1,
    MB10P_NOTIFY_SMS       = 2,
    MB10P_NOTIFY_ALARM     = 3,
    MB10P_NOTIFY_GOAL      = 4,
    MB10P_NOTIFY_HEARTRATE = 5,
    MB10P_NOTIFY_APP       = 6,
} mb10p_notify_kind_t;

/* Outcome of a post. Negative values are negated errno. */
enum {
    MB10P_NOTIFY_OK         =  0,
    MB10P_NOTIFY_ERR_FULL   = -1,
    MB10P_NOTIFY_ERR_INVAL  = -2,
    MB10P_NOTIFY_ERR_NOMEM  = -3,
};

struct notification_msg_t {
    uint64_t message_id; /* +0x0 */
    uint32_t repeat_count; /* +0x8 */
    void * title; /* +0xc */
    void * source; /* +0x10 */
    void * body; /* +0x14 */
    void * auxiliary_text; /* +0x18 */
    void * small_icon_path; /* +0x1c */
    void * large_icon_path; /* +0x20 */
    void * extension_text_36; /* +0x24 */
    void * extension_text_40; /* +0x28 */
    uint32_t timestamp; /* +0x2c */
    uint8_t _pad_30[8];
    void * action_callback; /* +0x38 */
    uint32_t action_context; /* +0x3c */
    uint32_t extension_64; /* +0x40 */
    uint32_t extension_68; /* +0x44 */
    void * open_callback; /* +0x48 */
    void * destroy_callback; /* +0x4c */
    uint8_t start_reminder; /* +0x50 */
    uint8_t flags_81; /* +0x51 */
    uint8_t flags_82; /* +0x52 */
    uint8_t _pad_53[1];
    void * callback_data; /* +0x54 */
};

/* Firmware notification_insert entry point. Thumb-bitted address binding
 * lives in platform/mb10p_platform.h. Returns 0 on success, negative
 * errno on failure. */
typedef int (*notification_insert_t)(const struct notification_msg_t *msg);

#endif /* MB10P_NOTIFICATIONS_H */
