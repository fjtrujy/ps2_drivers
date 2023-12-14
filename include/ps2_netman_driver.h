/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_NETMAN_DRIVER
*/

#ifndef PS2_NETMAN_DRIVER
#define PS2_NETMAN_DRIVER

#ifdef __cplusplus
extern "C" {
#endif

enum NETMAN_INIT_STATUS {
    NETMAN_INIT_STATUS_EE_ERROR = -3,
    NETMAN_INIT_STATUS_IRX_ERROR = -2,
    NETMAN_INIT_STATUS_UNKNOWN = -1,
    NETMAN_INIT_STATUS_OK = 0,
    NETMAN_INIT_STATUS_IRX_OK = 1,
};

enum NETMAN_INIT_STATUS init_netman_driver(void);
void deinit_netman_driver(void);

#ifdef __cplusplus
}
#endif

#endif /* PS2_NETMAN_DRIVER */
