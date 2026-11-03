#ifndef WATCHS4_MIWEAR_SYSTEM_H
#define WATCHS4_MIWEAR_SYSTEM_H
#if WATCHS4_FW_VERSION == 542
#include "fw_5.4.2/miwear_system.h"
#else
#error "Unsupported Watch S4 firmware; use WATCHS4_FW_VERSION=542 for Global 5.4.2"
#endif
#endif
