/* Host regression tests for the S4 calculator. MIT; see LICENSE. */
#include <stdio.h>
#include <string.h>
#include "calc_state.h"

static int failures;
static unsigned checks;

static void expect(int ok, const char *what)
{
    ++checks;
    if (!ok) { fprintf(stderr, "FAIL: %s\n", what); ++failures; }
}

static void sequence(const char *keys, const char *display, float stored, int error)
{
    calc_state_reset();
    for (const char *p = keys; *p; ++p) calc_state_press_key(*p);
    ++checks;
    float diff = calc_state_stored() - stored;
    if (diff < 0.0f) diff = -diff;
    if (strcmp(calc_state_display(), display) || diff > 0.0001f || calc_state_error() != error) {
        fprintf(stderr, "FAIL: %s -> %s / %.9g / error=%d, expected %s / %.9g / error=%d\n",
                keys, calc_state_display(), (double)calc_state_stored(), calc_state_error(),
                display, (double)stored, error);
        ++failures;
    }
}

static void formatted(float value, const char *expected)
{
    char text[32];
    calc_format_value(value, text, sizeof(text));
    ++checks;
    if (strcmp(text, expected)) {
        fprintf(stderr, "FAIL: format %.9g = '%s', expected '%s'\n", (double)value, text, expected);
        ++failures;
    }
}

int main(void)
{
    sequence("2+3=", "5", 5.0f, 0);
    sequence("2+3", "3", 2.0f, 0); /* RHS remains editable until commit. */
    sequence("9-4=", "5", 5.0f, 0);
    sequence("6/2=", "3", 3.0f, 0);
    sequence("7*6=", "42", 42.0f, 0);
    sequence("2+3*4=", "20", 20.0f, 0); /* Pocket-calculator left-to-right. */
    sequence("1.5+2.5=", "4", 4.0f, 0);
    sequence(".5*2=", "1", 1.0f, 0);
    sequence("1..5+2=", "3.5", 3.5f, 0);
    sequence("0.1+0.2=", "0.3", 0.3f, 0);
    sequence("1/3=", "0.333333", 1.0f / 3.0f, 0);
    sequence("2/3=", "0.666667", 2.0f / 3.0f, 0);
    sequence("1-2.2=", "-1.2", -1.2f, 0);
    sequence("12C", "0", 0.0f, 0);
    sequence("9+C", "0", 0.0f, 0);
    sequence("123<", "12", 0.0f, 0); /* Editing does not update accumulator. */
    sequence("123<=", "12", 12.0f, 0);
    sequence("1<<", "0", 0.0f, 0);
    sequence("5+-", "5", 5.0f, 0);
    sequence("5+-2=", "3", 3.0f, 0);
    sequence("5+=", "5", 5.0f, 0);
    sequence("2+3==", "5", 5.0f, 0); /* Repeated '=' does not repeat operation. */
    sequence("2+3=7+1=", "8", 8.0f, 0);
    sequence("2+3=*2=", "10", 10.0f, 0);
    sequence("2+50%=", "3", 3.0f, 0);
    sequence("100+10%=", "110", 110.0f, 0);
    sequence("100-10%=", "90", 90.0f, 0);
    sequence("200*10%=", "20", 20.0f, 0);
    sequence("200/10%=", "2000", 2000.0f, 0);
    sequence("5+10%", "10%", 5.0f, 0);
    sequence("100+10%+5%=", "115.5", 115.5f, 0);
    sequence("5+10%-", "5.5", 5.5f, 0);
    sequence("5%", "5%", 0.0f, 0);
    sequence("5%%", "5%", 0.0f, 0);
    sequence("5%=", "0.05", 0.05f, 0);
    sequence("5%*200=", "10", 10.0f, 0);
    sequence("5+2%=", "5.1", 5.1f, 0);
    sequence("5%<", "5", 0.0f, 0);
    sequence("5+10%<=", "15", 15.0f, 0);
    sequence("5/0=", "Error", 0.0f, 1);
    sequence("5/0*=", "Error", 0.0f, 1);
    sequence("5/0=2+3=", "5", 5.0f, 0);
    sequence("5/0=C", "0", 0.0f, 0);
    sequence("5/0=<", "0", 0.0f, 0);
    sequence("5/0=.5=", "0.5", 0.5f, 0);
    sequence("4294967296=", "Error", 0.0f, 1);
    sequence("1000000000*10=", "Error", 0.0f, 1);
    sequence("1000000000+1000000000=", "2000000000", 2000000000.0f, 0);

    expect(calc_round_f(1.9f) == 2.0f, "positive round");
    expect(calc_round_f(-1.2f) == -1.0f, "negative round sign");
    expect(calc_round_f(1.5f) == 2.0f, "positive half away from zero");
    expect(calc_round_f(-1.5f) == -2.0f, "negative half away from zero");
    expect(calc_round_f(0.49999997f) == 0.0f, "below positive half");
    expect(calc_round_f(-0.49999997f) == 0.0f, "above negative half");
    expect(calc_round_f(8388608.0f) == 8388608.0f, "large round bypass cast");
    expect(calc_round_f(-8388608.0f) == -8388608.0f, "negative large round");

    formatted(1.2345678f, "1.234568");
    formatted(-1.2345678f, "-1.234568");
    formatted(1.9999998f, "2");
    formatted(-0.0000004f, "0");
    formatted(0.0000006f, "0.000001");
    formatted(-0.0000006f, "-0.000001");
    formatted(1000000.125f, "1000000.125");
    formatted(4294967040.0f, "4294967040"); /* Largest binary32 below 2^32. */
    formatted(4294967296.0f, "Error");
    formatted(-4294967296.0f, "Error");
    union { uint32_t bits; float value; } special;
    special.bits = 0x7f800000u; formatted(special.value, "Error");
    special.bits = 0xff800000u; formatted(special.value, "Error");
    special.bits = 0x7fc00000u; formatted(special.value, "Error");
    expect(calc_round_f(special.value) != calc_round_f(special.value), "NaN rounding preserves NaN");

    char guarded[4] = {'L', 'x', 'x', 'R'};
    calc_format_value(1.0f, guarded + 1, 0);
    expect(guarded[1] == 'x', "zero-size output is untouched");
    calc_format_value(1.0f, guarded + 1, 1);
    expect(guarded[1] == 0 && guarded[0] == 'L' && guarded[2] == 'x', "one-byte output terminated");
    calc_format_value(12.0f, guarded + 1, 2);
    expect(guarded[1] == '1' && guarded[2] == 0 && guarded[3] == 'R', "truncated output bounded");
    calc_format_value(special.value, guarded + 1, 2);
    expect(guarded[1] == 'E' && guarded[2] == 0 && guarded[3] == 'R', "error output bounded");

    calc_state_reset();
    for (unsigned i = 0; i < 100; ++i) calc_state_press_key('9');
    expect(calc_strlen(calc_state_display()) == CALC_DISP_BUFSZ - 1, "input is bounded");
    calc_state_press_key('%');
    expect(!calc_state_pending_modulo(), "full buffer does not hide percent marker");
    calc_state_press_key('=');
    expect(calc_state_error(), "oversized typed value reports Error");

    printf("%u checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
