#include "hdd_args.h"

#include <string.h>

static int is_hdd_partition_path(const char *arg)
{
    return arg != NULL &&
           (strncmp(arg, "hdd0:", 5) == 0 || strncmp(arg, "hdd:", 4) == 0) &&
           (strstr(arg, ":pfs:") != NULL || strstr(arg, ":pfs0:") != NULL);
}

int ps2_irximg_hdd_target_index(int argc, const char *const argv[])
{
    int index = 1;

    if (argc <= 1 || argv == NULL)
        return argc;

    /* wLaunchELF passes argv[1] = hdd0:<partition>:pfs:/... and argv[2]
     * = -r or -nr. Neither is a target ELF, and both must be consumed before
     * reading the application's configured target from elf_path.ini. */
    while (index < argc && argv[index] != NULL &&
           (is_hdd_partition_path(argv[index]) ||
            strcmp(argv[index], "-r") == 0 ||
            strcmp(argv[index], "-nr") == 0))
        ++index;

    return index;
}
