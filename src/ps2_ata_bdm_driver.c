#include <stdint.h>

#include <ps2_ata_bdm_driver.h>
#include <ps2_bdm_driver.h>
#include <ps2_dev9_driver.h>
#include <ps2_fileXio_driver.h>
#include <irx_common_macros.h>

#include <sifrpc.h>
#include <loadfile.h>

EXTERN_PS2_IRX_MODULE(ata_bd);

#ifdef F_internals_ps2_ata_bdm_driver
enum ATA_BDM_INIT_STATUS __ata_bdm_init_status = ATA_BDM_INIT_STATUS_UNKNOWN;
DECL_IRX_VARS(ata_bd);
#else
extern enum ATA_BDM_INIT_STATUS __ata_bdm_init_status;
EXTERN_IRX_VARS(ata_bd);
#endif

#ifdef F_init_ps2_ata_bdm_driver
enum ATA_BDM_INIT_STATUS init_ata_bdm_driver(bool init_dependencies)
{
    if (init_dependencies) {
        if (init_fileXio_driver() != FILEXIO_INIT_STATUS_OK ||
            init_dev9_driver() != DEV9_INIT_STATUS_OK ||
            init_bdm_driver() != BDM_INIT_STATUS_OK) {
            __ata_bdm_init_status = ATA_BDM_INIT_STATUS_DEPENDENCY_ERROR;
            return __ata_bdm_init_status;
        }
    }

    /* ata_bd exports ATAD *and* attaches the ATA device to BDM. The BDM
     * filesystem driver must be resident before this module starts. */
    if (CHECK_IRX_LOAD(ata_bd)) {
        __ata_bd_id = ps2_irx_exec(&ps2_irx_ata_bd, 0, NULL, &__ata_bd_ret);
        if (CHECK_IRX_ERR(ata_bd)) {
            __ata_bdm_init_status = ATA_BDM_INIT_STATUS_IRX_ERROR;
            return __ata_bdm_init_status;
        }
    }

    __ata_bdm_init_status = ATA_BDM_INIT_STATUS_OK;
    return __ata_bdm_init_status;
}
#endif

#ifdef F_deinit_ps2_ata_bdm_driver
void deinit_ata_bdm_driver(bool deinit_dependencies)
{
    if (CHECK_IRX_UNLOAD(ata_bd)) {
        SifUnloadModule(__ata_bd_id);
        RESET_IRX_VARS(ata_bd);
    }

    if (deinit_dependencies) {
        deinit_bdm_driver();
        deinit_dev9_driver();
        deinit_fileXio_driver();
    }
    __ata_bdm_init_status = ATA_BDM_INIT_STATUS_UNKNOWN;
}
#endif
