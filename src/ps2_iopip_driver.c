/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_IOPIP_DRIVER
*/

#include <stdint.h>
#include <stddef.h>
#include <ps2_netman_driver.h>
#include <ps2_smap_driver.h>
#include <ps2_iopip_driver.h>
#include <irx_common_macros.h>

#include <sifrpc.h>
#include <loadfile.h>
#include <ps2ips.h>

EXTERN_IRX(ps2ips_irx);
EXTERN_IRX(ps2ip_nm_irx);

#ifdef F_internals_ps2_iopip_driver
enum IOPIP_INIT_STATUS __iopip_init_status = IOPIP_INIT_STATUS_UNKNOWN;
DECL_IRX_VARS(ps2ip_nm);
DECL_IRX_VARS(ps2ips);
#else
extern enum IOPIP_INIT_STATUS __iopip_init_status;
EXTERN_IRX_VARS(ps2ip_nm);
EXTERN_IRX_VARS(ps2ips);
#endif

#ifdef F_init_ps2_iopip_driver
static enum IOPIP_INIT_STATUS loadIRXs(void) {
    /* PS2IP_NM.IRX */
    if (CHECK_IRX_LOAD(ps2ip_nm)) {
        __ps2ip_nm_id = SifExecModuleBuffer(&ps2ip_nm_irx, size_ps2ip_nm_irx, 0, NULL, &__ps2ip_nm_ret);
        if (CHECK_IRX_ERR(ps2ip_nm))
            return IOPIP_INIT_STATUS_PS2IP_NM_IRX_ERROR;
    }
    
    /* PS2IPS.IRX */
    if (CHECK_IRX_LOAD(ps2ips)) {
        __ps2ips_id = SifExecModuleBuffer(&ps2ips_irx, size_ps2ips_irx, 0, NULL, &__ps2ips_ret);
        if (CHECK_IRX_ERR(ps2ips))
            return IOPIP_INIT_STATUS_PS2IPS_IRX_ERROR;
    }

    return IOPIP_INIT_STATUS_OK;
}

static enum IOPIP_INIT_STATUS initLibraries(void) {
    /* Initializes NETMAN library */
    if (ps2ip_init())
        return IOPIP_INIT_STATUS_PS2IPS_ERROR;

    return IOPIP_INIT_STATUS_OK;
}

enum IOPIP_INIT_STATUS init_iopip_driver(bool init_dependencies) {
    
    if (init_dependencies) {
        // Requires to have NETMAN
        if (init_netman_driver() != NETMAN_INIT_STATUS_OK)
            return IOPIP_INIT_STATUS_DEPENDENCY_IRX_ERROR;

        // Requires to have SMAP
        if (init_smap_driver() != SMAP_INIT_STATUS_OK)
            return IOPIP_INIT_STATUS_DEPENDENCY_IRX_ERROR;
    } 

    __iopip_init_status = loadIRXs();
    if (__iopip_init_status < 0)
        return __iopip_init_status;
    
    __iopip_init_status = initLibraries();

    return __iopip_init_status;
}
#endif

#ifdef F_deinit_ps2_iopip_driver
static void deinitLibraries(void) {
    ps2ip_deinit();
}

static void unloadIRXs(void) {
    /* PS2IPS.IRX */
    if (CHECK_IRX_UNLOAD(ps2ips)) {
        SifUnloadModule(__ps2ips_id);
        RESET_IRX_VARS(ps2ips);
    }

    /* PS2IP_NM.IRX */
    if (CHECK_IRX_UNLOAD(ps2ip_nm)) {
        SifUnloadModule(__ps2ip_nm_id);
        RESET_IRX_VARS(ps2ip_nm);
    }
}

void deinit_iopip_driver(bool deinit_dependencies) {
    deinitLibraries();
    unloadIRXs();

    if (deinit_dependencies) {
        // Requires to have SMAP
        deinit_smap_driver();
        // Requires to have NETMAN
        deinit_netman_driver();
    }
}
#endif
