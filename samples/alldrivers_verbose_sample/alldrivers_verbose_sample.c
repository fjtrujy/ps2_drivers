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

#include <debug.h>
#include <ps2_fileXio_driver.h>
#include <ps2_memcard_driver.h>
#include <ps2_usb_driver.h>
#include <ps2_cdfs_driver.h>
#include <ps2_joystick_driver.h>
#include <ps2_audio_driver.h>
#include <ps2_poweroff_driver.h>
#include <ps2_hdd_driver.h>
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
	enum FILEXIO_INIT_STATUS filexio_status = init_fileXio_driver();
	scr_printf("init_fileXio_driver: %d\n", filexio_status);

	enum MEMCARD_INIT_STATUS memcard_status = init_memcard_driver(true);
	scr_printf("init_memcard_driver: %d\n", memcard_status);

	enum USB_INIT_STATUS usb_status = init_usb_driver(true);
	scr_printf("init_usb_driver: %d\n", usb_status);

	enum CDFS_INIT_STATUS cdfs_status = init_cdfs_driver();
	scr_printf("init_cdfs_driver: %d\n", cdfs_status);

	enum JOYSTICK_INIT_STATUS joystick_status = init_joystick_driver(true);
	scr_printf("init_joystick_driver: %d\n", joystick_status);

	enum AUDIO_INIT_STATUS audio_status = init_audio_driver();
	scr_printf("init_audio_driver: %d\n", audio_status);

	enum POWEROFF_INIT_STATUS poweroff_status = init_poweroff_driver();
	scr_printf("init_poweroff_driver: %d\n", poweroff_status);

	enum HDD_INIT_STATUS hdd_status = init_hdd_driver(true, true);
	scr_printf("init_hdd_driver: %d\n", hdd_status);
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
    init_scr();
    scr_printf("\n\n\nAll drivers verbose example!\n\n\n");
    init_drivers();
    deinit_drivers();
    SleepThread();
    return 0;
}
