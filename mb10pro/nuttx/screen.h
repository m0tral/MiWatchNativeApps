#ifndef MB10P_SCREEN_H
#define MB10P_SCREEN_H

/* mb10pro screen power management (MiWearScreen module).
 *
 * Two function groups:
 *
 *   1. Top-level power:
 *        screen_turn_on / screen_turn_off / screen_set_keepon /
 *        screen_toggle_keepon_duration
 *      take a `tag` const-char* used by the firmware for syslog lines
 *      and wakeup-attribution. Pass a stable string literal.
 *
 *   2. Screen session API (operates on an opaque session struct):
 *        screen_session_start / stop / restore_default /
 *        screen_session_set_full_power / set_brightness_value /
 *        set_auto_brightness / set_keepon / set_touch_palm /
 *        set_wrist_drop
 *      The session handle's layout is intentionally NOT defined here --
 *      it lives in the firmware. The C API exposes it as an opaque
 *      pointer so a TU can hold one without knowing its fields.
 *
 * All thunks are Thumb; address bindings (with `| 1` Thumb bit) live
 * in platform/mb10p_platform.h.
 */

#include <stdint.h>

/* Opaque screen-session handle. Firmware allocates these; the C side
 * only ever passes them back through the setters. */
typedef struct screen_session screen_session_t;

/* ---- Top-level power ---- */

/* Bring the screen up. `param` is forwarded to the internal bring-up
 * routine -- typically 0 or a brightness hint, opaque to us.
 * Returns 0 on success, non-zero (firmware-specific) on failure or if
 * the system is shutting down (refused). */
int screen_turn_on(const char *tag, int param);

/* Tear down screen state. Returns the tag pointer on success. */
const char *screen_turn_off(const char *tag);

/* Toggle the screen-stays-on flag. `enable` is 0/1. Returns the tag. */
const char *screen_set_keepon(const char *tag, int enable);

/* Re-arm the keepon timer for `duration` units (firmware-specific).
 * Setting it to 0 disables keepon. Returns the syslog rc. */
int screen_toggle_keepon_duration(const char *tag, int duration);

/* ---- Session setters ---- */

/* Bind a session to the active list with `auto_stop` = 0/1 (whether
 * the session tears down on user inactivity). */
int screen_session_start(screen_session_t *session, int auto_stop);

/* Unbind a session from the active list. */
int screen_session_stop(screen_session_t *session);

/* Reset the session's brightness/auto-brightness/keepon/touch-palm/
 * wrist-drop fields to firmware defaults. */
int screen_session_restore_default(screen_session_t *session);

/* Force full-power (no auto-dim) for this session. `status` is 0/1. */
int screen_session_set_full_power(screen_session_t *session, int status);

/* Set the session's manual brightness value (0..N, firmware-specific). */
int screen_session_set_brightness_value(screen_session_t *session, int value);

/* Toggle auto-brightness for this session. `status` is 0/1. */
int screen_session_set_auto_brightness(screen_session_t *session, int status);

/* Toggle keepon for this session. `status` is 0/1. */
int screen_session_set_keepon(screen_session_t *session, int status);

/* Toggle touch-palm wakeup for this session. `status` is 0/1. */
int screen_session_set_touch_palm(screen_session_t *session, int status);

/* Toggle wrist-drop wakeup for this session. `status` is 0/1. */
int screen_session_set_wrist_drop(screen_session_t *session, int status);

#endif /* MB10P_SCREEN_H */
