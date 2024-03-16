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

#ifndef PS2_EEIP_DRIVER
#define PS2_EEIP_DRIVER

#include <stdbool.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

enum EEIP_INIT_STATUS {
    EEIP_INIT_STATUS_DEPENDENCY_IRX_ERROR = -2,
    EEIP_INIT_STATUS_UNKNOWN = -1,
    EEIP_INIT_STATUS_OK = 0,
};

enum EEIP_INIT_STATUS init_eeip_driver(bool init_dependencies);
void deinit_eeip_driver(bool deinit_dependencies);

enum EEIP_CONFIG_STATUS {
    EEIP_CONFIG_STATUS_CANT_GET_VALID_DHCP_STATUS = -4,
    EEIP_CONFIG_STATUS_CANT_GET_VALID_LINK_STATUS = -3,
    EEIP_CONFIG_STATUS_CANT_SET_IP_CONFIG = -2,
    EEIP_CONFIG_STATUS_CANT_SET_LINK_MODE = -1,
    EEIP_CONFIG_STATUS_OK = 0,
};

enum EEIP_CONFIG_STATUS config_network_ps2_eeip_driver(void);
enum EEIP_CONFIG_STATUS config_network_custom_ps2_eeip_driver(struct ip4_addr *IP, struct ip4_addr *NM, struct ip4_addr *GW);

#ifdef __cplusplus
}
#endif

#endif /* PS2_EEIP_DRIVER */
