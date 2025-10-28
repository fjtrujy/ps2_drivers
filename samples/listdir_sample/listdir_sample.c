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

// If you want to also start HDD drivers even if booted from host, put the value to 0
#define ENABLED_HDD_IF_BOOT_FROM_HDD 1

static void reset_IOP() {
    SifInitRpc(0);
#if !defined(DEBUG) || defined(BUILD_FOR_PCSX2)
    /* Comment this line if you don't wanna debug the output */
    while (!SifIopReset(NULL, 0)) {};
#endif

    while (!SifIopSync()) {};
    SifInitRpc(0);
    sbv_patch_enable_lmb();
    sbv_patch_disable_prefix_check();
}

#if !defined(ENABLED_HDD_IF_BOOT_FROM_HDD)
static void mount_default_partition() {
    int res = mount_hdd_partition("pfs:", "hdd0:__common");
    printf("mount_hdd_partition res=%i\n", res);
}

static void umount_default_partition() {
    umount_hdd_partition("pfs:");
}
#endif

static void deinit_drivers(bool deinit_powerOff) {
    deinit_hdd_driver(false);
    deinit_usb_driver(true);
    deinit_memcard_driver(true);
    deinit_fileXio_driver();

    if (deinit_powerOff)
        deinit_poweroff_driver();
}

static void prepare_for_exit(bool deinit_powerOff) {
#if defined(ENABLED_HDD_IF_BOOT_FROM_HDD)
    umount_current_hdd_partition();
#else
    umount_default_partition();
#endif

    deinit_drivers(deinit_powerOff);
}

static void poweroffHandler(void *arg) {
    prepare_for_exit(false);
    poweroffShutdown();
}

static void init_drivers() {
    init_poweroff_driver();
    init_fileXio_driver();
    init_memcard_driver(true);
    init_usb_driver(true);
    init_hdd_driver(false, false);

    poweroffSetCallback(&poweroffHandler, NULL);
#if defined(ENABLED_HDD_IF_BOOT_FROM_HDD)
    mount_current_hdd_partition();
#else
    mount_default_partition();
#endif
}


static void print_folder(const char *path) {
    DIR *dp;
    struct dirent *ep;
    int max = 10;

    printf("\n\nTrying to open %s\n\n", path);

    dp = opendir(path);
    if (dp != NULL) {
        int count = 0;
        while ((ep = readdir(dp)) != NULL && count != max) {
            printf(ep->d_name);
            printf(" ");

            char fname[1024];
            snprintf(fname, 1024, "%s%s", path, ep->d_name);
            struct stat st;
            stat(fname, &st);

            char size[10];
            itoa(st.st_size, size, 10);
            scr_printf(size);

            scr_printf(ep->d_type == DT_DIR ? "DIR" : "FILE");

            count++;
        }
        closedir(dp);
    } else {
        scr_printf("Couldn't open the directory\n");
    }
}

void create_log_file(const char *path) {
    FILE *pFile;
    pFile = fopen(path, "a");
    if (pFile) {
        fprintf(pFile, "This is a test to check if it working\n");
        fclose(pFile);
    } else {
        scr_printf("Couldn't create Log.txt file\n");
    }
}

char *concat(const char *s1, const char *s2) {
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

int main(int argc, char **argv) {
    bool ready;

    reset_IOP();
    init_scr();
    scr_printf("\n\n\n LIST DIR example!\n\n\n");

    init_drivers();

#if defined(ENABLED_HDD_IF_BOOT_FROM_HDD)
    char cwd[FILENAME_MAX];
    getcwd(cwd, sizeof(cwd));
#else
    const char *cwd = "pfs:/tests_sdl/";
#endif
    ready = waitUntilDeviceIsReady(cwd);
    scr_printf("     Path %s is ready=%i!\n", cwd, ready);
    if (!ready) {
        prepare_for_exit(true);
        sleep(10);
        return -1;
    }

    const char *log_file = concat(cwd, "Log.txt");
    print_folder(cwd);
    create_log_file(log_file);

    prepare_for_exit(true);
    sleep(10);
}
