/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_MX4SIO_DRIVER
*/

#include <stdint.h>
#include <stddef.h>
#include <ps2_mx4sio_driver.h>
#include <ps2_sio2man_driver.h>
#include <ps2_bdm_driver.h>
#include <irx_common_macros.h>

#include <sifrpc.h>
#include <loadfile.h>

EXTERN_IRX(mx4sio_bd_irx);

#ifdef F_internals_ps2_mx4sio_driver
enum MX4SIO_INIT_STATUS __mx4sio_init_status = MX4SIO_INIT_STATUS_UNKNOWN;
DECL_IRX_VARS(mx4sio_bd);
#else
extern enum MX4SIO_INIT_STATUS __mx4sio_init_status;
EXTERN_IRX_VARS(mx4sio_bd);
#endif

#ifdef F_init_ps2_mx4sio_driver
static enum MX4SIO_INIT_STATUS loadIRXs(void) {
    /* MX4SIO_BD.IRX */
    if (CHECK_IRX_LOAD(mx4sio_bd)) {
        __mx4sio_bd_id = SifExecModuleBuffer(&mx4sio_bd_irx, size_mx4sio_bd_irx, 0, NULL, &__mx4sio_bd_ret);
        if (CHECK_IRX_ERR(mx4sio_bd))
            return MX4SIO_INIT_STATUS_IRX_ERROR;
    }

    return MX4SIO_INIT_STATUS_OK;
}

enum MX4SIO_INIT_STATUS init_mx4sio_driver(bool init_dependencies) {
    // Requires to have SIO2MAN
    if (init_dependencies && init_sio2man_driver() < 0)
        return MX4SIO_INIT_STATUS_DEPENDENCY_SIO2MAN_ERROR;

    /* Requires to have SIO2MAN BDM */
    if (init_dependencies && init_bdm_driver() < 0)
        return MX4SIO_INIT_STATUS_DEPENDENCY_BDM_ERROR;

    __mx4sio_init_status = loadIRXs();
    return __mx4sio_init_status;
}
#endif

#ifdef F_deinit_ps2_mx4sio_driver
static void unloadIRXs(void) {
    /* MX4SIO_BD.IRX */
    if (CHECK_IRX_UNLOAD(mx4sio_bd)) {
        SifUnloadModule(__mx4sio_bd_id);
        RESET_IRX_VARS(mx4sio_bd);
    }
}

void deinit_mx4sio_driver(bool deinit_dependencies) {
    unloadIRXs();

    if (deinit_dependencies) {
        deinit_bdm_driver();
        deinit_sio2man_driver();
    }
}
#endif
