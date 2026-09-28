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

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <kernel.h>
#include <libpwroff.h>

#include <ps2_filesystem_driver.h>


#if F___internal_deinit_ps2_filesystem_driver
void __internal_deinit_ps2_filesystem_driver(bool deinit_powerOff) {
    deinit_hdd_driver(false);
    deinit_cdfs_driver();
    init_mx4sio_driver(false);
    deinit_usb_driver(true);
    deinit_memcard_driver(true);
    deinit_fileXio_driver();
    deinit_dev9_driver();

    if (deinit_powerOff)
        deinit_poweroff_driver();
}
#else
void __internal_deinit_ps2_filesystem_driver(bool deinit_powerOff);
#endif

#if F_deinit_ps2_filesystem_driver
void deinit_ps2_filesystem_driver() {
    umount_current_hdd_partition();

    __internal_deinit_ps2_filesystem_driver(true);
}
#endif
#if F___internal_deinit_only_boot_ps2_filesystem_driver
enum BootDeviceIDs __boot_device_id = BOOT_DEVICE_UNKNOWN;

void __internal_deinit_only_boot_ps2_filesystem_driver(bool deinit_powerOff) {
    switch (__boot_device_id) {
        case BOOT_DEVICE_MC0:
        case BOOT_DEVICE_MC1:
            deinit_memcard_driver(true);
            break;
        case BOOT_DEVICE_CDROM:
        case BOOT_DEVICE_CDFS:
            deinit_cdfs_driver();
            break;
        case BOOT_DEVICE_MASS:
        case BOOT_DEVICE_MASS0:
        case BOOT_DEVICE_MASS1:
        case BOOT_DEVICE_MX4SIO:
        case BOOT_DEVICE_MX4SIO0:
        case BOOT_DEVICE_MX4SIO1:
            deinit_usb_driver(true);
            deinit_mx4sio_driver(true);
            break;
        case BOOT_DEVICE_HDD:
        case BOOT_DEVICE_HDD0: {
            deinit_hdd_driver(false);
            deinit_dev9_driver();
            if (deinit_powerOff) {
                deinit_poweroff_driver();
            }
            break;
        }
        case BOOT_DEVICE_HOST:
        case BOOT_DEVICE_HOST0:
        case BOOT_DEVICE_HOST1:
            break;
        default:
            break;
    }

    deinit_fileXio_driver();
}
#else
extern enum BootDeviceIDs __boot_device_id;
void __internal_deinit_only_boot_ps2_filesystem_driver(bool deinit_powerOff);
#endif

#if F_deinit_only_boot_ps2_filesystem_driver
void deinit_only_boot_ps2_filesystem_driver() {
    switch (__boot_device_id) {
        case BOOT_DEVICE_HDD:
        case BOOT_DEVICE_HDD0:
            umount_current_hdd_partition();
            break;
        default:
            break;
    }

    __internal_deinit_only_boot_ps2_filesystem_driver(true);
}
#endif

#if F_init_ps2_filesystem_driver
static void poweroffHandler(void *arg) {
    __internal_deinit_ps2_filesystem_driver(false);
    poweroffShutdown();
}

void init_ps2_filesystem_driver() {
    char cwd[FILENAME_MAX];

    init_poweroff_driver();
    init_fileXio_driver();
    init_memcard_driver(true);
    init_usb_driver(true);
    init_mx4sio_driver(false);
    init_cdfs_driver();
    init_dev9_driver();
    init_hdd_driver(false, true);

    poweroffSetCallback(&poweroffHandler, NULL);
    mount_current_hdd_partition();

    getcwd(cwd, sizeof(cwd));
    waitUntilDeviceIsReady(cwd);
}
#endif

#if F_init_only_boot_ps2_filesystem_driver
static void poweroffHandler(void *arg) {
    __internal_deinit_only_boot_ps2_filesystem_driver(false);
    poweroffShutdown();
}

void init_only_boot_ps2_filesystem_driver() {
    // get current working directory
    char cwd[FILENAME_MAX];
    getcwd(cwd, sizeof(cwd));

    // get current boot device
    enum BootDeviceIDs boot_device_id = getBootDeviceID(cwd);

    // Only init the boot device
    init_fileXio_driver();

    switch (boot_device_id) {
        case BOOT_DEVICE_MC0:
        case BOOT_DEVICE_MC1:
            init_memcard_driver(true);
            break;
        case BOOT_DEVICE_CDROM:
        case BOOT_DEVICE_CDFS:
            init_cdfs_driver();
            break;
        case BOOT_DEVICE_MASS:
        case BOOT_DEVICE_MASS0:
        case BOOT_DEVICE_MASS1:
        case BOOT_DEVICE_MX4SIO:
        case BOOT_DEVICE_MX4SIO0:
        case BOOT_DEVICE_MX4SIO1:
            init_usb_driver(true);
            init_mx4sio_driver(true);
            break;
        case BOOT_DEVICE_HDD:
        case BOOT_DEVICE_HDD0:
            init_poweroff_driver();
            init_dev9_driver();
            init_hdd_driver(false, false);
            poweroffSetCallback(&poweroffHandler, NULL);
            mount_current_hdd_partition();
            // When mounting the HDD, the current working directory is changed
            getcwd(cwd, sizeof(cwd));
            break;
        case BOOT_DEVICE_HOST:
        case BOOT_DEVICE_HOST0:
        case BOOT_DEVICE_HOST1:
            break;
        default:
            break;
    }

    waitUntilDeviceIsReady(cwd);
}
#endif

#if F_waitUntilDeviceIsReady_ps2_filesystem_driver
/* When booting from a USB device, it is not directly ready
 * so we try to open the folder again until it succeeds.
 */
bool waitUntilDeviceIsReady(char *path) {
    struct stat buffer;
    int ret = -1;
    int retries = 500;

    while (ret != 0 && retries > 0) {
        ret = stat(path, &buffer);
        /* Wait untill the device is ready */
        nopdelay();

        retries--;
    }

    return ret == 0;
}
#endif
