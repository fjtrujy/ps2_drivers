/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_MOUSE_DRIVER
*/

#include <stdint.h>
#include <stddef.h>
#include <ps2_mouse_driver.h>
#include <ps2_usbd_driver.h>
#include <irx_common_macros.h>

#include <sifrpc.h>
#include <loadfile.h>
#include <libmouse.h>

/* References to PS2MOUSE.IRX */
EXTERN_IRX(ps2mouse_irx);

#ifdef F_internals_ps2_mouse_driver
enum MOUSE_INIT_STATUS __mouse_init_status = MOUSE_INIT_STATUS_UNKNOWN;
DECL_IRX_VARS(mouse);
#else
extern enum MOUSE_INIT_STATUS __mouse_init_status;
EXTERN_IRX_VARS(mouse);
#endif

#ifdef F_init_ps2_mouse_driver
static enum MOUSE_INIT_STATUS loadIRXs(bool init_dependencies) {
    if (init_dependencies) {
        /* USBD.IRX */
        if (init_usbd_driver() < 0)
            return MOUSE_INIT_STATUS_DEPENDENCY_ERROR;
    }

    /* PS2MOUSE.IRX */
    __mouse_id = SifExecModuleBuffer(&ps2mouse_irx, size_ps2mouse_irx, 0, NULL, &__mouse_ret);
    if (CHECK_IRX_ERR(mouse))
        return MOUSE_INIT_STATUS_IRX_ERROR;

    return MOUSE_INIT_STATUS_IRX_OK;
}

static enum MOUSE_INIT_STATUS initLibraries(void) {
    /* Initializes mouse library */
    if (PS2MouseInit() < 0)
        return MOUSE_INIT_STATUS_EERPC_ERROR;

    PS2MouseReset();

    return MOUSE_INIT_STATUS_OK;
}

enum MOUSE_INIT_STATUS init_mouse_driver(bool init_dependencies) {
    __mouse_init_status = loadIRXs(init_dependencies);
    if (__mouse_init_status < 0)
        return __mouse_init_status;

    __mouse_init_status = initLibraries();

    return __mouse_init_status;
}
#endif

#ifdef F_deinit_ps2_mouse_driver
static void unloadIRXs(bool deinit_dependencies) {
    /* PS2MOUSE.IRX */
    if (CHECK_IRX_UNLOAD(mouse)) {
        SifUnloadModule(__mouse_id);
        RESET_IRX_VARS(mouse);
    }

    if (deinit_dependencies) {
        deinit_usbd_driver();
    }
}

void deinit_mouse_driver(bool deinit_dependencies) {
    unloadIRXs(deinit_dependencies);
}
#endif
