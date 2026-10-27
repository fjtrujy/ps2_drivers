#ifndef PS2_IRXIMG_BOOT_ARGS_H
#define PS2_IRXIMG_BOOT_ARGS_H

enum ps2_irximg_boot_kind {
    PS2_IRXIMG_BOOT_UNKNOWN = -1,
    PS2_IRXIMG_BOOT_OTHER = 0,
    PS2_IRXIMG_BOOT_MASS = 1,
    PS2_IRXIMG_BOOT_HDD = 2
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

#endif
