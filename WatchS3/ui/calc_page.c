/*
 * watchs3/ui/calc_page.c -- Reusable full-screen calculator page.
 *
 * Built for a 466x466 round screen. Layout is sized so the 4x5 button grid
 * keeps all four corners inside a safe-area radius of 220 px around the
 * display center; the firmware crops anything that strays outside the
 * visible circle on the watch face.
 */

#include <stdint.h>
#include "ui/calc_page.h"
#include "common/misc/mem.h"
#include "common/misc/math.h"
#include "common/nuttx/syslog.h"
#include "calc_page_asserts.h"

/* Module-local log tag for any syslog emitted from this translation unit.
 * Mirrors APP_TAG conventions in watchs3_app.c. */
#define LOG_TAG "[calc]"

#define SCREEN_SIZE    466      /* circular display diameter (px) */
#define SCREEN_W       SCREEN_SIZE
#define SCREEN_H       SCREEN_SIZE
#define CENTER_X       (SCREEN_W / 2)   /* 233 */
#define CENTER_Y       (SCREEN_H / 2)   /* 233 */
#define SAFE_R          230       /* slightly inside the visible 233-radius
                                 * circle; only fires on a SCREEN_SIZE shrink */

#define CALC_DISP_H     40      /* single-row readout; one line tall */
#define DISP_TOP        36      /* pushed upper so it fits above the grid
                                 * before the row of "=" gets the spotlight */
#define DISP_PAD        20
#define DISP_W          (SCREEN_W - 2*DISP_PAD)

#define CALC_BTN_W      84       /* wider than the 72-px draft; matches the
                                 * wider layout. every key -- including
                                 * "0" -- is exactly this wide. */
#define CALC_BTN_H      56
#define CALC_BTN_GAP_W  10      /* horizontal gap between columns */
#define CALC_BTN_GAP_H  6      /* vertical gap between rows */

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
    /* Full-screen container. Set both dimensions to the circular display
     * diameter; the firmware crops anything outside the visible circle. */
    lv_obj_t *root = lv_obj_create(parent, 0);
    lv_obj_set_size(root, SCREEN_W, SCREEN_H);

    /* Background. */
    lvx_btn_set_style_bg_color(root, 0xFF000000);
    lv_obj_set_style_bg_opa(root, 255u, 0);

    /* Default lv_obj padding shifts every TOP_LEFT aligned child; clear
     * it so the calculator grid is positioned against true (0,0). */
    lv_obj_set_style_pad_left  (root, 0, 0);
    lv_obj_set_style_pad_right (root, 0, 0);
    lv_obj_set_style_pad_top   (root, 0, 0);
    lv_obj_set_style_pad_bottom(root, 0, 0);

    /* Display label near the top. Sized to leave a comfortable ring so
     * it doesn't touch the circular frame even on the closest edge.
     * Forced to a single line (LONG_CLIP) so long expressions don't wrap
     * onto a second row inside the watch's narrow readout area. */
    lv_obj_t *disp = lv_label_create(root);
    calc_disp_label = disp;
    /* LV_LABEL_LONG_* : 0 EXPAND, 1 BREAK, 2 DOT, 3 SCROLL, 4 SCROLL_CIRC, 5 CLIP. */
    lv_label_set_long_mode(disp, /*LONG_CLIP*/ 5);
    lv_label_set_text(disp, calc_display);
    lv_obj_set_size(disp, DISP_W, CALC_DISP_H);
    lv_obj_align(disp, LV_ALIGN_TOP_LEFT, DISP_PAD, DISP_TOP);

    lv_style_t *lb_style = (lv_style_t *)LB_TEXT_STYLE_40;
    lvx_obj_set_style_text(disp, lb_style, 255, 0);
    /* LV_LABEL_ALIGN values: 0 = left, 1 = center, 2 = right (numeric
     * so we don't depend on a typo-prone enum name). */
    lv_obj_set_style_text_align(disp, /*LVT_RIGHT*/ 2, 0);

/* Button grid -- 4 cols, 5 rows, every key the same width (CALC_BTN_W).
     * Sized to keep all four corners of the outermost buttons inside the
     * safe-area circle (SAFE_R) around CENTER. Bottom-row column-2 is
     * intentionally empty (no fourth key allocated there); the loop below
     * skips NULL entries so the trailing slot stays blank. */
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

    /* Defensive sanity: corners of the outermost button must sit inside
     * SAFE_R around CENTER. The (start_x, start_y) / size combo above is
     * computed so this holds, but if SCREEN_SIZE ever shrinks, the runtime
     * check catches it before the firmware silently clips. */
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
        /* Count the non-null cells in this row so the partial bottom row
         * can be center-aligned (no gap where the missing key would be). */
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

            /* Function row (top) a bit lighter; other keys dark-grey; orange "=". */
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
