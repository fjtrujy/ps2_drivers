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
#include <stdbool.h>
#include <stddef.h>
#include <ps2_sio2man_driver.h>
#include <ps2_joystick_driver.h>

#include <sifrpc.h>
#include <loadfile.h>
#include <libmtap.h>
#include <libpad.h>

/* References to MTAPMAN.IRX */
extern unsigned char mtapman_irx[] __attribute__((aligned(16)));
extern unsigned int size_mtapman_irx;

/* References to PADMAN.IRX */
extern unsigned char padman_irx[] __attribute__((aligned(16)));
extern unsigned int size_padman_irx;

#ifdef F_internals_ps2_joystick_driver
enum JOYSTICK_INIT_STATUS __joystick_init_status = JOYSTICK_INIT_STATUS_UNKNOWN;
int32_t __padman_id = -1;
int32_t __mtapman_id = -1;
#else
extern enum JOYSTICK_INIT_STATUS __joystick_init_status;
extern int32_t __padman_id;
extern int32_t __mtapman_id;
#endif

#ifdef F_init_ps2_joystick_driver
static enum JOYSTICK_INIT_STATUS loadIRXs(void) {
    /* MTAPMAN.IRX */
    __mtapman_id = SifExecModuleBuffer(&mtapman_irx, size_mtapman_irx, 0, NULL, NULL);
    if (__mtapman_id < 0)
        return JOYSTICK_INIT_STATUS_MTAP_IRX_ERROR;

    /* PADMAN.IRX */
    __padman_id = SifExecModuleBuffer(&padman_irx, size_padman_irx, 0, NULL, NULL);
    if (__padman_id < 0)
        return JOYSTICK_INIT_STATUS_PAD_IRX_ERROR;

    return JOYSTICK_INIT_STATUS_IRX_OK;
}

static enum JOYSTICK_INIT_STATUS initLibraries(void) {
    /* Initializes pad un multitap libraries */
    if (mtapInit() != 1)
        return JOYSTICK_INIT_STATUS_MTAP_ERROR;

    if (padInit(0) != 1)
        return JOYSTICK_INIT_STATUS_PAD_ERROR;

    return JOYSTICK_INIT_STATUS_OK;
}

enum JOYSTICK_INIT_STATUS init_joystick_driver(bool init_dependencies) {
    // Requires to have SIO2MAN
    if (init_dependencies && init_sio2man_driver() < 0)
        return JOYSTICK_INIT_STATUS_DEPENDENCY_IRX_ERROR;

    __joystick_init_status = loadIRXs();
    if (__joystick_init_status < 0)
        return __joystick_init_status;

    __joystick_init_status = initLibraries();

    return __joystick_init_status;
}
#endif

#ifdef F_deinit_ps2_joystick_driver
static void deinitLibraries(void) {
    padEnd();
}

static void unloadIRXs(void) {
    /* MTAPMAN.IRX */
    if (__mtapman_id > 0) {
        SifUnloadModule(__mtapman_id);
        __mtapman_id = -1;
    }

    /* MTAPMAN.IRX */
    if (__padman_id > 0) {
        SifUnloadModule(__padman_id);
        __padman_id = -1;
    }
}

void deinit_joystick_driver(bool deinit_dependencies) {
    deinitLibraries();
    unloadIRXs();

    // Requires to have SIO2MAN
    if (deinit_dependencies)
        deinit_sio2man_driver();
}
#endif
