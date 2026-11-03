/* Include after platform/miwear_system.h (or host firmware substitutes).
 * Only called from the UI thread after Calculator registration succeeds.
 */
#ifndef S4_INSTALLER_FINISH_H
#define S4_INSTALLER_FINISH_H
#include "platform/watchs4/notification.h"
#include "installer_identity.h"

static void show_install_notification(lv_timer_t *timer) {
    lv_timer_delete(timer);
    static s4_notification_t done;
    done.message_id = UINT64_C(0xBE57BEEF);
    done.title = "Voilà! 🎉";
    done.source = "Calculator";
    done.body = "I'm in apps. Find me there.";
    done.small_icon_path = "/data/s4_calc_logo_112.png";
    done.large_icon_path = "/data/s4_calc_logo_112.png";
    /* Save once, then use the stock foreground-open path for this explicit
     * installation confirmation. Do not also enqueue an automatic popup. */
    done.start_reminder = 0;
    const int inserted = fw_notification_insert(&done);
    fw_log(inserted ? 4 : 3, "[s4_calculator] success notification inserted=%d\n", inserted);
    if (inserted) fw_notification_present(done.message_id, 1);
}

static void finish_install(void) {
    /* A user may change faces while registration is pending. Never delete
     * whichever face happens to be current without checking our exact ID.
     * Use our own ID literal: deleting a face frees its old entry/ID. */
    if (s4_equal(fw_current_face_id(), S4_INSTALLER_FACE_ID)) {
        const int rc = fw_watchface_delete(S4_INSTALLER_FACE_ID);
        /* The manager may return zero even when inner deletion fails.
         * Refresh if it switched the current face, including partial cleanup. */
        if (!s4_equal(fw_current_face_id(), S4_INSTALLER_FACE_ID))
            fw_watchface_reset(0, 1);
        fw_log(4, "[s4_calculator] installer delete rc=%d remains=%d\n",
               rc, fw_watchface_lookup(S4_INSTALLER_FACE_ID) != 0);
    } else {
        fw_log(4, "[s4_calculator] installer not current; cleanup skipped\n");
    }
    /* Reset queues UI work beyond its return. Allow the face transition to
     * settle before requesting the popup; r4's popup was missing during reset. */
    if (!lv_timer_create(show_install_notification, 2000, 0))
        fw_log(3, "[s4_calculator] notification timer allocation failed\n");
}
#endif
