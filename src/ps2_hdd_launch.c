#define NEWLIB_PORT_AWARE 1

#include <ps2_hdd_launch.h>

#include <string.h>
#include <unistd.h>

#ifdef __PS2__
#include <ps2_dev9_driver.h>
#include <ps2_fileXio_driver.h>
#include <ps2_hdd_driver.h>
#include <ps2_poweroff_driver.h>
#include <fileXio_rpc.h>
#endif

static int is_pfs_directory(const char *path)
{
    return path != NULL &&
           ((strncmp(path, "pfs:/", 5) == 0) ||
            (strncmp(path, "pfs0:/", 6) == 0));
}

int ps2_hdd_launch_capture(ps2_hdd_launch_context_t *context,
                           int argc, const char *const argv[],
                           const char *directory)
{
    int i;
    size_t length;

    if (context == NULL || argc < 0 || (argc > 0 && argv == NULL))
        return PS2_HDD_LAUNCH_ERR_ARGUMENT;

    memset(context, 0, sizeof(*context));
    if (is_pfs_directory(directory) &&
        strlen(directory) >= sizeof(context->directory))
        return PS2_HDD_LAUNCH_ERR_DIRECTORY;

    for (i = 0; i < argc; ++i) {
        const char *arg = argv[i];
        const char *device_end;
        const char *separator;
        size_t partition_length;
        size_t separator_length = 5;

        if (arg == NULL)
            continue;
        if (strncmp(arg, "hdd0:", 5) == 0)
            device_end = arg + 5;
        else if (strncmp(arg, "hdd:", 4) == 0)
            device_end = arg + 4;
        else
            continue;

        separator = strstr(device_end, ":pfs:");
        if (separator == NULL) {
            separator = strstr(device_end, ":pfs0:");
            separator_length = 6;
        }
        if (separator == NULL || separator[separator_length] != '/' ||
            separator == device_end ||
            memchr(device_end, ':', (size_t)(separator - device_end)) != NULL)
            continue;

        partition_length = (size_t)(separator - arg);
        if (partition_length >= sizeof(context->partition))
            continue;

        if (is_pfs_directory(directory)) {
            length = strlen(directory);
            memcpy(context->directory, directory, length + 1);
        } else {
            /* wLaunchELF can pass an hdd0:<APA>:pfs:/... argv path while
             * newlib's initial cwd is an unusable hdd0:... string. Recover
             * the mounted directory from the qualified launch path itself. */
            const char *pfs_path = separator + 1;
            const char *last_slash = strrchr(pfs_path, '/');

            if (last_slash == NULL)
                continue;
            length = (size_t)(last_slash - pfs_path) + 1;
            if (length >= sizeof(context->directory))
                continue;
            memcpy(context->directory, pfs_path, length);
            context->directory[length] = '\0';
        }
        memcpy(context->partition, arg, partition_length);
        context->partition[partition_length] = '\0';
        return PS2_HDD_LAUNCH_OK;
    }

    return is_pfs_directory(directory) ?
        PS2_HDD_LAUNCH_ERR_PARTITION : PS2_HDD_LAUNCH_ERR_DIRECTORY;
}

int ps2_hdd_launch_capture_current(ps2_hdd_launch_context_t *context,
                                   int argc, char *argv[])
{
    char cwd[FILENAME_MAX];

    if (getcwd(cwd, sizeof(cwd)) == NULL)
        cwd[0] = '\0';
    return ps2_hdd_launch_capture(context, argc,
                                  (const char *const *)argv, cwd);
}

int ps2_hdd_launch_format_argument(const ps2_hdd_launch_context_t *context,
                                   const char *path, char *result, size_t size)
{
    int written;

    if (context == NULL || context->partition[0] == '\0' ||
        path == NULL || !is_pfs_directory(path) || result == NULL || size == 0)
        return PS2_HDD_LAUNCH_ERR_ARGUMENT;

    written = snprintf(result, size, "%s:%s", context->partition, path);
    if (written < 0 || (size_t)written >= size)
        return PS2_HDD_LAUNCH_ERR_ARGUMENT;
    return PS2_HDD_LAUNCH_OK;
}

#ifdef __PS2__
int ps2_hdd_launch_restore(const ps2_hdd_launch_context_t *context)
{
    const char *mountpoint;

    if (context == NULL || context->partition[0] == '\0' ||
        !is_pfs_directory(context->directory))
        return PS2_HDD_LAUNCH_ERR_ARGUMENT;

    if (init_fileXio_driver() < 0 || init_poweroff_driver() < 0 ||
        init_dev9_driver() < 0 ||
        init_hdd_driver(false, false) != HDD_INIT_STATUS_IRX_OK)
        return PS2_HDD_LAUNCH_ERR_DRIVERS;

    mountpoint = strncmp(context->directory, "pfs0:/", 6) == 0 ? "pfs0:" : "pfs:";
    if (mount_hdd_partition(mountpoint, context->partition) != HDD_MOUNT_STATUS_OK)
        return PS2_HDD_LAUNCH_ERR_MOUNT;
    if (chdir(context->directory) != 0) {
        umount_hdd_partition(mountpoint);
        return PS2_HDD_LAUNCH_ERR_DIRECTORY;
    }
    return PS2_HDD_LAUNCH_OK;
}
#endif
