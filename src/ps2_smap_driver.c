/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_SMAP_DRIVER
*/

#include <stdint.h>
#include <stddef.h>
#include <ps2_smap_driver.h>
#include <irx_common_macros.h>

#include <sifrpc.h>
#include <loadfile.h>

EXTERN_IRX(smap_irx);
#ifdef F_internals_ps2_smap_driver
enum SMAP_INIT_STATUS __smap_init_status = SMAP_INIT_STATUS_UNKNOWN;
DECL_IRX_VARS(smap);
#else
extern enum SMAP_INIT_STATUS __smap_init_status;
EXTERN_IRX_VARS(smap);
#endif

#ifdef F_init_ps2_smap_driver
static enum SMAP_INIT_STATUS loadIRXs(void) {
    /* SMAP.IRX */
    if (CHECK_IRX_LOAD(smap)) {
        __smap_id = SifExecModuleBuffer(&smap_irx, size_smap_irx, 0, NULL, &__smap_ret);
        if (CHECK_IRX_ERR(smap))
            return SMAP_INIT_STATUS_IRX_ERROR;
    }

    return SMAP_INIT_STATUS_OK;
}

enum SMAP_INIT_STATUS init_smap_driver(void) {
    __smap_init_status = loadIRXs();
    return __smap_init_status;
}
#endif

#ifdef F_deinit_ps2_smap_driver
static void unloadIRXs(void) {
    /* SMAP.IRX */
    if (CHECK_IRX_UNLOAD(smap)) {
        SifUnloadModule(__smap_id);
        RESET_IRX_VARS(smap);
    }
}

void deinit_smap_driver(void) {
    unloadIRXs();
}
#endif
