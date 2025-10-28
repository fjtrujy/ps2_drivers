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
#include <ps2_bdm_driver.h>
#include <ps2_usb_driver.h>
#include <ps2_usbd_driver.h>
#include <irx_common_macros.h>

#include <sifrpc.h>
#include <loadfile.h>

EXTERN_IRX(bdmfs_fatfs_irx);
EXTERN_IRX(usbmass_bd_irx);

#ifdef F_internals_ps2_usb_driver
enum USB_INIT_STATUS __usb_init_status = USB_INIT_STATUS_UNKNOWN;
DECL_IRX_VARS(usbmass_bd);
#else
extern enum USB_INIT_STATUS __usb_init_status;
EXTERN_IRX_VARS(usbmass_bd);
#endif

#ifdef F_init_ps2_usb_driver
static enum USB_INIT_STATUS loadIRXs(void) {
    /* USBMASS_BD.IRX */
    if (CHECK_IRX_LOAD(usbmass_bd)) {
        __usbmass_bd_id = SifExecModuleBuffer(&usbmass_bd_irx, size_usbmass_bd_irx, 0, NULL, &__usbmass_bd_ret);
        if (CHECK_IRX_ERR(usbmass_bd))
            return USB_INIT_STATUS_USBMASS_BD_IRX_ERROR;
    }

    return USB_INIT_STATUS_IRX_OK;
}

enum USB_INIT_STATUS init_usb_driver(bool init_dependencies) {
    /* Requires to have BDM and USBD */
    if (init_dependencies && init_bdm_driver() < 0)
        return USB_INIT_STATUS_DEPENDENCY_BMD_ERROR;
    
    if (init_dependencies && init_usbd_driver() < 0)
        return USB_INIT_STATUS_DEPENDENCY_USBD_ERROR;

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
}

void deinit_usb_driver(bool deinit_dependencies) {
    unloadIRXs();

    if (deinit_dependencies) {
        deinit_usbd_driver();
        deinit_bdm_driver();
    }
}
#endif
