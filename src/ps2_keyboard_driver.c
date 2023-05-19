/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_KEYBOARD_DRIVER
*/

#include <stdint.h>
#include <stddef.h>
#include <ps2_keyboard_driver.h>
#include <ps2_usbd_driver.h>

#include <sifrpc.h>
#include <loadfile.h>
#include <libkbd.h>

/* References to PS2KBD.IRX */
extern unsigned char ps2kbd_irx[] __attribute__((aligned(16)));
extern unsigned int size_ps2kbd_irx;

#ifdef F_internals_ps2_keyboard_driver
enum KEYBOARD_INIT_STATUS __keyboard_init_status = KEYBOARD_INIT_STATUS_UNKNOWN;
int32_t __keyboard_id = -1;
#else
extern enum KEYBOARD_INIT_STATUS __keyboard_init_status;
extern int32_t __keyboard_id;
#endif

#ifdef F_init_ps2_keyboard_driver
static enum KEYBOARD_INIT_STATUS loadIRXs(bool init_dependencies) {
    if (init_dependencies) {
        /* USBD.IRX */
        if (init_usbd_driver() < 0)
            return KEYBOARD_INIT_STATUS_DEPENDENCY_ERROR;
    }

    /* PS2KBD.IRX */
    __keyboard_id = SifExecModuleBuffer(&ps2kbd_irx, size_ps2kbd_irx, 0, NULL, NULL);
    if (__keyboard_id < 0)
        return KEYBOARD_INIT_STATUS_IRX_ERROR;
    
    return KEYBOARD_INIT_STATUS_IRX_OK;
}

static enum KEYBOARD_INIT_STATUS initLibraries(void) {
    /* Initializes keyboard library */
    if (PS2KbdInit() < 0)
        return KEYBOARD_INIT_STATUS_EERPC_ERROR;

    return KEYBOARD_INIT_STATUS_OK;
}

enum KEYBOARD_INIT_STATUS init_keyboard_driver(bool init_dependencies) {
    __keyboard_init_status = loadIRXs(init_dependencies);
    if (__keyboard_init_status < 0)
        return __keyboard_init_status;

    __keyboard_init_status = initLibraries();

    return __keyboard_init_status;
}
#endif

#ifdef F_deinit_ps2_keyboard_driver
static void unloadIRXs(bool deinit_dependencies) {
    /* PS2KBD.IRX */
    if (__keyboard_id > 0) {
        SifUnloadModule(__keyboard_id);
        __keyboard_id = -1;
    }

    if (deinit_dependencies) {
        deinit_usbd_driver();
    }
}

void deinit_keyboard_driver(bool deinit_dependencies) {   
    PS2KbdClose();

    unloadIRXs(deinit_dependencies);
}
#endif