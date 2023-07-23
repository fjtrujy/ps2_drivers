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

#include "gs_privileged.h"
#include "libpad.h"
#include "libmtap.h"
#include <ps2_joystick_driver.h>

static char *padBuf[2][4];
static u32 padConnected[2][4]; // 2 ports, 4 slots
static u32 padOpen[2][4];
static u32 mtapConnected[2];
static u32 maxslot[2];


int wait_vsync(void) {

    // Enable the vsync interrupt.
    *GS_REG_CSR |= GS_SET_CSR(0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0);

    // Wait for the vsync interrupt.
    while (!(*GS_REG_CSR & (GS_SET_CSR(0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0)))) {}

    // Disable the vsync interrupt.
    *GS_REG_CSR &= ~GS_SET_CSR(0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0);

    // End function.
    return 0;
}

void find_controllers() {
    u32 port, slot;
    u32 mtapcon;

    // Look for multitaps and controllers on both ports
    for (port = 0; port < 2; port++) {
        // if(mtapConnected[port] == 0) mtapPortOpen(port);

        mtapcon = mtapGetConnection(port);

        // if(mtapcon == 0) mtapPortClose(port);

        if ((mtapcon == 1) && (mtapConnected[port] == 0)) {
            printf("Multitap (%i) connected\n", (int)port);
        }

        if ((mtapcon == 0) && (mtapConnected[port] == 1)) {
            printf("Multitap (%i) disconnected(int argc, char **argv)\n", (int)port);
        }

        mtapConnected[port] = mtapcon;

        // Check for multitap
        if (mtapConnected[port] == 1)
            maxslot[port] = 4;
        else
            maxslot[port] = 1;

        // Find any connected controllers
        for (slot = 0; slot < maxslot[port]; slot++) {
            if (padOpen[port][slot] == 0) {
                padOpen[port][slot] = padPortOpen(port, slot, padBuf[port][slot]);
                // if(padOpen[port][slot])
                //	printf("padOpen(%i, %i) = %i\n", port, slot, padOpen[port][slot] );
            }

            if (padOpen[port][slot] == 1) {

                if (padGetState(port, slot) == PAD_STATE_STABLE) {
                    if (padConnected[port][slot] == 0) {
                        printf("Controller (%i,%i) connected\n", (int)port, (int)slot);
                    }

                    padConnected[port][slot] = 1;
                } else {
                    if ((padGetState(port, slot) == PAD_STATE_DISCONN) && (padConnected[port][slot] == 1)) {
                        printf("Controller (%i,%i) disconnected\n", (int)port, (int)slot);
                        padConnected[port][slot] = 0;
                    }
                }
            }
        }

        // Close controllers when multitap is disconnected

        if (mtapConnected[port] == 0) {
            for (slot = 1; slot < 4; slot++) {
                if (padOpen[port][slot] == 1) {
                    padPortClose(port, slot);
                    padOpen[port][slot] = 0;
                }
            }
        }
    }
}

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


int main(int argc, char **argv) {
    u32 i;
    enum JOYSTICK_INIT_STATUS joy_res;

    struct padButtonStatus buttons;
    u32 paddata;
    u32 old_pad[2][4];
    u32 new_pad[2][4];
    s32 ret;

    reset_IOP();
    printf("\n\nlibmtap sample\n\n");
    joy_res = init_joystick_driver(true);
    printf("init_joystick_driver returns:%i\n", joy_res);

    mtapConnected[0] = 0;
    mtapConnected[1] = 0;

    mtapPortOpen(0);
    mtapPortOpen(1);

    for (i = 0; i < 4; i++) {
        padConnected[0][i] = 0;
        padConnected[1][i] = 0;
        padOpen[0][i] = 0;
        padOpen[1][i] = 0;
        old_pad[0][i] = 0;
        old_pad[1][i] = 0;
        new_pad[0][i] = 0;
        new_pad[1][i] = 0;

        padBuf[0][i] = memalign(64, 256);
        padBuf[1][i] = memalign(64, 256);
    }

    while (1) {
        u32 port, slot;

        find_controllers();

        for (port = 0; port < 2; port++) {
            for (slot = 0; slot < maxslot[port]; slot++) {
                if (padOpen[port][slot] && padConnected[port][slot]) {
                    ret = padRead(port, slot, &buttons);

                    if (ret != 0) {
                        paddata = 0xffff ^ buttons.btns;

                        new_pad[port][slot] = paddata & ~old_pad[port][slot];
                        old_pad[port][slot] = paddata;

                        if (new_pad[port][slot])
                            printf("Controller (%i,%i) button(s) pressed: ", (int)port, (int)slot);

                        if (new_pad[port][slot] & PAD_LEFT)
                            printf("LEFT ");
                        if (new_pad[port][slot] & PAD_RIGHT)
                            printf("RIGHT ");
                        if (new_pad[port][slot] & PAD_UP)
                            printf("UP ");
                        if (new_pad[port][slot] & PAD_DOWN)
                            printf("DOWN ");
                        if (new_pad[port][slot] & PAD_START)
                            printf("START ");
                        if (new_pad[port][slot] & PAD_SELECT)
                            printf("SELECT ");
                        if (new_pad[port][slot] & PAD_SQUARE)
                            printf("SQUARE ");
                        if (new_pad[port][slot] & PAD_TRIANGLE)
                            printf("TRIANGLE ");
                        if (new_pad[port][slot] & PAD_CIRCLE)
                            printf("CIRCLE ");
                        if (new_pad[port][slot] & PAD_CROSS)
                            printf("CROSS ");
                        if (new_pad[port][slot] & PAD_L1)
                            printf("L1 ");
                        if (new_pad[port][slot] & PAD_L2)
                            printf("L2 ");
                        if (new_pad[port][slot] & PAD_L3)
                            printf("L3 ");
                        if (new_pad[port][slot] & PAD_R1)
                            printf("R1 ");
                        if (new_pad[port][slot] & PAD_R2)
                            printf("R2 ");
                        if (new_pad[port][slot] & PAD_R3)
                            printf("R3 ");

                        if (new_pad[port][slot])
                            printf("\n");
                    }
                }
            }
        }

        wait_vsync();
    }

    deinit_joystick_driver(true);
    return 0;
}
