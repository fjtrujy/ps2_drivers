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

#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <iopcontrol.h>
#include <sbv_patches.h>

#include <ps2_filesystem_driver.h>

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
    init_only_boot_ps2_filesystem_driver();
}

static void deinit_drivers() {
    deinit_only_boot_ps2_filesystem_driver();
}

void write_to_file(const char *path) {
    FILE *pFile;
    pFile = fopen(path, "a");

    if (pFile) {
        fprintf(pFile, "fjtrujy rocks!\n");
        // print current working directory
        char cwd[FILENAME_MAX];
        getcwd(cwd, sizeof(cwd));
        fprintf(pFile, "Current working directory: %s\n", cwd);
        fclose(pFile);
    } else {
        printf("Couldn't create Log.txt file\n");
    }
}

int main(int argc, char **argv) {
    reset_IOP();
    init_drivers();

    write_to_file("dummy.txt");

    deinit_drivers();
}
