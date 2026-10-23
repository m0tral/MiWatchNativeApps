#ifndef XIAOMI_DECIMAL_H
#define XIAOMI_DECIMAL_H

#include <stddef.h>
#include <stdint.h>

/* ---- Decimal arithmetic for the calculator --------------------------------
 *
 * The Cortex-M33 FPU is binary: 0.1, 0.2 or 100.1 have no exact float, and a
 * float32 carries only ~7 significant digits (99999 * 99999 = 9999800001
 * comes out as 9999800320). This header does the calculator's arithmetic in
 * decimal with 32-bit integer instructions only (MUL, UDIV, MLS): no float,
 * no double, no 64-bit division, no libc/libgcc helpers, no heap.
 *
 * Precision contract
 *   value     sign * m * 10^-15, m an integer, |value| < 10^15
 *   literal   at most 15 significant digits and 15 fractional digits:
 *             stored exactly; anything longer is rejected, never truncated
 *   + - * /   each result is the exact result rounded ONCE to 15
 *             significant digits, but not finer than 10^-15; ties round
 *             away from zero. +, -, * are exact whenever the exact result
 *             fits in that format.
 *   range     a rounded result with |value| >= 10^15 is DEC_ERR_OVERFLOW;
 *             one below 0.5 * 10^-15 becomes 0. x / 0 is DEC_ERR_DIV0.
 *   text      plain decimal, no exponent, trailing zeros removed, never
 *             "-0"; at most 18 characters ("-0." + 15 digits), and every
 *             result is again a valid literal.
 *
 * Magnitudes are little-endian base-10^4 limbs, so every limb product and
 * every partial remainder fits in 32 bits. DEC_LIMBS = 16 (64 digits) holds
 * the largest intermediate: a 30 x 30 digit product, or the 45-digit
 * dividend m * 10^15. All loops are bounded by DEC_LIMBS or by the digit
 * count of the operands. */

#define DEC_SIG      15        /* significant digits of a result        */
#define DEC_SCALE    15        /* fixed fractional digits of m          */
#define DEC_MAX_M    30        /* m < 10^30  <=>  |value| < 10^15        */
#define DEC_LIMBS    16
#define DEC_BASE     10000u

enum {
    DEC_OK = 0,
    DEC_ERR_DIV0,              /* division by zero                      */
    DEC_ERR_OVERFLOW,          /* |result| >= 10^15                     */
    DEC_ERR_INPUT,             /* literal longer than the contract      */
    DEC_ERR_SYNTAX             /* malformed expression                  */
};

typedef struct { uint16_t l[DEC_LIMBS]; } dec_big;
typedef struct { uint8_t neg; dec_big m; } dec_t;

__attribute__((unused)) static const uint16_t dec_pow10_small[4] = { 1u, 10u, 100u, 1000u };

/* ---- magnitude helpers ---------------------------------------------------- */

__attribute__((unused)) static void dec_big_zero(dec_big *a)
{
    for (int i = 0; i < DEC_LIMBS; ++i) a->l[i] = 0u;
}

__attribute__((unused)) static int dec_big_is_zero(const dec_big *a)
{
    for (int i = 0; i < DEC_LIMBS; ++i) if (a->l[i] != 0u) return 0;
    return 1;
}

__attribute__((unused)) static int dec_big_cmp(const dec_big *a, const dec_big *b)
{
    for (int i = DEC_LIMBS - 1; i >= 0; --i)
        if (a->l[i] != b->l[i]) return (a->l[i] > b->l[i]) ? 1 : -1;
    return 0;
}

/* r = a + b (r may alias a or b). Returns the carry out of the top limb. */
__attribute__((unused)) static uint32_t dec_big_add(dec_big *r, const dec_big *a, const dec_big *b)
{
    uint32_t carry = 0u;
    for (int i = 0; i < DEC_LIMBS; ++i) {
        uint32_t t = (uint32_t)a->l[i] + b->l[i] + carry;
        carry = (t >= DEC_BASE) ? 1u : 0u;
        r->l[i] = (uint16_t)(t - carry * DEC_BASE);
    }
    return carry;
}

/* r = a - b, requires a >= b (r may alias a or b). */
__attribute__((unused)) static void dec_big_sub(dec_big *r, const dec_big *a, const dec_big *b)
{
    uint32_t borrow = 0u;
    for (int i = 0; i < DEC_LIMBS; ++i) {
        uint32_t sub = (uint32_t)b->l[i] + borrow;
        uint32_t ai = a->l[i];
        borrow = (ai < sub) ? 1u : 0u;
        r->l[i] = (uint16_t)(ai + borrow * DEC_BASE - sub);
    }
}

/* a = a * k + add, k <= DEC_BASE, add < DEC_BASE. Returns the carry out. */
__attribute__((unused)) static uint32_t dec_big_mul_add_small(dec_big *a, uint32_t k, uint32_t add)
{
    uint32_t carry = add;
    for (int i = 0; i < DEC_LIMBS; ++i) {
        uint32_t t = (uint32_t)a->l[i] * k + carry;    /* <= 9999*10000 + 10000 */
        carry = t / DEC_BASE;
        a->l[i] = (uint16_t)(t - carry * DEC_BASE);
    }
    return carry;
}

/* a = a / d, 1 <= d <= DEC_BASE. Returns the remainder. */
__attribute__((unused)) static uint32_t dec_big_div_small(dec_big *a, uint32_t d)
{
    uint32_t rem = 0u;
    for (int i = DEC_LIMBS - 1; i >= 0; --i) {
        uint32_t t = rem * DEC_BASE + a->l[i];          /* < d * 10^4 <= 10^8 */
        uint32_t q = t / d;
        rem = t - q * d;
        a->l[i] = (uint16_t)q;
    }
    return rem;
}

/* r = a * b. Returns 0, or 1 if the product does not fit in DEC_LIMBS. */
__attribute__((unused)) static int dec_big_mul(dec_big *r, const dec_big *a, const dec_big *b)
{
    uint16_t acc[2 * DEC_LIMBS];                     /* every entry < DEC_BASE */
    for (int i = 0; i < 2 * DEC_LIMBS; ++i) acc[i] = 0u;
    for (int i = 0; i < DEC_LIMBS; ++i) {
        uint32_t ai = a->l[i], carry = 0u;
        if (ai == 0u) continue;
        for (int j = 0; j < DEC_LIMBS; ++j) {
            uint32_t t = acc[i + j] + ai * b->l[j] + carry;   /* < 10^8 + 2*10^4 */
            carry = t / DEC_BASE;
            acc[i + j] = (uint16_t)(t - carry * DEC_BASE);
        }
        acc[i + DEC_LIMBS] = (uint16_t)carry;        /* not written by earlier rows */
    }
    for (int i = DEC_LIMBS; i < 2 * DEC_LIMBS; ++i) if (acc[i] != 0u) return 1;
    for (int i = 0; i < DEC_LIMBS; ++i) r->l[i] = acc[i];
    return 0;
}

/* Number of decimal digits of a (0 for a == 0). */
__attribute__((unused)) static int dec_big_digits(const dec_big *a)
{
    for (int i = DEC_LIMBS - 1; i >= 0; --i) {
        uint32_t v = a->l[i];
        if (v != 0u) {
            int d = 1;
            while (d < 4 && v >= dec_pow10_small[d]) ++d;
            return 4 * i + d;
        }
    }
    return 0;
}

/* Decimal digit i of a (i = 0 is the units digit). */
__attribute__((unused)) static uint32_t dec_big_digit(const dec_big *a, int i)
{
    if (i < 0 || i >= 4 * DEC_LIMBS) return 0u;
    return ((uint32_t)a->l[i / 4] / dec_pow10_small[i % 4]) % 10u;
}

/* a = a * 10^k. Returns 1 (a unchanged) if the result would not fit. */
__attribute__((unused)) static int dec_big_shl10(dec_big *a, int k)
{
    if (k <= 0 || dec_big_is_zero(a)) return 0;
    if (dec_big_digits(a) + k > 4 * DEC_LIMBS) return 1;
    int limbs = k / 4;
    for (int i = DEC_LIMBS - 1; i >= 0; --i)
        a->l[i] = (i >= limbs) ? a->l[i - limbs] : 0u;
    (void)dec_big_mul_add_small(a, dec_pow10_small[k % 4], 0u);
    return 0;
}

/* a = floor(a / 10^k), k >= 1. Returns 1 if the dropped part was >= half a
 * unit of the new last place (its leading digit is >= 5), else 0. */
__attribute__((unused)) static int dec_big_shr10(dec_big *a, int k)
{
    int up = (dec_big_digit(a, k - 1) >= 5u);
    int limbs = k / 4;
    for (int i = 0; i < DEC_LIMBS; ++i)
        a->l[i] = (i + limbs < DEC_LIMBS) ? a->l[i + limbs] : 0u;
    (void)dec_big_div_small(a, dec_pow10_small[k % 4]);
    return up;
}

/* ---- rounding --------------------------------------------------------------
 * x is the exact magnitude at scale 10^-s (s = 15 or 30). For a quotient,
 * x is its floor and rem_half tells where the discarded remainder lies:
 * 0 none or below half, 1 at or above half. One rounding to DEC_SIG
 * significant digits, not finer than 10^-15, ties away from zero. */
__attribute__((unused)) static int dec_round(dec_big *x, int s, int rem_half, dec_big *out)
{
    int d = dec_big_digits(x);
    int k = d - DEC_SIG;                      /* digits beyond DEC_SIG     */
    if (k < s - DEC_SCALE) k = s - DEC_SCALE; /* digits finer than 10^-15  */
    int up;
    if (k > 0) {
        /* For a quotient the remainder lies below the last dropped digit:
         * it cannot lift a dropped part < half to half, and at exactly
         * half the tie rounds up anyway, so the leading digit decides. */
        up = dec_big_shr10(x, k);
    } else {
        up = rem_half;
        k = 0;
    }
    if (up) {
        dec_big one;
        dec_big_zero(&one);
        one.l[0] = 1u;
        (void)dec_big_add(x, x, &one);       /* x < 10^DEC_SIG + 1: no carry out */
    }
    if (dec_big_shl10(x, k - (s - DEC_SCALE)))
        return DEC_ERR_OVERFLOW;
    if (dec_big_digits(x) > DEC_MAX_M)
        return DEC_ERR_OVERFLOW;
    *out = *x;
    return DEC_OK;
}

__attribute__((unused)) static int dec_finish(dec_t *r, int neg, dec_big *x, int s, int rem_half)
{
    int st = dec_round(x, s, rem_half, &r->m);
    if (st != DEC_OK) return st;
    r->neg = (uint8_t)(neg && !dec_big_is_zero(&r->m));   /* never -0 */
    return DEC_OK;
}

/* ---- arithmetic -------------------------------------------------------------
 * r may alias a or b. On error r is left unchanged. */

__attribute__((unused)) static int dec_add(dec_t *r, const dec_t *a, const dec_t *b, int subtract)
{
    int bneg = b->neg ^ (subtract ? 1 : 0);
    dec_big x;
    int neg;
    if (a->neg == bneg) {
        (void)dec_big_add(&x, &a->m, &b->m);        /* < 2 * 10^30: no carry out */
        neg = a->neg;
    } else if (dec_big_cmp(&a->m, &b->m) >= 0) {
        dec_big_sub(&x, &a->m, &b->m);
        neg = a->neg;
    } else {
        dec_big_sub(&x, &b->m, &a->m);
        neg = bneg;
    }
    return dec_finish(r, neg, &x, DEC_SCALE, 0);
}

__attribute__((unused)) static int dec_mul(dec_t *r, const dec_t *a, const dec_t *b)
{
    dec_big x;
    if (dec_big_mul(&x, &a->m, &b->m))              /* < 10^60: cannot happen */
        return DEC_ERR_OVERFLOW;
    return dec_finish(r, a->neg ^ b->neg, &x, 2 * DEC_SCALE, 0);
}

__attribute__((unused)) static int dec_div(dec_t *r, const dec_t *a, const dec_t *b)
{
    if (dec_big_is_zero(&b->m))
        return DEC_ERR_DIV0;
    /* q = floor(a.m * 10^15 / b.m), schoolbook in decimal digits: at most
     * 45 dividend digits, each needing at most 9 subtractions. */
    dec_big num = a->m, q, rem;
    if (dec_big_shl10(&num, DEC_SCALE))
        return DEC_ERR_OVERFLOW;
    dec_big_zero(&q);
    dec_big_zero(&rem);
    for (int i = dec_big_digits(&num) - 1; i >= 0; --i) {
        (void)dec_big_mul_add_small(&rem, 10u, dec_big_digit(&num, i));
        uint32_t qd = 0u;
        while (dec_big_cmp(&rem, &b->m) >= 0) {
            dec_big_sub(&rem, &rem, &b->m);
            ++qd;
        }
        (void)dec_big_mul_add_small(&q, 10u, qd);
    }
    /* remainder at or above half of the divisor: 2 * rem >= b.m (rem < b.m
     * < 10^30, so 2 * rem fits; num is reused as scratch) */
    (void)dec_big_add(&num, &rem, &rem);
    return dec_finish(r, a->neg ^ b->neg, &q, DEC_SCALE, dec_big_cmp(&num, &b->m) >= 0);
}

/* ---- text ------------------------------------------------------------------ */

__attribute__((unused)) static int dec_is_digit(char c) { return c >= '0' && c <= '9'; }

/* Parse one unsigned literal starting at *pp ("12", "0.5", "5.", ".5").
 * Advances *pp past it. DEC_ERR_INPUT if it has more than DEC_SIG
 * significant or DEC_SCALE fractional digits; DEC_ERR_SYNTAX if empty. */
__attribute__((unused)) static int dec_parse_literal(const char **pp, dec_t *out)
{
    const char *p = *pp;
    int sig = 0, frac = 0, dot = 0, any = 0;
    dec_big m;
    dec_big_zero(&m);
    for (;; ++p) {
        if (dec_is_digit(*p)) {
            any = 1;
            if (dot) ++frac;
            if (sig > 0 || *p != '0') ++sig;
            if (sig > DEC_SIG || frac > DEC_SCALE) return DEC_ERR_INPUT;
            (void)dec_big_mul_add_small(&m, 10u, (uint32_t)(*p - '0'));   /* < 10^30 */
        } else if (*p == '.' && !dot) {
            dot = 1;
        } else {
            break;
        }
    }
    if (!any) return DEC_ERR_SYNTAX;
    (void)dec_big_shl10(&m, DEC_SCALE - frac);     /* < 10^30: fits */
    out->neg = 0u;
    out->m = m;
    *pp = p;
    return DEC_OK;
}

/* Evaluate "a op b op c ..." with op in + - * /, * and / binding tighter,
 * left to right; an optional leading '-' negates the first literal (a
 * previous negative result). Every operation is rounded as described in
 * the contract, in the same order as the original float evaluator. */
__attribute__((unused)) static int dec_eval(const char *s, dec_t *out)
{
    dec_t sum, term, f;
    int st, neg_first = 0, pending = 0;        /* 0: first term, '+' or '-' */
    if (*s == '-') { neg_first = 1; ++s; }
    if ((st = dec_parse_literal(&s, &term)) != DEC_OK) return st;
    term.neg = (uint8_t)(neg_first && !dec_big_is_zero(&term.m));
    sum.neg = 0u;
    dec_big_zero(&sum.m);
    for (;;) {
        char op = *s;
        if (op == '*' || op == '/') {
            ++s;
            if ((st = dec_parse_literal(&s, &f)) != DEC_OK) return st;
            st = (op == '*') ? dec_mul(&term, &term, &f) : dec_div(&term, &term, &f);
            if (st != DEC_OK) return st;
            continue;
        }
        /* end of a term: fold it into the running sum (the first term is
         * taken as is, so a lone literal is never re-rounded) */
        if (pending == 0)
            sum = term;
        else if ((st = dec_add(&sum, &sum, &term, pending == '-')) != DEC_OK)
            return st;
        if (op == '\0') break;
        if (op != '+' && op != '-') return DEC_ERR_SYNTAX;
        pending = op;
        ++s;
        if ((st = dec_parse_literal(&s, &term)) != DEC_OK) return st;
    }
    *out = sum;
    return DEC_OK;
}

/* Write v as text (see the contract). bufsz >= 20 always fits; a smaller
 * buffer gets a truncated, NUL-terminated string. */
__attribute__((unused)) static void dec_format(const dec_t *v, char *buf, size_t bufsz)
{
    char digits[DEC_MAX_M + DEC_SCALE + 2];
    int n = dec_big_digits(&v->m), o = 0, top = (n > DEC_SCALE) ? n - 1 : DEC_SCALE;
    int last = 0;                                   /* lowest non-zero fractional digit */
    for (int i = 0; i < DEC_SCALE; ++i)
        if (dec_big_digit(&v->m, i) != 0u) { last = i; break; } else last = DEC_SCALE;
    if (v->neg && n > 0) digits[o++] = '-';
    for (int i = top; i >= DEC_SCALE; --i)          /* integer part, at least "0" */
        digits[o++] = (char)('0' + dec_big_digit(&v->m, i));
    if (last < DEC_SCALE) {
        digits[o++] = '.';
        for (int i = DEC_SCALE - 1; i >= last; --i)
            digits[o++] = (char)('0' + dec_big_digit(&v->m, i));
    }
    if (bufsz == 0u) return;
    size_t i = 0u;
    for (; i < (size_t)o && i + 1u < bufsz; ++i) buf[i] = digits[i];
    buf[i] = '\0';
}

/* Can one more digit be typed at the end of the expression `s` without
 * breaking the literal contract? (The calculator ignores the key if not.) */
__attribute__((unused)) static int dec_literal_accepts_digit(const char *s, char digit)
{
    const char *start = s;
    for (const char *p = s; *p; ++p)
        if (*p == '+' || *p == '-' || *p == '*' || *p == '/') start = p + 1;
    int sig = 0, frac = 0, dot = 0;
    for (const char *p = start; *p; ++p) {
        if (*p == '.') { dot = 1; continue; }
        if (dot) ++frac;
        if (sig > 0 || *p != '0') ++sig;
    }
    if (dot) ++frac;
    if (sig > 0 || digit != '0') ++sig;
    return sig <= DEC_SIG && frac <= DEC_SCALE;
}

#endif /* XIAOMI_DECIMAL_H */
