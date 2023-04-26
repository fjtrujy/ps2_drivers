/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_DEV9_DRIVER
*/

#ifndef PS2_DEV9_DRIVER
#define PS2_DEV9_DRIVER

#ifdef __cplusplus
extern "C" {
#endif

enum DEV9_INIT_STATUS {
    DEV9_INIT_STATUS_IRX_ERROR = -2,
    DEV9_INIT_STATUS_UNKNOWN = -1,
    DEV9_INIT_STATUS_OK = 0,
};

enum DEV9_INIT_STATUS init_dev9_driver(void);
void deinit_dev9_driver(void);

#ifdef __cplusplus
}
#endif

#endif /* PS2_DEV9_DRIVER */