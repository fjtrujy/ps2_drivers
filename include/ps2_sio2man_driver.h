/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_SIO2MAN_DRIVER
*/

#ifndef PS2_SIO2MAN_DRIVER
#define PS2_SIO2MAN_DRIVER

#ifdef __cplusplus
extern "C" {
#endif

enum SIO2MAN_INIT_STATUS {
    SIO2MAN_INIT_STATUS_IRX_ERROR = -2,
    SIO2MAN_INIT_STATUS_UNKNOWN = -1,
    SIO2MAN_INIT_STATUS_OK = 0,
};

enum SIO2MAN_INIT_STATUS init_sio2man_driver(void);
void deinit_sio2man_driver(void);

#ifdef __cplusplus
}
#endif

#endif /* PS2_SIO2MAN_DRIVER */