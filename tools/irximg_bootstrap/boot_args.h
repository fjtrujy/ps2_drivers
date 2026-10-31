#ifndef PS2_IRXIMG_BOOT_ARGS_H
#define PS2_IRXIMG_BOOT_ARGS_H

#include <stddef.h>

enum ps2_irximg_boot_kind {
    PS2_IRXIMG_BOOT_UNKNOWN = -1,
    PS2_IRXIMG_BOOT_OTHER = 0,
    PS2_IRXIMG_BOOT_MASS = 1,
    PS2_IRXIMG_BOOT_HDD = 2,
    PS2_IRXIMG_BOOT_ATA = 3
};

enum ps2_irximg_mass_transport {
    PS2_IRXIMG_MASS_UNKNOWN = 0,
    PS2_IRXIMG_MASS_USB,
    PS2_IRXIMG_MASS_MX4SIO,
    PS2_IRXIMG_MASS_ATA
};

enum ps2_irximg_boot_kind ps2_irximg_boot_kind(
    int argc, const char *const argv[], const char *cwd);
int ps2_irximg_target_index(int argc, const char *const argv[]);
const char *ps2_irximg_mass_name(enum ps2_irximg_mass_transport transport);

/* wLaunchELF-R3Z uses ataN: even though our ATA/BDM driver uses massN:.
 * Extract the source directory without inferring which mass slot it will
 * occupy after a reset. Return -1 for missing or invalid launch paths. */
int ps2_irximg_ata_launch_directory(int argc, const char *const argv[],
                                    const char *cwd, char *directory,
                                    size_t size);
int ps2_irximg_ata_mass_directory(const char *ata_directory,
                                  unsigned int mass_unit,
                                  char *directory, size_t size);

#endif
