/* Host checks for installer identity, partial failures and notification order. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "platform/watchs4/notification.h"
#include "installer_identity.h"

typedef struct { int unused; } lv_timer_t;
typedef void (*timer_cb_t)(lv_timer_t *);
static lv_timer_t fake_timer;
static timer_cb_t queued;
static const char *current;
static int delete_calls, reset_calls, notify_calls, timer_deletes, present_calls;
static int delete_mode, timer_fail, notify_result, remains;
static int errors, checks;

static void expect(int ok, const char *description) {
    ++checks;
    if (!ok) { ++errors; fprintf(stderr, "FAIL: %s\n", description); }
}
static int s4_equal(const char *a, const char *b) { return a && b && !strcmp(a,b); }
static void fw_log(int level, const char *format, ...) { (void)level; (void)format; }
static const char *fw_current_face_id(void) { return current; }
static int fw_watchface_delete(const char *id) {
    ++delete_calls;
    expect(s4_equal(id,S4_INSTALLER_FACE_ID), "only delete our installer ID");
    if (delete_mode == 1) return 0; /* Firmware masks a failed deletion. */
    current = "normal-face";
    remains = delete_mode == 2; /* Switched face, but failed storage cleanup. */
    return 0;
}
static void *fw_watchface_lookup(const char *id) {
    expect(s4_equal(id,S4_INSTALLER_FACE_ID), "verify the deleted ID");
    return remains ? &fake_timer : NULL;
}
static void fw_watchface_reset(int force, int restart) {
    ++reset_calls;
    expect(force == 0 && restart == 1, "stock S4 reset arguments");
    expect(!s4_equal(current,S4_INSTALLER_FACE_ID), "reset only after face switch");
}
static lv_timer_t *lv_timer_create(timer_cb_t cb, uint32_t ms, void *data) {
    expect(ms == 2000 && !data, "wait two seconds after cleanup before the reminder");
    if (timer_fail) return NULL;
    queued = cb;
    return &fake_timer;
}
static void lv_timer_delete(lv_timer_t *timer) {
    expect(timer == &fake_timer, "delete the notification's own timer");
    ++timer_deletes;
}
static int fw_notification_insert(const s4_notification_t *msg) {
    ++notify_calls;
    expect(timer_deletes == 1, "timer removed before posting notification");
    expect(msg->message_id == UINT64_C(0xBE57BEEF), "upstream notification ID");
    expect(!strcmp(msg->title,"Voilà! 🎉") && !strcmp(msg->source,"Calculator") &&
           !strcmp(msg->body,"I'm in apps. Find me there."), "upstream success text");
    expect(msg->start_reminder == 0, "avoid duplicate automatic popup when opening explicitly");
    expect(!strcmp(msg->small_icon_path,"/data/s4_calc_logo_112.png") &&
           !strcmp(msg->large_icon_path,msg->small_icon_path), "persisted icon paths");
    expect(!msg->action_callback && !msg->open_callback && !msg->destroy_callback &&
           !msg->callback_data, "notification does not retain module callbacks");
    return notify_result;
}
static void fw_notification_present(uint64_t id, int foreground) {
    ++present_calls;
    expect(notify_calls == 1 && notify_result != 0, "open only after successful insertion");
    expect(id == UINT64_C(0xBE57BEEF) && foreground == 1, "open only our saved success card");
}
#include "installer_finish.h"

static void reset_test(const char *face) {
    current = face;
    delete_calls = reset_calls = notify_calls = timer_deletes = present_calls = 0;
    delete_mode = timer_fail = 0;
    notify_result = remains = 1;
    queued = NULL;
}
static void fire_notification(void) {
    expect(queued != NULL && notify_calls == 0, "no notification before queued timer fires");
    if (queued) { timer_cb_t cb = queued; queued = NULL; cb(&fake_timer); }
    expect(notify_calls == 1 && !queued, "exactly one success notification");
    expect(present_calls == (notify_result != 0), "one foreground open on success, none on insertion failure");
}
int main(void) {
    reset_test(S4_INSTALLER_FACE_ID);
    finish_install();
    expect(delete_calls == 1 && reset_calls == 1 && !remains, "remove installer and restore face");
    fire_notification();

    reset_test("unrelated-face");
    finish_install();
    expect(!delete_calls && !reset_calls, "preserve a face selected during installation");
    fire_notification();

    reset_test(NULL);
    finish_install();
    expect(!delete_calls && !reset_calls, "missing current face never triggers deletion");
    fire_notification();

    reset_test(S4_INSTALLER_FACE_ID);
    delete_mode = 1;
    finish_install();
    expect(delete_calls == 1 && !reset_calls && remains, "masked deletion failure does not reset current installer");
    fire_notification();

    reset_test(S4_INSTALLER_FACE_ID);
    delete_mode = 2;
    finish_install();
    expect(delete_calls == 1 && reset_calls == 1 && remains, "refresh after face switch even if storage cleanup failed");
    fire_notification();

    reset_test(S4_INSTALLER_FACE_ID);
    timer_fail = 1;
    finish_install();
    expect(!queued && !notify_calls, "failed timer allocation does not call through NULL");

    reset_test("unrelated-face");
    notify_result = 0;
    finish_install();
    fire_notification();
    expect(!delete_calls && notify_calls == 1, "notification failure does not repeat cleanup or insertion");
    printf("%d installer checks, %d failures\n",checks,errors);
    return errors ? 1 : 0;
}
