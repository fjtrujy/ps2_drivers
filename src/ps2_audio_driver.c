/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# PS2_AUDIO_DRIVER
*/

#include <stdint.h>
#include <stddef.h>
#include <ps2_audio_driver.h>

#include <sifrpc.h>
#include <loadfile.h>
#include <audsrv.h>

/* References to LIBSD.IRX */
extern unsigned char libsd_irx[] __attribute__((aligned(16)));
extern unsigned int size_libsd_irx;

/* References to AUDSRV.IRX */
extern unsigned char audsrv_irx[] __attribute__((aligned(16)));
extern unsigned int size_audsrv_irx;

#ifdef F_internals_ps2_audio_driver
enum AUDIO_INIT_STATUS __audio_init_status = AUDIO_INIT_STATUS_UNKNOWN;
int32_t __libsd_id = -1;
int32_t __audsrv_id = -1;
#else
extern enum AUDIO_INIT_STATUS __audio_init_status;
extern int32_t __libsd_id;
extern int32_t __audsrv_id;
#endif

#ifdef F_init_ps2_audio_driver
static enum AUDIO_INIT_STATUS loadIRXs(void) {
    /* LIBSD.IRX */
    __libsd_id = SifExecModuleBuffer(&libsd_irx, size_libsd_irx, 0, NULL, NULL);
    if (__libsd_id < 0)
        return AUDIO_INIT_STATUS_LIBSD_IRX_ERROR;

    /* AUDSRV.IRX */
    __audsrv_id = SifExecModuleBuffer(&audsrv_irx, size_audsrv_irx, 0, NULL, NULL);
    if (__audsrv_id < 0)
        return AUDIO_INIT_STATUS_AUDSRV_IRX_ERROR;
    
    return AUDIO_INIT_STATUS_IRX_OK;
}

static enum AUDIO_INIT_STATUS initLibraries(void) {
    /* Initializes audsrv libraries */
    if (audsrv_init() != 0)
        return AUDIO_INIT_STATUS_EEAUDSRV_ERROR;

    return AUDIO_INIT_STATUS_OK;
}

enum AUDIO_INIT_STATUS init_audio_driver() {
    __audio_init_status = loadIRXs();
    if (__audio_init_status < 0)
        return __audio_init_status;

    __audio_init_status = initLibraries();

    return __audio_init_status;
}
#endif

#ifdef F_deinit_ps2_audio_driver
static void deinitLibraries(void) {
    audsrv_quit();
}

static void unloadIRXs(void) {
    /* AUDSRV.IRX */
    if (__audsrv_id > 0) {
        SifUnloadModule(__audsrv_id);
        __audsrv_id = -1;
    }
    
    /* LIBSD.IRX */
    if (__libsd_id > 0) {
        SifUnloadModule(__libsd_id);
        __libsd_id = -1;
    }
}

void deinit_audio_driver() {        
    deinitLibraries();
    unloadIRXs();
}
#endif