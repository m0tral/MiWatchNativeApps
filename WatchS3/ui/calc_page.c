/*
 * watchs3/ui/calc_page.c -- Reusable full-screen calculator page.
 *
 * Built for a 466x466 round screen. Layout is sized so the 4x5 button
 * grid keeps all four corners inside a safe-area radius of 220 px
 * around the display center; the firmware crops anything that strays
 * outside the visible circle on the watch face.
 *
 * The state machine (display string, stored, pending op, %) lives in
 * calc_state.c and is host-testable; this file is just the lvgl UI
 * glue + per-button op highlighting.
 */

#include <stdint.h>
#include "ui/calc_page.h"
#include "ui/calc_state.h"
#include "common/misc/mem.h"
#include "common/misc/math.h"
#include "common/nuttx/syslog.h"
#include "calc_page_asserts.h"

#define LOG_TAG "[calc]"

#define SCREEN_SIZE    466
#define SCREEN_W       SCREEN_SIZE
#define SCREEN_H       SCREEN_SIZE
#define CENTER_X       (SCREEN_W / 2)
#define CENTER_Y       (SCREEN_H / 2)
#define SAFE_R         230

#define CALC_DISP_H    40
#define DISP_TOP       36
#define DISP_PAD       40                          /* readout 80 px narrower than screen */
#define DISP_W         (SCREEN_W - 2*DISP_PAD)

#define CALC_BTN_W     84
#define CALC_BTN_H     56
#define CALC_BTN_GAP_W 10
#define CALC_BTN_GAP_H 6

#define LV_EVENT_CLICKED 7u

/* ---- UI-only state ------------------------------------------------------- */
static lv_obj_t   *calc_disp_label;

/* The four binary op buttons (/ * - +) -- populated at calc_page_create.
 * Indexed by op_to_idx() below. '%' is excluded: it's a unary postfix
 * action, not a pending state, so it doesn't get a highlight. */
static lv_obj_t   *calc_op_btns[4];
static const uint32_t calc_op_active_bg   = 0x7F4A00u;  /* 50% darker than "=" 0xFF9500 */
static const uint32_t calc_op_inactive_bg = 0x333333u;

static int op_to_idx(char op)
{
    switch (op) {
        case '/': return 0;
        case '*': return 1;
        case '-': return 2;
        case '+': return 3;
        default:  return -1;
    }
}

static void calc_clear_op_highlight(void)
{
    for (int i = 0; i < 4; ++i)
        if (calc_op_btns[i] != 0)
            lvx_btn_set_style_bg_color(calc_op_btns[i], calc_op_inactive_bg);
}

static void calc_set_op_highlight(char op)
{
    int idx = op_to_idx(op);
    if (idx < 0) return;
    calc_clear_op_highlight();
    if (calc_op_btns[idx] != 0)
        lvx_btn_set_style_bg_color(calc_op_btns[idx], calc_op_active_bg);
}

/* Push the state machine's display string into the LVGL label. */
static void calc_ui_rebuild(void)
{
    if (calc_disp_label != 0)
        lv_label_set_text(calc_disp_label, calc_state_display());
}

/* After each key press, re-sync the highlighted op button with the
 * state machine's pending_op. */
static void calc_ui_refresh_op_highlight(void)
{
    calc_clear_op_highlight();
    calc_set_op_highlight(calc_state_pending_op());
}

/* ---- Event handler ------------------------------------------------------- */
static void calc_on_press(lv_event_t *e)
{
    if (lv_obj_get_event_code((lv_obj_t *)e) != LV_EVENT_CLICKED)
        return;

    const char *label = (const char *)lv_event_get_user_data(e);
    calc_state_press_key(label[0]);

    calc_ui_rebuild();
    calc_ui_refresh_op_highlight();
}

/* ---- Public API ---------------------------------------------------------- */
lv_obj_t *calc_page_create(lv_obj_t *parent)
{
    lv_obj_t *root = lv_obj_create(parent, 0);
    lv_obj_set_size(root, SCREEN_W, SCREEN_H);

    lvx_btn_set_style_bg_color(root, 0xFF000000);
    lv_obj_set_style_bg_opa(root, 255u, 0);

    /* Clear default lv_obj padding so TOP_LEFT aligned children sit at true (0,0). */
    lv_obj_set_style_pad_left  (root, 0, 0);
    lv_obj_set_style_pad_right (root, 0, 0);
    lv_obj_set_style_pad_top   (root, 0, 0);
    lv_obj_set_style_pad_bottom(root, 0, 0);

    /* Readout: single-line (LONG_CLIP=5) so long expressions don't wrap. */
    lv_obj_t *disp = lv_label_create(root);
    calc_disp_label = disp;
    lv_label_set_long_mode(disp, /*LONG_CLIP*/ 5);
    lv_label_set_text(disp, calc_state_display());
    lv_obj_set_size(disp, DISP_W, CALC_DISP_H);
    lv_obj_align(disp, LV_ALIGN_TOP_LEFT, DISP_PAD, DISP_TOP);

    lv_style_t *lb_style = (lv_style_t *)LB_TEXT_STYLE_40;
    lvx_obj_set_style_text(disp, lb_style, 255, 0);
    lv_obj_set_style_text_align(disp, /*LVT_RIGHT*/ 2, 0);

    /* 4 cols x 5 rows; row 4 col 1 (".", the missing key) is the gap. */
    static const char *const labels[5][4] = {
        { "C",  "<=",  "%",  "/"  },
        { "7",  "8",   "9",  "*"  },
        { "4",  "5",   "6",  "-"  },
        { "1",  "2",   "3",  "+"  },
        { "0",  ".",          "="  },
    };

    lv_coord_t grid_w = 4 * CALC_BTN_W + 3 * CALC_BTN_GAP_W;
    lv_coord_t grid_h = 5 * CALC_BTN_H + 4 * CALC_BTN_GAP_H;
    lv_coord_t start_x = CENTER_X - grid_w / 2;
    lv_coord_t start_y = DISP_TOP + CALC_DISP_H + (SCREEN_H - (DISP_TOP + CALC_DISP_H) - grid_h) / 2;

    /* Defensive sanity: corners of outermost button must sit inside SAFE_R. */
    {
        const lv_coord_t right_x = start_x + grid_w;
        const lv_coord_t bottom_y = start_y + grid_h;
        const int dx = right_x - CENTER_X;
        const int dy = bottom_y - CENTER_Y;
        const int r2 = dx*dx + dy*dy;
        const int safe2 = SAFE_R * SAFE_R;
        if (r2 > safe2) {
            syslog(LOG_ERR, "[%s] grid corner (%d,%d) outside safe r=%d",
                   LOG_TAG, dx, dy, SAFE_R);
        }
    }

    for (int r = 0; r < 5; ++r)
    {
        /* Center-align the partial bottom row (no fourth key). */
        int row_count = 0;
        for (int c = 0; c < 4; ++c) {
            const char *lbl = labels[r][c];
            if (lbl != 0 && lbl[0] != '\0') row_count++;
        }
        const lv_coord_t row_w =
            row_count * CALC_BTN_W + (row_count > 0 ? (row_count - 1) * CALC_BTN_GAP_W : 0);
        const lv_coord_t row_start_x = CENTER_X - row_w / 2;
        const lv_coord_t col_stride  = CALC_BTN_W + CALC_BTN_GAP_W;

        for (int c = 0; c < 4; ++c)
        {
            const char *lbl = labels[r][c];
            if (lbl == 0 || lbl[0] == '\0') continue;

            const lv_coord_t x = row_start_x + c * col_stride;
            const lv_coord_t y = start_y + r * (CALC_BTN_H + CALC_BTN_GAP_H);

            lv_obj_t *btn = lvx_btn_create(root);
            lv_obj_set_size(btn, CALC_BTN_W, CALC_BTN_H);
            lv_obj_align(btn, LV_ALIGN_TOP_LEFT, x, y);
            lvx_btn_set_text_fmt(btn, lbl);

            if      (lbl[0] == '=') lvx_btn_set_style_bg_color(btn, 0xFF9500u);
            else if (r == 0)        lvx_btn_set_style_bg_color(btn, 0x5C5C5Cu);
            else                    lvx_btn_set_style_bg_color(btn, 0x333333u);

            /* Capture the four binary op buttons so we can highlight the
             * active one. '%' is excluded -- unary postfix, not a state. */
            if (r >= 1 && r <= 4 && c == 3) {
                int idx = op_to_idx(lbl[0]);
                if (idx >= 0) calc_op_btns[idx] = btn;
            }

            lv_obj_add_event_cb(btn, calc_on_press, LV_EVENT_CLICKED, (void *)lbl);
        }
    }

    return root;
}
