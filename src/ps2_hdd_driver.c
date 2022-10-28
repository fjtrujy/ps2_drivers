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
#include <ps2_hdd_driver.h>

#include <sifrpc.h>
#include <loadfile.h>
#define NEWLIB_PORT_AWARE
#include <fileXio_rpc.h>
#include <hdd-ioctl.h>

/* References to PS2DEV9.IRX */
extern unsigned char ps2dev9_irx[] __attribute__((aligned(16)));
extern unsigned int size_ps2dev9_irx;

/* References to PS2ATAD.IRX */
extern unsigned char ps2atad_irx[] __attribute__((aligned(16)));
extern unsigned int size_ps2atad_irx;

/* References to PS2HDD.IRX */
extern unsigned char ps2hdd_irx[] __attribute__((aligned(16)));
extern unsigned int size_ps2hdd_irx;

/* References to PS2FS.IRX */
extern unsigned char ps2fs_irx[] __attribute__((aligned(16)));
extern unsigned int size_ps2fs_irx;

#ifdef F_internals_ps2_hdd_driver
enum HDD_INIT_STATUS __hdd_init_status = HDD_INIT_STATUS_UNKNOWN;
int32_t __ps2dev9_id = -1;
int32_t __ps2atad_id = -1;
int32_t __ps2hdd_id = -1;
int32_t __ps2fs_id = -1;
#else
extern enum HDD_INIT_STATUS __hdd_init_status;
extern int32_t __ps2dev9_id;
extern int32_t __ps2atad_id;
extern int32_t __ps2hdd_id;
extern int32_t __ps2fs_id;
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
    char hddarg[] = "-o" "\0" "4" "\0" "-n" "\0" "20";

    /* PS2DEV9.IRX */
    __ps2dev9_id = SifExecModuleBuffer(&ps2dev9_irx, size_ps2dev9_irx, 0, NULL, NULL);
    if (__ps2dev9_id < 0)
        return HDD_INIT_STATUS_PS2DEV9_IRX_ERROR;
    
    /* PS2ATAD.IRX */
    __ps2atad_id = SifExecModuleBuffer(&ps2atad_irx, size_ps2atad_irx, 0, NULL, NULL);
    if (__ps2atad_id < 0)
        return HDD_INIT_STATUS_PS2ATAD_IRX_ERROR;

    /* PS2HDD.IRX */
    __ps2hdd_id = SifExecModuleBuffer(&ps2hdd_irx, size_ps2hdd_irx, sizeof(hddarg), hddarg, NULL);
    if (__ps2hdd_id < 0)
        return HDD_INIT_STATUS_PS2HDD_IRX_ERROR;

    /* Check if HDD is formatted and ready to be used */
    if (hddCheck() < 0)
        return HDD_INIT_STATUS_HDD_NOT_READY_ERROR;
    
    /* PS2FS.IRX */
    __ps2fs_id = SifExecModuleBuffer(&ps2fs_irx, size_ps2fs_irx, 0, NULL, NULL);
    if (__ps2fs_id < 0)
        return HDD_INIT_STATUS_PS2FS_IRX_ERROR;
    
    return HDD_INIT_STATUS_IRX_OK;
}

enum HDD_INIT_STATUS init_hdd_driver() {
    __hdd_init_status = loadIRXs();
    return __hdd_init_status;
}
#endif

#ifdef F_deinit_ps2_hdd_driver
static void unloadIRXs(void) {
    /* PS2FS.IRX */
    if (__ps2fs_id > 0) {
        SifUnloadModule(__ps2fs_id);
        __ps2fs_id = -1;
    }

    /* PS2HDD.IRX */
    if (__ps2hdd_id > 0) {
        SifUnloadModule(__ps2hdd_id);
        __ps2hdd_id = -1;
    }

    /* PS2ATAD.IRX */
    if (__ps2atad_id > 0) {
        SifUnloadModule(__ps2atad_id);
        __ps2atad_id = -1;
    }

    /* PS2DEV9.IRX */
    if (__ps2dev9_id > 0) {
        SifUnloadModule(__ps2dev9_id);
        __ps2dev9_id = -1;
    }
}

void deinit_hdd_driver() {
    unloadIRXs();
}
#endif