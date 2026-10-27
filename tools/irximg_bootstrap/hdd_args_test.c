#include "hdd_args.h"

#include <stdio.h>

static int check(const char *name, int argc, const char *const argv[], int expected)
{
    int actual = ps2_irximg_hdd_target_index(argc, argv);
    if (actual != expected) {
        fprintf(stderr, "%s: target index %d (expected %d)\n", name, actual, expected);
        return 1;
    }
    return 0;
}

int main(void)
{
    static const char *const wle_reset[] = {
        "pfs0:/NJEMU/MVS/BOOT_HDD.ELF",
        "hdd0:__common:pfs:/NJEMU/MVS/BOOT_HDD.ELF",
        "-r"
    };
    static const char *const wle_no_reset[] = {
        "pfs0:/NJEMU/MVS/BOOT_HDD.ELF",
        "hdd0:__common:pfs:/NJEMU/MVS/BOOT_HDD.ELF",
        "-nr"
    };
    static const char *const explicit_target[] = {
        "pfs0:/NJEMU/MVS/BOOT_HDD.ELF",
        "hdd0:__common:pfs0:/NJEMU/MVS/BOOT_HDD.ELF",
        "-nr", "MVS", "--game"
    };
    static const char *const direct_target[] = {
        "host:BOOT_HDD.ELF", "MVS"
    };
    static const char *const no_args[] = {"pfs:/BOOT_HDD.ELF"};
    static const char *const missing_partition[] = {
        "pfs0:/BOOT_HDD.ELF", "-r"
    };

    if (check("wLE reset", 3, wle_reset, 3) ||
        check("wLE no-reset", 3, wle_no_reset, 3) ||
        check("wLE explicit target", 5, explicit_target, 3) ||
        check("direct explicit target", 2, direct_target, 1) ||
        check("no target", 1, no_args, 1) ||
        check("no partition", 2, missing_partition, 2))
        return 1;

    puts("HDD bootstrap wLaunchELF argument tests passed");
    return 0;
}
