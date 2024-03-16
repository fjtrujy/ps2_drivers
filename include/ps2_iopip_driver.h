/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_IOPIP_DRIVER
*/

#ifndef PS2_IOPIP_DRIVER
#define PS2_IOPIP_DRIVER

#include <stdbool.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

enum IOPIP_INIT_STATUS {
    IOPIP_INIT_STATUS_DEPENDENCY_IRX_ERROR = -5,
    IOPIP_INIT_STATUS_PS2IP_NM_IRX_ERROR = -4,
    IOPIP_INIT_STATUS_PS2IPS_IRX_ERROR = -3,
    IOPIP_INIT_STATUS_PS2IPS_ERROR = -2,
    IOPIP_INIT_STATUS_UNKNOWN = -1,
    IOPIP_INIT_STATUS_OK = 0,
};

enum IOPIP_INIT_STATUS init_iopip_driver(bool init_dependencies, struct ip4_addr *IP, struct ip4_addr *NM, struct ip4_addr *GW);
void deinit_iopip_driver(bool deinit_dependencies);

#ifdef __cplusplus
}
#endif

#endif /* PS2_IOPIP_DRIVER */
