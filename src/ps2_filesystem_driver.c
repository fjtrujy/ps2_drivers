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

#define DEVICE_MC0 "mc0:"
#define DEVICE_MC1 "mc1:"
#define DEVICE_CDROM "cdrom0:"
#define DEVICE_CDFS "cdfs:"
#define DEVICE_MASS "mass:"
#define DEVICE_MASS0 "mass0:"
#define DEVICE_HDD "hdd:"
#define DEVICE_HDD0 "hdd0:"
#define DEVICE_HOST "host:"
#define DEVICE_HOST0 "host0:"
#define DEVICE_HOST1 "host1:"
#define DEVICE_HOST2 "host2:"
#define DEVICE_HOST3 "host3:"
#define DEVICE_HOST4 "host4:"
#define DEVICE_HOST5 "host5:"
#define DEVICE_HOST6 "host6:"
#define DEVICE_HOST7 "host7:"
#define DEVICE_HOST8 "host8:"
#define DEVICE_HOST9 "host9:"

#define DEVICE_MC0_PATH DEVICE_MC0 DEVICE_SLASH
#define DEVICE_MC1_PATH DEVICE_MC1 DEVICE_SLASH
#define DEVICE_CDFS_PATH DEVICE_CDFS DEVICE_SLASH
#define DEVICE_CDROM_PATH DEVICE_CDROM DEVICE_SLASH
#define DEVICE_MASS_PATH DEVICE_MASS DEVICE_SLASH
#define DEVICE_MASS0_PATH DEVICE_MASS0 DEVICE_SLASH
#define DEVICE_HDD_PATH DEVICE_HDD DEVICE_SLASH
#define DEVICE_HDD0_PATH DEVICE_HDD0 DEVICE_SLASH
#define DEVICE_HOST_PATH DEVICE_HOST DEVICE_SLASH
#define DEVICE_HOST0_PATH DEVICE_HOST0 DEVICE_SLASH
#define DEVICE_HOST1_PATH DEVICE_HOST1 DEVICE_SLASH
#define DEVICE_HOST2_PATH DEVICE_HOST2 DEVICE_SLASH
#define DEVICE_HOST3_PATH DEVICE_HOST3 DEVICE_SLASH
#define DEVICE_HOST4_PATH DEVICE_HOST4 DEVICE_SLASH
#define DEVICE_HOST5_PATH DEVICE_HOST5 DEVICE_SLASH
#define DEVICE_HOST6_PATH DEVICE_HOST6 DEVICE_SLASH
#define DEVICE_HOST7_PATH DEVICE_HOST7 DEVICE_SLASH
#define DEVICE_HOST8_PATH DEVICE_HOST8 DEVICE_SLASH
#define DEVICE_HOST9_PATH DEVICE_HOST9 DEVICE_SLASH

#if F___internal_deinit_ps2_filesystem_driver
void __internal_deinit_ps2_filesystem_driver(bool deinit_powerOff) {
    deinit_hdd_driver(false);
    deinit_cdfs_driver();
    deinit_usb_driver();
    deinit_memcard_driver(true);
    deinit_fileXio_driver();

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

#if F_init_ps2_filesystem_driver
static void poweroffHandler(void *arg) {
    __internal_deinit_ps2_filesystem_driver(false);
    poweroffShutdown();
}

/* When booting from a USB device, it is not directly ready
 * so we try to open the folder again until it succeeds.
 */
static bool waitUntilDeviceIsReady(char *path) {
    struct stat buffer;
    int ret = -1;
    int retries = 50;

    while(ret != 0 && retries > 0) {
        ret = stat(path, &buffer);
        /* Wait untill the device is ready */
        nopdelay();

        retries--;
    }

    return ret == 0;
}

void init_ps2_filesystem_driver() {
    char cwd[FILENAME_MAX];

    init_poweroff_driver();
    init_fileXio_driver();
    init_memcard_driver(true);
    init_usb_driver();
    init_cdfs_driver();
    init_hdd_driver(false, true);

    poweroffSetCallback(&poweroffHandler, NULL);
    mount_current_hdd_partition();

    getcwd(cwd, sizeof(cwd));
    waitUntilDeviceIsReady(cwd);
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
            return DEVICE_MASS_PATH;
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
        case BOOT_DEVICE_HOST2:
            return DEVICE_HOST2_PATH;
        case BOOT_DEVICE_HOST3:
            return DEVICE_HOST3_PATH;
        case BOOT_DEVICE_HOST4:
            return DEVICE_HOST4_PATH;
        case BOOT_DEVICE_HOST5:
            return DEVICE_HOST5_PATH;
        case BOOT_DEVICE_HOST6:
            return DEVICE_HOST6_PATH;
        case BOOT_DEVICE_HOST7:
            return DEVICE_HOST7_PATH;
        case BOOT_DEVICE_HOST8:
            return DEVICE_HOST8_PATH;
        case BOOT_DEVICE_HOST9:
            return DEVICE_HOST9_PATH;
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
    else if (!strncmp(path, DEVICE_HOST2, 6))
        return BOOT_DEVICE_HOST2;
    else if (!strncmp(path, DEVICE_HOST3, 6))
        return BOOT_DEVICE_HOST3;
    else if (!strncmp(path, DEVICE_HOST4, 6))
        return BOOT_DEVICE_HOST4;
    else if (!strncmp(path, DEVICE_HOST5, 6))
        return BOOT_DEVICE_HOST5;
    else if (!strncmp(path, DEVICE_HOST6, 6))
        return BOOT_DEVICE_HOST6;
    else if (!strncmp(path, DEVICE_HOST7, 6))
        return BOOT_DEVICE_HOST7;
    else if (!strncmp(path, DEVICE_HOST8, 6))
        return BOOT_DEVICE_HOST8;
    else if (!strncmp(path, DEVICE_HOST9, 6))
        return BOOT_DEVICE_HOST9;
    else
        return BOOT_DEVICE_UNKNOWN;
}
#endif