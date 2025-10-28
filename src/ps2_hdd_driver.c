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

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <ps2_hdd_driver.h>
#include <ps2_fileXio_driver.h>
#include <ps2_dev9_driver.h>
#include <ps2_bdm_driver.h>
#include <irx_common_macros.h>

#include <sifrpc.h>
#include <loadfile.h>
#define NEWLIB_PORT_AWARE
#include <fileXio_rpc.h>
#include <hdd-ioctl.h>
#include <io_common.h>

EXTERN_IRX(ps2atad_irx);
EXTERN_IRX(ps2hdd_irx);
EXTERN_IRX(ps2fs_irx);

#ifdef F_internals_ps2_hdd_driver
enum HDD_INIT_STATUS __hdd_init_status = HDD_INIT_STATUS_UNKNOWN;
DECL_IRX_VARS(ps2atad);
DECL_IRX_VARS(ps2hdd);
DECL_IRX_VARS(ps2fs);
char __mountString[10];
enum HDD_MOUNT_STATUS __mount_status = HDD_MOUNT_STATUS_UKNOWN;

static inline bool prefix(const char *pre, const char *str) {
    return strncmp(pre, str, strlen(pre)) == 0;
}

bool __cwd_is_hdd() {
    char cwd[FILENAME_MAX];
    getcwd(cwd, sizeof(cwd));

    return prefix("hdd:", cwd) || prefix("hdd0:", cwd);
}
#else
extern enum HDD_INIT_STATUS __hdd_init_status;
EXTERN_IRX_VARS(ps2atad);
EXTERN_IRX_VARS(ps2hdd);
EXTERN_IRX_VARS(ps2fs);
extern char __mountString[];
extern enum HDD_MOUNT_STATUS __mount_status;
bool __cwd_is_hdd();
#endif

#ifdef F_init_ps2_hdd_driver
static int hddCheck(void) {
    int ret = fileXioDevctl("hdd0:", HDIOC_STATUS, NULL, 0, NULL, 0);
    /* 0 = HDD connected and formatted, 1 = not formatted, 2 = HDD not usable, 3 = HDD not connected. */
    if ((ret >= 3) || (ret < 0))
        return -1;
    return ret;
}

static enum HDD_INIT_STATUS loadIRXs(void) {
    char hddarg[] = "-o"
                    "\0"
                    "4"
                    "\0"
                    "-n"
                    "\0"
                    "20";

    /* PS2ATAD.IRX */
    if (CHECK_IRX_LOAD(ps2atad)) {
       __ps2atad_id = SifExecModuleBuffer(&ps2atad_irx, size_ps2atad_irx, 0, NULL, &__ps2atad_ret);
        if (CHECK_IRX_ERR(ps2atad))
            return HDD_INIT_STATUS_PS2ATAD_IRX_ERROR;
    }

    /* PS2HDD.IRX */
    if (CHECK_IRX_LOAD(ps2hdd)) {
        __ps2hdd_id = SifExecModuleBuffer(&ps2hdd_irx, size_ps2hdd_irx, sizeof(hddarg), hddarg, &__ps2hdd_ret);
        if (CHECK_IRX_ERR(ps2hdd))
            return HDD_INIT_STATUS_PS2HDD_IRX_ERROR;
    }

    /* Check if HDD is formatted and ready to be used */
    if (hddCheck() < 0)
        return HDD_INIT_STATUS_HDD_NOT_READY_ERROR;

    /* PS2FS.IRX */
    if (CHECK_IRX_LOAD(ps2fs)) {
        __ps2fs_id = SifExecModuleBuffer(&ps2fs_irx, size_ps2fs_irx, sizeof(hddarg), hddarg, &__ps2fs_ret);
        if (CHECK_IRX_ERR(ps2fs))
            return HDD_INIT_STATUS_PS2FS_IRX_ERROR;
    }

    return HDD_INIT_STATUS_IRX_OK;
}

enum HDD_INIT_STATUS init_hdd_driver(bool init_dependencies, bool only_if_booted_from_hdd) {
    int ret;

    if (only_if_booted_from_hdd && !__cwd_is_hdd()) {
        __hdd_init_status = HDD_INIT_WRONG_CWD;
        return __hdd_init_status;
    }

    // Requires to have FILEXIO and DEV9 drivers loaded
    if (init_dependencies) {
        ret = init_fileXio_driver();

        if (ret != FILEXIO_INIT_STATUS_OK) {
            __hdd_init_status = HDD_INIT_STATUS_DEPENDENCY_IRX_ERROR;
            return __hdd_init_status;
        }

        ret = init_dev9_driver();
        if (ret != DEV9_INIT_STATUS_OK) {
            __hdd_init_status = HDD_INIT_STATUS_DEPENDENCY_IRX_ERROR;
            return __hdd_init_status;
        }

        ret = init_bdm_driver();
        if (ret != BDM_INIT_STATUS_OK) {
            __hdd_init_status = HDD_INIT_STATUS_DEPENDENCY_IRX_ERROR;
            return __hdd_init_status;
        }
    }

    __hdd_init_status = loadIRXs();
    return __hdd_init_status;
}
#endif

#ifdef F_deinit_ps2_hdd_driver
static void unloadIRXs(void) {
    /* PS2FS.IRX */
    if (CHECK_IRX_UNLOAD(ps2fs)) {
        SifUnloadModule(__ps2fs_id);
        RESET_IRX_VARS(ps2fs);
    }

    /* PS2HDD.IRX */
    if (CHECK_IRX_UNLOAD(ps2hdd)) {
        SifUnloadModule(__ps2hdd_id);
        RESET_IRX_VARS(ps2hdd);
    }

    /* PS2ATAD.IRX */
    if (CHECK_IRX_UNLOAD(ps2atad)) {
        SifUnloadModule(__ps2atad_id);
        RESET_IRX_VARS(ps2atad);
    }
}

void deinit_hdd_driver(bool deinit_dependencies) {
    unloadIRXs();

    // Requires to have FILEXIO and DEV9 drivers loaded
    if (deinit_dependencies) {
        deinit_bdm_driver();
        deinit_dev9_driver();
        deinit_fileXio_driver();
    }
}
#endif

#ifdef F_mount_hdd_partition_ps2_hdd_driver
enum HDD_MOUNT_STATUS mount_hdd_partition(const char *mountpoint, const char *blockdev) {
    if (__hdd_init_status != HDD_INIT_STATUS_IRX_OK)
        return HDD_MOUNT_INIT_STATUS_NOT_READY;

    if (fileXioMount(mountpoint, blockdev, FIO_MT_RDWR) < 0) {
        return HDD_MOUNT_STATUS_ERROR;
    }

    return HDD_MOUNT_STATUS_OK;
}
#endif

#ifdef F_mount_current_partition_ps2_hdd_driver
static char **str_split(char *a_str, const char a_delim) {
    char **result = 0;
    size_t count = 0;
    char *tmp = a_str;
    char *last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp) {
        if (a_delim == *tmp) {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char *) * count);

    if (result) {
        size_t idx = 0;
        char *token = strtok(a_str, delim);

        while (token) {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

/* This method returns true if it can extract needed info from path, otherwise false.
 * In case of true, it also updates mountString, mountPoint and newCWD parameters
 * It splits path by ":", and requires a minimum of 3 elements
 * Example: if path = hdd0:__common:pfs:/retroarch/ then
 * mountString = "pfs:"
 * mountPoint = "hdd0:__common"
 * newCWD = pfs:/retroarch/
 * return true
 */
static bool getMountInfo(char *path, char *mountString, char *mountPoint, char *newCWD) {
    int expected_items = 4;
    int i = 0;
    char *items[expected_items];
    char **tokens = str_split(path, ':');

    if (!tokens)
        return false;

    for (i = 0; *(tokens + i); i++) {
        if (i < expected_items) {
            items[i] = *(tokens + i);
        } else {
            free(*(tokens + i));
        }
    }

    if (i < 3)
        return false;

    sprintf(mountPoint, "%s:%s", items[0], items[1]);
    sprintf(mountString, "%s:", items[2]);
    sprintf(newCWD, "%s%s", mountString, i > 3 ? items[3] : "");

    free(items[0]);
    free(items[1]);
    free(items[2]);

    if (i > 3)
        free(items[3]);

    return true;
}

enum HDD_MOUNT_STATUS mount_current_hdd_partition() {
    // Mount HDD partitions if CWD is HDD
    char cwd[FILENAME_MAX];
    char new_cwd[FILENAME_MAX];
    char mountPoint[50];

    if (__hdd_init_status != HDD_INIT_STATUS_IRX_OK)
        return HDD_MOUNT_INIT_STATUS_NOT_READY;

    if (!__cwd_is_hdd())
        return HDD_MOUNT_STATUS_WRONG_CWD;

    getcwd(cwd, sizeof(cwd));
    if (!getMountInfo(cwd, __mountString, mountPoint, new_cwd))
        return HDD_MOUNT_STATUS_INFO_ERROR;

    if (fileXioMount(__mountString, mountPoint, FIO_MT_RDWR) < 0) {
        return HDD_MOUNT_STATUS_ERROR;
    }

    // Update chdir with the mounted unit
    chdir(new_cwd);

    __mount_status = HDD_MOUNT_STATUS_OK;

    return HDD_MOUNT_STATUS_OK;
}
#endif

#ifdef F_umount_hdd_partition_ps2_hdd_driver
void umount_hdd_partition(const char *mountpoint) {
    if (__mount_status == HDD_MOUNT_STATUS_OK) {
        fileXioUmount(mountpoint);
        fileXioDevctl(mountpoint, PDIOC_CLOSEALL, NULL, 0, NULL, 0);
        fileXioDevctl("hdd0:", HDIOC_IDLEIMM, NULL, 0, NULL, 0);

        __mount_status = HDD_MOUNT_STATUS_UKNOWN;
        memset(__mountString, 0, 10);
    }
}
#else
void umount_hdd_partition(const char *mountpoint);
#endif

#ifdef F_umount_current_partition_ps2_hdd_driver
void umount_current_hdd_partition() {
    umount_hdd_partition(__mountString);
}
#endif
