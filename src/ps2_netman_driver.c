/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_NETMAN_DRIVER
*/

#include <stdint.h>
#include <stddef.h>
#include <ps2_netman_driver.h>
#include <irx_common_macros.h>

#include <sifrpc.h>
#include <loadfile.h>

#include <netman.h>

EXTERN_IRX(netman_irx);
#ifdef F_internals_ps2_netman_driver
enum NETMAN_INIT_STATUS __netman_init_status = NETMAN_INIT_STATUS_UNKNOWN;
DECL_IRX_VARS(netman);
#else
extern enum NETMAN_INIT_STATUS __netman_init_status;
EXTERN_IRX_VARS(netman);
#endif

#ifdef F_init_ps2_netman_driver
static enum NETMAN_INIT_STATUS loadIRXs(void) {
    /* NETMAN.IRX */
    if (__netman_id > 0)
        return NETMAN_INIT_STATUS_OK;

    __netman_id = SifExecModuleBuffer(&netman_irx, size_netman_irx, 0, NULL, &__netman_ret);
    if (CHECK_IRX_ERR(netman))
        return NETMAN_INIT_STATUS_IRX_ERROR;

    return NETMAN_INIT_STATUS_IRX_OK;
}

static enum NETMAN_INIT_STATUS initLibraries(void) {
    return NetManInit() ? NETMAN_INIT_STATUS_EE_ERROR : NETMAN_INIT_STATUS_OK;
}

enum NETMAN_INIT_STATUS init_netman_driver(void) {
    __netman_init_status = loadIRXs();
     if (__netman_init_status < 0)
        return __netman_init_status;

    __netman_init_status = initLibraries();

    return __netman_init_status;
}
#endif

#ifdef F_deinit_ps2_netman_driver
static void unloadIRXs(void) {
    /* NETMAN.IRX */
    if (CHECK_IRX_UNLOAD(netman)) {
        SifUnloadModule(__netman_id);
        RESET_IRX_VARS(netman);
    }
}

void deinit_netman_driver(void) {
    NetManDeinit();
    unloadIRXs();
}
#endif
