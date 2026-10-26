#define NEWLIB_PORT_AWARE 1

#include <ps2_ata_bdm_launch.h>

#include <string.h>
#include <unistd.h>

#ifdef __PS2__
#include <kernel.h>
#include <ps2_ata_bdm_driver.h>
#include <ps2_bdm_driver.h>
#include <ps2_dev9_driver.h>
#include <ps2_fileXio_driver.h>
#include <ps2_poweroff_driver.h>
#endif

static int is_mass_directory(const char *path)
{
    const char *cursor;

    if (path == NULL || strncmp(path, "mass", 4) != 0)
        return 0;
    cursor = path + 4;
    while (*cursor >= '0' && *cursor <= '9')
        ++cursor;
    return cursor[0] == ':' && cursor[1] == '/';
}

int ps2_ata_bdm_launch_capture_explicit(ps2_ata_bdm_launch_context_t *context,
                                        const char *directory)
{
    size_t size;

    if (context == NULL)
        return PS2_ATA_BDM_LAUNCH_ERR_ARGUMENT;
    memset(context, 0, sizeof(*context));
    if (!is_mass_directory(directory))
        return PS2_ATA_BDM_LAUNCH_ERR_DIRECTORY;
    size = strlen(directory);
    if (size >= sizeof(context->directory))
        return PS2_ATA_BDM_LAUNCH_ERR_DIRECTORY;
    memcpy(context->directory, directory, size + 1);
    return PS2_ATA_BDM_LAUNCH_OK;
}

int ps2_ata_bdm_launch_capture(ps2_ata_bdm_launch_context_t *context,
                               int argc, const char *const argv[],
                               const char *directory)
{
    int i;

    if (context == NULL || argc < 0 || (argc > 0 && argv == NULL))
        return PS2_ATA_BDM_LAUNCH_ERR_ARGUMENT;
    memset(context, 0, sizeof(*context));
    for (i = 0; i < argc; ++i) {
        if (argv[i] != NULL &&
            strcmp(argv[i], PS2_ATA_BDM_LAUNCH_MARKER) == 0)
            return ps2_ata_bdm_launch_capture_explicit(context, directory);
    }
    return PS2_ATA_BDM_LAUNCH_ERR_TRANSPORT;
}

int ps2_ata_bdm_launch_capture_current(ps2_ata_bdm_launch_context_t *context,
                                       int argc, char *argv[])
{
    char cwd[FILENAME_MAX];

    if (getcwd(cwd, sizeof(cwd)) == NULL)
        return PS2_ATA_BDM_LAUNCH_ERR_DIRECTORY;
    return ps2_ata_bdm_launch_capture(context, argc,
                                      (const char *const *)argv, cwd);
}

#ifdef __PS2__
int ps2_ata_bdm_launch_restore(const ps2_ata_bdm_launch_context_t *context)
{
    int retries = 400;

    if (context == NULL || !is_mass_directory(context->directory))
        return PS2_ATA_BDM_LAUNCH_ERR_ARGUMENT;

    if (init_fileXio_driver() != FILEXIO_INIT_STATUS_OK ||
        init_poweroff_driver() != POWEROFF_INIT_STATUS_OK ||
        init_dev9_driver() != DEV9_INIT_STATUS_OK ||
        init_bdm_driver() != BDM_INIT_STATUS_OK ||
        init_ata_bdm_driver(false) != ATA_BDM_INIT_STATUS_OK)
        return PS2_ATA_BDM_LAUNCH_ERR_DRIVERS;

    /* BDM publishes the mass: mount asynchronously after ata_bd probes the
     * device. Preserve the exact massN: slot; never silently select USB. */
    while (retries-- > 0) {
        if (chdir(context->directory) == 0)
            return PS2_ATA_BDM_LAUNCH_OK;
        usleep(5000);
    }
    return PS2_ATA_BDM_LAUNCH_ERR_DIRECTORY;
}
#endif
