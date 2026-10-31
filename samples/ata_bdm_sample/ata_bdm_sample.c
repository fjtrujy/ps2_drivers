#define NEWLIB_PORT_AWARE 1

#include <stdio.h>
#include <unistd.h>

#include <fileXio_rpc.h>
#ifdef PS2_DRIVERS_R3Z_SMOKE
#include <debug.h>
#endif
#include <iopcontrol.h>
#include <kernel.h>
#include <sifrpc.h>
#include <sbv_patches.h>

#include <ps2_ata_bdm_driver.h>
#include <ps2_ata_bdm_launch.h>
#ifdef PS2_DRIVERS_IMG_FLAVOR
#include <ps2_drivers_img.h>
#endif

int main(int argc, char **argv)
{
    int result;
    int retries = 400;
    int directory = -1;
    ps2_ata_bdm_launch_context_t launch;
    int from_ata_bootstrap =
        ps2_ata_bdm_launch_capture_current(&launch, argc, argv) ==
        PS2_ATA_BDM_LAUNCH_OK;
    const char *probe_path = from_ata_bootstrap ? launch.directory : "mass0:/";

#ifdef PS2_DRIVERS_R3Z_SMOKE
    init_scr();
    scr_printf("PS2_DRIVERS R3Z ATA/exFAT smoke test\n");
    scr_printf("ATA marker received: %s\n",
               from_ata_bootstrap ? "yes" : "NO");
#endif

#ifdef PS2_DRIVERS_IMG_FLAVOR
    if (from_ata_bootstrap) {
        result = ps2_drivers_img_stage_default_for_ata_bdm(0);
        printf("[ata-bdm-test] staged external image result=%d modules=%u\n",
               result, (unsigned int)ps2_drivers_img_staged_modules());
        if (result != PS2_DRIVERS_IMG_OK)
            return 1;
    }
#endif

    SifInitRpc(0);
    while (!SifIopReset(NULL, 0)) {}
    while (!SifIopSync()) {}
    SifInitRpc(0);
    sbv_patch_enable_lmb();
    sbv_patch_disable_prefix_check();

    result = from_ata_bootstrap
        ? ps2_ata_bdm_launch_restore(&launch)
        : init_ata_bdm_driver(true);
    printf("[ata-bdm-test] driver restore=%d marked=%d\n",
           result, from_ata_bootstrap);
    if (result != 0)
        return 2;

#ifdef PS2_DRIVERS_IMG_FLAVOR
    if (from_ata_bootstrap) {
        printf("[ata-bdm-test] remaining staged modules=%u bytes=%u\n",
               (unsigned int)ps2_drivers_img_staged_modules(),
               (unsigned int)ps2_drivers_img_staged_bytes());
        if (ps2_drivers_img_staged_modules() != 0u ||
            ps2_drivers_img_staged_bytes() != 0u)
            return 4;
    }
#endif

    while (retries-- > 0) {
        directory = fileXioDopen(probe_path);
        if (directory >= 0)
            break;
        usleep(5000);
    }
    printf("[ata-bdm-test] %s directory fd=%d\n", probe_path, directory);
    if (directory < 0)
        return 3;

    fileXioDclose(directory);
    deinit_ata_bdm_driver(true);
    printf("[ata-bdm-test] success\n");
#ifdef PS2_DRIVERS_R3Z_SMOKE
    scr_printf("PASS: ATA/BDM driver restore and file access.\n");
    scr_printf("Test completed. Power off or reset the PS2.\n");
    for (;;)
        sleep(1);
#endif
    return 0;
}
