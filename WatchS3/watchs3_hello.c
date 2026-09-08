/*
 * watchs3_hello.c
 *
 * Loadable NuttX userspace module for Xiaomi Watch S3 (watchs3).
 * Loaded at runtime via the NSH command:
 *
 *     insmod /data/watchs3_hello.elf hello_module
 *
 * ============================================================================
 */

#include <stdint.h>
#include "common/nuttx/syslog.h"
#include "watchs3_hello_asserts.h"

static int say_hello_module(void)
{
    syslog(LOG_WARN, "[hello_module] hello");
    return 0;
}

// ---- ELF ENTRY POINT --------------------------------------------------- *

__attribute__((constructor)) int module_initialize(void)
{
    syslog(LOG_WARN, "[hello_module] loaded");
	
	say_hello_module();

    return 0;   /* OK; loader adds us to the module registry */
}

__attribute__((destructor, used)) static void module_uninitialize(void)
{
    syslog(LOG_WARN, "[hello_module] unloaded");
}
