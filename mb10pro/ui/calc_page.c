/*
 * mb10pro/ui/calc_page.c -- Reusable full-screen calculator page.
 *
 * Builds a 336x480 calculator grid on the supplied parent. The page owns
 * its own state (display string, result flag) and exposes only a create
 * function plus a close-callback hook.
 */

#include <stdint.h>
#include "ui/calc_page.h"
#include "misc/mem.h"
#include "misc/math.h"
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
static lv_obj_t *calc_disp_label;          /* set during create */

/* Fractional-digit limit for display rendering. Matches the original
 * fixed-point precision (6 fractional digits) and keeps the output
 * buffer bounded regardless of the value's true magnitude. */
#define CALC_FRAC_DIGITS  6

/* ---- Forward decls ------------------------------------------------------- */
static void   calc_ui_rebuild(void);
static void   calc_on_press(lv_event_t *e);
static float  calc_evaluate(void);
static int    calc_precedence(char op);
static float  calc_apply_op(float a, float b, char op);
static int    calc_last_is_operator(void);
static int    calc_last_number_has_dot(void);
static void   calc_backspace(void);
static void   calc_append_char(char c);
static void   calc_format_value(float v, char *buf, size_t bufsz);

/* ---- Layout helpers ------------------------------------------------------ */
static void calc_ui_rebuild(void)
{
    if (calc_disp_label != 0)
        lv_label_set_text(calc_disp_label, calc_display);
}

static int calc_precedence(char op)
{
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}

static float calc_apply_op(float a, float b, char op)
{
    switch (op)
    {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return (b != 0.0f) ? a / b : 0.0f;
        default:  return 0.0f;
    }
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

static void calc_backspace(void)
{
    size_t n = strlen(calc_display);
    if (n > 0)
        calc_display[n - 1] = '\0';
    if (calc_display[0] == '\0')
        snprintf(calc_display, sizeof(calc_display), "0");
}

/* Write the unsigned 32-bit value n to the end of buf as decimal digits,
 * returns a pointer to the first digit written. Used to avoid pulling in
 * printf/long-long formatting paths. */
static char *calc_u32_to_str(char *buf_end, uint32_t n)
{
    char *p = buf_end;
    if (n == 0)
    {
        *--p = '0';
        return p;
    }
    while (n > 0)
    {
        *--p = (char)('0' + (n % 10u));
        n /= 10u;
    }
    return p;
}

static void calc_format_value(float v, char *buf, size_t bufsz)
{
    int negative = (v < 0.0f);
    if (negative) v = -v;

    /* Round to the display's fractional precision so binary-float
     * representation noise ("0.5000000001") doesn't leak into the UI,
     * and so the trailing-zero trim below is meaningful. */
    float scale   = math_pow10_f(CALC_FRAC_DIGITS);
    float rounded = math_round_f(v * scale) / scale;

    /* Clamp magnitude: float->int64 conversion would need __aeabi_f2lz
     * / __aeabi_f2ulz soft helpers, so we stay in uint32 / float land.
     * Anything outside [0, 1e15) won't fit a watch display anyway. */
    float max_int = math_pow10_f(CALC_FRAC_DIGITS + 9); /* 1e15 */
    if (rounded > max_int) rounded = max_int;

    /* Integer part via (uint32_t) -- vcvtr.u32.f32 is hardware on FPv5. */
    uint32_t int_part  = (uint32_t)rounded;
    uint32_t frac_part = (uint32_t)math_round_f((rounded - (float)int_part) * scale);

    /* Rounding could push the fraction to 1.0; carry into the integer. */
    if (frac_part >= (uint32_t)scale)
    {
        int_part++;
        frac_part = 0;
    }

    /* Build the string manually so we never call printf with %f/%lld
     * (those promote through double and pull in soft-float helpers). */
    char tmp[32];
    size_t t = 0;

    char *int_str = calc_u32_to_str(tmp + sizeof(tmp), int_part);
    size_t int_len = (size_t)((tmp + sizeof(tmp)) - int_str);
    for (size_t i = 0; i < int_len; ++i) tmp[t++] = int_str[i];

    if (frac_part != 0)
    {
        tmp[t++] = '.';

        char frac_buf[16];
        char *frac_str = calc_u32_to_str(frac_buf + sizeof(frac_buf), frac_part);
        /* Left-pad to CALC_FRAC_DIGITS characters so "0.5" stays
         * "0.5" rather than "0.500000". */
        size_t fd_len = (size_t)((frac_buf + sizeof(frac_buf)) - frac_str);
        size_t pad    = (size_t)CALC_FRAC_DIGITS - fd_len;
        for (size_t i = 0; i < pad; ++i) tmp[t++] = '0';
        for (size_t i = 0; i < fd_len; ++i) tmp[t++] = frac_str[i];

        /* Trim trailing zeros from the fractional part; drop a bare '.'. */
        while (t > 0 && tmp[t - 1] == '0') t--;
        if (t > 0 && tmp[t - 1] == '.') t--;     /* "<int>." -> "<int>" */
    }
    tmp[t] = '\0';

    /* Copy into the caller's buffer, prepending '-' for negatives. */
    size_t out = 0;
    if (negative && out + 1 < bufsz) buf[out++] = '-';
    for (size_t i = 0; i < t && out + 1 < bufsz; ++i)
        buf[out++] = tmp[i];
    buf[out] = '\0';
}

static float calc_evaluate(void)
{
    float values[16]; int val_top = 0;
    char  ops[16];    int op_top = 0;
    const char *p = calc_display;
    int    in_num   = 0;
    int    has_dot  = 0;
    int    frac_pos = 0;   /* number of fractional digits accumulated so far */
    float  current  = 0.0f; /* value of the number being parsed */

    while (*p)
    {
        char c = *p;
        if (c >= '0' && c <= '9')
        {
            float digit = (float)(c - '0');
            if (has_dot)
            {
                if (frac_pos < CALC_FRAC_DIGITS)
                {
                    frac_pos++;
                    current += digit / math_pow10_f(frac_pos);
                }
                /* digits beyond CALC_FRAC_DIGITS are dropped */
            }
            else
            {
                current = current * 10.0f + digit;
            }
            in_num = 1;
        }
        else if (c == '.')
        {
            has_dot = 1;
            in_num = 1;
        }
        else if (c == '+' || c == '-' || c == '*' || c == '/')
        {
            if (in_num)
            {
                values[val_top++] = current;
                current  = 0.0f;
                has_dot  = 0;
                frac_pos = 0;
            }
            in_num = 0;

            while (op_top > 0 &&
                   calc_precedence(ops[op_top - 1]) >= calc_precedence(c))
            {
                float b = values[--val_top];
                float a = values[--val_top];
                char op = ops[--op_top];
                values[val_top++] = calc_apply_op(a, b, op);
            }
            ops[op_top++] = c;
        }
        p++;
    }

    if (in_num)
        values[val_top++] = current;

    while (op_top > 0)
    {
        if (val_top < 2)
            break;
        float b = values[--val_top];
        float a = values[--val_top];
        char op = ops[--op_top];
        values[val_top++] = calc_apply_op(a, b, op);
    }

    return (val_top > 0) ? values[0] : 0.0f;
}

/* ---- Event handlers ------------------------------------------------------ */
static void calc_on_press(lv_event_t *e)
{
    /* Filter: ignore anything that isn't a clicked event. */
    if (lv_obj_get_event_code((lv_obj_t *)e) != LV_EVENT_CLICKED)
        return;

    const char *label = (const char *)lv_event_get_user_data(e);
    char c = label[0];

    if (c >= '0' && c <= '9')
    {
        if (calc_result_shown)
        {
            snprintf(calc_display, sizeof(calc_display), "0");
            calc_result_shown = 0;
        }
        if (calc_display[0] == '0' && calc_display[1] == '\0')
            calc_display[0] = c;
        else
            calc_append_char(c);
    }
    else if (c == '.')
    {
        if (calc_result_shown)
        {
            snprintf(calc_display, sizeof(calc_display), "0");
            calc_result_shown = 0;
        }
        if (calc_last_is_operator())
            calc_append_char('0');
        if (!calc_last_number_has_dot())
            calc_append_char('.');
    }
    else if (c == 'C')
    {
        snprintf(calc_display, sizeof(calc_display), "0");
        calc_result_shown = 1;
    }
    else if (c == '<')              /* "<=" backspace */
    {
        if (calc_result_shown)
        {
            snprintf(calc_display, sizeof(calc_display), "0");
            calc_result_shown = 1;
        }
        else
        {
            calc_backspace();
        }
    }
    else if (c == '+' || c == '-' || c == '*' || c == '/')
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
    else if (c == '=')
    {
        if (calc_last_is_operator())
        {
            size_t n = strlen(calc_display);
            calc_display[n - 1] = '\0';
        }
        if (strlen(calc_display) > 0)
        {
            float result = calc_evaluate();
            calc_format_value(result, calc_display, sizeof(calc_display));
            calc_result_shown = 1;
        }
    }

    calc_ui_rebuild();
}

/* ---- Public API ---------------------------------------------------------- */
lv_obj_t *calc_page_create(lv_obj_t *parent)
{
    /* Full-screen container. */
    lv_obj_t *root = lv_obj_create(parent, 0);
    lv_obj_set_size(root, SCREEN_W, SCREEN_H);

    /* Background. */
    lvx_obj_set_style_bg_color(root, 0xFF101018u);
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

            lv_obj_t *btn = lv_btn2_create(root);
            lv_obj_set_size(btn, w, CALC_BTN_H);
            lv_obj_align(btn, LV_ALIGN_TOP_LEFT, x,
                         start_y + r * (CALC_BTN_H + CALC_BTN_GAP));
            lv_btn_set_text(btn, lbl);

            /* Function row (top) a bit lighter; other keys dark-grey;
             * orange "=". */
            if (lbl[0] == '=')
                lvx_obj_set_style_bg_color(btn, 0xFF9500u);
            else if (r == 0)
                lvx_obj_set_style_bg_color(btn, 0x5C5C5Cu);
            else
                lvx_obj_set_style_bg_color(btn, 0x333333u);

            lv_obj_add_event_cb(btn, calc_on_press, LV_EVENT_CLICKED,
                                (void *)lbl);
        }
    }

    return root;
}
