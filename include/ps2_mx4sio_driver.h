/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_MX4SIO_DRIVER
*/

#ifndef PS2_MX4SIO_DRIVER
#define PS2_MX4SIO_DRIVER

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum MX4SIO_INIT_STATUS {
    MX4SIO_INIT_STATUS_IRX_NOT_SUPPORTED = -5,
    MX4SIO_INIT_STATUS_DEPENDENCY_BDM_ERROR = -4,
    MX4SIO_INIT_STATUS_DEPENDENCY_SIO2MAN_ERROR = -3,
    MX4SIO_INIT_STATUS_IRX_ERROR = -2,
    MX4SIO_INIT_STATUS_UNKNOWN = -1,
    MX4SIO_INIT_STATUS_OK = 0,
};

/* MX4SIO DRIVER REQUIRES SIO2MAN DRIVER AS DEPENDENCY */
enum MX4SIO_INIT_STATUS init_mx4sio_driver(bool init_dependencies);
void deinit_mx4sio_driver(bool deinit_dependencies);

#ifdef __cplusplus
}
#endif

#endif /* PS2_MX4SIO_DRIVER */
