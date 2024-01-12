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
#include <stdbool.h>
#include <stddef.h>
#include <ps2_sio2man_driver.h>
#include <ps2_joystick_driver.h>
#include <irx_common_macros.h>

#include <sifrpc.h>
#include <loadfile.h>
#include <libmtap.h>
#include <libpad.h>

EXTERN_IRX(mtapman_irx);
EXTERN_IRX(padman_irx);


#ifdef F_internals_ps2_joystick_driver
enum JOYSTICK_INIT_STATUS __joystick_init_status = JOYSTICK_INIT_STATUS_UNKNOWN;
DECL_IRX_VARS(mtapman);
DECL_IRX_VARS(padman);
#else
extern enum JOYSTICK_INIT_STATUS __joystick_init_status;
EXTERN_IRX_VARS(mtapman);
EXTERN_IRX_VARS(padman);
#endif

#ifdef F_init_ps2_joystick_driver
static enum JOYSTICK_INIT_STATUS loadIRXs(void) {
    /* MTAPMAN.IRX */
    __mtapman_id = SifExecModuleBuffer(&mtapman_irx, size_mtapman_irx, 0, NULL, &__mtapman_ret);
    if (CHECK_IRX_ERR(mtapman))
        return JOYSTICK_INIT_STATUS_MTAP_IRX_ERROR;

    /* PADMAN.IRX */
    __padman_id = SifExecModuleBuffer(&padman_irx, size_padman_irx, 0, NULL, &__padman_ret);
    if (CHECK_IRX_ERR(padman))
        return JOYSTICK_INIT_STATUS_PAD_IRX_ERROR;

    return JOYSTICK_INIT_STATUS_IRX_OK;
}

static enum JOYSTICK_INIT_STATUS initLibraries(void) {
    /* Initializes pad un multitap libraries */
    if (mtapInit() != 1)
        return JOYSTICK_INIT_STATUS_MTAP_ERROR;

    if (padInit(0) != 1)
        return JOYSTICK_INIT_STATUS_PAD_ERROR;

    return JOYSTICK_INIT_STATUS_OK;
}

enum JOYSTICK_INIT_STATUS init_joystick_driver(bool init_dependencies) {
    // Requires to have SIO2MAN
    if (init_dependencies && init_sio2man_driver() != SIO2MAN_INIT_STATUS_OK)
        return JOYSTICK_INIT_STATUS_DEPENDENCY_IRX_ERROR;

    __joystick_init_status = loadIRXs();
    if (__joystick_init_status < 0)
        return __joystick_init_status;

    __joystick_init_status = initLibraries();

    return __joystick_init_status;
}
#endif

#ifdef F_deinit_ps2_joystick_driver
static void deinitLibraries(void) {
    padEnd();
}

static void unloadIRXs(void) {
    /* MTAPMAN.IRX */
    if (CHECK_IRX_UNLOAD(mtapman)) {
        SifUnloadModule(__mtapman_id);
        RESET_IRX_VARS(mtapman);
    }

    /* MTAPMAN.IRX */
    if (CHECK_IRX_UNLOAD(padman)) {
        SifUnloadModule(__padman_id);
        RESET_IRX_VARS(padman);
    }
}

void deinit_joystick_driver(bool deinit_dependencies) {
    deinitLibraries();
    unloadIRXs();

    // Requires to have SIO2MAN
    if (deinit_dependencies)
        deinit_sio2man_driver();
}
#endif
