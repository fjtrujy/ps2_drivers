#define NEWLIB_PORT_AWARE 1

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <elf-loader.h>
#include <fileio.h>
#include <kernel.h>
#include <loadfile.h>
#include <sifrpc.h>
#include <sbv_patches.h>

/* Some installed PS2SDK headers predate the exported no-reset entry point,
 * while libelf-loader.a already provides it. Keep this declaration here until
 * those installations catch up. */
int LoadELFFromFileWithPartitionNoReset(
    const char *filename, const char *partition, int argc, char *argv[]);

#ifdef PS2_IRXIMG_BOOTSTRAP_HDD
#include <iopcontrol.h>
#include <debug.h>
#include <fileXio_rpc.h>
#include <iox_stat.h>
#include <ps2_hdd_launch.h>
#else

extern unsigned char iomanX_irx[] __attribute__((aligned(16)));
extern unsigned int size_iomanX_irx;
extern unsigned char fileXio_irx[] __attribute__((aligned(16)));
extern unsigned int size_fileXio_irx;
extern unsigned char bdm_irx[] __attribute__((aligned(16)));
extern unsigned int size_bdm_irx;
extern unsigned char bdmfs_fatfs_irx[] __attribute__((aligned(16)));
extern unsigned int size_bdmfs_fatfs_irx;
extern unsigned char usbd_irx[] __attribute__((aligned(16)));
extern unsigned int size_usbd_irx;
extern unsigned char usbmass_bd_irx[] __attribute__((aligned(16)));
extern unsigned int size_usbmass_bd_irx;

int fileXioInit(void);

static int exec_irx(const char *name, unsigned char *data, unsigned int size)
{
    int module_result = -1;
    int module_id = SifExecModuleBuffer(data, size, 0, NULL, &module_result);

    if (module_id >= 0 && module_result != 1)
        return 0;

    printf("[irximg-bootstrap] %s failed (id=%d result=%d)\n",
        name, module_id, module_result);
    return -1;
}

static int wait_for_path(const char *path)
{
    struct stat buffer;
    int retries = 500;

    while (retries-- > 0) {
        if (stat(path, &buffer) == 0)
            return 0;
        nopdelay();
    }

    return -1;
}

static int restore_mass(const char *path)
{
    io_stat_t buffer;

    /* Launchers such as wLaunchELF keep their mass: filesystem alive while
     * handing off to BOOT.ELF. Reuse that stack instead of trying to register
     * a second iomanX/fileXio instance. The real application will stage its
     * IRX image and perform the definitive IOP reset shortly afterwards. */
    fioInit();
    if (fioGetstat(path, &buffer) == 0) {
        printf("[irximg-bootstrap] launch filesystem already available\n");
        return 0;
    }

    printf("[irximg-bootstrap] launch filesystem unavailable; restoring mass:\n");

    sceSifInitRpc(0);
    sbv_patch_enable_lmb();
    sbv_patch_disable_prefix_check();

    if (exec_irx("iomanX", iomanX_irx, size_iomanX_irx) < 0 ||
        exec_irx("fileXio", fileXio_irx, size_fileXio_irx) < 0)
        return -1;

    if (fileXioInit() < 0)
        return -1;

    if (exec_irx("bdm", bdm_irx, size_bdm_irx) < 0 ||
        exec_irx("bdmfs_fatfs", bdmfs_fatfs_irx, size_bdmfs_fatfs_irx) < 0 ||
        exec_irx("usbd", usbd_irx, size_usbd_irx) < 0 ||
        exec_irx("usbmass_bd", usbmass_bd_irx, size_usbmass_bd_irx) < 0)
        return -1;

    return wait_for_path(path);
}
#endif

#ifdef PS2_IRXIMG_BOOTSTRAP_HDD
/* The SDK ELF loader uses newlib stat(), while the recovered PFS mount is
 * registered through fileXio. Only adapt the PFS path used by this loader. */
int __real_stat(const char *path, struct stat *buffer);
int __wrap_stat(const char *path, struct stat *buffer)
{
    iox_stat_t fs_stat;

    if (strncmp(path, "pfs:", 4) != 0 && strncmp(path, "pfs0:", 5) != 0)
        return __real_stat(path, buffer);
    if (fileXioGetStat(path, &fs_stat) < 0) {
        errno = ENOENT;
        return -1;
    }
    memset(buffer, 0, sizeof(*buffer));
    buffer->st_mode = S_IFREG | 0444;
    buffer->st_size = (off_t)fs_stat.size;
    return 0;
}

static int restore_hdd(const ps2_hdd_launch_context_t *context)
{
    int status;

    sceSifInitRpc(0);
    while (!SifIopReset(NULL, 0)) {}
    while (!SifIopSync()) {}
    sceSifInitRpc(0);
    sbv_patch_enable_lmb();
    sbv_patch_disable_prefix_check();
    sbv_patch_fileio();

    status = ps2_hdd_launch_restore(context);
    if (status != PS2_HDD_LAUNCH_OK)
        printf("[irximg-bootstrap] HDD/PFS restore failed (%d)\n", status);
    return status;
}
#endif

static void trim_line(char *line)
{
    size_t length = strlen(line);
    while (length > 0 && (line[length - 1] == '\n' || line[length - 1] == '\r'))
        line[--length] = '\0';
}

static int read_default_target(char *path, size_t size)
{
    FILE *file = fopen("elf_path.ini", "r");

    if (file == NULL)
        return -1;
    if (fgets(path, size, file) == NULL) {
        fclose(file);
        return -1;
    }
    fclose(file);
    trim_line(path);
    return path[0] != '\0' ? 0 : -1;
}

static int resolve_target_path(
    const char *cwd, const char *target, char *resolved, size_t size)
{
    int length;

    if (strchr(target, ':') != NULL) {
        length = snprintf(resolved, size, "%s", target);
    } else {
        size_t cwd_length = strlen(cwd);
        const char *separator =
            cwd_length > 0 && cwd[cwd_length - 1] == '/' ? "" : "/";
        length = snprintf(resolved, size, "%s%s%s", cwd, separator, target);
    }

    return length >= 0 && (size_t)length < size ? 0 : -1;
}

#ifdef PS2_IRXIMG_BOOTSTRAP_HDD
static int is_hdd_launch_argument(const char *arg)
{
    return arg != NULL &&
           (strncmp(arg, "hdd0:", 5) == 0 || strncmp(arg, "hdd:", 4) == 0) &&
           (strstr(arg, ":pfs:") != NULL || strstr(arg, ":pfs0:") != NULL);
}
#endif

int main(int argc, char **argv)
{
    char cwd[FILENAME_MAX];
    char configured_target[PATH_MAX];
    char resolved_target[PATH_MAX];
    const char *target;
    int target_argc;
    char **target_argv;
#ifdef PS2_IRXIMG_BOOTSTRAP_HDD
    ps2_hdd_launch_context_t hdd_context;
    char launch_arg[PATH_MAX + PS2_HDD_LAUNCH_PARTITION_MAX + 2];
    char **hdd_argv;
    int i;
    int target_index = 1;
#endif

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        printf("[irximg-bootstrap] getcwd failed\n");
        return -EIO;
    }

    printf("[irximg-bootstrap] restoring launch filesystem at %s\n", cwd);
#ifdef PS2_IRXIMG_BOOTSTRAP_HDD
    init_scr();
    if (ps2_hdd_launch_capture(&hdd_context, argc,
            (const char *const *)argv, cwd) != PS2_HDD_LAUNCH_OK) {
        scr_printf("[irximg-bootstrap] missing/invalid HDD partition launch argument\n");
        return -EINVAL;
    }
    if (restore_hdd(&hdd_context) != PS2_HDD_LAUNCH_OK)
        return -EIO;
#else
    if (restore_mass(cwd) < 0) {
        printf("[irximg-bootstrap] failed to restore mass filesystem\n");
        return -EIO;
    }
#endif

#ifdef PS2_IRXIMG_BOOTSTRAP_HDD
    /* Some HDD ELF browsers pass the original hdd0:<APA>:pfs:/... path in
     * argv[1]. It is launch context, not an explicit application target. */
    if (target_index < argc && is_hdd_launch_argument(argv[target_index]))
        ++target_index;
    if (target_index < argc && argv[target_index] != NULL &&
        argv[target_index][0] != '\0') {
        target = argv[target_index];
        target_argc = argc - target_index - 1;
        target_argv = argv + target_index + 1;
#else
    if (argc >= 2 && argv[1][0] != '\0') {
        target = argv[1];
        target_argc = argc - 2;
        target_argv = argv + 2;
#endif
    } else {
        if (read_default_target(configured_target, sizeof(configured_target)) < 0) {
            printf("[irximg-bootstrap] expected target ELF in argv[1] or elf_path.ini\n");
            return -EINVAL;
        }
        target = configured_target;
        target_argc = 0;
        target_argv = NULL;
    }

    if (resolve_target_path(cwd, target, resolved_target, sizeof(resolved_target)) < 0) {
        printf("[irximg-bootstrap] target path is too long: %s\n", target);
        return -ENAMETOOLONG;
    }

    printf("[irximg-bootstrap] loading %s without IOP reset\n", resolved_target);
#ifdef PS2_IRXIMG_BOOTSTRAP_HDD
    if (ps2_hdd_launch_format_argument(&hdd_context, resolved_target,
            launch_arg, sizeof(launch_arg)) != PS2_HDD_LAUNCH_OK)
        return -EINVAL;

    hdd_argv = (char **)calloc((size_t)target_argc + 1, sizeof(*hdd_argv));
    if (hdd_argv == NULL)
        return -ENOMEM;
    hdd_argv[0] = launch_arg;
    for (i = 0; i < target_argc; ++i)
        hdd_argv[i + 1] = target_argv[i];
    return LoadELFFromFileWithPartitionNoReset(
        resolved_target, NULL, target_argc + 1, hdd_argv);
#else
    return LoadELFFromFileWithPartitionNoReset(
        resolved_target, NULL, target_argc, target_argv);
#endif
}
