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

#include <ps2_poweroff_driver.h>
#include <ps2_hdd_driver.h>
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

static void init_drivers() {
	init_hdd_driver(true);
}

static void deinit_drivers() {
	deinit_hdd_driver(true);
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
		scr_printf("Couldn't open the directory");
	}
}

int main(int argc, char **argv) {
	reset_IOP();
	init_scr();

	init_drivers();

	scr_printf("\n\n\nHDD example!\n\n\n");
	mount_current_hdd_partition();
	print_current_folder();

	umount_current_hdd_partition();
	deinit_drivers();
	sleep(5);
}
