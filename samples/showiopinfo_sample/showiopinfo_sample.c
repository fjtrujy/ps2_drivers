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
#include <smod.h>
#include <smem.h>

#include <ps2_filesystem_driver.h>
#include <ps2_audio_driver.h>
#include <ps2_joystick_driver.h>

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
    init_audio_driver();
    // init_joystick_driver(false);
}

static void deinit_drivers() {
    // init_joystick_driver(false);
    deinit_audio_driver();
    deinit_ps2_filesystem_driver();
}

#define NDW -20
#define HDW -9
static void print_iop_modules()
{
    smod_mod_info_t info;
    smod_mod_info_t *curr = NULL;
    char sName[21];
    int iPage = 1;
    int iModsPerPage = 0;
    int rv = -1;
    int i;
    u32 txtsz_total = 0;
    u32 dtasz_total = 0;
    u32 bsssz_total = 0;

    while (rv != 0) {
        // scr_clear();
        printf("\t\tLoaded Modules page %d:\n", iPage);
        printf("\t\t%*s   %*s %*s %*s %*s\n", NDW, "name", HDW, "txtst", HDW, "txtsz", HDW, "dtasz", HDW, "bsssz");
        printf("\t\t-----------------------------------------------------------\n");
        while ((rv = smod_get_next_mod(curr, &info)) != 0) {
            smem_read(info.name, sName, 20);
            sName[20] = 0;
            txtsz_total += info.text_size;
            dtasz_total += info.data_size;
            bsssz_total += info.bss_size;

            printf("\t\t%*s | 0x%5x | 0x%5x | 0x%5x | 0x%5x\n", NDW, sName, info.text_start, info.text_size, info.data_size, info.bss_size);
            curr = &info;

            // iModsPerPage++;
            // if (iModsPerPage >= 16) {
            //     iModsPerPage = 0;
            //     iPage++;

            //     printf("\t\tNext page in ");
            //     for (i = 3; i > 0; i--) {
            //         printf("%d ", i);
            //         sleep(1);
            //     }

            //     break;
            // }
        }
    }

    u32 total = txtsz_total + dtasz_total + bsssz_total;
    printf("\t\t-----------------------------------------------------------\n");
    printf("\t\t%*s |         | 0x%5x | 0x%5x | 0x%5x\n", NDW, "subtotal:", txtsz_total, dtasz_total, bsssz_total);
    printf("\t\t%*s | 0x%5x (%dKiB)\n", NDW, "total:", total, total / 1024);
    printf("\t\t-----------------------------------------------------------\n");
}

int main(int argc, char **argv) {
    reset_IOP();
    // init_scr();
    // scr_printf("\n\n\n SHOW IOP INFO example!\n\n\n");
    init_drivers();

    sleep(5);
    print_iop_modules();

    deinit_drivers();
    
    SleepThread();
    return 0;
}
