/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_POWEROFF_DRIVER
*/

#ifndef PS2_POWEROFF_DRIVER
#define PS2_POWEROFF_DRIVER

#ifdef __cplusplus
extern "C" {
#endif

enum POWEROFF_INIT_STATUS {
    POWEROFF_INIT_STATUS_IRX_ERROR = -3,
    POWEROFF_INIT_STATUS_EERPC_ERROR = -2,
    POWEROFF_INIT_STATUS_UNKNOWN = -1,
    POWEROFF_INIT_STATUS_OK = 0,
    POWEROFF_INIT_STATUS_IRX_OK = 1,
};

enum POWEROFF_INIT_STATUS init_poweroff_driver(void);
void deinit_poweroff_driver(void);

#ifdef __cplusplus
}
#endif

#endif /* PS2_POWEROFF_DRIVER */