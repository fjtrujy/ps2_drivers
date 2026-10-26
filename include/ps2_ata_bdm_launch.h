#ifndef PS2_ATA_BDM_LAUNCH_H
#define PS2_ATA_BDM_LAUNCH_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Explicit transport marker passed by BOOT_ATA.ELF to the application.
 * massN: is shared by USB, MX4SIO and ATA; it cannot identify ATA alone. */
#define PS2_ATA_BDM_LAUNCH_MARKER "--ps2-drivers-ata-bdm"

enum ps2_ata_bdm_launch_status {
    PS2_ATA_BDM_LAUNCH_OK = 0,
    PS2_ATA_BDM_LAUNCH_ERR_ARGUMENT = -1,
    PS2_ATA_BDM_LAUNCH_ERR_DIRECTORY = -2,
    PS2_ATA_BDM_LAUNCH_ERR_TRANSPORT = -3,
    PS2_ATA_BDM_LAUNCH_ERR_DRIVERS = -4,
};

typedef struct ps2_ata_bdm_launch_context {
    char directory[FILENAME_MAX];
} ps2_ata_bdm_launch_context_t;

/* Parse the marker from a known ATA bootstrap, not from a mass: path alone. */
int ps2_ata_bdm_launch_capture(ps2_ata_bdm_launch_context_t *context,
                               int argc, const char *const argv[],
                               const char *directory);
int ps2_ata_bdm_launch_capture_current(ps2_ata_bdm_launch_context_t *context,
                                       int argc, char *argv[]);

/* For applications which themselves explicitly select the internal ATA
 * transport, without needing a bootstrap-provided marker. */
int ps2_ata_bdm_launch_capture_explicit(ps2_ata_bdm_launch_context_t *context,
                                        const char *directory);

/* After an IOP reset, SBV patches and external IRX staging, recover massN:. */
int ps2_ata_bdm_launch_restore(const ps2_ata_bdm_launch_context_t *context);

#ifdef __cplusplus
}
#endif

#endif
