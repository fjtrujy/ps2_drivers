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

#include <stdint.h>
#include <stddef.h>
#include <ps2_sio2man_driver.h>
#include <irx_common_macros.h>

#include <sifrpc.h>
#include <loadfile.h>

EXTERN_IRX(sio2man_irx);
#ifdef F_internals_ps2_sio2man_driver
enum SIO2MAN_INIT_STATUS __sio2man_init_status = SIO2MAN_INIT_STATUS_UNKNOWN;
DECL_IRX_VARS(sio2man);
#else
extern enum SIO2MAN_INIT_STATUS __sio2man_init_status;
EXTERN_IRX_VARS(sio2man);
#endif

#ifdef F_init_ps2_sio2man_driver
static enum SIO2MAN_INIT_STATUS loadIRXs(void) {
    /* SIO2MAN.IRX */
    if (__sio2man_id > 0)
        return SIO2MAN_INIT_STATUS_OK;

    __sio2man_id = SifExecModuleBuffer(&sio2man_irx, size_sio2man_irx, 0, NULL, &__sio2man_ret);
    if (CHECK_IRX_ERR(sio2man))
        return SIO2MAN_INIT_STATUS_IRX_ERROR;

    return SIO2MAN_INIT_STATUS_OK;
}

enum SIO2MAN_INIT_STATUS init_sio2man_driver(void) {
    __sio2man_init_status = loadIRXs();
    return __sio2man_init_status;
}
#endif

#ifdef F_deinit_ps2_sio2man_driver
static void unloadIRXs(void) {
    /* SIO2MAN.IRX */
    if (CHECK_IRX_UNLOAD(sio2man)) {
        SifUnloadModule(__sio2man_id);
        RESET_IRX_VARS(sio2man);
    }
}

void deinit_sio2man_driver(void) {
    unloadIRXs();
}
#endif
