/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_CDFS_DRIVER
*/

#include <stdint.h>
#include <stddef.h>
#include <ps2_cdfs_driver.h>
#include <irx_common_macros.h>

#include <sifrpc.h>
#include <loadfile.h>

/* References to CDFS.IRX */
EXTERN_IRX(cdfs_irx);

#ifdef F_internals_ps2_cdfs_driver
enum CDFS_INIT_STATUS __cdfs_init_status = CDFS_INIT_STATUS_UNKNOWN;
DECL_IRX_VARS(cdfs);
#else
extern enum CDFS_INIT_STATUS __cdfs_init_status;
EXTERN_IRX_VARS(cdfs);
#endif

#ifdef F_init_ps2_cdfs_driver
static enum CDFS_INIT_STATUS loadIRXs(void) {
    /* CDFS.IRX */
    if (CHECK_IRX_LOAD(cdfs)) {
        __cdfs_id = SifExecModuleBuffer(&cdfs_irx, size_cdfs_irx, 0, NULL, &__cdfs_ret);
        if (CHECK_IRX_ERR(cdfs))
            return CDFS_INIT_STATUS_IRX_ERROR;
    }

    return CDFS_INIT_STATUS_IRX_OK;
}

enum CDFS_INIT_STATUS init_cdfs_driver() {
    __cdfs_init_status = loadIRXs();
    return __cdfs_init_status;
}
#endif

#ifdef F_deinit_ps2_cdfs_driver
static void unloadIRXs(void) {
    /* CDFS.IRX */
    if (CHECK_IRX_UNLOAD(cdfs)) {
        SifUnloadModule(__cdfs_id);
        RESET_IRX_VARS(cdfs);
    }
}

void deinit_cdfs_driver() {
    unloadIRXs();
}
#endif
