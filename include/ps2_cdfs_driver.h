/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_CDFS_DRIVER
*/

#ifndef PS2_CDFS_DRIVER
#define PS2_CDFS_DRIVER

#ifdef __cplusplus
extern "C" {
#endif

enum CDFS_INIT_STATUS {
    CDFS_INIT_STATUS_IRX_ERROR = -2,
    CDFS_INIT_STATUS_UNKNOWN = -1,
    CDFS_INIT_STATUS_IRX_OK = 0,
};

enum CDFS_INIT_STATUS init_cdfs_driver();
void deinit_cdfs_driver();

#ifdef __cplusplus
}
#endif

#endif /* PS2_CDFS_DRIVER */