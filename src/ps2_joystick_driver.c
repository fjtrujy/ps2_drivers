/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_JOYSTICK_DRIVER
*/

#include <stdint.h>
#include <stddef.h>
#include <ps2_joystick_driver.h>

#include <sifrpc.h>
#include <loadfile.h>
#include <libmtap.h>
#include <libpad.h>

/* References to PADMAN.IRX */
extern unsigned char padman_irx[] __attribute__((aligned(16)));
extern unsigned int size_padman_irx;

/* References to MTAPMAN.IRX */
extern unsigned char mtapman_irx[] __attribute__((aligned(16)));
extern unsigned int size_mtapman_irx;

static enum JOYSTICK_INIT_STATUS __joystick_init_status = JOYSTICK_INIT_STATUS_UNKNOWN;

static enum JOYSTICK_INIT_STATUS loadControllerIRXs(void) {
    /* Controllers */
    if (SifExecModuleBuffer(&mtapman_irx, size_mtapman_irx, 0, NULL, NULL) < 0)
        return JOYSTICK_INIT_STATUS_MTAP_IRX_ERROR;
    if (SifExecModuleBuffer(&padman_irx, size_padman_irx, 0, NULL, NULL) < 0)
        return JOYSTICK_INIT_STATUS_PAD_IRX_ERROR;
    
    return JOYSTICK_INIT_STATUS_IRX_OK;
}

static enum JOYSTICK_INIT_STATUS initalizeControllers(void) {
    /* Initializes pad un multitap libraries */
    if (mtapInit() != 1) {
        return JOYSTICK_INIT_STATUS_MTAP_ERROR;
    }
   
    if (padInit(0) != 1)
    {
        return JOYSTICK_INIT_STATUS_PAD_ERROR;
    }

    return JOYSTICK_INIT_STATUS_OK;
}

enum JOYSTICK_INIT_STATUS init_joystick_driver(void) {
    __joystick_init_status = loadControllerIRXs();
    if (__joystick_init_status < 0)
        return __joystick_init_status;

    __joystick_init_status = initalizeControllers();

    return __joystick_init_status;
}
