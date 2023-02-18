/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_FILEXIO_DRIVER
*/

#ifndef PS2_FILEXIO_DRIVER
#define PS2_FILEXIO_DRIVER

#ifdef __cplusplus
extern "C" {
#endif

enum FILEXIO_INIT_STATUS {
    FILEXIO_INIT_STATUS_DEPENDENCY_IRX_ERROR = -4,
    FILEXIO_INIT_STATUS_FILEXIO_IRX_ERROR = -3,
    FILEXIO_INIT_STATUS_IOMANX_IRX_ERROR = -2,
    FILEXIO_INIT_STATUS_UNKNOWN = -1,
    FILEXIO_INIT_STATUS_OK = 0,
    FILEXIO_INIT_STATUS_IRX_OK = 1,
};

/* FILEXIO DRIVER REQUIRES SIO2MAN DRIVER AS DEPENDENCY */
enum FILEXIO_INIT_STATUS init_fileXio_driver();
void deinit_fileXio_driver();

#ifdef __cplusplus
}
#endif

#endif /* PS2_FILEXIO_DRIVER */