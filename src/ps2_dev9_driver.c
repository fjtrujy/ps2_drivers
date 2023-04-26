/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_DEV9_DRIVER
*/

#include <stdint.h>
#include <stddef.h>
#include <ps2_dev9_driver.h>

#include <sifrpc.h>
#include <loadfile.h>

/* References to PS2DEV9.IRX */
extern unsigned char ps2dev9_irx[] __attribute__((aligned(16)));
extern unsigned int size_ps2dev9_irx;

#ifdef F_internals_ps2_dev9_driver
enum DEV9_INIT_STATUS __dev9_init_status = DEV9_INIT_STATUS_UNKNOWN;
int32_t __ps2dev9_id = -1;
#else
extern enum DEV9_INIT_STATUS __dev9_init_status;
extern int32_t __ps2dev9_id;
#endif

#ifdef F_init_ps2_dev9_driver
static enum DEV9_INIT_STATUS loadIRXs(void) {
    /* PS2DEV9.IRX */
    __ps2dev9_id = SifExecModuleBuffer(&ps2dev9_irx, size_ps2dev9_irx, 0, NULL, NULL);
    if (__ps2dev9_id < 0)
        return DEV9_INIT_STATUS_IRX_ERROR;
    
    return DEV9_INIT_STATUS_OK;
}

enum DEV9_INIT_STATUS init_dev9_driver() {
    __dev9_init_status = loadIRXs();
    return __dev9_init_status;
}
#endif

#ifdef F_deinit_ps2_dev9_driver
static void unloadIRXs(void) {
    /* DEV9.IRX */
    if (__ps2dev9_id > 0) {
        SifUnloadModule(__ps2dev9_id);
        __ps2dev9_id = -1;
    }
}

void deinit_dev9_driver() {        
    unloadIRXs();
}
#endif