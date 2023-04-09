/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_HDD_DRIVER
*/

#ifndef PS2_HDD_DRIVER
#define PS2_HDD_DRIVER

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum HDD_INIT_STATUS {
    HDD_INIT_STATUS_DEPENDENCY_IRX_ERROR = 7,
    HDD_INIT_STATUS_HDD_NOT_READY_ERROR = -6,
    HDD_INIT_STATUS_PS2FS_IRX_ERROR = -5,
    HDD_INIT_STATUS_PS2HDD_IRX_ERROR = -4,
    HDD_INIT_STATUS_PS2ATAD_IRX_ERROR = -3,
    HDD_INIT_WRONG_CWD = -2,
    HDD_INIT_STATUS_UNKNOWN = -1,
    HDD_INIT_STATUS_IRX_OK = 0,
};

enum HDD_MOUNT_STATUS {
    HDD_MOUNT_STATUS_ERROR = -5,
    HDD_MOUNT_STATUS_INFO_ERROR = -4,
    HDD_MOUNT_STATUS_WRONG_CWD = -3,
    HDD_MOUNT_STATUS_UKNOWN = -2,
    HDD_MOUNT_INIT_STATUS_NOT_READY = -1,
    HDD_MOUNT_STATUS_OK = 0,
};

/* MEMCARD DRIVER REQUIRES FILEXIO DRIVER AS DEPENDENCY */
enum HDD_INIT_STATUS init_hdd_driver(bool init_dependencies, bool only_if_booted_from_hdd);
void deinit_hdd_driver(bool deinit_dependencies);
enum HDD_MOUNT_STATUS mount_current_hdd_partition();
enum HDD_MOUNT_STATUS mount_hdd_partition(const char* mountpoint, const char* blockdev);
void umount_current_hdd_partition();
void umount_hdd_partition(const char* mountpoint);

#ifdef __cplusplus
}
#endif

#endif /* PS2_HDD_DRIVER */