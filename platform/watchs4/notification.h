/* S4 Global 5.4.2 notification ABI. Kept separate from the S3 layout. */
#ifndef S4_NOTIFICATION_H
#define S4_NOTIFICATION_H
#include <stdint.h>

typedef struct {
    uint64_t message_id;
    uint32_t repeat_count;
    const char *title, *source, *body, *auxiliary_text;
    const char *small_icon_path, *large_icon_path;
    const char *extension_text_24, *extension_text_28;
    uint32_t timestamp;
    uint8_t reserved_30[8];
    void *action_callback;
    uint32_t action_context, extension_40, extension_44;
    void *open_callback, *destroy_callback;
    uint8_t start_reminder, flags_51, flags_52, reserved_53;
    void *callback_data;
} s4_notification_t;
#endif
