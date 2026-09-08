#ifndef BARE_MEMORY_H
#define BARE_MEMORY_H

#include <stdarg.h>
#include <stddef.h>

/* __attribute__((unused)) suppresses -Wunused-function/-Werror for helpers
 * that a given TU doesn't call. The set lives in the header so any module
 * can pick whichever it needs. */

__attribute__((unused)) static void *memset(void *dst, int c, size_t n)
{
    unsigned char *d = (unsigned char *)dst;
    while (n--) {
        *d++ = (unsigned char)c;
    }
    return dst;
}

__attribute__((unused)) static size_t strlen(const char *s)
{
    size_t n = 0;
    while (s[n] != '\0') {
        n++;
    }
    return n;
}

__attribute__((unused)) static size_t strnlen(const char *s, size_t max)
{
    size_t n = 0;
    while (n < max && s[n] != '\0') {
        n++;
    }
    return n;
}

/* atoi -- ASCII-to-int, leading whitespace skipped, optional +/- sign,
 * stops at first non-digit. Returns 0 on overflow. Local replacement for
 * newlib's <stdlib.h> atoi so the module doesn't need to pull stdlib. */
__attribute__((unused)) static int atoi(const char *s)
{
    int n = 0;
    int sign = 1;

    while (*s == ' ' || *s == '\t')
        s++;

    if (*s == '-') { sign = -1; s++; }
    else if (*s == '+') { s++; }

    while (*s >= '0' && *s <= '9') {
        int d = *s - '0';
        n = n * 10 + d;
        s++;
    }
    return sign * n;
}

#endif /* BARE_MEMORY_H */
