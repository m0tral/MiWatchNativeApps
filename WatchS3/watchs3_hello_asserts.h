#ifndef WATCHS3_HELLO_ASSERTS_H
#define WATCHS3_HELLO_ASSERTS_H

/* Per-TU address asserts for watchs3_hello.c.
 *
 * Pulls in the platform header so the typed pointer bindings (syslog, ...)
 * are visible, then declares _Static_assert blocks ONLY for the symbols
 * this translation unit actually references. Filling in the real address
 * satisfies the assert; the assert only fires for symbols consumed here.
 */

#include "platform/miwear_system.h"

/* logger */
_Static_assert((SYSLOG_ADDR & ~1u) != 0u,
               "syslog address is NULL");

#endif /* WATCHS3_HELLO_ASSERTS_H */
