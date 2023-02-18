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

#include <ps2_poweroff_driver.h>
#include <ps2_sio2man_driver.h>
#include <ps2_fileXio_driver.h>
#include <ps2_memcard_driver.h>
#include <ps2_usb_driver.h>
#include <ps2_cdfs_driver.h>
#include <ps2_hdd_driver.h>

#ifdef __cplusplus
extern "C" {
#endif

enum BootDeviceIDs {
   BOOT_DEVICE_UNKNOWN = -1,
   BOOT_DEVICE_MC0 = 0,
   BOOT_DEVICE_MC1,
   BOOT_DEVICE_CDROM,
   BOOT_DEVICE_CDFS,
   BOOT_DEVICE_MASS,
   BOOT_DEVICE_MASS0,
   BOOT_DEVICE_HDD,
   BOOT_DEVICE_HDD0,
   BOOT_DEVICE_HOST,
   BOOT_DEVICE_HOST0,
   BOOT_DEVICE_HOST1,
   BOOT_DEVICE_HOST2,
   BOOT_DEVICE_HOST3,
   BOOT_DEVICE_HOST4,
   BOOT_DEVICE_HOST5,
   BOOT_DEVICE_HOST6,
   BOOT_DEVICE_HOST7,
   BOOT_DEVICE_HOST8,
   BOOT_DEVICE_HOST9,
   BOOT_DEVICE_COUNT,
};

void init_ps2_filesystem_driver();
void deinit_ps2_filesystem_driver();
bool waitUntilDeviceIsReady(char *path);
char *rootDevicePath(enum BootDeviceIDs device_id);
enum BootDeviceIDs getBootDeviceID(char *path);

#ifdef __cplusplus
}
#endif

#endif /* PS2_FILESYSTEM_DRIVER */