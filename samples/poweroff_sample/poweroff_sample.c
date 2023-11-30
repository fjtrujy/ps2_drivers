/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdbool.h>
#include <unistd.h>

#include <ps2_poweroff_driver.h>
#include <libpwroff.h>
#include <sifrpc.h>
#include <iopcontrol.h>
#include <sbv_patches.h>

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
    init_poweroff_driver();
}

static void deinit_drivers() {
    deinit_poweroff_driver();
}

int main(int argc, char **argv) {
    int seconds = 5;
    reset_IOP();
    init_drivers();
    printf("Power off example!\n");
    while (seconds > 0) {
        printf("Shutting down in %i\n", seconds);
        sleep(1);
        seconds--;
    }

    deinit_drivers();
    poweroffShutdown();
}
