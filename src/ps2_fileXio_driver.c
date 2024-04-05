/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_FILEXIO_DRIVER
*/

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <ps2_sio2man_driver.h>
#include <ps2_fileXio_driver.h>
#include <irx_common_macros.h>

#include <sifrpc.h>
#include <loadfile.h>

int fileXioInit(void);
void fileXioExit(void);

EXTERN_IRX(iomanX_irx);
EXTERN_IRX(fileXio_irx);

#ifdef F_internals_ps2_fileXio_driver
enum FILEXIO_INIT_STATUS __fileXio_init_status = FILEXIO_INIT_STATUS_UNKNOWN;
DECL_IRX_VARS(iomanX);
DECL_IRX_VARS(fileXio);
#else
extern enum FILEXIO_INIT_STATUS __fileXio_init_status;
EXTERN_IRX_VARS(iomanX);
EXTERN_IRX_VARS(fileXio);
#endif

#ifdef F_init_ps2_fileXio_driver
static enum FILEXIO_INIT_STATUS loadIRXs(void) {
    /* IOMANX.IRX */
    if (CHECK_IRX_LOAD(iomanX)) {
        __iomanX_id = SifExecModuleBuffer(&iomanX_irx, size_iomanX_irx, 0, NULL, &__iomanX_ret);
        if (CHECK_IRX_ERR(iomanX))
            return FILEXIO_INIT_STATUS_IOMANX_IRX_ERROR;
    }

    /* FILEXIO.IRX */
    if (CHECK_IRX_LOAD(fileXio)) {
        __fileXio_id = SifExecModuleBuffer(&fileXio_irx, size_fileXio_irx, 0, NULL, &__fileXio_ret);
        if (CHECK_IRX_ERR(fileXio))
            return FILEXIO_INIT_STATUS_FILEXIO_IRX_ERROR;
    }

    return FILEXIO_INIT_STATUS_IRX_OK;
}

static enum FILEXIO_INIT_STATUS initLibraries(void) {
    fileXioInit();

    return FILEXIO_INIT_STATUS_OK;
}

enum FILEXIO_INIT_STATUS init_fileXio_driver() {
    __fileXio_init_status = loadIRXs();
    if (__fileXio_init_status < 0)
        return __fileXio_init_status;

    __fileXio_init_status = initLibraries();

    return __fileXio_init_status;
}
#endif

#ifdef F_deinit_ps2_fileXio_driver
static void unloadIRXs(void) {
    /* FILEXIO.IRX */
    if (CHECK_IRX_UNLOAD(fileXio)) {
        SifUnloadModule(__fileXio_id);
        RESET_IRX_VARS(fileXio);
    }

    /* IOMANX.IRX */
    if (CHECK_IRX_UNLOAD(iomanX)) {
        SifUnloadModule(__iomanX_id);
        RESET_IRX_VARS(iomanX);
    }
}

void deinit_fileXio_driver() {
    fileXioExit();
    unloadIRXs();
}
#endif
