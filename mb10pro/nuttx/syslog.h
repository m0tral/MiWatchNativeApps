#ifndef MB10P_SYSLOG_H
#define MB10P_SYSLOG_H

/* mb10pro firmware syslog -- signature and level enum.
 *
 * Level values reverse-engineered from vela_ap.bin (fw_3.101.043):
 *   syslog_level_filter (0x2C1F0CF8) uses the level as a bit index into
 *   g_syslog_level_mask (0x200C8958, runtime = 0xFF = all enabled).
 *   syslog2() (0x2C840FF4) hardcodes level 6 -> LOG_INFO = 6.
 * This matches the POSIX / NuttX syslog convention.
 */

#include <stdint.h>

typedef enum {
    LOG_EMERG   = 0,   /* system is unusable */
    LOG_ALERT   = 1,   /* action must be taken immediately */
    LOG_CRIT    = 2,   /* critical conditions */
    LOG_ERR     = 3,   /* error conditions */
    LOG_WARN    = 4,   /* warning conditions */
    LOG_NOTICE  = 5,   /* normal but significant condition */
    LOG_INFO    = 6,   /* informational */
    LOG_DEBUG   = 7,   /* debug-level messages */
} loglevel_t;

/* int syslog(int level, const char *fmt, ...);
 * Thumb address 0x2C1F0D20. The Thumb-bit-set function pointer macro
 * lives in mb10p_platform.h. */
typedef int (*syslog_t)(int level, const char *fmt, ...);

#endif /* MB10P_SYSLOG_H */
