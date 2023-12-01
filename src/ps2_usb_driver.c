/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_USB_DRIVER
*/

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <ps2_usb_driver.h>
#include <ps2_usbd_driver.h>
#include <irx_common_macros.h>

#include <sifrpc.h>
#include <loadfile.h>

EXTERN_IRX(bdm_irx);
EXTERN_IRX(bdmfs_fatfs_irx);
EXTERN_IRX(usbmass_bd_irx);

#ifdef F_internals_ps2_usb_driver
enum USB_INIT_STATUS __usb_init_status = USB_INIT_STATUS_UNKNOWN;
DECL_IRX_VARS(bdm);
DECL_IRX_VARS(bdmfs_fatfs);
DECL_IRX_VARS(usbmass_bd);
#else
extern enum USB_INIT_STATUS __usb_init_status;
EXTERN_IRX_VARS(bdm);
EXTERN_IRX_VARS(bdmfs_fatfs);
EXTERN_IRX_VARS(usbmass_bd);
#endif

#ifdef F_init_ps2_usb_driver
static enum USB_INIT_STATUS loadIRXs(void) {
    /* BDM.IRX */
    __bdm_id = SifExecModuleBuffer(&bdm_irx, size_bdm_irx, 0, NULL, &__bdm_ret);
    if (CHECK_IRX_ERR(bdm))
        return USB_INIT_STATUS_BDM_IRX_ERROR;

    /* BDMFS_FATFS.IRX */
    __bdmfs_fatfs_id = SifExecModuleBuffer(&bdmfs_fatfs_irx, size_bdmfs_fatfs_irx, 0, NULL, &__bdmfs_fatfs_ret);
    if (CHECK_IRX_ERR(bdmfs_fatfs))
        return USB_INIT_STATUS_BDMFS_FATFS_IRX_ERROR;

    /* USBD.IRX */
    init_usbd_driver();

    /* USBMASS_BD.IRX */
    __usbmass_bd_id = SifExecModuleBuffer(&usbmass_bd_irx, size_usbmass_bd_irx, 0, NULL, &__usbmass_bd_ret);
    if (CHECK_IRX_ERR(usbmass_bd))
        return USB_INIT_STATUS_USBMASS_BD_IRX_ERROR;

    return USB_INIT_STATUS_IRX_OK;
}

enum USB_INIT_STATUS init_usb_driver() {
    __usb_init_status = loadIRXs();

    return __usb_init_status;
}
#endif

#ifdef F_deinit_ps2_usb_driver
static void unloadIRXs(void) {
    /* USBMASS_BD.IRX */
    if (CHECK_IRX_UNLOAD(usbmass_bd)) {
        SifUnloadModule(__usbmass_bd_id);
        RESET_IRX_VARS(usbmass_bd);
    }

    /* USBD.IRX */
    deinit_usbd_driver();

    /* BDMFS_FATFS.IRX */
    if (CHECK_IRX_UNLOAD(bdmfs_fatfs)) {
        SifUnloadModule(__bdmfs_fatfs_id);
        RESET_IRX_VARS(bdmfs_fatfs);
    }

    /* BDM.IRX */
    if (CHECK_IRX_UNLOAD(bdm)) {
        SifUnloadModule(__bdm_id);
        RESET_IRX_VARS(bdm);
    }
}

void deinit_usb_driver() {
    unloadIRXs();
}
#endif
