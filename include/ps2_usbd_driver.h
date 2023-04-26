/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_USBD_DRIVER
*/

#ifndef PS2_USBD_DRIVER
#define PS2_USBD_DRIVER

#ifdef __cplusplus
extern "C" {
#endif

enum USBD_INIT_STATUS {
    USBD_INIT_STATUS_IRX_ERROR = -2,
    USBD_INIT_STATUS_UNKNOWN = -1,
    USBD_INIT_STATUS_OK = 0,
};

enum USBD_INIT_STATUS init_usbd_driver(void);
void deinit_usbd_driver(void);

#ifdef __cplusplus
}
#endif

#endif /* PS2_USBD_DRIVER */