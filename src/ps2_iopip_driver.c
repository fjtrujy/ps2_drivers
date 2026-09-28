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
#include <unistd.h>
#include <ps2_dev9_driver.h>
#include <ps2_netman_driver.h>
#include <ps2_smap_driver.h>
#include <ps2_iopip_driver.h>
#include <irx_common_macros.h>

#include <sifrpc.h>
#include <loadfile.h>
#include <netman.h>
#include <ps2ips.h>

#define IOPIP_DEFAULT_TIMEOUT_SECONDS 10

EXTERN_PS2_IRX_MODULE(ps2ips);
EXTERN_PS2_IRX_MODULE(ps2ip_nm);

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
        __ps2ip_nm_id = ps2_irx_exec(&ps2_irx_ps2ip_nm, 0, NULL, &__ps2ip_nm_ret);
        if (CHECK_IRX_ERR(ps2ip_nm))
            return IOPIP_INIT_STATUS_PS2IP_NM_IRX_ERROR;
    }
    
    /* PS2IPS.IRX */
    if (CHECK_IRX_LOAD(ps2ips)) {
        __ps2ips_id = ps2_irx_exec(&ps2_irx_ps2ips, 0, NULL, &__ps2ips_ret);
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
        // Requires to have DEV9
        if (init_dev9_driver() != DEV9_INIT_STATUS_OK)
            return IOPIP_INIT_STATUS_DEPENDENCY_IRX_ERROR;

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
        // Requires to have DEV9
        deinit_dev9_driver();
    }
}
#endif

#ifdef F_iopip_network_config_default_ps2_iopip_driver
void iopip_network_config_default(iopip_network_config_t *cfg) {
    if (cfg == NULL) return;

    ip4_addr_set_zero(&cfg->ip);
    ip4_addr_set_zero(&cfg->netmask);
    ip4_addr_set_zero(&cfg->gateway);
    cfg->link_mode       = NETMAN_NETIF_ETH_LINK_MODE_AUTO;
    cfg->timeout_seconds = IOPIP_DEFAULT_TIMEOUT_SECONDS;
    cfg->on_progress     = NULL;
    cfg->user            = NULL;
}
#endif

#ifdef F_configure_iopip_network_ps2_iopip_driver
static inline void emit_progress(const iopip_network_config_t *cfg,
                                 enum IOPIP_PROGRESS_EVENT ev) {
    if (cfg->on_progress != NULL) {
        cfg->on_progress(ev, cfg->user);
    }
}

static int link_is_up(void) {
    return NetManIoctl(NETMAN_NETIF_IOCTL_GET_LINK_STATUS, NULL, 0, NULL, 0)
        == NETMAN_NETIF_ETH_LINK_STATE_UP;
}

static int wait_until(int (*check)(void), int timeout_seconds) {
    int retry;
    for (retry = 0; retry < timeout_seconds; retry++) {
        if (check()) return 0;
        usleep(1000 * 1000);
    }
    return -1;
}

static enum IOPIP_NET_STATUS apply_ip_config(const iopip_network_config_t *cfg) {
    t_ip_info ip_info;

    if (ps2ip_getconfig("sm0", &ip_info) < 0)
        return IOPIP_NET_STATUS_CONFIG_ERROR;

    ip_addr_set((struct ip4_addr *)&ip_info.ipaddr,  &cfg->ip);
    ip_addr_set((struct ip4_addr *)&ip_info.netmask, &cfg->netmask);
    ip_addr_set((struct ip4_addr *)&ip_info.gw,      &cfg->gateway);
    ip_info.dhcp_enabled = 0;

    if (ps2ip_setconfig(&ip_info) < 0)
        return IOPIP_NET_STATUS_CONFIG_ERROR;

    return IOPIP_NET_STATUS_OK;
}

enum IOPIP_NET_STATUS configure_iopip_network(const iopip_network_config_t *cfg) {
    if (cfg == NULL) return IOPIP_NET_STATUS_CONFIG_ERROR;

    int timeout = (cfg->timeout_seconds > 0)
                ? cfg->timeout_seconds
                : IOPIP_DEFAULT_TIMEOUT_SECONDS;

    /* 1. Link mode. SMAP defaults to AUTO; on PCSX2 the SET_LINK_MODE ioctl
       is a stub that fails on the no-op, so only push a non-AUTO request. */
    emit_progress(cfg, IOPIP_PROGRESS_SETTING_LINK_MODE);
    if (cfg->link_mode != NETMAN_NETIF_ETH_LINK_MODE_AUTO) {
        if (NetManSetLinkMode(cfg->link_mode) != 0)
            return IOPIP_NET_STATUS_LINK_MODE_ERROR;
    }

    /* 2. Apply static IP config. */
    emit_progress(cfg, IOPIP_PROGRESS_APPLYING_IP_CONFIG);
    {
        enum IOPIP_NET_STATUS rc = apply_ip_config(cfg);
        if (rc != IOPIP_NET_STATUS_OK) return rc;
    }

    /* 3. Wait for link up. */
    emit_progress(cfg, IOPIP_PROGRESS_WAITING_LINK_UP);
    if (wait_until(link_is_up, timeout) != 0)
        return IOPIP_NET_STATUS_LINK_TIMEOUT;
    emit_progress(cfg, IOPIP_PROGRESS_LINK_UP);

    emit_progress(cfg, IOPIP_PROGRESS_READY);
    return IOPIP_NET_STATUS_OK;
}
#endif

#ifdef F_iopip_get_current_config_ps2_iopip_driver
int iopip_get_current_config(struct ip4_addr *ip,
                             struct ip4_addr *nm,
                             struct ip4_addr *gw) {
    t_ip_info ip_info;

    if (ps2ip_getconfig("sm0", &ip_info) < 0)
        return -1;

    if (ip != NULL) ip_addr_set(ip, (struct ip4_addr *)&ip_info.ipaddr);
    if (nm != NULL) ip_addr_set(nm, (struct ip4_addr *)&ip_info.netmask);
    if (gw != NULL) ip_addr_set(gw, (struct ip4_addr *)&ip_info.gw);

    return 0;
}
#endif
