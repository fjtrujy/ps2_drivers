/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_CAMERA_DRIVER
*/

#include <stdint.h>
#include <stddef.h>
#include <ps2_camera_driver.h>
#include <ps2_usbd_driver.h>

#include <sifrpc.h>
#include <loadfile.h>
#include <ps2cam_rpc.h>
#include <irx_common_macros.h>

/* References to PSCAM.IRX */
EXTERN_IRX(ps2cam_irx);

#ifdef F_internals_ps2_camera_driver
enum CAMERA_INIT_STATUS __camera_init_status = CAMERA_INIT_STATUS_UNKNOWN;
DECL_IRX_VARS(camera);
#else
extern enum CAMERA_INIT_STATUS __camera_init_status;
EXTERN_IRX_VARS(camera);
#endif

#ifdef F_init_ps2_camera_driver
static enum CAMERA_INIT_STATUS loadIRXs(bool init_dependencies) {
    if (init_dependencies) {
        /* USBD.IRX */
        if (init_usbd_driver() < 0)
            return CAMERA_INIT_STATUS_DEPENDENCY_ERROR;
    }

    /* PSCAM.IRX */
    if (CHECK_IRX_LOAD(camera)) {
        __camera_id = SifExecModuleBuffer(&ps2cam_irx, size_ps2cam_irx, 0, NULL, &__camera_ret);
        if (CHECK_IRX_ERR(camera))
            return CAMERA_INIT_STATUS_IRX_ERROR;
    }

    return CAMERA_INIT_STATUS_IRX_OK;
}

static enum CAMERA_INIT_STATUS initLibraries(void) {
    /* Initializes camera library */
    if (PS2CamInit(0) < 0)
        return CAMERA_INIT_STATUS_EERPC_ERROR;

    return CAMERA_INIT_STATUS_OK;
}

enum CAMERA_INIT_STATUS init_camera_driver(bool init_dependencies) {
    __camera_init_status = loadIRXs(init_dependencies);
    if (__camera_init_status < 0)
        return __camera_init_status;

    __camera_init_status = initLibraries();

    return __camera_init_status;
}
#endif

#ifdef F_deinit_ps2_camera_driver
static void unloadIRXs(bool deinit_dependencies) {
    /* PSCAM.IRX */
    if (CHECK_IRX_UNLOAD(camera)) {
        SifUnloadModule(__camera_id);
        RESET_IRX_VARS(camera);
    }

    if (deinit_dependencies) {
        deinit_usbd_driver();
    }
}

void deinit_camera_driver(bool deinit_dependencies) {
    unloadIRXs(deinit_dependencies);
}
#endif
