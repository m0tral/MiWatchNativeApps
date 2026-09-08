#ifndef XIAOMI_UI_CALC_STATE_H
#define XIAOMI_UI_CALC_STATE_H

/* watchs3 calculator state machine -- header-only, host-testable.
 *
 * Pure C, no lvgl, no syslog, no platform bindings. Every translation
 * unit that includes this header gets its own private copy of the
 * state (because all state and helpers are `static`). The UI layer
 * (calc_page.c) and host tests (calc_state_test.c) each drive their
 * own instance; no cross-TU state to worry about.
 *
 * Keys match the keypad labels:
 *   '0'..'9'  digits
 *   '.'       decimal point
 *   'C'       clear all
 *   '<'       backspace
 *   '%'       unary postfix percentage
 *   '+' '-' '*' '/'   binary operators
 *   '='       commit pending op
 *
 * '%' semantics (matches the standard pocket-calculator behaviour):
 *   x + y%  ==  x + (x * y / 100)
 *   x - y%  ==  x - (x * y / 100)
 *   x * y%  ==  x * (y / 100)
 *   x / y%  ==  x / (y / 100)
 */

#include <stddef.h>
#include <stdint.h>
#include "common/misc/mem.h"
#include "common/misc/math.h"

/* No libc imports here on purpose. The only places we used to call
 * snprintf were `calc_state_reset()` and `calc_backspace()`, both of
 * which were formatting the literal "0" into the display buffer. We
 * do that with direct character assignment below -- no stdio.h, no
 * va_list, no float-formatter. Everything else (strlen via mem.h,
 * math via math.h) is statically inlined. */

#define CALC_DISP_BUFSZ  32
#define CALC_FRAC_DIGITS 6

/* ---- State (per-TU private) --------------------------------------------- */
static char  calc_display[CALC_DISP_BUFSZ] = "0";
static int   calc_result_shown    = 1;
static float calc_stored          = 0.0f;
static char  calc_pending_op      = 0;
static float calc_percent_base    = 0.0f;
static int   calc_pending_modulo  = 0;
static int   calc_rhs_committable = 0;

/* ---- Accessors ----------------------------------------------------------- */
static inline const char *calc_state_display(void)          { return calc_display; }
static inline float       calc_state_stored(void)           { return calc_stored; }
static inline char        calc_state_pending_op(void)       { return calc_pending_op; }
static inline int         calc_state_pending_modulo(void)    { return calc_pending_modulo; }
static inline int         calc_state_rhs_committable(void)   { return calc_rhs_committable; }

static inline void calc_state_reset(void)
{
    calc_display[0] = '0';
    calc_display[1] = '\0';
    calc_result_shown    = 1;
    calc_stored          = 0.0f;
    calc_pending_op      = 0;
    calc_percent_base    = 0.0f;
    calc_pending_modulo  = 0;
    calc_rhs_committable = 0;
}

/* ---- Helpers ------------------------------------------------------------- */
static inline int calc_last_is_operator(void)
{
    return calc_display[0] == '\0';
}

static inline int calc_last_number_has_dot(void)
{
    for (const char *p = calc_display; *p; ++p)
        if (*p == '.') return 1;
    return 0;
}

static inline void calc_append_char(char c)
{
    size_t n = strlen(calc_display);
    if (n + 1 < sizeof(calc_display)) {
        calc_display[n]     = c;
        calc_display[n + 1] = '\0';
    }
}

static inline void calc_backspace(void)
{
    size_t n = strlen(calc_display);
    if (n > 0) calc_display[n - 1] = '\0';
    if (calc_display[0] == '\0') {
        calc_display[0] = '0';
        calc_display[1] = '\0';
    }
}

static inline char *calc_u32_to_str(char *buf_end, uint32_t n)
{
    char *p = buf_end;
    if (n == 0) { *--p = '0'; return p; }
    while (n > 0) { *--p = (char)('0' + (n % 10u)); n /= 10u; }
    return p;
}

static inline float calc_display_as_float(void)
{
    float v = 0.0f, frac = 0.0f, frac_scale = 1.0f;
    int seen_dot = 0, negative = 0;
    const char *p = calc_display;
    if (*p == '-') { negative = 1; p++; }
    while (*p) {
        char c = *p;
        if (c >= '0' && c <= '9') {
            int d = c - '0';
            if (seen_dot) { frac_scale *= 10.0f; frac += (float)d / frac_scale; }
            else          { v = v * 10.0f + (float)d; }
        } else if (c == '.' && !seen_dot) {
            seen_dot = 1;
        }
        p++;
    }
    return negative ? -(v + frac) : (v + frac);
}

static inline void calc_format_value(float v, char *buf, size_t bufsz)
{
    int negative = (v < 0.0f);
    if (negative) v = -v;

    float scale   = math_pow10_f(CALC_FRAC_DIGITS);
    float rounded = math_round_f(v * scale) / scale;
    float max_int = math_pow10_f(CALC_FRAC_DIGITS + 9); /* 1e15 */
    if (rounded > max_int) rounded = max_int;

    uint32_t int_part  = (uint32_t)rounded;
    uint32_t frac_part = (uint32_t)math_round_f((rounded - (float)int_part) * scale);
    if (frac_part >= (uint32_t)scale) { int_part++; frac_part = 0; }

    char tmp[32];
    size_t t = 0;
    char *int_str = calc_u32_to_str(tmp + sizeof(tmp), int_part);
    size_t int_len = (size_t)((tmp + sizeof(tmp)) - int_str);
    for (size_t i = 0; i < int_len; ++i) tmp[t++] = int_str[i];

    if (frac_part != 0) {
        tmp[t++] = '.';
        char frac_buf[16];
        char *frac_str = calc_u32_to_str(frac_buf + sizeof(frac_buf), frac_part);
        size_t fd_len = (size_t)((frac_buf + sizeof(frac_buf)) - frac_str);
        size_t pad    = (size_t)CALC_FRAC_DIGITS - fd_len;
        for (size_t i = 0; i < pad; ++i)   tmp[t++] = '0';
        for (size_t i = 0; i < fd_len; ++i) tmp[t++] = frac_str[i];

        while (t > 0 && tmp[t - 1] == '0') t--;
        if (t > 0 && tmp[t - 1] == '.') t--;
    }
    tmp[t] = '\0';

    size_t out = 0;
    if (negative && out + 1 < bufsz) buf[out++] = '-';
    for (size_t i = 0; i < t && out + 1 < bufsz; ++i) buf[out++] = tmp[i];
    buf[out] = '\0';
}

/* Apply the (modulo-aware) RHS to calc_stored using the OLD pending op. */
static inline void calc_apply_pending_to_stored(void)
{
    if (calc_pending_op == 0 || !calc_rhs_committable) return;

    float rhs = calc_display_as_float();
    if (calc_pending_modulo) {
        switch (calc_pending_op) {
            case '+':
            case '-':
                rhs = calc_percent_base * rhs / 100.0f;
                break;
            case '*':
            case '/':
                rhs = rhs / 100.0f;
                break;
        }
        calc_pending_modulo = 0;
    }

    switch (calc_pending_op) {
        case '+': calc_stored = calc_stored + rhs; break;
        case '-': calc_stored = calc_stored - rhs; break;
        case '*': calc_stored = calc_stored * rhs; break;
        case '/': calc_stored = (rhs != 0.0f) ? calc_stored / rhs : 0.0f; break;
    }
    calc_rhs_committable = 0;
}

/* ---- Key dispatch -------------------------------------------------------- */
static inline void calc_state_press_key(char c)
{
    if (c >= '0' && c <= '9')
    {
        if (calc_result_shown) {
            calc_display[0] = '0';
            calc_display[1] = '\0';
            calc_result_shown = 0;
        }
        calc_pending_modulo  = 0;
        calc_rhs_committable = 1;
        if (calc_display[0] == '0' && calc_display[1] == '\0')
            calc_display[0] = c;
        else
            calc_append_char(c);
    }
    else if (c == '.')
    {
        if (calc_result_shown) {
            calc_display[0] = '0';
            calc_display[1] = '\0';
            calc_result_shown = 0;
        }
        calc_pending_modulo  = 0;
        calc_rhs_committable = 1;
        if (calc_last_is_operator())
            calc_append_char('0');
        if (!calc_last_number_has_dot())
            calc_append_char('.');
    }
    else if (c == 'C')
    {
        calc_display[0] = '0';
        calc_display[1] = '\0';
        calc_result_shown    = 1;
        calc_stored          = 0.0f;
        calc_pending_op      = 0;
        calc_percent_base    = 0.0f;
        calc_pending_modulo  = 0;
        calc_rhs_committable = 0;
    }
    else if (c == '<')                              /* backspace */
    {
        if (calc_result_shown) {
            calc_display[0] = '0';
            calc_display[1] = '\0';
            calc_result_shown = 1;
        } else {
            calc_backspace();
        }
        if (calc_pending_modulo) {
            int has_pct = 0;
            for (const char *p = calc_display; *p; ++p)
                if (*p == '%') { has_pct = 1; break; }
            if (!has_pct) calc_pending_modulo = 0;
        }
    }
    else if (c == '%')
    {
        size_t n = strlen(calc_display);
        char last = (n > 0) ? calc_display[n - 1] : '\0';
        if (last < '0' || last > '9') return;        /* must follow a digit */
        if (n + 1 < sizeof(calc_display)) {
            calc_display[n]     = '%';
            calc_display[n + 1] = '\0';
        }
        /* Snapshot the base for the + / - / * / / percentage math. */
        calc_percent_base    = calc_stored;
        calc_pending_modulo  = 1;
        calc_rhs_committable = 1;
        calc_result_shown    = 1;
    }
    else if (c == '+' || c == '-' || c == '*' || c == '/')
    {
        /* Fold (modulo-aware) RHS into calc_stored using the OLD op. */
        calc_apply_pending_to_stored();

        /* If there was no pending op yet (first op in chain), commit the
         * ORIGINAL display value as calc_stored. We must read BEFORE
         * formatting -- formatting writes calc_stored into calc_display,
         * which would clobber the user's typed number when calc_stored
         * is still 0 from a fresh reset. */
        if (calc_pending_op == 0) {
            calc_stored = calc_display_as_float();
            if (calc_pending_modulo) {
                calc_stored = calc_stored / 100.0f;  /* % as a literal decimal */
                calc_pending_modulo = 0;
            }
        }
        /* else: user changed op without typing RHS (e.g. "5 + -") --
         * calc_stored stays as-is, just switch pending_op. */

        calc_format_value(calc_stored, calc_display, sizeof(calc_display));
        calc_rhs_committable = 0;
        calc_pending_op      = c;
        calc_result_shown    = 1;
    }
    else if (c == '=')
    {
        if (calc_pending_op != 0 && calc_rhs_committable) {
            calc_apply_pending_to_stored();
            calc_format_value(calc_stored, calc_display, sizeof(calc_display));
        } else if (calc_pending_op != 0) {
            calc_format_value(calc_stored, calc_display, sizeof(calc_display));
        }

        calc_pending_op      = 0;
        calc_percent_base    = 0.0f;
        calc_pending_modulo  = 0;
        calc_rhs_committable = 0;
        calc_result_shown    = 1;
    }
}

#endif /* XIAOMI_UI_CALC_STATE_H */
