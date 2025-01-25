/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_BDM_DRIVER
*/

#ifndef PS2_BDM_DRIVER
#define PS2_BDM_DRIVER

#ifdef __cplusplus
extern "C" {
#endif

enum BDM_INIT_STATUS {
    BDM_INIT_STATUS_IRX_FATFS_ERROR = -3,
    BDM_INIT_STATUS_IRX_ERROR = -2,
    BDM_INIT_STATUS_UNKNOWN = -1,
    BDM_INIT_STATUS_OK = 0,
};

enum BDM_INIT_STATUS init_bdm_driver(void);
void deinit_bdm_driver(void);

#ifdef __cplusplus
}
#endif

#endif /* PS2_BDM_DRIVER */
