/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2026
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
*/

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <ps2_bdm_driver.h>
#include <ps2_cacheio_driver.h>
#include <ps2_fileXio_driver.h>
#include <irx_common_macros.h>

#include <sifrpc.h>
#include <loadfile.h>

EXTERN_PS2_IRX_MODULE(cacheio);

#ifdef F_internals_ps2_cacheio_driver
enum CACHEIO_INIT_STATUS __cacheio_init_status = CACHEIO_INIT_STATUS_UNKNOWN;
DECL_IRX_VARS(cacheio);
#else
extern enum CACHEIO_INIT_STATUS __cacheio_init_status;
EXTERN_IRX_VARS(cacheio);
#endif

#ifdef F_init_ps2_cacheio_driver
static enum CACHEIO_INIT_STATUS loadIRXs(void)
{
    if (CHECK_IRX_LOAD(cacheio)) {
        __cacheio_id = ps2_irx_exec(&ps2_irx_cacheio, 0, NULL, &__cacheio_ret);
        if (CHECK_IRX_ERR(cacheio))
            return CACHEIO_INIT_STATUS_IRX_ERROR;
    }

    return CACHEIO_INIT_STATUS_IRX_OK;
}

enum CACHEIO_INIT_STATUS init_cacheio_driver(bool init_dependencies)
{
    if (init_dependencies && init_fileXio_driver() < 0)
        return CACHEIO_INIT_STATUS_DEPENDENCY_FILEXIO_ERROR;

    if (init_dependencies && init_bdm_driver() < 0)
        return CACHEIO_INIT_STATUS_DEPENDENCY_BDM_ERROR;

    __cacheio_init_status = loadIRXs();
    if (__cacheio_init_status < 0)
        return __cacheio_init_status;

    if (cacheioInit() < 0) {
        __cacheio_init_status = CACHEIO_INIT_STATUS_LIBRARY_ERROR;
        return __cacheio_init_status;
    }

    __cacheio_init_status = CACHEIO_INIT_STATUS_OK;
    return __cacheio_init_status;
}
#endif

#ifdef F_deinit_ps2_cacheio_driver
static void unloadIRXs(void)
{
    if (CHECK_IRX_UNLOAD(cacheio)) {
        SifUnloadModule(__cacheio_id);
        RESET_IRX_VARS(cacheio);
    }
}

void deinit_cacheio_driver(bool deinit_dependencies)
{
    cacheioExit();
    unloadIRXs();

    if (deinit_dependencies) {
        deinit_bdm_driver();
        deinit_fileXio_driver();
    }

    __cacheio_init_status = CACHEIO_INIT_STATUS_UNKNOWN;
}
#endif
