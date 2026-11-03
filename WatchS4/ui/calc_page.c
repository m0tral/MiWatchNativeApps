/* Adapted from m0tral's WatchS3/ui/calc_page.c, MIT (see repository LICENSE).
 * S4-specific binding/enum fixes and complete operator highlighting.
 */
#include "platform/miwear_system.h"
#include "calc_state.h"

static lv_obj_t *display_label;
static lv_obj_t *operator_buttons[4];
static const char operators[] = "/*-+";

static void ui_reset_pointers(void) {
    display_label = 0;
    for (int i=0; i<4; ++i) operator_buttons[i] = 0;
}

static void ui_refresh(void) {
    if (display_label) lv_label_set_text(display_label, calc_state_display());
    for (int i=0; i<4; ++i) {
        if (!operator_buttons[i]) continue;
        uint32_t color = calc_state_pending_op() == operators[i] ? 0x7f4a00u :
                         (i == 0 ? 0x5c5c5cu : 0x333333u);
        lvx_btn_set_style_bg_color(operator_buttons[i], color);
    }
}

static void on_key(lv_event_t *event) {
    if (lv_event_get_code(event) != S4_CLICKED) return;
    const char *key = lv_event_get_user_data(event);
    if (!key || !*key) return;
    calc_state_press_key(*key);
    ui_refresh();
}

static int ui_create(lv_obj_t *parent) {
    ui_reset_pointers();
    calc_state_press_key('C');
    lv_obj_t *root = lv_obj_create(parent, 0);
    if (!root) return -12;
    lv_obj_set_size(root, 466, 466);
    lv_obj_set_style_bg_color(root, 0x000000u, 0);
    lv_obj_set_style_bg_opa(root, 255, 0);
    lv_obj_set_style_pad_top(root, 0, 0);
    lv_obj_set_style_pad_bottom(root, 0, 0);
    lv_obj_set_style_pad_left(root, 0, 0);
    lv_obj_set_style_pad_right(root, 0, 0);
    display_label = lv_label_create(root);
    if (!display_label) return -12;
    lv_label_set_long_mode(display_label, S4_LONG_CLIP);
    /* At y=60 the round screen is narrower than its 466px bounding box.
     * All four corners of this readout lie inside a 230px safe radius. */
    lv_obj_set_size(display_label, 290, 46);
    lv_obj_align(display_label, S4_TOP_LEFT, 88, 60);
    lv_obj_add_style(display_label, S4_FONT40_STYLE, 0);
    lv_obj_set_style_text_color(display_label, 0xffffffu, 0);

    static const char *const keys[5][4] = {
        {"C", "<=", "%", "/"}, {"7", "8", "9", "*"},
        {"4", "5", "6", "-"}, {"1", "2", "3", "+"},
        {"0", ".", "=", 0}
    };
    for (int row=0; row<5; ++row) {
        const int count = row == 4 ? 3 : 4;
        const int left = (466 - (count*84 + (count-1)*10))/2;
        for (int col=0; col<count; ++col) {
            const char *key = keys[row][col];
            lv_obj_t *button = lvx_btn_create(root);
            if (!button) return -12;
            lv_obj_set_size(button, 84, 56);
            lv_obj_align(button, S4_TOP_LEFT, left+col*94, 114+row*62);
            lvx_btn_set_text_fmt(button, "%s", key);
            lvx_btn_set_style_bg_color(button, *key=='=' ? 0xff9500u :
                                              (row==0 ? 0x5c5c5cu : 0x333333u));
            for (int i=0; i<4; ++i) if (*key==operators[i]) operator_buttons[i]=button;
            if (!lv_obj_add_event_cb(button, on_key, S4_CLICKED, (void *)key)) return -12;
        }
    }
    ui_refresh();
    return 0;
}
