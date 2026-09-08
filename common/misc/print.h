#ifndef XIAOMI_PRINT_H
#define XIAOMI_PRINT_H

#include <stdarg.h>
#include <stddef.h>

/* watchs3 stdio printf family.
 *
 * The actual implementation lives in vela_ap.bin (snprintf at 0x2C1EA8BC,
 * Thumb code). The Thumb-bitted address binding lives in
 * platform/watchs3_platform.h; this header owns only the C prototype so any
 * TU can declare / pass a function pointer of type `int (*)(char *,
 * size_t, const char *, ...)` without pulling in the platform header.
 */

int snprintf(char *str, size_t size, const char *fmt, ...);

#endif /* XIAOMI_PRINT_H */
