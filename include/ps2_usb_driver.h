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

#ifndef PS2_USB_DRIVER
#define PS2_USB_DRIVER

#ifdef __cplusplus
extern "C" {
#endif

enum USB_INIT_STATUS {
    USB_INIT_STATUS_DEPENDENCY_USBD_ERROR = -5,
    USB_INIT_STATUS_DEPENDENCY_BMD_ERROR = -4,
    USB_INIT_STATUS_USBMASS_BD_IRX_ERROR = -3,
    USB_INIT_STATUS_BDMFS_FATFS_IRX_ERROR = -2,
    USB_INIT_STATUS_UNKNOWN = -1,
    USB_INIT_STATUS_OK = 0,
    USB_INIT_STATUS_IRX_OK = 1,
};

/* USB DRIVER REQUIRES SIO2MAN DRIVER AS DEPENDENCY */
enum USB_INIT_STATUS init_usb_driver(bool init_dependencies);
void deinit_usb_driver(bool deinit_dependencies);

#ifdef __cplusplus
}
#endif

#endif /* PS2_USB_DRIVER */
