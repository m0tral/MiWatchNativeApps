/*
 * watchs3/ui/calc_state_test.c -- host-side test for the calculator
 * state machine. Drives calc_state_press_key() with hand-crafted
 * key sequences and asserts on the resulting display + stored value.
 *
 * The state machine lives in calc_state.h (header-only static-inline);
 * the test just includes the header and compiles on any host:
 *
 *     gcc -I. -IWatchS3 -Icommon -O2 \
 *         WatchS3/ui/calc_state_test.c \
 *         -o /tmp/calc_state_test
 *     /tmp/calc_state_test
 *
 * Exits 0 on success, non-zero on the first failed assertion. Each
 * case starts with calc_state_reset() so they are independent.
 *
 * Host-only: pulls in <stdio.h> for the FAIL/OK reporting. calc_state.h
 * itself is libc-free so it can be compiled into the bare-metal device
 * image with zero imports.
 */

#include <stdio.h>

#include "WatchS3/ui/calc_state.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures = 0;

static void press(const char *keys)
{
    for (const char *p = keys; *p; ++p)
        calc_state_press_key(*p);
}

static void check_display(const char *expected, const char *what)
{
    const char *got = calc_state_display();
    if (strcmp(got, expected) == 0) return;
    fprintf(stderr,
            "FAIL [%s]: display='%s' expected='%s'\n",
            what, got, expected);
    g_failures++;
}

static void check_stored(float expected, const char *what)
{
    float got = calc_state_stored();
    /* tolerance: 4 fractional digits is more than enough for these cases */
    float diff = got - expected;
    if (diff < 0) diff = -diff;
    if (diff < 0.0001f) return;
    fprintf(stderr,
            "FAIL [%s]: stored=%.6f expected=%.6f\n",
            what, (double)got, (double)expected);
    g_failures++;
}

static void run(const char *what,
                const char *keys,
                const char *expected_display,
                float       expected_stored)
{
    calc_state_reset();
    press(keys);
    check_display(expected_display, what);
    if (expected_display[0] != '\0')                 /* "" means "don't care" */
        check_stored(expected_stored, what);
}

int main(void)
{
    /* ---- basic arithmetic ---------------------------------------------- */
    run("2+3=",       "2+3=",     "5",     5.0f);
    run("2+3=",       "2+3",      "5",     5.0f);          /* op also commits */
    run("9-4=",       "9-4=",     "5",     5.0f);
    run("6/2=",       "6/2=",     "3",     3.0f);
    run("7*6=",       "7*6=",     "42",    42.0f);
    run("2+3*4=",     "2+3*4=",   "20",    20.0f);         /* left-to-right, no precedence */

    /* ---- decimal / dot ------------------------------------------------ */
    run("1.5+2.5=",   "1.5+2.5=", "4",     4.0f);
    run(".5*2=",      ".5*2=",    "1",     1.0f);          /* synthesised leading 0 */

    /* ---- clear / backspace -------------------------------------------- */
    run("C clears",   "12C",      "0",     0.0f);
    run("C after op", "9+C",      "0",     0.0f);
    run("backspace",  "123<",     "12",    12.0f);         /* final stored == 12 */
    run("backspace empty", "1<<",  "0",     0.0f);

    /* ---- op-change does not commit (5 + -) ---------------------------- */
    run("op change",  "5+-",      "5",     5.0f);

    /* ---- percent (the bug we're fixing) ------------------------------- */
    run("2+50%=3",    "2+50%=",   "3",     3.0f);          /* 2 + (2 * 50 / 100) */
    run("100+10%=110", "100+10%=", "110",  110.0f);
    run("100-10%=90",  "100-10%=", "90",    90.0f);
    run("200*10%=20",  "200*10%=", "20",    20.0f);         /* * treats % as decimal */
    run("200/10%=2000","200/10%=", "2000",  2000.0f);

    /* ---- percent visible in display before commit --------------------- */
    run("10% shows 10%",
        "5+10%",      "10%",      5.0f);                   /* RHS sits with '%' marker */

    /* ---- chained percent: 100 + 10% + 5% = 115.5 -------------------- */
    run("chained percent",
        "100+10%+5%=", "115.5", 115.5f);

    /* ---- op-change after percent -------------------------------------- */
    run("percent then op commits",
        "5+10%-",     "5.5",     5.5f);                    /* % made RHS committable,
                                                              * so '-' applies 5+0.5=5.5 */
    run("op change without rhs",
        "5+-",        "5",       5.0f);                    /* no RHS typed -> no commit */

    /* ---- modulo guard: % must follow a digit -------------------------- */
    run("% at start ignored",
        "%2=",        "2",       2.0f);
    run("%% stacked ignored",
        "5%%",        "5",       5.0f);

    /* ---- percent on first operand (no op yet) ------------------------- */
    run("5% alone",
        "5%",         "5%",      0.0f);                    /* no commit possible */

    /* ---- 5+2%  ==  6.1  (2% of 5 == 0.1) ----------------------------- */
    run("5+2%=5.1",
        "5+2%=",      "5.1",     5.1f);

    if (g_failures == 0) {
        printf("OK: all calc_state tests passed\n");
        return 0;
    }
    fprintf(stderr, "FAIL: %d assertion(s) failed\n", g_failures);
    return 1;
}
