#include "boot_args.h"

#include <stdio.h>
#include <string.h>

static int is_hdd_launch(const char *arg)
{
    const char *partition;
    const char *pfs;

    if (arg == NULL)
        return 0;
    partition = strncmp(arg, "hdd0:", 5) == 0 ? arg + 5 :
                strncmp(arg, "hdd:", 4) == 0 ? arg + 4 : NULL;
    if (partition == NULL)
        return 0;
    pfs = strstr(partition, ":pfs:");
    if (pfs == NULL)
        pfs = strstr(partition, ":pfs0:");
    return pfs && partition != pfs &&
           (strncmp(pfs, ":pfs:/", 6) == 0 ||
            strncmp(pfs, ":pfs0:/", 7) == 0);
}

static int is_mass(const char *path)
{
    const char *p;

    if (path == NULL || strncmp(path, "mass", 4) != 0)
        return 0;
    p = path + 4;
    while (*p >= '0' && *p <= '9')
        ++p;
    return p[0] == ':' && p[1] == '/';
}

static const char *ata_path_suffix(const char *path)
{
    const char *cursor;

    if (path == NULL || strncmp(path, "ata", 3) != 0)
        return NULL;
    cursor = path + 3;
    if (*cursor >= '0' && *cursor <= '9')
        ++cursor;
    if (cursor[0] != ':' || cursor[1] != '/')
        return NULL;
    return cursor + 1;
}

static int is_bootstrap_self_path(const char *path)
{
    const char *name;

    if (path == NULL)
        return 0;
    name = strrchr(path, '/');
    name = name ? name + 1 : path;
    /* A launcher may forward the just-loaded BOOT.ELF path as metadata.
     * Never interpret it as a target or recursively load the bootstrap. */
    return strcmp(name, "BOOT.ELF") == 0 || strcmp(name, "boot.elf") == 0;
}

enum ps2_irximg_boot_kind ps2_irximg_boot_kind(
    int argc, const char *const argv[], const char *cwd)
{
    int i;

    if (argc < 0 || (argc > 0 && argv == NULL))
        return PS2_IRXIMG_BOOT_UNKNOWN;
    for (i = 0; i < argc; ++i) {
        if (is_hdd_launch(argv[i]))
            return PS2_IRXIMG_BOOT_HDD;
    }
    if (ata_path_suffix(cwd) != NULL)
        return PS2_IRXIMG_BOOT_ATA;
    for (i = 0; i < argc; ++i) {
        if (ata_path_suffix(argv[i]) != NULL)
            return PS2_IRXIMG_BOOT_ATA;
    }
    if (cwd != NULL && (strncmp(cwd, "pfs:/", 5) == 0 ||
                        strncmp(cwd, "pfs0:/", 6) == 0 ||
                        strncmp(cwd, "hdd0:", 5) == 0))
        return PS2_IRXIMG_BOOT_UNKNOWN;
    if (is_mass(cwd))
        return PS2_IRXIMG_BOOT_MASS;
    if (cwd && cwd[0] != '\0')
        return PS2_IRXIMG_BOOT_OTHER;
    return PS2_IRXIMG_BOOT_UNKNOWN;
}

int ps2_irximg_target_index(int argc, const char *const argv[])
{
    int i = 1;

    if (argc <= 1 || argv == NULL)
        return argc;
    while (i < argc && argv[i] != NULL &&
           (is_hdd_launch(argv[i]) || strcmp(argv[i], "-r") == 0 ||
            strcmp(argv[i], "-nr") == 0 ||
            strncmp(argv[i], "-la=", 4) == 0 ||
            (i == 1 && is_bootstrap_self_path(argv[i]))))
        ++i;
    return i;
}

int ps2_irximg_ata_launch_directory(int argc, const char *const argv[],
                                    const char *cwd, char *directory,
                                    size_t size)
{
    const char *path = NULL;
    const char *suffix;
    const char *end;
    size_t length;
    int from_argv = 0;
    int i;

    if (!directory || !size || argc < 0 || (argc && !argv))
        return -1;
    directory[0] = '\0';

    /* Prefer the launcher's absolute ELF path: an IOP reset can make
     * newlib's initial cwd unavailable or malformed. */
    for (i = 0; i < argc; ++i) {
        if (ata_path_suffix(argv[i]) && is_bootstrap_self_path(argv[i])) {
            path = argv[i];
            from_argv = 1;
            break;
        }
    }
    if (!path && ata_path_suffix(cwd))
        path = cwd;
    if (!path || !(suffix = ata_path_suffix(path)))
        return -1;

    end = path + strlen(path);
    if (from_argv) {
        end = strrchr(suffix, '/');
        if (!end)
            return -1;
        ++end;
    }
    length = (size_t)(end - path);
    if (length >= size)
        return -1;
    memcpy(directory, path, length);
    directory[length] = '\0';
    return 0;
}

int ps2_irximg_ata_mass_directory(const char *ata_directory,
                                  unsigned int mass_unit,
                                  char *directory, size_t size)
{
    const char *suffix = ata_path_suffix(ata_directory);
    int length;

    if (!suffix || !directory || !size || mass_unit > 9)
        return -1;
    length = snprintf(directory, size, "mass%u:%s", mass_unit, suffix);
    return length >= 0 && (size_t)length < size ? 0 : -1;
}

const char *ps2_irximg_mass_name(enum ps2_irximg_mass_transport transport)
{
    switch (transport) {
    case PS2_IRXIMG_MASS_USB: return "USB";
    case PS2_IRXIMG_MASS_MX4SIO: return "MX4SIO";
    case PS2_IRXIMG_MASS_ATA: return "ATA/BDM";
    default: return "unknown";
    }
}
