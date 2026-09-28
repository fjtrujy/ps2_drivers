/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_FILESYSTEM_DRIVER
*/

#ifndef PS2_FILESYSTEM_DRIVER
#define PS2_FILESYSTEM_DRIVER

#include <stdbool.h>

#include <ps2_boot_device.h>
#include <ps2_poweroff_driver.h>
#include <ps2_sio2man_driver.h>
#include <ps2_fileXio_driver.h>
#include <ps2_memcard_driver.h>
#include <ps2_usb_driver.h>
#include <ps2_mx4sio_driver.h>
#include <ps2_cdfs_driver.h>
#include <ps2_dev9_driver.h>
#include <ps2_hdd_driver.h>

#ifdef __cplusplus
extern "C" {
#endif

void init_ps2_filesystem_driver();
void deinit_ps2_filesystem_driver();
void init_only_boot_ps2_filesystem_driver();
void deinit_only_boot_ps2_filesystem_driver();
bool waitUntilDeviceIsReady(char *path);

#ifdef __cplusplus
}
#endif

#endif /* PS2_FILESYSTEM_DRIVER */
