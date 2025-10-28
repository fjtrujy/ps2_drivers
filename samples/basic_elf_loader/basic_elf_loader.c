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

#include <fcntl.h>
#include <limits.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <elf-loader.h>
#include <kernel.h>
#include <iopcontrol.h>
#include <loadfile.h>
#include <sifrpc.h>
#include <sbv_patches.h>
#include <tamtypes.h>

#include <ps2_filesystem_driver.h>

// Used to get BDM driver name
#define NEWLIB_PORT_AWARE
#include <fileXio_rpc.h>
#include <io_common.h>
#include <usbhdfsd-common.h>
#include <ps2sdkapi.h>

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
    init_ps2_filesystem_driver();
}

static void deinit_drivers() {
    deinit_ps2_filesystem_driver();
}

int main(int argc, char **argv) {
    char elf_path[PATH_MAX];
    reset_IOP();
    init_drivers();

    printf("Starting tiny loader\n");
    
    // Get the game name from a file called game_name.ini
	FILE *fp = fopen("elf_path.ini", "r");
	if (fp) {
		fgets(elf_path, PATH_MAX, fp);
		fclose(fp);
	}

    printf("Loading ELF from %s\n", elf_path);
    waitUntilDeviceIsReady(elf_path);
    LoadELFFromFile(elf_path, argc, argv);

    printf("Done");

    deinit_drivers();
}
