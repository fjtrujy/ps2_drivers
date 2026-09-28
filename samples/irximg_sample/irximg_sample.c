#include <stdio.h>

#include <iopcontrol.h>
#include <sifrpc.h>
#include <sbv_patches.h>

#ifdef PS2_DRIVERS_IMG_FLAVOR
#include <ps2_drivers_img.h>
#endif
#include <ps2_joystick_driver.h>

static void reset_iop(void)
{
    SifInitRpc(0);

    while (!SifIopReset(NULL, 0)) {
    }

    while (!SifIopSync()) {
    }

    SifInitRpc(0);
    sbv_patch_enable_lmb();
    sbv_patch_disable_prefix_check();
}

int main(int argc, char **argv)
{
    enum JOYSTICK_INIT_STATUS joystick_status;

#ifdef PS2_DRIVERS_IMG_FLAVOR
    int result;

    SifInitRpc(0);

    result = argc > 1
        ? ps2_drivers_img_stage(argv[1], PS2_DRIVER_REQ_JOYSTICK)
        : ps2_drivers_img_stage_default(PS2_DRIVER_REQ_JOYSTICK);
    if (result != PS2_DRIVERS_IMG_OK) {
        printf("ps2_drivers image staging failed: %s (%d)\n",
               ps2_drivers_img_error_string(result),
               result);
        return 1;
    }

    printf("staged %u modules (%u bytes) before IOP reset\n",
           (unsigned int)ps2_drivers_img_staged_modules(),
           (unsigned int)ps2_drivers_img_staged_bytes());
#else
    (void)argc;
    (void)argv;
#endif

    reset_iop();

    joystick_status = init_joystick_driver(true);
    printf("init_joystick_driver returned %d\n", joystick_status);

#ifdef PS2_DRIVERS_IMG_FLAVOR
    printf("remaining staged modules=%u bytes=%u\n",
           (unsigned int)ps2_drivers_img_staged_modules(),
           (unsigned int)ps2_drivers_img_staged_bytes());
#endif

    if (joystick_status != JOYSTICK_INIT_STATUS_OK)
        return 2;

#ifdef PS2_DRIVERS_IMG_FLAVOR
    if (ps2_drivers_img_staged_modules() != 0u ||
        ps2_drivers_img_staged_bytes() != 0u)
        return 3;

    /*
     * Exercise E6 explicitly. This sample expects the image beside the ELF and
     * therefore uses the same relative path again when restaging modules after
     * they have been unloaded.
     */
    deinit_joystick_driver(true);

    result = ps2_drivers_img_restage_last();
    if (result != PS2_DRIVERS_IMG_OK) {
        printf("ps2_drivers_img_restage_last failed: %s (%d)\n",
               ps2_drivers_img_error_string(result),
               result);
        return 4;
    }

    if (ps2_drivers_img_staged_modules() != 3u ||
        ps2_drivers_img_staged_bytes() == 0u)
        return 5;

    joystick_status = init_joystick_driver(true);
    printf("reinitialized joystick driver: %d\n", joystick_status);
    if (joystick_status != JOYSTICK_INIT_STATUS_OK)
        return 6;

    if (ps2_drivers_img_staged_modules() != 0u ||
        ps2_drivers_img_staged_bytes() != 0u)
        return 7;

    ps2_drivers_img_forget_source();
#endif

    return 0;
}
