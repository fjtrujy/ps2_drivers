/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_MEMCARD_DRIVER
*/

#ifndef PS2_MEMCARD_DRIVER
#define PS2_MEMCARD_DRIVER

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum MEMCARD_INIT_STATUS {
    MEMCARD_INIT_STATUS_DEPENDENCY_IRX_ERROR = -4,
    MEMCARD_INIT_STATUS_MCSERV_IRX_ERROR = -3,
    MEMCARD_INIT_STATUS_MCMAN_IRX_ERROR = -2,
    MEMCARD_INIT_STATUS_UNKNOWN = -1,
    MEMCARD_INIT_STATUS_OK = 0,
    MEMCARD_INIT_STATUS_IRX_OK = 1,
};

/* MEMCARD DRIVER REQUIRES SIO2MAN DRIVER AS DEPENDENCY */
enum MEMCARD_INIT_STATUS init_memcard_driver(bool init_dependencies);
void deinit_memcard_driver(bool deinit_dependencies);

#ifdef __cplusplus
}
#endif

#endif /* PS2_MEMCARD_DRIVER */