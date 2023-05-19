/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_KEYBOARD_DRIVER
*/

#ifndef PS2_KEYBOARD_DRIVER
#define PS2_KEYBOARD_DRIVER

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum KEYBOARD_INIT_STATUS {
    KEYBOARD_INIT_STATUS_DEPENDENCY_ERROR = -4,
    KEYBOARD_INIT_STATUS_IRX_ERROR = -3,
    KEYBOARD_INIT_STATUS_EERPC_ERROR = -2,
    KEYBOARD_INIT_STATUS_UNKNOWN = -1,
    KEYBOARD_INIT_STATUS_OK = 0,
    KEYBOARD_INIT_STATUS_IRX_OK = 1,
};

/* KEYBOARD DRIVER REQUIRES USBD DRIVER AS DEPENDENCY */
enum KEYBOARD_INIT_STATUS init_keyboard_driver(bool init_dependencies);
void deinit_keyboard_driver(bool deinit_dependencies);

#ifdef __cplusplus
}
#endif

#endif /* PS2_KEYBOARD_DRIVER */