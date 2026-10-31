#include "boot_args.h"

#include <stdio.h>
#include <string.h>

#define CHECK(test) do { if (!(test)) { \
    fprintf(stderr, "Bootstrap arguments failed line %d: %s\n", \
            __LINE__, #test); return 1; \
} } while (0)

int main(void)
{
    static const char *const wle_hdd[] = {
        "pfs:/PS2DRIVERS/BOOT.ELF",
        "hdd0:__common:pfs:/PS2DRIVERS/BOOT.ELF", "-r"
    };
    static const char *const wle_explicit[] = {
        "pfs0:/PS2DRIVERS/BOOT.ELF",
        "hdd0:__common:pfs0:/PS2DRIVERS/BOOT.ELF", "-nr", "APP.ELF"
    };
    static const char *const usb[] = {"mass0:/BOOT.ELF"};
    static const char *const usb_explicit[] = {"mass0:/BOOT.ELF", "APP.ELF"};
    static const char *const usb_handoff[] = {
        "mass0:/TEST/BOOT.ELF", "mass0:/TEST/BOOT.ELF"
    };
    /* R3Z's embedded ELF loader strips its -la=ARH argument before
     * entering the target; some launchers still forward the handoff path. */
    static const char *const r3z_ata0[] = {
        "ata0:/NJEMU/MVS/BOOT.ELF"
    };
    static const char *const r3z_ata1[] = {
        "ata1:/NJEMU/CPS2/BOOT.ELF",
        "ata1:/NJEMU/CPS2/BOOT.ELF", "-la=AR"
    };
    static const char *const r3z_ata_alias[] = {
        "ata:/apps/BOOT.ELF", "-nr"
    };
    static const char *const r3z_ata_explicit[] = {
        "ata0:/NJEMU/MVS/BOOT.ELF", "MVS"
    };
    static const char *const host[] = {"host:BOOT.ELF", "APP.ELF"};
    static const char *const bad_hdd[] = {"hdd0:__common:pfs:bad"};
    char directory[128];
    char mass_directory[128];

    CHECK(ps2_irximg_boot_kind(3, wle_hdd, "hdd0:__common:pfs:PS2DRIVERS") ==
          PS2_IRXIMG_BOOT_HDD);
    CHECK(ps2_irximg_boot_kind(4, wle_explicit, "pfs0:/PS2DRIVERS/") ==
          PS2_IRXIMG_BOOT_HDD);
    CHECK(ps2_irximg_boot_kind(1, usb, "mass0:/PS2DRIVERS/") ==
          PS2_IRXIMG_BOOT_MASS);
    CHECK(ps2_irximg_boot_kind(2, usb_explicit, "mass:/") ==
          PS2_IRXIMG_BOOT_MASS);
    CHECK(ps2_irximg_boot_kind(2, host, "host:/") ==
          PS2_IRXIMG_BOOT_OTHER);
    CHECK(ps2_irximg_boot_kind(1, r3z_ata0, "host:/") ==
          PS2_IRXIMG_BOOT_ATA);
    CHECK(ps2_irximg_boot_kind(3, r3z_ata1, "ata1:/NJEMU/CPS2/") ==
          PS2_IRXIMG_BOOT_ATA);
    CHECK(ps2_irximg_boot_kind(0, NULL, "ata0:/") ==
          PS2_IRXIMG_BOOT_ATA);
    CHECK(ps2_irximg_boot_kind(0, NULL, "atad:/") ==
          PS2_IRXIMG_BOOT_OTHER);
    CHECK(ps2_irximg_boot_kind(1, bad_hdd, "pfs:/PS2DRIVERS/") ==
          PS2_IRXIMG_BOOT_UNKNOWN);
    CHECK(ps2_irximg_boot_kind(1, bad_hdd, "hdd0:unknown") ==
          PS2_IRXIMG_BOOT_UNKNOWN);
    CHECK(ps2_irximg_boot_kind(0, NULL, NULL) == PS2_IRXIMG_BOOT_UNKNOWN);
    CHECK(ps2_irximg_target_index(3, wle_hdd) == 3);
    CHECK(ps2_irximg_target_index(4, wle_explicit) == 3);
    CHECK(ps2_irximg_target_index(1, usb) == 1);
    CHECK(ps2_irximg_target_index(2, usb_explicit) == 1);
    CHECK(ps2_irximg_target_index(2, usb_handoff) == 2);
    CHECK(ps2_irximg_target_index(1, r3z_ata0) == 1);
    CHECK(ps2_irximg_target_index(3, r3z_ata1) == 3);
    CHECK(ps2_irximg_target_index(2, r3z_ata_alias) == 2);
    CHECK(ps2_irximg_target_index(2, r3z_ata_explicit) == 1);
    CHECK(ps2_irximg_ata_launch_directory(1, r3z_ata0, "bad-cwd",
                                          directory, sizeof(directory)) == 0);
    CHECK(!strcmp(directory, "ata0:/NJEMU/MVS/"));
    CHECK(ps2_irximg_ata_launch_directory(3, r3z_ata1, "",
                                          directory, sizeof(directory)) == 0);
    CHECK(!strcmp(directory, "ata1:/NJEMU/CPS2/"));
    CHECK(ps2_irximg_ata_mass_directory(directory, 2,
                                        mass_directory, sizeof(mass_directory)) == 0);
    CHECK(!strcmp(mass_directory, "mass2:/NJEMU/CPS2/"));
    CHECK(ps2_irximg_ata_launch_directory(0, NULL, "ata1:/existing/",
                                          directory, sizeof(directory)) == 0);
    CHECK(!strcmp(directory, "ata1:/existing/"));
    CHECK(ps2_irximg_ata_mass_directory("ata0:/app/", 0,
                                        directory, sizeof(directory)) == 0);
    CHECK(!strcmp(directory, "mass0:/app/"));
    CHECK(ps2_irximg_ata_mass_directory("ata0:/app/", 10,
                                        directory, sizeof(directory)) < 0);
    CHECK(ps2_irximg_ata_mass_directory("ata0:bad", 0,
                                        directory, sizeof(directory)) < 0);
    CHECK(ps2_irximg_ata_launch_directory(1, r3z_ata0, "bad",
                                          directory, 8) < 0);
    CHECK(ps2_irximg_target_index(2, host) == 1);
    CHECK(!strcmp(ps2_irximg_mass_name(PS2_IRXIMG_MASS_ATA), "ATA/BDM"));

    puts("Universal bootstrap argument and device detection tests passed");
    return 0;
}
