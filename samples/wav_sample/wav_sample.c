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

#include <audsrv.h>
#include <ps2_audio_driver.h>

static void prepare_IOP()
{
   SifInitRpc(0);
   sbv_patch_enable_lmb();
   sbv_patch_disable_prefix_check();
}

int main(int argc, char **argv)
{
	int ret;
	int played;
	int err;
	char chunk[2048];
	FILE *wav;
	struct audsrv_fmt_t format;
	enum AUDIO_INIT_STATUS audio_res;

	prepare_IOP();
	printf("\n\nwav sample\n\n");
    audio_res = init_audio_driver();
	printf("init_audio_driver returns:%i\n", audio_res);


	format.bits = 16;
	format.freq = 22050;
	format.channels = 2;
	err = audsrv_set_format(&format);
	printf("set format returned %d\n", err);
	printf("audsrv returned error string: %s\n", audsrv_get_error_string());

	audsrv_set_volume(MAX_VOLUME);

	wav = fopen("host:song_22k.wav", "rb");
	if (wav == NULL)
	{
		printf("failed to open wav file\n");
		audsrv_quit();
		return 1;
	}

	fseek(wav, 0x30, SEEK_SET);

	printf("starting play loop\n");
	played = 0;
	while (1)
	{
		ret = fread(chunk, 1, sizeof(chunk), wav);
		if (ret > 0)
		{
			audsrv_wait_audio(ret);
			audsrv_play_audio(chunk, ret);
		}

		if (ret < sizeof(chunk))
		{
			/* no more data */
			fseek(wav, 0x30, SEEK_SET);
			// printf("No more data\n!");
			// break;
		}

		played++;
		if (played % 8 == 0)
		{
			printf(".");
		}

		if (played == 512) {
			printf("Played already 512 times, quiting\n!");
			break;
		}
	}

	fclose(wav);

	printf("sample: stopping audsrv\n");
	audsrv_quit();

	printf("sample: ended\n");
	return 0;
}
