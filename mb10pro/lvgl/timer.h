#ifndef MB10P_TIMER_H
#define MB10P_TIMER_H

/* mb10pro LVGL timer API typedefs.
 *
 * The actual function implementations live in the apps/graphics/lvgl
 * region of the firmware (vela_ap.bin) and are reached via Thumb
 * thunks. The Thunk addresses and their `#define`s (with |1 Thumb
 * bit set) live in mb10p_platform.h. This header owns only the C
 * signatures -- call sites can include just this header when they
 * need to declare local function pointers of these types without
 * pulling in the address bindings.
 */

#include <stdint.h>

typedef struct _lv_timer_t lv_timer_t;

/* Standard LVGL timer callback. Receives the user_data pointer that
 * was passed to lv_timer_create. */
typedef void (*lv_timer_cb_t)(lv_timer_t *timer);

/* lv_timer_create(timer_cb, period_ms, user_data) -> lv_timer_t* (handle).
 * Returns NULL on allocation failure. The firmware thunk takes only
 * these three args -- there is no separate repeat_count; one-shot
 * behaviour must be implemented in the callback (e.g. by calling
 * lv_timer_delete from the first invocation, or by consulting a
 * static guard flag). */
typedef void *(*lv_timer_create_t)(lv_timer_cb_t timer_cb,
                                   uint32_t period_ms,
                                   void *user_data);

/* lv_timer_delete(timer) -- removes the timer from lv_timer_handler's
 * pump list and frees the handle. Safe to call with a NULL pointer. */
typedef void (*lv_timer_delete_t)(void *timer);

#endif /* MB10P_TIMER_H */
