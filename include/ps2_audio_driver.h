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

#ifndef PS2_AUDIO_DRIVER
#define PS2_AUDIO_DRIVER

#ifdef __cplusplus
extern "C" {
#endif

enum AUDIO_INIT_STATUS {
    AUDIO_INIT_STATUS_AUDSRV_IRX_ERROR = -4,
    AUDIO_INIT_STATUS_LIBSD_IRX_ERROR = -3,
    AUDIO_INIT_STATUS_EEAUDSRV_ERROR = -2,
    AUDIO_INIT_STATUS_UNKNOWN = -1,
    AUDIO_INIT_STATUS_OK = 0,
    AUDIO_INIT_STATUS_IRX_OK = 1,
};

enum AUDIO_INIT_STATUS init_audio_driver(void);
void deinit_audio_driver(void);

#ifdef __cplusplus
}
#endif

#endif /* PS2_AUDIO_DRIVER */