/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_EEIP_DRIVER
*/

#include <stdint.h>
#include <stddef.h>
#include <ps2_netman_driver.h>
#include <ps2_smap_driver.h>
#include <ps2_eeip_driver.h>
#include <irx_common_macros.h>

#include <sifrpc.h>
#include <loadfile.h>

#ifdef F_internals_ps2_eeip_driver
enum EEIP_INIT_STATUS __eeip_init_status = EEIP_INIT_STATUS_UNKNOWN;
#else
extern enum EEIP_INIT_STATUS __eeip_init_status;
#endif

#ifdef F_init_ps2_eeip_driver
static enum EEIP_INIT_STATUS loadIRXs(void) {
    return EEIP_INIT_STATUS_OK;
}

enum EEIP_INIT_STATUS init_eeip_driver(bool init_dependencies) {
    
    if (init_dependencies) {
        // Requires to have NETMAN
        if (init_netman_driver() != NETMAN_INIT_STATUS_OK)
            return EEIP_INIT_STATUS_DEPENDENCY_IRX_ERROR;

        // Requires to have SMAP
        if (init_smap_driver() != SMAP_INIT_STATUS_OK)
            return EEIP_INIT_STATUS_DEPENDENCY_IRX_ERROR;
    } 

    __eeip_init_status = loadIRXs();
    return __eeip_init_status;
}
#endif

#ifdef F_deinit_ps2_eeip_driver
static void deinitLibraries(void) {}

static void unloadIRXs(void) {}

void deinit_eeip_driver(bool deinit_dependencies) {
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
