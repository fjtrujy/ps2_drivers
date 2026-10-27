#define NEWLIB_PORT_AWARE 1

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <debug.h>
#include <elf-loader.h>
#include <fileXio_rpc.h>
#include <iopcontrol.h>
#include <io_common.h>
#include <iox_stat.h>
#include <loadfile.h>
#include <sifrpc.h>
#include <smod.h>
#include <sbv_patches.h>

#include <ps2_ata_bdm_driver.h>
#include <ps2_ata_bdm_launch.h>
#include <ps2_bdm_driver.h>
#include <ps2_dev9_driver.h>
#include <ps2_fileXio_driver.h>
#include <ps2_hdd_launch.h>
#include <ps2_mx4sio_driver.h>
#include <ps2_usb_driver.h>

#include "boot_args.h"

/* Available in libelf-loader even with older installed SDK headers. */
int LoadELFFromFileWithPartitionNoReset(
    const char *filename, const char *partition, int argc, char *argv[]);

/* The SDK ELF loader uses newlib stat(). PFS and BDM mass mounts are
 * registered through fileXio, not necessarily through newlib's stat shim. */
int __real_stat(const char *path, struct stat *buffer);
int __wrap_stat(const char *path, struct stat *buffer)
{
    iox_stat_t stat_result;

    if (strncmp(path, "pfs:", 4) != 0 &&
        strncmp(path, "pfs0:", 5) != 0 &&
        strncmp(path, "mass", 4) != 0)
        return __real_stat(path, buffer);
    if (fileXioGetStat(path, &stat_result) < 0) {
        errno = ENOENT;
        return -1;
    }
    memset(buffer, 0, sizeof(*buffer));
    buffer->st_mode = S_IFREG | 0444;
    buffer->st_size = (off_t)stat_result.size;
    return 0;
}

static void reset_iop(void)
{
    SifInitRpc(0);
    while (!SifIopReset(NULL, 0)) {}
    while (!SifIopSync()) {}
    SifInitRpc(0);
    sbv_patch_enable_lmb();
    sbv_patch_disable_prefix_check();
    sbv_patch_fileio();
}

static void trim_target(char *target)
{
    size_t length = strlen(target);
    while (length && (target[length - 1] == '\n' ||
                      target[length - 1] == '\r'))
        target[--length] = '\0';
}

/* Verify a real file, not merely a massN: cwd (chdir alone can succeed
 * before a BDM filesystem has actually been mounted). */
static int read_mass_target(const char *directory, char *target, size_t size)
{
    char path[FILENAME_MAX + 32];
    char contents[PATH_MAX] __attribute__((aligned(64)));
    int fd;
    int count;

    if (size > sizeof(contents) ||
        snprintf(path, sizeof(path), "%s%s%s", directory,
                 directory[0] && directory[strlen(directory) - 1] == '/' ? "" : "/",
                 "elf_path.ini") >= (int)sizeof(path))
        return -1;
    fd = fileXioOpen(path, FIO_O_RDONLY, 0);
    if (fd < 0)
        return -1;
    count = fileXioRead(fd, contents, (int)size - 1);
    fileXioClose(fd);
    if (count <= 0 || (size_t)count >= size)
        return -1;
    contents[count] = '\0';
    trim_target(contents);
    if (!contents[0])
        return -1;
    memcpy(target, contents, strlen(contents) + 1);
    return 0;
}

static int wait_for_mass(const char *directory, const char *expected_target,
                         char *selected_target, size_t selected_size)
{
    int tries;

    for (tries = 0; tries < 600; ++tries) {
        if (read_mass_target(directory, selected_target, selected_size) == 0) {
            if (!expected_target[0] ||
                strcmp(expected_target, selected_target) == 0)
                return chdir(directory);
            /* A different application's mass0: means the wrong physical
             * volume claimed this slot. Try the next provider immediately. */
            printf("[irximg-bootstrap] ignoring different target: %s\n",
                   selected_target);
            return -1;
        }
        usleep(5000);
    }
    return -1;
}

/* Probe one transport per clean IOP. This never leaves USB, MX4SIO and
 * ATA providers resident together, nor loads both incompatible ATADs. */
static int restore_mass(const char *directory, const char *expected_target,
                        char *selected_target, size_t selected_size,
                        enum ps2_irximg_mass_transport *selected)
{
    static const enum ps2_irximg_mass_transport order[] = {
        PS2_IRXIMG_MASS_USB, PS2_IRXIMG_MASS_MX4SIO, PS2_IRXIMG_MASS_ATA
    };
    unsigned int index;

    for (index = 0; index < sizeof(order) / sizeof(order[0]); ++index) {
        int result = -1;

        reset_iop();
        if (init_fileXio_driver() < 0 || init_bdm_driver() < 0)
            return -EIO;

        switch (order[index]) {
        case PS2_IRXIMG_MASS_USB:
            printf("[irximg-bootstrap] probing USB at %s\n", directory);
            result = init_usb_driver(true);
            break;
        case PS2_IRXIMG_MASS_MX4SIO:
            printf("[irximg-bootstrap] probing MX4SIO at %s\n", directory);
            result = init_mx4sio_driver(true);
            break;
        case PS2_IRXIMG_MASS_ATA:
            printf("[irximg-bootstrap] probing ATA/BDM at %s\n", directory);
            result = init_dev9_driver();
            if (result >= 0)
                result = init_ata_bdm_driver(false);
            break;
        default:
            break;
        }

        if (result >= 0 && wait_for_mass(directory, expected_target,
                                        selected_target, selected_size) == 0) {
            *selected = order[index];
            printf("[irximg-bootstrap] storage selected: %s\n",
                   ps2_irximg_mass_name(*selected));
            return 0;
        }
    }
    return -ENOENT;
}

/* An ELF browser normally leaves its launch filesystem mounted. Reuse a
 * uniquely identifiable live provider rather than resetting a working IOP.
 * Never guess ATA from massN: alone (or from the presence of ATAD alongside
 * an active USB/MX4SIO provider). */
static enum ps2_irximg_mass_transport live_mass_transport(void)
{
    smod_mod_info_t info;
    int usb = smod_get_mod_by_name("usbmass_bd", &info) > 0;
    int mx4sio = smod_get_mod_by_name("mx4sio", &info) > 0;
    int ata = smod_get_mod_by_name("atad", &info) > 0;

    printf("[irximg-bootstrap] live transports USB=%d MX4SIO=%d ATA=%d\n",
           usb, mx4sio, ata);

    if (usb + mx4sio + ata != 1)
        return PS2_IRXIMG_MASS_UNKNOWN;
    if (usb)
        return PS2_IRXIMG_MASS_USB;
    if (mx4sio)
        return PS2_IRXIMG_MASS_MX4SIO;
    return PS2_IRXIMG_MASS_ATA;
}

static int read_default_target(char *target, size_t size)
{
    FILE *file = fopen("elf_path.ini", "r");

    if (file == NULL)
        return -1;
    if (fgets(target, size, file) == NULL) {
        fclose(file);
        return -1;
    }
    fclose(file);
    trim_target(target);
    return target[0] ? 0 : -1;
}

static int resolve_target(const char *cwd, const char *target,
                          char *resolved, size_t size)
{
    int length;

    if (strchr(target, ':') != NULL)
        length = snprintf(resolved, size, "%s", target);
    else
        length = snprintf(resolved, size, "%s%s%s", cwd,
                          cwd[0] && cwd[strlen(cwd) - 1] == '/' ? "" : "/",
                          target);
    return length >= 0 && (size_t)length < size ? 0 : -1;
}

int main(int argc, char **argv)
{
    char cwd[FILENAME_MAX];
    char target_path[PATH_MAX];
    char expected_target[PATH_MAX] = {0};
    char resolved[PATH_MAX];
    char hdd_argument[PATH_MAX + PS2_HDD_LAUNCH_PARTITION_MAX + 2];
    char **forward_argv;
    ps2_hdd_launch_context_t hdd;
    enum ps2_irximg_boot_kind kind;
    enum ps2_irximg_mass_transport mass_transport = PS2_IRXIMG_MASS_UNKNOWN;
    const char *target;
    int target_index;
    int forwarded;
    int i;

    SifInitRpc(0);
    if (getcwd(cwd, sizeof(cwd)) == NULL)
        cwd[0] = '\0';

    kind = ps2_irximg_boot_kind(argc, (const char *const *)argv, cwd);
    printf("[irximg-bootstrap] launch cwd=%s kind=%d\n", cwd, kind);
    if (kind == PS2_IRXIMG_BOOT_HDD) {
        init_scr();
        if (ps2_hdd_launch_capture(&hdd, argc,
                (const char *const *)argv, cwd) != PS2_HDD_LAUNCH_OK) {
            scr_printf("[irximg-bootstrap] missing HDD launch partition\n");
            return -EINVAL;
        }
        reset_iop();
        if (ps2_hdd_launch_restore(&hdd) != PS2_HDD_LAUNCH_OK)
            return -EIO;
        snprintf(cwd, sizeof(cwd), "%s", hdd.directory);
    } else if (kind == PS2_IRXIMG_BOOT_MASS) {
        /* A surviving original filesystem lets us recognize the *same*
         * application volume after reset, even if USB/ATA both claim mass0.
         * If the launcher's reset destroyed the source, a valid target
         * manifest on the recovered candidate is the minimum requirement. */
        if (read_mass_target(cwd, expected_target,
                             sizeof(expected_target)) == 0) {
            printf("[irximg-bootstrap] expected target: %s\n", expected_target);
            mass_transport = live_mass_transport();
        }
        if (mass_transport != PS2_IRXIMG_MASS_UNKNOWN) {
            printf("[irximg-bootstrap] reusing live %s filesystem\n",
                   ps2_irximg_mass_name(mass_transport));
            snprintf(target_path, sizeof(target_path), "%s", expected_target);
        } else {
            if (restore_mass(cwd, expected_target, target_path,
                             sizeof(target_path), &mass_transport) < 0) {
                printf("[irximg-bootstrap] failed to restore mass filesystem\n");
                return -ENOENT;
            }
        }
    } else if (kind != PS2_IRXIMG_BOOT_OTHER) {
        printf("[irximg-bootstrap] missing or unsupported launch path\n");
        return -EINVAL;
    }

    target_index = ps2_irximg_target_index(argc, (const char *const *)argv);
    if (target_index < argc && argv[target_index] && argv[target_index][0]) {
        target = argv[target_index];
        forwarded = argc - target_index - 1;
    } else {
        if (kind == PS2_IRXIMG_BOOT_MASS) {
            /* Already verified against the original manifest if available. */
            target = target_path;
        } else if (read_default_target(target_path, sizeof(target_path)) < 0) {
            printf("[irximg-bootstrap] no target in elf_path.ini\n");
            return -EINVAL;
        } else {
            target = target_path;
        }
        forwarded = 0;
    }
    if (resolve_target(cwd, target, resolved, sizeof(resolved)) < 0)
        return -ENAMETOOLONG;

    forward_argv = calloc((size_t)forwarded + 2, sizeof(*forward_argv));
    if (forward_argv == NULL)
        return -ENOMEM;

    if (kind == PS2_IRXIMG_BOOT_HDD) {
        if (ps2_hdd_launch_format_argument(&hdd, resolved, hdd_argument,
                sizeof(hdd_argument)) != PS2_HDD_LAUNCH_OK)
            return -EINVAL;
        forward_argv[0] = hdd_argument;
    } else if (mass_transport == PS2_IRXIMG_MASS_ATA) {
        forward_argv[0] = (char *)PS2_ATA_BDM_LAUNCH_MARKER;
    }
    for (i = 0; i < forwarded; ++i)
        forward_argv[i + 1] = argv[target_index + 1 + i];

    printf("[irximg-bootstrap] loading %s without IOP reset\n", resolved);
    return LoadELFFromFileWithPartitionNoReset(
        resolved, NULL, forwarded + (forward_argv[0] != NULL),
        forward_argv[0] ? forward_argv : forward_argv + 1);
}
