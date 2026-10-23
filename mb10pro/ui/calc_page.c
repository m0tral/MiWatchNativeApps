/*
 * mb10pro/ui/calc_page.c -- Reusable full-screen calculator page.
 *
 * Builds a 336x480 calculator grid on the supplied parent. The page owns
 * its own state (display string, result flag) and exposes only a create
 * function plus a close-callback hook.
 */

#include <stdint.h>
#include "ui/calc_page.h"
#include "common/misc/mem.h"
#include "common/misc/decimal.h"
#include "platform/miwear_system.h"

#define SCREEN_W       336
#define SCREEN_H       480
#define CALC_BTN_W      76
#define CALC_BTN_H      60
#define CALC_BTN_GAP     8
#define CALC_DISP_H     90

#define LV_EVENT_CLICKED 7u

/* ---- Page state ---------------------------------------------------------- */
static char   calc_display[32] = "0";
static int    calc_result_shown = 1;       /* display currently holds a result */
static int    calc_error_shown  = 0;       /* ...and that result is an error   */
static lv_obj_t *calc_disp_label;          /* set during create */

/* Arithmetic is decimal (common/misc/decimal.h): literals are exact, every
 * + - * / is rounded once to 15 significant digits (ties away from zero),
 * |result| < 10^15. See the precision contract there. */

/* ---- Forward decls ------------------------------------------------------- */
static void   calc_ui_rebuild(void);
static void   calc_on_press(lv_event_t *e);
static void   calc_handle_key(char c);
static int    calc_last_is_operator(void);
static int    calc_last_number_has_dot(void);
static void   calc_backspace(void);
static void   calc_append_char(char c);
static void   calc_set_display(const char *s);
static void   calc_show_result(void);

/* ---- Layout helpers ------------------------------------------------------ */
static void calc_ui_rebuild(void)
{
    if (calc_disp_label != 0)
        lv_label_set_text(calc_disp_label, calc_display);
}

static int calc_last_is_operator(void)
{
    size_t n = strlen(calc_display);
    if (n == 0) return 0;
    char c = calc_display[n - 1];
    return c == '+' || c == '-' || c == '*' || c == '/';
}

static int calc_last_number_has_dot(void)
{
    size_t n = strlen(calc_display);
    for (size_t i = n; i > 0; --i)
    {
        char c = calc_display[i - 1];
        if (c == '+' || c == '-' || c == '*' || c == '/')
            return 0;
        if (c == '.')
            return 1;
    }
    return 0;
}

static void calc_append_char(char c)
{
    size_t n = strlen(calc_display);
    if (n + 1 < sizeof(calc_display))
    {
        calc_display[n] = c;
        calc_display[n + 1] = '\0';
    }
}

static void calc_set_display(const char *s)
{
    size_t i = 0;
    for (; s[i] != '\0' && i + 1 < sizeof(calc_display); ++i)
        calc_display[i] = s[i];
    calc_display[i] = '\0';
}

static void calc_backspace(void)
{
    size_t n = strlen(calc_display);
    if (n > 0)
        calc_display[n - 1] = '\0';
    if (calc_display[0] == '\0')
        calc_set_display("0");
}

/* Evaluate the display and replace it with the result or an error text. */
static void calc_show_result(void)
{
    dec_t v;
    int st = dec_eval(calc_display, &v);
    if (st == DEC_OK)
        dec_format(&v, calc_display, sizeof(calc_display));
    else
        calc_set_display(st == DEC_ERR_OVERFLOW ? "Overflow" : "Error");
    calc_error_shown = (st != DEC_OK);
    calc_result_shown = 1;
}

/* ---- Key handling (pure state machine, no LVGL) --------------------------- */
static void calc_handle_key(char c)
{
    if (c >= '0' && c <= '9')
    {
        if (calc_result_shown)
        {
            calc_set_display("0");
            calc_result_shown = 0;
            calc_error_shown = 0;
        }
        if (calc_display[0] == '0' && calc_display[1] == '\0')
            calc_display[0] = c;
        else if (dec_literal_accepts_digit(calc_display, c))
            calc_append_char(c);
        /* else: the number already has 15 significant or 15 fractional
         * digits; ignore the key rather than lose a digit later */
    }
    else if (c == '.')
    {
        if (calc_result_shown)
        {
            calc_set_display("0");
            calc_result_shown = 0;
            calc_error_shown = 0;
        }
        if (calc_last_is_operator())
            calc_append_char('0');
        if (!calc_last_number_has_dot())
            calc_append_char('.');
    }
    else if (c == 'C')
    {
        calc_set_display("0");
        calc_result_shown = 1;
        calc_error_shown = 0;
    }
    else if (c == '<')              /* "<=" backspace */
    {
        if (calc_result_shown)
        {
            calc_set_display("0");
            calc_result_shown = 1;
            calc_error_shown = 0;
        }
        else
        {
            calc_backspace();
        }
    }
    else if (c == '+' || c == '-' || c == '*' || c == '/')
    {
        if (calc_error_shown)
        {
            /* no operand to continue from: ignore */
        }
        else
        {
            if (calc_result_shown)
                calc_result_shown = 0;

            if (strlen(calc_display) == 0)
            {
                /* ignore operator with no preceding number */
            }
            else if (calc_last_is_operator())
            {
                size_t n = strlen(calc_display);
                calc_display[n - 1] = c;
            }
            else
            {
                calc_append_char(c);
            }
        }
    }
    else if (c == '=')
    {
        if (!calc_error_shown)
        {
            if (calc_last_is_operator())
            {
                size_t n = strlen(calc_display);
                calc_display[n - 1] = '\0';
            }
            if (strlen(calc_display) > 0)
                calc_show_result();
            else
                calc_set_display("0");
        }
    }
}

/* ---- Event handlers ------------------------------------------------------ */
static void calc_on_press(lv_event_t *e)
{
    /* Filter: ignore anything that isn't a clicked event. */
    if (lv_obj_get_event_code((lv_obj_t *)e) != LV_EVENT_CLICKED)
        return;

    const char *label = (const char *)lv_event_get_user_data(e);
    calc_handle_key(label[0]);
    calc_ui_rebuild();
}

/* ---- Public API ---------------------------------------------------------- */
lv_obj_t *calc_page_create(lv_obj_t *parent)
{
    /* Full-screen container. */
    lv_obj_t *root = lv_obj_create(parent, 0);
    lv_obj_set_size(root, SCREEN_W, SCREEN_H);

    /* Background. */
    lvx_btn_set_style_bg_color(root, 0xFF101018u);
    lv_obj_set_style_bg_opa(root, 255u, 0);

    /* Default lv_obj padding shifts every TOP_LEFT aligned child; clear
     * it so the calculator grid is positioned against true (0,0). */
    lv_obj_set_style_pad_left  (root, 0, 0);
    lv_obj_set_style_pad_right (root, 0, 0);
    lv_obj_set_style_pad_top   (root, 0, 0);
    lv_obj_set_style_pad_bottom(root, 0, 0);

    /* Display label at the top. Spans full screen width with a uniform
     * left+right gap so it doesn't kiss either edge. */
    lv_obj_t *disp = lv_label_create(root);
    calc_disp_label = disp;
    lv_label_set_text(disp, calc_display);
    lv_obj_set_size(disp, SCREEN_W - 16, CALC_DISP_H);
    lv_obj_align(disp, LV_ALIGN_TOP_RIGHT, -8, 12);

    lv_style_t *lb_style = (lv_style_t *)LB_TEXT_STYLE_1;
    lvx_obj_set_style_text(disp, lb_style, 255, 0);
    lv_obj_set_style_text_align(disp, LV_ALIGN_TOP_RIGHT, 0);

    /* Button grid -- 4 cols, 5 rows. The "0" key spans two columns. */
    static const char *const labels[5][4] = {
        { "C",  "<=",  "%",  "/"  },
        { "7",  "8",   "9",  "*"  },
        { "4",  "5",   "6",  "-"  },
        { "1",  "2",   "3",  "+"  },
        { "0",  "",    ".",  "="  },
    };

    lv_coord_t total_w = 4 * CALC_BTN_W + 3 * CALC_BTN_GAP;
    lv_coord_t start_x = (SCREEN_W - total_w) / 2;
    lv_coord_t start_y = CALC_DISP_H + 24;

    for (int r = 0; r < 5; ++r)
    {
        for (int c = 0; c < 4; ++c)
        {
            const char *lbl = labels[r][c];
            if (lbl[0] == '\0') continue;

            lv_coord_t w = CALC_BTN_W;
            if (r == 4 && c == 0)
                w = 2 * CALC_BTN_W + CALC_BTN_GAP;

            lv_coord_t x = start_x + c * (CALC_BTN_W + CALC_BTN_GAP);
            if (r == 4 && c == 0)
            {
                /* Wide "0" spans columns 0-1; anchor it flush-left. */
                x = start_x;
            }

            lv_obj_t *btn = lvx_btn_create(root);
            lv_obj_set_size(btn, w, CALC_BTN_H);
            lv_obj_align(btn, LV_ALIGN_TOP_LEFT, x,
                         start_y + r * (CALC_BTN_H + CALC_BTN_GAP));
            lvx_btn_set_text_fmt(btn, lbl);

            /* Function row (top) a bit lighter; other keys dark-grey;
             * orange "=". */
            if (lbl[0] == '=')
                lvx_btn_set_style_bg_color(btn, 0xFF9500u);
            else if (r == 0)
                lvx_btn_set_style_bg_color(btn, 0x5C5C5Cu);
            else
                lvx_btn_set_style_bg_color(btn, 0x333333u);

            lv_obj_add_event_cb(btn, calc_on_press, LV_EVENT_CLICKED,
                                (void *)lbl);
        }
    }

    return root;
}
