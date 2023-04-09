/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_USBD_DRIVER
*/

#include <stdint.h>
#include <stddef.h>
#include <ps2_usbd_driver.h>

#include <sifrpc.h>
#include <loadfile.h>

/* References to USBD.IRX */
extern unsigned char usbd_irx[] __attribute__((aligned(16)));
extern unsigned int size_usbd_irx;

#ifdef F_internals_ps2_usbd_driver
enum USBD_INIT_STATUS __usbd_init_status = USBD_INIT_STATUS_UNKNOWN;
int32_t __usbd_id = -1;
#else
extern enum USBD_INIT_STATUS __usbd_init_status;
extern int32_t __usbd_id;
#endif

#ifdef F_init_ps2_usbd_driver
static enum USBD_INIT_STATUS loadIRXs(void) {
    /* USBD.IRX */
    __usbd_id = SifExecModuleBuffer(&usbd_irx, size_usbd_irx, 0, NULL, NULL);
    if (__usbd_id < 0)
        return USBD_INIT_STATUS_IRX_ERROR;
    
    return USBD_INIT_STATUS_IRX_OK;
}

enum USBD_INIT_STATUS init_usbd_driver() {
    __usbd_init_status = loadIRXs();
    return __usbd_init_status;
}
#endif

#ifdef F_deinit_ps2_usbd_driver
static void unloadIRXs(void) {
    /* USBD.IRX */
    if (__usbd_id > 0) {
        SifUnloadModule(__usbd_id);
        __usbd_id = -1;
    }
}

void deinit_usbd_driver() {        
    unloadIRXs();
}
#endif