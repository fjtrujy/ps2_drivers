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
#include <string.h>

#include <kernel.h>
#include <libpwroff.h>

#include <ps2_filesystem_driver.h>

#define DEVICE_SLASH "/"

#define DEVICE_MC0   "mc0:"
#define DEVICE_MC1   "mc1:"
#define DEVICE_CDROM "cdrom0:"
#define DEVICE_CDFS  "cdfs:"
#define DEVICE_MASS  "mass:"
#define DEVICE_MASS0 "mass0:"
#define DEVICE_MASS1 "mass1:"
#define DEVICE_MX4SIO "mx4sio:"
#define DEVICE_MX4SIO0 "mx4sio0:"
#define DEVICE_MX4SIO1 "mx4sio1:"
#define DEVICE_HDD   "hdd:"
#define DEVICE_HDD0  "hdd0:"
#define DEVICE_HOST  "host:"
#define DEVICE_HOST0 "host0:"
#define DEVICE_HOST1 "host1:"

#define DEVICE_MC0_PATH   DEVICE_MC0 DEVICE_SLASH
#define DEVICE_MC1_PATH   DEVICE_MC1 DEVICE_SLASH
#define DEVICE_CDFS_PATH  DEVICE_CDFS DEVICE_SLASH
#define DEVICE_CDROM_PATH DEVICE_CDROM DEVICE_SLASH
#define DEVICE_MASS_PATH  DEVICE_MASS DEVICE_SLASH
#define DEVICE_MASS0_PATH DEVICE_MASS0 DEVICE_SLASH
#define DEVICE_MASS1_PATH DEVICE_MASS1 DEVICE_SLASH
#define DEVICE_MX4SIO_PATH  DEVICE_MX4SIO DEVICE_SLASH
#define DEVICE_MX4SIO0_PATH DEVICE_MX4SIO0 DEVICE_SLASH
#define DEVICE_MX4SIO1_PATH DEVICE_MX4SIO1 DEVICE_SLASH
#define DEVICE_HDD_PATH   DEVICE_HDD DEVICE_SLASH
#define DEVICE_HDD0_PATH  DEVICE_HDD0 DEVICE_SLASH
#define DEVICE_HOST_PATH  DEVICE_HOST DEVICE_SLASH
#define DEVICE_HOST0_PATH DEVICE_HOST0 DEVICE_SLASH
#define DEVICE_HOST1_PATH DEVICE_HOST1 DEVICE_SLASH

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

#if F_rootDevicePath_ps2_filesystem_driver
char *rootDevicePath(enum BootDeviceIDs device_id) {
    switch (device_id) {
        case BOOT_DEVICE_MC0:
            return DEVICE_MC0_PATH;
        case BOOT_DEVICE_MC1:
            return DEVICE_MC1_PATH;
        case BOOT_DEVICE_CDROM:
            return DEVICE_CDROM_PATH;
        case BOOT_DEVICE_CDFS:
            return DEVICE_CDFS_PATH;
        case BOOT_DEVICE_MASS:
            return DEVICE_MASS_PATH;
        case BOOT_DEVICE_MASS0:
            return DEVICE_MASS0_PATH;
        case BOOT_DEVICE_MASS1:
            return DEVICE_MASS1_PATH;
        case BOOT_DEVICE_MX4SIO:
            return DEVICE_MX4SIO_PATH;
        case BOOT_DEVICE_MX4SIO0:
            return DEVICE_MX4SIO0_PATH;
        case BOOT_DEVICE_MX4SIO1:
            return DEVICE_MX4SIO1_PATH;
        case BOOT_DEVICE_HDD:
            return DEVICE_HDD_PATH;
        case BOOT_DEVICE_HDD0:
            return DEVICE_HDD0_PATH;
        case BOOT_DEVICE_HOST:
            return DEVICE_HOST_PATH;
        case BOOT_DEVICE_HOST0:
            return DEVICE_HOST0_PATH;
        case BOOT_DEVICE_HOST1:
            return DEVICE_HOST1_PATH;
        default:
            return "";
    }
}
#endif

#if F_getBootDeviceID_ps2_filesystem_driver
enum BootDeviceIDs getBootDeviceID(char *path) {
    if (!strncmp(path, DEVICE_MC0, 4))
        return BOOT_DEVICE_MC0;
    else if (!strncmp(path, DEVICE_MC1, 4))
        return BOOT_DEVICE_MC1;
    else if (!strncmp(path, DEVICE_CDROM, 7))
        return BOOT_DEVICE_CDROM;
    else if (!strncmp(path, DEVICE_CDFS, 5))
        return BOOT_DEVICE_CDFS;
    else if (!strncmp(path, DEVICE_MASS, 5))
        return BOOT_DEVICE_MASS;
    else if (!strncmp(path, DEVICE_MASS0, 6))
        return BOOT_DEVICE_MASS0;
    else if (!strncmp(path, DEVICE_MASS1, 6))
        return BOOT_DEVICE_MASS1;
    else if (!strncmp(path, DEVICE_MX4SIO, 5))
        return BOOT_DEVICE_MX4SIO;
    else if (!strncmp(path, DEVICE_MX4SIO0, 6))
        return BOOT_DEVICE_MX4SIO0;
    else if (!strncmp(path, DEVICE_MX4SIO1, 6))
        return BOOT_DEVICE_MX4SIO1;
    else if (!strncmp(path, DEVICE_HDD, 4))
        return BOOT_DEVICE_HDD;
    else if (!strncmp(path, DEVICE_HDD0, 5))
        return BOOT_DEVICE_HDD0;
    else if (!strncmp(path, DEVICE_HOST, 5))
        return BOOT_DEVICE_HOST;
    else if (!strncmp(path, DEVICE_HOST0, 6))
        return BOOT_DEVICE_HOST0;
    else if (!strncmp(path, DEVICE_HOST1, 6))
        return BOOT_DEVICE_HOST1;
    else
        return BOOT_DEVICE_UNKNOWN;
}
#endif
