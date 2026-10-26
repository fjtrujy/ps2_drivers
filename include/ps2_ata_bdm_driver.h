#ifndef PS2_ATA_BDM_DRIVER_H
#define PS2_ATA_BDM_DRIVER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum ATA_BDM_INIT_STATUS {
    ATA_BDM_INIT_STATUS_DEPENDENCY_ERROR = -3,
    ATA_BDM_INIT_STATUS_IRX_ERROR = -2,
    ATA_BDM_INIT_STATUS_UNKNOWN = -1,
    ATA_BDM_INIT_STATUS_OK = 0,
};

/* Internal ATA FAT/exFAT, not APA/PFS. Load DEV9, BDM and BDMFS before
 * ata_bd; do not load ps2atad and ata_bd in the same IOP session. */
enum ATA_BDM_INIT_STATUS init_ata_bdm_driver(bool init_dependencies);
void deinit_ata_bdm_driver(bool deinit_dependencies);

#ifdef __cplusplus
}
#endif

#endif
