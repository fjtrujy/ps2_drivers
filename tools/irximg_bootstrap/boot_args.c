#include "boot_args.h"

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
            (i == 1 && is_bootstrap_self_path(argv[i]))))
        ++i;
    return i;
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
