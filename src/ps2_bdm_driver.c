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
#include <ps2_bdm_driver.h>
#include <irx_common_macros.h>

#include <sifrpc.h>
#include <loadfile.h>

EXTERN_IRX(bdm_irx);
#ifdef F_internals_ps2_bdm_driver
enum BDM_INIT_STATUS __bdm_init_status = BDM_INIT_STATUS_UNKNOWN;
DECL_IRX_VARS(bdm);
#else
extern enum BDM_INIT_STATUS __bdm_init_status;
EXTERN_IRX_VARS(bdm);
#endif

#ifdef F_init_ps2_bdm_driver
static enum BDM_INIT_STATUS loadIRXs(void) {
    /* BDM.IRX */
    if (CHECK_IRX_LOAD(bdm)) {
        __bdm_id = SifExecModuleBuffer(&bdm_irx, size_bdm_irx, 0, NULL, &__bdm_ret);
        if (CHECK_IRX_ERR(bdm))
            return BDM_INIT_STATUS_IRX_ERROR;
    }

    return BDM_INIT_STATUS_OK;
}

enum BDM_INIT_STATUS init_bdm_driver(void) {
    __bdm_init_status = loadIRXs();
    return __bdm_init_status;
}
#endif

#ifdef F_deinit_ps2_bdm_driver
static void unloadIRXs(void) {
    /* BDM.IRX */
    if (CHECK_IRX_UNLOAD(bdm)) {
        SifUnloadModule(__bdm_id);
        RESET_IRX_VARS(bdm);
    }
}

void deinit_bdm_driver(void) {
    unloadIRXs();
}
#endif
