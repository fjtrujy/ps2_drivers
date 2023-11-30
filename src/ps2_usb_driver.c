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

#include <sifrpc.h>
#include <loadfile.h>

/* References to BDM.IRX */
extern unsigned char bdm_irx[] __attribute__((aligned(16)));
extern unsigned int size_bdm_irx;

/* References to BDMFS_FATFS.IRX */
extern unsigned char bdmfs_fatfs_irx[] __attribute__((aligned(16)));
extern unsigned int size_bdmfs_fatfs_irx;

/* References to USBD.IRX */
extern unsigned char usbd_irx[] __attribute__((aligned(16)));
extern unsigned int size_usbd_irx;

/* References to USBMASS_BD.IRX */
extern unsigned char usbmass_bd_irx[] __attribute__((aligned(16)));
extern unsigned int size_usbmass_bd_irx;

#ifdef F_internals_ps2_usb_driver
enum USB_INIT_STATUS __usb_init_status = USB_INIT_STATUS_UNKNOWN;
int32_t __bdm_id = -1;
int32_t __bdmfs_fatfs_id = -1;
int32_t __usbmass_bd_id = -1;
#else
extern enum USB_INIT_STATUS __usb_init_status;
extern int32_t __bdm_id;
extern int32_t __bdmfs_fatfs_id;
extern int32_t __usbmass_bd_id;
#endif

#ifdef F_init_ps2_usb_driver
static enum USB_INIT_STATUS loadIRXs(void) {
    /* BDM.IRX */
    __bdm_id = SifExecModuleBuffer(&bdm_irx, size_bdm_irx, 0, NULL, NULL);
    if (__bdm_id < 0)
        return USB_INIT_STATUS_BDM_IRX_ERROR;

    /* BDMFS_FATFS.IRX */
    __bdmfs_fatfs_id = SifExecModuleBuffer(&bdmfs_fatfs_irx, size_bdmfs_fatfs_irx, 0, NULL, NULL);
    if (__bdmfs_fatfs_id < 0)
        return USB_INIT_STATUS_BDMFS_FATFS_IRX_ERROR;

    /* USBD.IRX */
    init_usbd_driver();

    /* USBMASS_BD.IRX */
    __usbmass_bd_id = SifExecModuleBuffer(&usbmass_bd_irx, size_usbmass_bd_irx, 0, NULL, NULL);
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

    /* USBD.IRX */
    deinit_usbd_driver();

    /* BDMFS_FATFS.IRX */
    if (__bdmfs_fatfs_id > 0) {
        SifUnloadModule(__bdmfs_fatfs_id);
        __bdmfs_fatfs_id = -1;
    }

    /* BDM.IRX */
    if (__bdm_id > 0) {
        SifUnloadModule(__bdm_id);
        __bdm_id = -1;
    }
}

void deinit_usb_driver() {
    unloadIRXs();
}
#endif
