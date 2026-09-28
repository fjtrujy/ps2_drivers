/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2026
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef PS2_CACHEIO_DRIVER
#define PS2_CACHEIO_DRIVER

#include <stdbool.h>
#include <cacheio.h>

#ifdef __cplusplus
extern "C" {
#endif

enum CACHEIO_INIT_STATUS {
    CACHEIO_INIT_STATUS_DEPENDENCY_FILEXIO_ERROR = -5,
    CACHEIO_INIT_STATUS_DEPENDENCY_BDM_ERROR = -4,
    CACHEIO_INIT_STATUS_LIBRARY_ERROR = -3,
    CACHEIO_INIT_STATUS_IRX_ERROR = -2,
    CACHEIO_INIT_STATUS_UNKNOWN = -1,
    CACHEIO_INIT_STATUS_OK = 0,
    CACHEIO_INIT_STATUS_IRX_OK = 1,
};

enum CACHEIO_INIT_STATUS init_cacheio_driver(bool init_dependencies);
void deinit_cacheio_driver(bool deinit_dependencies);

#ifdef __cplusplus
}
#endif

#endif /* PS2_CACHEIO_DRIVER */
