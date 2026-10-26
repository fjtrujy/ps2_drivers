#ifndef PS2_HDD_LAUNCH_H
#define PS2_HDD_LAUNCH_H

#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PS2_HDD_LAUNCH_PARTITION_MAX 64

enum ps2_hdd_launch_status {
    PS2_HDD_LAUNCH_OK = 0,
    PS2_HDD_LAUNCH_ERR_ARGUMENT = -1,
    PS2_HDD_LAUNCH_ERR_DIRECTORY = -2,
    PS2_HDD_LAUNCH_ERR_PARTITION = -3,
    PS2_HDD_LAUNCH_ERR_DRIVERS = -4,
    PS2_HDD_LAUNCH_ERR_MOUNT = -5,
};

typedef struct ps2_hdd_launch_context {
    char partition[PS2_HDD_LAUNCH_PARTITION_MAX];
    char directory[FILENAME_MAX];
} ps2_hdd_launch_context_t;

/* Capture before an IOP reset. argv must contain an original launcher path
 * such as hdd0:__common:pfs:/apps/BOOT_HDD.ELF; pfs:/ alone is insufficient. */
int ps2_hdd_launch_capture(ps2_hdd_launch_context_t *context,
                           int argc, const char *const argv[],
                           const char *directory);
int ps2_hdd_launch_capture_current(ps2_hdd_launch_context_t *context,
                                   int argc, char *argv[]);

/* Build the handoff argument understood by capture() in the next ELF. */
int ps2_hdd_launch_format_argument(const ps2_hdd_launch_context_t *context,
                                   const char *path, char *result, size_t size);

/* After the caller resets/syncs the IOP and applies SBV patches, initialize
 * the APA/PFS stack, mount pfs:, and restore the preserved directory. */
int ps2_hdd_launch_restore(const ps2_hdd_launch_context_t *context);

#ifdef __cplusplus
}
#endif

#endif /* PS2_HDD_LAUNCH_H */
