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

#include <sifrpc.h>
#include <loadfile.h>

/* References to USBD.IRX */
extern unsigned char usbd_irx[] __attribute__((aligned(16)));
extern unsigned int size_usbd_irx;

/* References to BDM.IRX */
extern unsigned char bdm_irx[] __attribute__((aligned(16)));
extern unsigned int size_bdm_irx;

/* References to BDMFS_VFAT.IRX */
extern unsigned char bdmfs_vfat_irx[] __attribute__((aligned(16)));
extern unsigned int size_bdmfs_vfat_irx;

/* References to USBMASS_BD.IRX */
extern unsigned char usbmass_bd_irx[] __attribute__((aligned(16)));
extern unsigned int size_usbmass_bd_irx;

#ifdef F_internals_ps2_usb_driver
enum USB_INIT_STATUS __usb_init_status = USB_INIT_STATUS_UNKNOWN;
int32_t __usbd_id = -1;
int32_t __bdm_id = -1;
int32_t __bdmfs_vfat_id = -1;
int32_t __usbmass_bd_id = -1;
#else
extern enum USB_INIT_STATUS __usb_init_status;
extern int32_t __usbd_id;
extern int32_t __bdm_id;
extern int32_t __bdmfs_vfat_id;
extern int32_t __usbmass_bd_id;
#endif

#ifdef F_init_ps2_usb_driver
static enum USB_INIT_STATUS loadIRXs(void) {
    /* USBD.IRX */
    __usbd_id = SifExecModuleBuffer(&usbd_irx, size_usbd_irx, 0, NULL, NULL);
    if (__usbd_id < 0)
        return USB_INIT_STATUS_USBD_IRX_ERROR;

    /* BDM.IRX */
    __bdm_id = SifExecModuleBuffer(&bdm_irx, size_bdm_irx, 0, NULL, NULL);
    if (__bdm_id < 0)
        return USB_INIT_STATUS_BDM_IRX_ERROR;
    
    /* BDMFS_VFAT.IRX */
    __bdmfs_vfat_id = SifExecModuleBuffer(&bdmfs_vfat_irx, size_bdmfs_vfat_irx, 0, NULL, NULL);
    if (__bdmfs_vfat_id < 0)
        return USB_INIT_STATUS_BDMFS_VFAT_IRX_ERROR;

    /* USBMASS_BD.IRX */
    __usbmass_bd_id = SifExecModuleBuffer(&bdm_irx, size_bdm_irx, 0, NULL, NULL);
    if (__usbmass_bd_id < 0)
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
    if (__usbmass_bd_id > 0) {
        SifUnloadModule(__usbmass_bd_id);
        __usbmass_bd_id = -1;
    }

    /* BDMFS_VFAT.IRX */
    if (__bdmfs_vfat_id > 0) {
        SifUnloadModule(__bdmfs_vfat_id);
        __bdmfs_vfat_id = -1;
    }

    /* BDM.IRX */
    if (__bdm_id > 0) {
        SifUnloadModule(__bdm_id);
        __bdm_id = -1;
    }

    /* USBD.IRX */
    if (__usbd_id > 0) {
        SifUnloadModule(__usbd_id);
        __usbd_id = -1;
    }
}

void deinit_usb_driver() {
    unloadIRXs();
}
#endif