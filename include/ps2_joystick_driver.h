/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_JOYSTICK_DRIVER
*/

#ifndef PS2_JOYSTICK_DRIVER
#define PS2_JOYSTICK_DRIVER

enum JOYSTICK_INIT_STATUS{
    JOYSTICK_INIT_STATUS_PAD_IRX_ERROR = -5,
    JOYSTICK_INIT_STATUS_MTAP_IRX_ERROR = -4,
    JOYSTICK_INIT_STATUS_PAD_ERROR = -3,
    JOYSTICK_INIT_STATUS_MTAP_ERROR = -2,
    JOYSTICK_INIT_STATUS_UNKNOWN = -1,
    JOYSTICK_INIT_STATUS_OK = 0,
    JOYSTICK_INIT_STATUS_IRX_OK = 1,
};

enum JOYSTICK_INIT_STATUS init_joystick_driver(void);
void deinit_joystick_driver(void);

#endif /* PS2_JOYSTICK_DRIVER */