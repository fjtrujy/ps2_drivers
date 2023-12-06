/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_MEMCARD_DRIVER
*/

#include <stdint.h>
#include <stddef.h>
#include <ps2_memcard_driver.h>
#include <ps2_sio2man_driver.h>
#include <irx_common_macros.h>

#include <sifrpc.h>
#include <loadfile.h>

EXTERN_IRX(mcman_irx);
EXTERN_IRX(mcserv_irx);


#ifdef F_internals_ps2_memcard_driver
enum MEMCARD_INIT_STATUS __memcard_init_status = MEMCARD_INIT_STATUS_UNKNOWN;
DECL_IRX_VARS(mcman);
DECL_IRX_VARS(mcserv);
#else
extern enum MEMCARD_INIT_STATUS __memcard_init_status;
EXTERN_IRX_VARS(mcman);
EXTERN_IRX_VARS(mcserv);
#endif

#ifdef F_init_ps2_memcard_driver
static enum MEMCARD_INIT_STATUS loadIRXs(void) {
    /* MCMAN.IRX */
    __mcman_id = SifExecModuleBuffer(&mcman_irx, size_mcman_irx, 0, NULL, &__mcman_ret);
    if (CHECK_IRX_ERR(mcman))
        return MEMCARD_INIT_STATUS_MCMAN_IRX_ERROR;

    /* MCSERV.IRX */
    __mcserv_id = SifExecModuleBuffer(&mcserv_irx, size_mcserv_irx, 0, NULL, &__mcserv_ret);
    if (CHECK_IRX_ERR(mcserv))
        return MEMCARD_INIT_STATUS_MCSERV_IRX_ERROR;

    return MEMCARD_INIT_STATUS_IRX_OK;
}

enum MEMCARD_INIT_STATUS init_memcard_driver(bool init_dependencies) {
    // Requires to have SIO2MAN
    if (init_dependencies && init_sio2man_driver() < 0)
        return MEMCARD_INIT_STATUS_DEPENDENCY_IRX_ERROR;

    __memcard_init_status = loadIRXs();
    return __memcard_init_status;
}
#endif

#ifdef F_deinit_ps2_memcard_driver
static void unloadIRXs(void) {
    /* MCMAN.IRX */
    if (CHECK_IRX_UNLOAD(mcman)) {
        SifUnloadModule(__mcman_id);
        RESET_IRX_VARS(mcman);
    }

    /* MCSERV.IRX */
    if (CHECK_IRX_UNLOAD(mcserv)) {
        SifUnloadModule(__mcserv_id);
        RESET_IRX_VARS(mcserv);
    }
}

void deinit_memcard_driver(bool deinit_dependencies) {
    unloadIRXs();

    // Requires to have SIO2MAN
    if (deinit_dependencies)
        deinit_sio2man_driver();
}
#endif
