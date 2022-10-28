/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_HDD_DRIVER
*/

#include <stdbool.h>

#ifndef PS2_HDD_DRIVER
#define PS2_HDD_DRIVER

enum HDD_INIT_STATUS {
    HDD_INIT_STATUS_HDD_NOT_READY_ERROR = -6,
    HDD_INIT_STATUS_PS2FS_IRX_ERROR = -5,
    HDD_INIT_STATUS_PS2HDD_IRX_ERROR = -4,
    HDD_INIT_STATUS_PS2ATAD_IRX_ERROR = -3,
    HDD_INIT_STATUS_PS2DEV9_IRX_ERROR = -2,
    HDD_INIT_STATUS_UNKNOWN = -1,
    HDD_INIT_STATUS_IRX_OK = 0,
};

enum HDD_INIT_STATUS init_hdd_driver();
void deinit_hdd_driver();

#endif /* PS2_HDD_DRIVER */