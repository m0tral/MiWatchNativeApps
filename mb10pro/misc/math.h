#ifndef BARE_MATH_H
#define BARE_MATH_H

#include <stdint.h>

/* ---- Single-precision float helpers ---------------------------------------
 *
 * These helpers operate in float so the Cortex-M33's fpv5-sp-d16 FPU
 * computes them natively (vmul.f32 / vadd.f32 / vdiv.f32) instead of
 * pulling in soft-float __aeabi_d* runtime helpers.
 *
 * The range guard before the (int)x cast avoids undefined behavior when
 * the value can't fit in a 32-bit int; for such magnitudes the value's
 * fractional component is well below display precision and we just
 * return x. */

/* round -- nearest integer, half-away-from-zero. Returns float. */
__attribute__((unused)) static float math_round_f(float x)
{
    /* Truncate via (int) when the magnitude fits; otherwise the
     * fractional part is irrelevant at display precision. */
    float truncated = (x >= 2147483648.0f || x <= -2147483648.0f)
                          ? x
                          : (float)(int)x;
    float adjusted = (truncated > x) ? truncated - 1.0f : truncated;
    if (x >= 0.0f) return adjusted;
    return -adjusted;
}

/* fabs -- absolute value. */
__attribute__((unused)) static float math_fabs_f(float x)
{
    return (x < 0.0f) ? -x : x;
}

/* pow10_f -- 10^n for small integer n (n >= 0). */
__attribute__((unused)) static float math_pow10_f(int n)
{
    float r = 1.0f;
    while (n-- > 0)
        r *= 10.0f;
    return r;
}

#endif /* BARE_MATH_H */
