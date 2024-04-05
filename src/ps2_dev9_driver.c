/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_DEV9_DRIVER
*/

#include <stdint.h>
#include <stddef.h>
#include <ps2_dev9_driver.h>
#include <irx_common_macros.h>

#include <sifrpc.h>
#include <loadfile.h>

#define NEWLIB_PORT_AWARE
#include <fileXio_rpc.h>
#include <hdd-ioctl.h>

EXTERN_IRX(ps2dev9_irx);

#ifdef F_internals_ps2_dev9_driver
enum DEV9_INIT_STATUS __dev9_init_status = DEV9_INIT_STATUS_UNKNOWN;
DECL_IRX_VARS(ps2dev9);
#else
extern enum DEV9_INIT_STATUS __dev9_init_status;
EXTERN_IRX_VARS(ps2dev9);
#endif

#ifdef F_init_ps2_dev9_driver
static enum DEV9_INIT_STATUS loadIRXs(void) {
    /* PS2DEV9.IRX */
    if (CHECK_IRX_LOAD(ps2dev9)) {
        __ps2dev9_id = SifExecModuleBuffer(&ps2dev9_irx, size_ps2dev9_irx, 0, NULL, &__ps2dev9_ret);
        if (CHECK_IRX_ERR(ps2dev9))
            return DEV9_INIT_STATUS_IRX_ERROR;
    }

    return DEV9_INIT_STATUS_OK;
}

enum DEV9_INIT_STATUS init_dev9_driver() {
    __dev9_init_status = loadIRXs();
    return __dev9_init_status;
}
#endif

#ifdef F_deinit_ps2_dev9_driver
static void unloadIRXs(void) {
    /* DEV9.IRX */
    if (CHECK_IRX_UNLOAD(ps2dev9)) {
        SifUnloadModule(__ps2dev9_id);
        RESET_IRX_VARS(ps2dev9);
    }
}

void deinit_dev9_driver() {
    if (__dev9_init_status == DEV9_INIT_STATUS_OK) {
        while (fileXioDevctl("dev9x:", DDIOC_OFF, NULL, 0, NULL, 0) < 0) {};
    }
    unloadIRXs();
}
#endif
