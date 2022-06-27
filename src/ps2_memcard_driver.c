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
#include <ps2_memcard_driver.h>
#include <ps2_sio2man_driver.h>

#include <sifrpc.h>
#include <loadfile.h>

/* References to MCMAN.IRX */
extern unsigned char mcman_irx[] __attribute__((aligned(16)));
extern unsigned int size_mcman_irx;

/* References to MCSERV.IRX */
extern unsigned char mcserv_irx[] __attribute__((aligned(16)));
extern unsigned int size_mcserv_irx;

#ifdef F_internals_ps2_memcard_driver
enum MEMCARD_INIT_STATUS __memcard_init_status = MEMCARD_INIT_STATUS_UNKNOWN;
int32_t __mcman_id = -1;
int32_t __mcserv_id = -1;
#else
extern enum MEMCARD_INIT_STATUS __memcard_init_status;
extern int32_t __mcman_id;
extern int32_t __mcserv_id;
#endif

#ifdef F_init_ps2_memcard_driver
static enum MEMCARD_INIT_STATUS loadIRXs(void) {
    /* MCMAN.IRX */
    __mcman_id = SifExecModuleBuffer(&mcman_irx, size_mcman_irx, 0, NULL, NULL);
    if (__mcman_id < 0)
        return MEMCARD_INIT_STATUS_MCMAN_IRX_ERROR;

    /* PADMAN.IRX */
    __mcserv_id = SifExecModuleBuffer(&mcserv_irx, size_mcserv_irx, 0, NULL, NULL);
    if (__mcserv_id < 0)
        return MEMCARD_INIT_STATUS_MCSERV_IRX_ERROR;
    
    return MEMCARD_INIT_STATUS_IRX_OK;
}

enum MEMCARD_INIT_STATUS init_memcard_driver(bool init_dependencies) {
    // Requires to have SIO2MAN
    if (init_dependencies && init_sio2man_driver() < 0)
        return MEMCARD_INIT_STATUS_DEPENDENCY_IRX_ERROR;

    __memcard_init_status = loadIRXs();
    return __memcard_init_status;
}
#endif

#ifdef F_deinit_ps2_memcard_driver
static void unloadIRXs(void) {
    /* MCMAN.IRX */
    if (__mcman_id > 0) {
        SifUnloadModule(__mcman_id);
        __mcman_id = -1;
    }
    
    /* PADMAN.IRX */
    if (__mcserv_id > 0) {
        SifUnloadModule(__mcserv_id);
        __mcserv_id = -1;
    }
}

void deinit_memcard_driver(bool init_dependencies) {
    // Requires to have SIO2MAN
    if (init_dependencies)
        deinit_sio2man_driver();
    
    unloadIRXs();
}
#endif