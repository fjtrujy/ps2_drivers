/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

#include <kernel.h>
#include <ps2_all_drivers.h>
#include <libpwroff.h>
#include <sifrpc.h>
#include <iopcontrol.h>
#include <sbv_patches.h>
#include <debug.h>

static void reset_IOP() {
	SifInitRpc(0);
	#if !defined(DEBUG) || defined(BUILD_FOR_PCSX2)
	/* Comment this line if you don't wanna debug the output */
	while(!SifIopReset(NULL, 0)){};
	#endif

	while(!SifIopSync()){};
	SifInitRpc(0);
	sbv_patch_enable_lmb();
	sbv_patch_disable_prefix_check();
}

static bool waitUntilDeviceIsReady(char *path)
{
    struct stat buffer;
    int ret = -1;
    int retries = 50;

    while(ret != 0 && retries > 0) {
        ret = stat(path, &buffer);
        /* Wait untill the device is ready */
        nopdelay();

        retries--;
    }

    return ret == 0;
}

static void deinit_drivers(bool deinit_powerOff) {
	deinit_hdd_driver(false);
	deinit_usb_driver();
	deinit_memcard_driver(true);
	deinit_fileXio_driver();
	
	if (deinit_powerOff)
		deinit_poweroff_driver();
}

static void prepare_for_exit(bool deinit_powerOff) {
	umount_current_hdd_partition();

	deinit_drivers(deinit_powerOff);
}

static void poweroffHandler(void *arg)
{
   prepare_for_exit(false);
   poweroffShutdown();
}

static void init_drivers() {
	init_poweroff_driver();
	init_fileXio_driver();
	init_memcard_driver(true);
	init_usb_driver();
	init_hdd_driver(false);

	poweroffSetCallback(&poweroffHandler, NULL);
	mount_current_hdd_partition();
}

static void print_current_folder() {
	char cwd[FILENAME_MAX];
	DIR* dp;
    struct dirent* ep;
    int max = 10;

    getcwd(cwd, sizeof(cwd));
	scr_printf("\n\nTrying to open %s\n\n", cwd);

    dp = opendir(cwd);
    if (dp != NULL) {
		int count = 0;
        while ((ep = readdir(dp)) != NULL && count != max) {
            scr_printf(ep->d_name);
            scr_printf(" ");

            char fname[1024];
            snprintf(fname, 1024, "%s%s", cwd,ep->d_name);
            struct stat st;
            stat(fname, &st);
            
            char size[10];
            itoa(st.st_size, size, 10);
            scr_printf(size);

            scr_printf(S_ISDIR(st.st_mode) ? " DIR\n" : " FILE\n");

            count++;
        }
		closedir(dp);
	} else {
		scr_printf("Couldn't open the directory\n");
	}
}

void create_log_file() {
	FILE*        pFile;
	pFile = fopen ("Log.txt", "a");
	if (pFile) {
		fprintf(pFile, "This is a test to check if it working\n");
		fclose (pFile);
	} else {
		scr_printf("Couldn't create Log.txt file\n");
	}
}

int main(int argc, char **argv) {
	char cwd[FILENAME_MAX];
	bool ready;

	reset_IOP();
	init_scr();
	scr_printf("\n\n\n LIST DIR example!\n\n\n");

	init_drivers();

	getcwd(cwd, sizeof(cwd));
	ready = waitUntilDeviceIsReady(cwd);
	scr_printf("     Path %s is ready=%i!\n", cwd, ready);
	if (!ready) {
		prepare_for_exit(true);
		sleep(10);
		return -1;
	}

	print_current_folder();
	create_log_file();

	prepare_for_exit(true);
	sleep(10);
}
