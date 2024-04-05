/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_USBD_DRIVER
*/

#include <stdint.h>
#include <stddef.h>
#include <ps2_usbd_driver.h>
#include <irx_common_macros.h>

#include <sifrpc.h>
#include <loadfile.h>

EXTERN_IRX(usbd_irx);

#ifdef F_internals_ps2_usbd_driver
enum USBD_INIT_STATUS __usbd_init_status = USBD_INIT_STATUS_UNKNOWN;
DECL_IRX_VARS(usbd);
#else
extern enum USBD_INIT_STATUS __usbd_init_status;
EXTERN_IRX_VARS(usbd);
#endif

#ifdef F_init_ps2_usbd_driver
static enum USBD_INIT_STATUS loadIRXs(void) {
    /* USBD.IRX */
    if (CHECK_IRX_LOAD(usbd)) {
        __usbd_id = SifExecModuleBuffer(&usbd_irx, size_usbd_irx, 0, NULL, &__usbd_ret);
        if (CHECK_IRX_ERR(usbd))
            return USBD_INIT_STATUS_IRX_ERROR;
    }

    return USBD_INIT_STATUS_OK;
}

enum USBD_INIT_STATUS init_usbd_driver() {
    __usbd_init_status = loadIRXs();
    return __usbd_init_status;
}
#endif

#ifdef F_deinit_ps2_usbd_driver
static void unloadIRXs(void) {
    /* USBD.IRX */
    if (CHECK_IRX_UNLOAD(usbd)) {
        SifUnloadModule(__usbd_id);
        RESET_IRX_VARS(usbd);
    }
}

void deinit_usbd_driver() {
    unloadIRXs();
}
#endif
