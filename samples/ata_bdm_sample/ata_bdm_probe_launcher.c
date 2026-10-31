#include <stdio.h>
#include <unistd.h>

#include <iopcontrol.h>
#include <sifrpc.h>
#include <sbv_patches.h>

#include <ps2_ata_bdm_driver.h>

int LoadELFFromFileWithPartitionNoReset(
    const char *filename, const char *partition, int argc, char *argv[]);

int main(void)
{
    int status;
#ifdef PS2_DRIVERS_TEST_R3Z_ATA_HANDOFF
    /* Simulate the arguments R3Z passes when loading BOOT.ELF from ata0:.
     * The bootstrap must translate this alias after its own IOP reset. */
    char *args[] = { "ata0:/BOOT.ELF" };
#endif

    SifInitRpc(0);
    while (!SifIopReset(NULL, 0)) {}
    while (!SifIopSync()) {}
    SifInitRpc(0);
    sbv_patch_enable_lmb();
    sbv_patch_disable_prefix_check();

    status = init_ata_bdm_driver(true);
    printf("[ata-launcher-test] init ATA/BDM=%d\n", status);
    if (status != ATA_BDM_INIT_STATUS_OK)
        return 1;

    for (int retry = 0; retry < 400; ++retry) {
        if (chdir("mass0:/") == 0)
            break;
        usleep(5000);
    }

    printf("[ata-launcher-test] entering universal BOOT.ELF\n");
    return LoadELFFromFileWithPartitionNoReset(
        "mass0:/BOOT.ELF", NULL,
#ifdef PS2_DRIVERS_TEST_R3Z_ATA_HANDOFF
        1, args
#else
        0, NULL
#endif
    );
}
