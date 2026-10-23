/*
 * common/misc/decimal_test.c -- host-side test for common/misc/decimal.h
 * (the mb10pro calculator's arithmetic).
 *
 *     gcc -std=gnu11 -O2 -Wall -Wextra -I. common/misc/decimal_test.c -o /tmp/decimal_test
 *     /tmp/decimal_test            table of cases (fast, exits 0 on success)
 *     /tmp/decimal_test --eval     one expression per stdin line ->
 *                                  "expression<TAB>result" (used by
 *                                  common/misc/decimal_ref.py --check)
 *
 * decimal.h uses only 32-bit integer arithmetic, so the host computes
 * exactly what the Cortex-M33 computes. This is still an offline test.
 */

#include <stdio.h>

#include "common/misc/decimal.h"

static int g_fail, g_total;

static const char *dec_status_name(int st)
{
    switch (st) {
    case DEC_ERR_DIV0:     return "ERR_DIV0";
    case DEC_ERR_OVERFLOW: return "ERR_OVERFLOW";
    case DEC_ERR_INPUT:    return "ERR_INPUT";
    case DEC_ERR_SYNTAX:   return "ERR_SYNTAX";
    default:               return "ERR_?";
    }
}

static void eval_to_text(const char *expr, char *out, size_t outsz)
{
    dec_t v;
    int st = dec_eval(expr, &v);
    if (st == DEC_OK) {
        dec_format(&v, out, outsz);
    } else {
        const char *n = dec_status_name(st);
        size_t i = 0;
        for (; n[i] && i + 1 < outsz; ++i) out[i] = n[i];
        out[i] = '\0';
    }
}

static int same(const char *a, const char *b)
{
    while (*a && *a == *b) { ++a; ++b; }
    return *a == *b;
}

static void t(const char *expr, const char *want)
{
    char got[32];
    eval_to_text(expr, got, sizeof got);
    ++g_total;
    if (!same(got, want)) {
        ++g_fail;
        printf("FAIL %-36s -> %-22s want %s\n", expr, got, want);
    }
}

static void t_accept(const char *display, char digit, int want)
{
    ++g_total;
    if (dec_literal_accepts_digit(display, digit) != want) {
        ++g_fail;
        printf("FAIL accepts_digit(\"%s\", '%c') != %d\n", display, digit, want);
    }
}

int main(int argc, char **argv)
{
    if (argc > 1 && same(argv[1], "--eval")) {
        char line[256], out[32];
        while (fgets(line, sizeof line, stdin)) {
            size_t n = 0;
            while (line[n] && line[n] != '\n' && line[n] != '\r') ++n;
            line[n] = '\0';
            eval_to_text(line, out, sizeof out);
            printf("%s\t%s\n", line, out);
        }
        return 0;
    }

    /* ---- the reported float32 problems ----------------------------------- */
    t("1872-17", "1855");
    t("0.1+0.2", "0.3");
    t("0.7-0.4", "0.3");
    t("100.1+1", "101.1");
    t("99999*99999", "9999800001");
    t("0.0078125", "0.0078125");
    t("12.345678", "12.345678");
    t("2/3", "0.666666666666667");
    t("1000/3", "333.333333333333");
    t("1-1", "0");
    t("0*5", "0");
    t("-0", "0");
    t("0-0.5*0", "0");                          /* no "-0" */

    /* ---- precedence, left associativity, leading minus -------------------- */
    t("2+3*4", "14");
    t("2*3+4", "10");
    t("10-2-3", "5");
    t("100/10/2", "5");
    t("1+2*3-4/2", "5");
    t("8/2*4", "16");
    t("-5+3", "-2");
    t("-1.5*-2", "ERR_SYNTAX");                 /* only a leading '-' is a sign */
    t("-0.5*2", "-1");
    t("5-6", "-1");
    t("1-2.5", "-1.5");

    /* ---- carry / borrow ---------------------------------------------------- */
    t("999999999999999+0", "999999999999999");
    t("0.999999999999999+0.000000000000001", "1");
    t("1-0.000000000000001", "0.999999999999999");
    t("100000000000000-0.5", "99999999999999.5");
    t("9999.9999+0.0001", "10000");

    /* ---- rounding to 15 significant digits, ties away from zero ------------ */
    t("1/3", "0.333333333333333");
    t("2/3*3", "2");                            /* 2.000000000000001 has 16 digits */
    t("1/7", "0.142857142857143");
    t("22/7", "3.14285714285714");
    t("123456789012345+0.5", "123456789012346");   /* tie: away from zero */
    t("123456789012344+0.5", "123456789012345");   /* tie: away from zero */
    t("-123456789012345-0.5", "-123456789012346");
    t("123456789012345+0.49", "123456789012345");
    t("0.000000000000001/2", "0.000000000000001");  /* 5e-16: tie at 1e-15 -> up */
    t("0.000000000000001/3", "0");              /* below half of 1e-15 */
    t("0.000000000000001*0.4", "0");
    t("0.000000000000001*0.5", "0.000000000000001");
    t("1.00000000000001/2", "0.500000000000005");
    t("5/9", "0.555555555555556");
    t("2/6", "0.333333333333333");
    t("200000000000000/3", "66666666666666.7");
    t("0.1/3", "0.033333333333333");            /* quantum 10^-15 */
    t("0.000001/3", "0.000000333333333");       /* stops at 10^-15 */
    t("12345678.9*1000000", "12345678900000");
    t("99999*99999*99999", "999970000299999");  /* 15 digits, exact */
    t("111111111111111*9", "999999999999999");  /* largest value, exact */
    t("111111111111111*8", "888888888888888");
    t("123456789*123456789", "ERR_OVERFLOW");
    t("12345678.9*12345678.9", "152415787501905");   /* 152415787501905.21 */
    t("999999999999999.5+0", "ERR_INPUT");      /* 16 significant digits */
    t("999999999999999+0.4", "999999999999999");
    t("999999999999999+0.5", "ERR_OVERFLOW");   /* rounds to 10^15 */

    /* ---- division by zero, overflow, underflow ------------------------------ */
    t("5/0", "ERR_DIV0");
    t("0/0", "ERR_DIV0");
    t("1+2/0*3", "ERR_DIV0");
    t("999999999999999*10", "ERR_OVERFLOW");
    t("999999999999999+1", "ERR_OVERFLOW");
    t("-999999999999999-1", "ERR_OVERFLOW");
    t("1/0.000000000000001", "ERR_OVERFLOW");
    t("100/0.000000000000001", "ERR_OVERFLOW");
    t("0.1/0.000000000000001", "100000000000000");
    t("0.000000000000001*0.000000000000001", "0");

    /* ---- literal limits: never truncated ------------------------------------ */
    t("123456789012345", "123456789012345");
    t("1234567890123456", "ERR_INPUT");
    t("0.123456789012345", "0.123456789012345");
    t("0.1234567890123456", "ERR_INPUT");
    t("0.000000000000001", "0.000000000000001");
    t("0.0000000000000001", "ERR_INPUT");       /* 16 fractional digits */
    t("000000000000000000001", "1");            /* leading zeros are not significant */
    t("1.500000000000000", "ERR_INPUT");        /* 16th fractional digit */
    t("1.50000000000000", "1.5");               /* no fake precision in the output */
    t("5.", "5");
    t(".5", "0.5");

    /* ---- malformed input (the UI never builds these) ------------------------ */
    t("", "ERR_SYNTAX");
    t("+", "ERR_SYNTAX");
    t("5+", "ERR_SYNTAX");
    t("5++3", "ERR_SYNTAX");
    t("Error", "ERR_SYNTAX");
    t(".", "ERR_SYNTAX");
    t("1.2.3", "ERR_SYNTAX");

    /* ---- digit acceptance used by the key handler ---------------------------- */
    t_accept("12345678901234", '5', 1);
    t_accept("123456789012345", '6', 0);
    t_accept("123456789012345", '0', 0);
    t_accept("0.", '0', 1);
    t_accept("0.00000000000000", '1', 1);       /* 15th fractional digit */
    t_accept("0.000000000000001", '0', 0);
    t_accept("1+123456789012345", '1', 0);
    t_accept("123456789012345+", '1', 1);       /* new literal */
    t_accept("-12345678901234", '5', 1);
    t_accept("0000", '0', 1);

    /* ---- zero is never negative in the value itself either -------------------- */
    {
        static const char *const zeros[] = { "-0", "0-0.5*0", "-5+5", "-0.000000000000001*0.4", "0*-0", 0 };
        for (int i = 0; zeros[i]; ++i) {
            dec_t v;
            int st = dec_eval(zeros[i], &v);
            ++g_total;
            if (st == DEC_OK && (v.neg || !dec_big_is_zero(&v.m))) {
                ++g_fail; printf("FAIL %s: neg=%d (zero must be positive)\n", zeros[i], v.neg);
            }
        }
    }

    /* ---- output buffer bounds ---------------------------------------------- */
    {
        dec_t v;
        char small[5] = "xxxx", z[1] = { 'x' };
        int st = dec_eval("-123.5", &v);
        dec_format(&v, small, sizeof small);
        ++g_total;
        if (st != DEC_OK || !same(small, "-123")) { ++g_fail; printf("FAIL bufsz 5: %s\n", small); }
        dec_format(&v, z, 0);
        ++g_total;
        if (z[0] != 'x') { ++g_fail; printf("FAIL bufsz 0 wrote\n"); }
        char big[32];
        (void)dec_eval("-0.000000000000001+0.000000000000001-123456789.123456", &v);
        dec_format(&v, big, sizeof big);
        ++g_total;
        if (!same(big, "-123456789.123456")) { ++g_fail; printf("FAIL longest: %s\n", big); }
    }

    if (g_fail) {
        printf("FAILED %d of %d\n", g_fail, g_total);
        return 1;
    }
    printf("OK: %d decimal tests passed\n", g_total);
    return 0;
}
