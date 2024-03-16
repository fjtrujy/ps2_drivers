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

#ifndef PS2_NETWORK_DRIVER
#define PS2_NETWORK_DRIVER

#include <ps2_eeip_driver.h>
#include <ps2_iopip_driver.h>

#ifdef __cplusplus
extern "C" {
#endif

// By default we will use the EEIP driver
static inline enum EEIP_INIT_STATUS init_network_driver(bool init_dependencies)
{
    return init_eeip_driver(init_dependencies);
}

static inline void deinit_network_driver(bool deinit_dependencies)
{
    deinit_eeip_driver(deinit_dependencies);
}

static inline enum EEIP_INIT_STATUS init_network_EE_driver(bool init_dependencies)
{
    return init_eeip_driver(init_dependencies);
}

static inline void deinit_network_EE_driver(bool deinit_dependencies)
{
    deinit_eeip_driver(deinit_dependencies);
}

static inline enum IOPIP_INIT_STATUS init_network_IOP_driver(bool init_dependencies)
{
    return init_iopip_driver(init_dependencies);
}

static inline void deinit_network_IOP_driver(bool deinit_dependencies)
{
    init_iopip_driver(deinit_dependencies);
}

#ifdef __cplusplus
}
#endif

#endif /* PS2_EEIP_DRIVER */
