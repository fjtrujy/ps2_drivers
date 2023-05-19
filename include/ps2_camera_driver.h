/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_CAMERA_DRIVER
*/

#ifndef PS2_CAMERA_DRIVER
#define PS2_CAMERA_DRIVER

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum CAMERA_INIT_STATUS {
    CAMERA_INIT_STATUS_DEPENDENCY_ERROR = -4,
    CAMERA_INIT_STATUS_IRX_ERROR = -3,
    CAMERA_INIT_STATUS_EERPC_ERROR = -2,
    CAMERA_INIT_STATUS_UNKNOWN = -1,
    CAMERA_INIT_STATUS_OK = 0,
    CAMERA_INIT_STATUS_IRX_OK = 1,
};

/* CAMERA DRIVER REQUIRES USBD DRIVER AS DEPENDENCY */
enum CAMERA_INIT_STATUS init_camera_driver(bool init_dependencies);
void deinit_camera_driver(bool deinit_dependencies);

#ifdef __cplusplus
}
#endif

#endif /* PS2_CAMERA_DRIVER */