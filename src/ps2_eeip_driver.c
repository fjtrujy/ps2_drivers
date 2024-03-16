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
#include <tcpip.h>
#include <sys/socket.h>

#include <ps2_dev9_driver.h>
#include <ps2_netman_driver.h>
#include <ps2_smap_driver.h>
#include <ps2_eeip_driver.h>
#include <irx_common_macros.h>

#include <sifrpc.h>
#include <loadfile.h>
#include <netman.h>
#include <ps2ip.h>

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
        // Requires to have DEV9
        if (init_dev9_driver() != DEV9_INIT_STATUS_OK)
            return EEIP_INIT_STATUS_DEPENDENCY_IRX_ERROR;

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
        // Requires to have DEV9
        deinit_dev9_driver();
    }
}
#endif

static inline int ethGetNetIFLinkStatus(void) {
    return (NetManIoctl(NETMAN_NETIF_IOCTL_GET_LINK_STATUS, NULL, 0, NULL, 0) == NETMAN_NETIF_ETH_LINK_STATE_UP);
}

static inline int ethApplyNetIFConfig(int mode) {
    int result;
    // By default, auto-negotiation is used.
    static int CurrentMode = NETMAN_NETIF_ETH_LINK_MODE_AUTO;

    if (CurrentMode != mode) { // Change the setting, only if different.
        if ((result = NetManSetLinkMode(mode)) == 0)
            CurrentMode = mode;
    } else
        result = 0;

    return result;
}

static inline int WaitValidNetState(int (*checkingFunction)(void)) {
    int retry_cycles;

    // Wait for a valid network status;
    for (retry_cycles = 0; checkingFunction() == 0; retry_cycles++) { // Sleep for 1000ms.
        usleep(1000 * 1000);

        if (retry_cycles >= 10) // 10s = 10*1000ms
            return -1;
    }

    return 0;
}

static inline int ethWaitValidNetIFLinkState(void) {
    return WaitValidNetState(&ethGetNetIFLinkStatus);
}

static inline int getConfig(t_ip_info *ip_info) {
    return libcglue_ps2ip_getconfig("sm0", ip_info);
}

static inline int ethApplyDHCPConfig(void) {
    t_ip_info ip_info;
    int result;

    // SMAP is registered as the "sm0" device to the TCP/IP stack.
    if ((result = libcglue_ps2ip_getconfig("sm0", &ip_info)) >= 0) {
        ip_info.dhcp_enabled = 1;
        result = libcglue_ps2ip_setconfig(&ip_info);
    }

    return result;
}

static inline int ethApplyIPConfig(const struct ip4_addr *ip, const struct ip4_addr *netmask, const struct ip4_addr *gateway) {
    t_ip_info ip_info;
    int result;

    // SMAP is registered as the "sm0" device to the TCP/IP stack.
    if ((result = getConfig(&ip_info)) >= 0) {
        // Check if it's the same. Otherwise, apply the new configuration.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"

        if ((!ip_addr_cmp(ip, (struct ip4_addr *)&ip_info.ipaddr) ||
            !ip_addr_cmp(netmask, (struct ip4_addr *)&ip_info.netmask) ||
            !ip_addr_cmp(gateway, (struct ip4_addr *)&ip_info.gw))) {
            
            ip_addr_set((struct ip4_addr *)&ip_info.ipaddr, ip);
            ip_addr_set((struct ip4_addr *)&ip_info.netmask, netmask);
            ip_addr_set((struct ip4_addr *)&ip_info.gw, gateway);

            ip_info.dhcp_enabled = 0;
#pragma GCC diagnostic pop

            // Update settings.
            result = libcglue_ps2ip_setconfig(&ip_info);
        } else
            result = 0;
    }

    return result;
}

static inline int ethGetDHCPStatus(void) {
    t_ip_info ip_info;
    int result;

    result = getConfig(&ip_info);
    if (result >= 0) { // Check for a successful state if DHCP is enabled.
        result = (ip_info.dhcp_status == DHCP_STATE_BOUND || (ip_info.dhcp_status == DHCP_STATE_OFF));
    }

    return result;
}

static inline int ethWaitValidDHCPState(void) {
    return WaitValidNetState(&ethGetDHCPStatus);
}

#ifdef F_config_network_ps2_eeip_driver
enum EEIP_CONFIG_STATUS config_network_ps2_eeip_driver(void) {
    struct ip4_addr IP, NM, GW;

    // The DHCP server will provide us this information.
    ip4_addr_set_zero(&IP);
    ip4_addr_set_zero(&NM);
    ip4_addr_set_zero(&GW);

    // Attempt to apply the new link setting.
    if (ethApplyNetIFConfig(NETMAN_NETIF_ETH_LINK_MODE_AUTO) != 0) {
        return EEIP_CONFIG_STATUS_CANT_SET_LINK_MODE;
    }

    // Initialize the TCP/IP protocol stack.
    ps2ipInit(&IP, &NM, &GW);

    if (ethApplyDHCPConfig() < 0) {
        return EEIP_CONFIG_STATUS_CANT_SET_IP_CONFIG;
    }

    // Wait for the link to become ready.
    if (ethWaitValidNetIFLinkState()) {
        return EEIP_CONFIG_STATUS_CANT_GET_VALID_LINK_STATUS;
    }

    // dprintf("Waiting for DHCP lease...");
    if (ethWaitValidDHCPState()) {
        return EEIP_CONFIG_STATUS_CANT_GET_VALID_DHCP_STATUS;
    }

    return EEIP_CONFIG_STATUS_OK;
}
#endif

#ifdef F_config_network_custom_ps2_eeip_driver
enum EEIP_CONFIG_STATUS config_network_custom_ps2_eeip_driver(struct ip4_addr *IP, struct ip4_addr *NM, struct ip4_addr *GW) {
    // Attempt to apply the new link setting.
    if (ethApplyNetIFConfig(NETMAN_NETIF_ETH_LINK_MODE_AUTO) != 0) {
        return EEIP_CONFIG_STATUS_CANT_SET_LINK_MODE;
    }

    // Initialize the TCP/IP protocol stack.
    ps2ipInit(IP, NM, GW);

    if (ethApplyIPConfig(IP, NM, GW) < 0) {
        return EEIP_CONFIG_STATUS_CANT_SET_IP_CONFIG;
    }

    // Wait for the link to become ready.
    if (ethWaitValidNetIFLinkState()) {
        return EEIP_CONFIG_STATUS_CANT_GET_VALID_LINK_STATUS;
    }

    // For now set the DNS server to the gateway.
    libcglue_dns_setserver(0, GW);

    return EEIP_CONFIG_STATUS_OK;
}
#endif