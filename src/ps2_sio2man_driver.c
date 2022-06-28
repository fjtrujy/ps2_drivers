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

#include <sifrpc.h>
#include <loadfile.h>

/* References to SIO2MAN.IRX */
extern unsigned char sio2man_irx[] __attribute__((aligned(16)));
extern unsigned int size_sio2man_irx;

#ifdef F_internals_ps2_sio2man_driver
enum SIO2MAN_INIT_STATUS __sio2man_init_status = SIO2MAN_INIT_STATUS_UNKNOWN;
int32_t __sio2man_id = -1;
#else
extern enum SIO2MAN_INIT_STATUS __sio2man_init_status;
extern int32_t __sio2man_id;
#endif

#ifdef F_init_ps2_sio2man_driver
static enum SIO2MAN_INIT_STATUS loadIRXs(void) {
    /* SIO2MAN.IRX */
    if (__sio2man_id > 0)
        return SIO2MAN_INIT_STATUS_OK;

    __sio2man_id = SifExecModuleBuffer(&sio2man_irx, size_sio2man_irx, 0, NULL, NULL);
    if (__sio2man_id < 0)
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
    /* MTAPMAN.IRX */
    if (__sio2man_id > 0) {
        SifUnloadModule(__sio2man_id);
        __sio2man_id = -1;
    }
}

void deinit_sio2man_driver(void) {
    unloadIRXs();
}
#endif