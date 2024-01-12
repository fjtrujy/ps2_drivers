/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_EEIP_DRIVER
*/

#ifndef PS2_EEIP_DRIVER
#define PS2_EEIP_DRIVER

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum EEIP_INIT_STATUS {
    EEIP_INIT_STATUS_DEPENDENCY_IRX_ERROR = -2,
    EEIP_INIT_STATUS_UNKNOWN = -1,
    EEIP_INIT_STATUS_OK = 0,
};

enum EEIP_INIT_STATUS init_eeip_driver(bool init_dependencies);
void deinit_eeip_driver(bool deinit_dependencies);

#ifdef __cplusplus
}
#endif

#endif /* PS2_EEIP_DRIVER */
