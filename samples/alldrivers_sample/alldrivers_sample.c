/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

// Based on pad sample by pukko, check the pad samples for more advanced features.

#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <iopcontrol.h>
#include <sbv_patches.h>
#include <stdbool.h>

#include <ps2_all_drivers.h>

static void reset_IOP() {
    SifInitRpc(0);
#if !defined(DEBUG) || defined(BUILD_FOR_PCSX2)
    /* Comment this line if you don't wanna debug the output */
    while (!SifIopReset(NULL, 0)) {};
#endif

    while (!SifIopSync()) {};
    SifInitRpc(0);
    sbv_patch_enable_lmb();
    sbv_patch_disable_prefix_check();
}

static void init_drivers() {
    init_fileXio_driver();
    init_memcard_driver(true);
    init_usb_driver(true);
    init_cdfs_driver();
    init_joystick_driver(true);
    init_audio_driver();
    init_poweroff_driver();
    init_hdd_driver(true, true);
}

static void deinit_drivers() {
    deinit_poweroff_driver();
    deinit_audio_driver();
    deinit_joystick_driver(false);
    deinit_usb_driver(false);
    deinit_cdfs_driver();
    deinit_memcard_driver(true);
    deinit_hdd_driver(false);
    deinit_fileXio_driver();
}

int main(int argc, char **argv) {
    reset_IOP();
    init_drivers();
    printf("Hello world with all drivers initied!\n");
    deinit_drivers();
}
