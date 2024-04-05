/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_POWEROFF_DRIVER
*/

#include <stdint.h>
#include <stddef.h>
#include <ps2_poweroff_driver.h>
#include <irx_common_macros.h>

#include <sifrpc.h>
#include <loadfile.h>
#include <libpwroff.h>

EXTERN_IRX(poweroff_irx);
#ifdef F_internals_ps2_poweroff_driver
enum POWEROFF_INIT_STATUS __poweroff_init_status = POWEROFF_INIT_STATUS_UNKNOWN;
DECL_IRX_VARS(poweroff);
#else
extern enum POWEROFF_INIT_STATUS __poweroff_init_status;
EXTERN_IRX_VARS(poweroff);
#endif

#ifdef F_init_ps2_poweroff_driver
static enum POWEROFF_INIT_STATUS loadIRXs(void) {
    /* POWEROFF.IRX */
    if (CHECK_IRX_LOAD(poweroff)) {
        __poweroff_id = SifExecModuleBuffer(&poweroff_irx, size_poweroff_irx, 0, NULL, &__poweroff_ret);
        if (CHECK_IRX_ERR(poweroff))
            return POWEROFF_INIT_STATUS_IRX_ERROR;
    }

    return POWEROFF_INIT_STATUS_IRX_OK;
}

static enum POWEROFF_INIT_STATUS initLibraries(void) {
    /* Initializes poweroff library */
    if (poweroffInit() != 0)
        return POWEROFF_INIT_STATUS_EERPC_ERROR;

    return POWEROFF_INIT_STATUS_OK;
}

enum POWEROFF_INIT_STATUS init_poweroff_driver() {
    __poweroff_init_status = loadIRXs();
    if (__poweroff_init_status < 0)
        return __poweroff_init_status;

    __poweroff_init_status = initLibraries();

    return __poweroff_init_status;
}
#endif

#ifdef F_deinit_ps2_poweroff_driver
static void unloadIRXs(void) {
    /* POWEROFF.IRX */
    if (CHECK_IRX_UNLOAD(poweroff)) {
        SifUnloadModule(__poweroff_id);
        RESET_IRX_VARS(poweroff);
    }
}

void deinit_poweroff_driver() {
    unloadIRXs();
}
#endif
