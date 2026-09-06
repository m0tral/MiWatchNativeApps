/*
 * mb10p_hello.c
 *
 * Loadable NuttX userspace module for Xiaomi Mi Band 10 Pro (mb10pro).
 * Loaded at runtime via the NSH command:
 *
 *     insmod /data/mb10p_hello.elf hello_module
 *
 * ============================================================================
 */

#include <stdint.h>
#include "nuttx/syslog.h"
#include "platform/miwear_system.h"

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
