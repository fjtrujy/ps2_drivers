#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ps2_boot_device.h>
#include <ps2_drivers_img.h>
#include <ps2_irx_image_format.h>

#include "ps2_drivers_img_internal.h"

struct stage_case {
    const char *name;
    uint32_t requirements;
    size_t expected_modules;
};

struct boot_case {
    const char *path;
    enum BootDeviceIDs device;
    uint32_t expected_requirements;
};

static void consume_all_staged_modules(void)
{
    uint32_t id;

    for (id = 1u; id <= IRXIMG_MAX_ENTRIES; ++id)
        ps2_drivers_img_release_staged(id);
}

static int check_boot_helpers(void)
{
    static const struct boot_case cases[] = {
        {"mc0:/apps/test.elf", BOOT_DEVICE_MC0,
         PS2_DRIVER_REQ_FILEXIO | PS2_DRIVER_REQ_MEMCARD},
        {"mc1:/apps/test.elf", BOOT_DEVICE_MC1,
         PS2_DRIVER_REQ_FILEXIO | PS2_DRIVER_REQ_MEMCARD},
        {"cdrom0:/TEST.ELF;1", BOOT_DEVICE_CDROM,
         PS2_DRIVER_REQ_FILEXIO | PS2_DRIVER_REQ_CDFS},
        {"cdfs:/TEST.ELF", BOOT_DEVICE_CDFS,
         PS2_DRIVER_REQ_FILEXIO | PS2_DRIVER_REQ_CDFS},
        {"mass:/apps/test.elf", BOOT_DEVICE_MASS,
         PS2_DRIVER_REQ_FILEXIO | PS2_DRIVER_REQ_USB | PS2_DRIVER_REQ_MX4SIO},
        {"mass0:/apps/test.elf", BOOT_DEVICE_MASS0,
         PS2_DRIVER_REQ_FILEXIO | PS2_DRIVER_REQ_USB | PS2_DRIVER_REQ_MX4SIO},
        {"mass1:/apps/test.elf", BOOT_DEVICE_MASS1,
         PS2_DRIVER_REQ_FILEXIO | PS2_DRIVER_REQ_USB | PS2_DRIVER_REQ_MX4SIO},
        {"mx4sio:/apps/test.elf", BOOT_DEVICE_MX4SIO,
         PS2_DRIVER_REQ_FILEXIO | PS2_DRIVER_REQ_USB | PS2_DRIVER_REQ_MX4SIO},
        {"mx4sio0:/apps/test.elf", BOOT_DEVICE_MX4SIO0,
         PS2_DRIVER_REQ_FILEXIO | PS2_DRIVER_REQ_USB | PS2_DRIVER_REQ_MX4SIO},
        {"mx4sio1:/apps/test.elf", BOOT_DEVICE_MX4SIO1,
         PS2_DRIVER_REQ_FILEXIO | PS2_DRIVER_REQ_USB | PS2_DRIVER_REQ_MX4SIO},
        {"hdd:/apps/test.elf", BOOT_DEVICE_HDD,
         PS2_DRIVER_REQ_FILEXIO | PS2_DRIVER_REQ_POWEROFF | PS2_DRIVER_REQ_HDD},
        {"hdd0:/apps/test.elf", BOOT_DEVICE_HDD0,
         PS2_DRIVER_REQ_FILEXIO | PS2_DRIVER_REQ_POWEROFF | PS2_DRIVER_REQ_HDD},
        {"host:/apps/test.elf", BOOT_DEVICE_HOST, PS2_DRIVER_REQ_FILEXIO},
        {"host0:/apps/test.elf", BOOT_DEVICE_HOST0, PS2_DRIVER_REQ_FILEXIO},
        {"host1:/apps/test.elf", BOOT_DEVICE_HOST1, PS2_DRIVER_REQ_FILEXIO},
        {"/native/host/path/test.elf", BOOT_DEVICE_UNKNOWN, PS2_DRIVER_REQ_FILEXIO},
    };
    size_t i;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        uint32_t requirements = 0u;
        enum BootDeviceIDs device = getBootDeviceID(cases[i].path);
        int result;

        if (device != cases[i].device) {
            fprintf(stderr,
                    "boot path mismatch for %s: got=%d expected=%d\n",
                    cases[i].path,
                    (int)device,
                    (int)cases[i].device);
            return 1;
        }

        if (ps2_drivers_img_requirements_for_boot_device(device) !=
            cases[i].expected_requirements) {
            fprintf(stderr, "boot requirement mismatch for %s\n", cases[i].path);
            return 2;
        }

        result = ps2_drivers_img_requirements_for_path(
            cases[i].path,
            PS2_DRIVER_REQ_AUDIO,
            &requirements);
        if (result != PS2_DRIVERS_IMG_OK ||
            requirements != (cases[i].expected_requirements | PS2_DRIVER_REQ_AUDIO)) {
            fprintf(stderr,
                    "path requirement mismatch for %s: result=%d requirements=0x%08x\n",
                    cases[i].path,
                    result,
                    (unsigned int)requirements);
            return 3;
        }

        if (device != BOOT_DEVICE_UNKNOWN) {
            const char *root = rootDevicePath(device);

            if (root == NULL || root[0] == '\0' || getBootDeviceID(root) != device) {
                fprintf(stderr, "root path round-trip failed for device %d\n", (int)device);
                return 4;
            }
        }
    }

    return 0;
}

static int check_convenience_staging(void)
{
    int result;

    result = ps2_drivers_img_stage_default_for_boot_device(
        BOOT_DEVICE_HOST,
        PS2_DRIVER_REQ_AUDIO | PS2_DRIVER_REQ_JOYSTICK);
    if (result != PS2_DRIVERS_IMG_OK ||
        ps2_drivers_img_staged_modules() != 7u) {
        fprintf(stderr,
                "default boot-device staging failed: result=%d modules=%zu\n",
                result,
                ps2_drivers_img_staged_modules());
        return 1;
    }

    consume_all_staged_modules();
    if (ps2_drivers_img_staged_modules() != 0u ||
        ps2_drivers_img_staged_bytes() != 0u) {
        fprintf(stderr, "default helper execution-style release did not return to zero\n");
        return 2;
    }

    result = ps2_drivers_img_restage_last();
    if (result != PS2_DRIVERS_IMG_OK ||
        ps2_drivers_img_staged_modules() != 7u) {
        fprintf(stderr,
                "restage-last failed: result=%d modules=%zu\n",
                result,
                ps2_drivers_img_staged_modules());
        return 3;
    }
    ps2_drivers_img_discard_staged();

    result = ps2_drivers_img_stage_default_for_path(
        "mc0:/apps/test.elf",
        PS2_DRIVER_REQ_JOYSTICK);
    if (result != PS2_DRIVERS_IMG_OK ||
        ps2_drivers_img_staged_modules() != 7u) {
        fprintf(stderr,
                "default path staging failed: result=%d modules=%zu\n",
                result,
                ps2_drivers_img_staged_modules());
        return 4;
    }
    ps2_drivers_img_discard_staged();

    result = ps2_drivers_img_stage_default_for_current_boot(
        PS2_DRIVER_REQ_JOYSTICK);
    if (result != PS2_DRIVERS_IMG_OK ||
        ps2_drivers_img_staged_modules() != 5u) {
        fprintf(stderr,
                "default current-boot staging failed: result=%d modules=%zu\n",
                result,
                ps2_drivers_img_staged_modules());
        return 5;
    }
    ps2_drivers_img_discard_staged();

    result = ps2_drivers_img_stage_default_for_all_filesystems(
        PS2_DRIVER_REQ_AUDIO);
    if (result != PS2_DRIVERS_IMG_OK ||
        ps2_drivers_img_staged_modules() != 18u) {
        fprintf(stderr,
                "default all-filesystems staging failed: result=%d modules=%zu\n",
                result,
                ps2_drivers_img_staged_modules());
        return 6;
    }
    ps2_drivers_img_discard_staged();

    ps2_drivers_img_forget_source();
    if (ps2_drivers_img_restage_last() != PS2_DRIVERS_IMG_ERR_NO_SOURCE) {
        fprintf(stderr, "restage-last unexpectedly succeeded after forgetting source\n");
        return 7;
    }

    return 0;
}

int main(int argc, char **argv)
{
    static const struct stage_case cases[] = {
        {"sio2man", PS2_DRIVER_REQ_SIO2MAN, 1u},
        {"filexio", PS2_DRIVER_REQ_FILEXIO, 2u},
        {"memcard", PS2_DRIVER_REQ_MEMCARD, 3u},
        {"bdm", PS2_DRIVER_REQ_BDM, 2u},
        {"usbd", PS2_DRIVER_REQ_USBD, 1u},
        {"usb", PS2_DRIVER_REQ_USB, 4u},
        {"mx4sio", PS2_DRIVER_REQ_MX4SIO, 4u},
        {"cdfs", PS2_DRIVER_REQ_CDFS, 1u},
        {"dev9", PS2_DRIVER_REQ_DEV9, 1u},
        {"hdd", PS2_DRIVER_REQ_HDD, 8u},
        {"joystick", PS2_DRIVER_REQ_JOYSTICK, 3u},
        {"audio", PS2_DRIVER_REQ_AUDIO, 2u},
        {"poweroff", PS2_DRIVER_REQ_POWEROFF, 1u},
        {"mouse", PS2_DRIVER_REQ_MOUSE, 2u},
        {"keyboard", PS2_DRIVER_REQ_KEYBOARD, 2u},
        {"camera", PS2_DRIVER_REQ_CAMERA, 2u},
        {"netman", PS2_DRIVER_REQ_NETMAN, 1u},
        {"smap", PS2_DRIVER_REQ_SMAP, 1u},
        {"eeip", PS2_DRIVER_REQ_EEIP, 3u},
        {"iopip", PS2_DRIVER_REQ_IOPIP, 5u},
        {"filesystem", PS2_DRIVER_REQ_FILESYSTEM_ALL, 16u},
    };
    size_t i;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <ps2_drivers.irximg>\n", argv[0]);
        return 2;
    }

    if (strcmp(ps2_drivers_img_error_string(PS2_DRIVERS_IMG_ERR_OPEN),
               "failed to open IRX image") != 0 ||
        ps2_drivers_img_error_string(-999) == NULL) {
        fprintf(stderr, "error-string helper returned an unexpected result\n");
        return 3;
    }

    if (check_boot_helpers() != 0)
        return 4;

    if (ps2_drivers_img_restage(PS2_DRIVER_REQ_JOYSTICK) !=
            PS2_DRIVERS_IMG_ERR_NO_SOURCE ||
        ps2_drivers_img_restage_last() != PS2_DRIVERS_IMG_ERR_NO_SOURCE) {
        fprintf(stderr, "restage unexpectedly succeeded without a remembered source\n");
        return 5;
    }

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        size_t staged_bytes;
        int result = ps2_drivers_img_stage(argv[1], cases[i].requirements);

        if (result != PS2_DRIVERS_IMG_OK) {
            fprintf(stderr, "%s staging failed: %d\n", cases[i].name, result);
            return 10 + (int)i;
        }

        staged_bytes = ps2_drivers_img_staged_bytes();
        if (ps2_drivers_img_staged_modules() != cases[i].expected_modules ||
            staged_bytes == 0u) {
            fprintf(stderr,
                    "%s mismatch: modules=%zu bytes=%zu\n",
                    cases[i].name,
                    ps2_drivers_img_staged_modules(),
                    staged_bytes);
            return 40 + (int)i;
        }

        if (ps2_drivers_img_stage(argv[1], cases[i].requirements) !=
                PS2_DRIVERS_IMG_ERR_ALREADY_STAGED ||
            ps2_drivers_img_restage(cases[i].requirements) !=
                PS2_DRIVERS_IMG_ERR_ALREADY_STAGED ||
            ps2_drivers_img_restage_last() !=
                PS2_DRIVERS_IMG_ERR_ALREADY_STAGED) {
            fprintf(stderr, "%s duplicate staging was not rejected\n", cases[i].name);
            return 70 + (int)i;
        }

        printf("%s: modules=%zu bytes=%zu\n",
               cases[i].name,
               ps2_drivers_img_staged_modules(),
               staged_bytes);

        consume_all_staged_modules();
        if (ps2_drivers_img_staged_modules() != 0u ||
            ps2_drivers_img_staged_bytes() != 0u) {
            fprintf(stderr, "%s execution-style release did not return to zero\n", cases[i].name);
            return 100 + (int)i;
        }

        result = ps2_drivers_img_restage(cases[i].requirements);
        if (result != PS2_DRIVERS_IMG_OK ||
            ps2_drivers_img_staged_modules() != cases[i].expected_modules ||
            ps2_drivers_img_staged_bytes() != staged_bytes) {
            fprintf(stderr,
                    "%s restage mismatch: result=%d modules=%zu bytes=%zu\n",
                    cases[i].name,
                    result,
                    ps2_drivers_img_staged_modules(),
                    ps2_drivers_img_staged_bytes());
            return 130 + (int)i;
        }

        ps2_drivers_img_discard_staged();
    }

    if (ps2_drivers_img_stage(
            "__ps2_drivers_missing_image__.irximg",
            PS2_DRIVER_REQ_JOYSTICK) != PS2_DRIVERS_IMG_ERR_OPEN) {
        fprintf(stderr, "missing replacement image did not fail as expected\n");
        return 190;
    }

    if (ps2_drivers_img_restage(PS2_DRIVER_REQ_JOYSTICK) != PS2_DRIVERS_IMG_OK ||
        ps2_drivers_img_staged_modules() != 3u) {
        fprintf(stderr, "failed stage attempt replaced the remembered source\n");
        return 191;
    }
    ps2_drivers_img_discard_staged();

    ps2_drivers_img_forget_source();
    if (ps2_drivers_img_restage(PS2_DRIVER_REQ_JOYSTICK) !=
            PS2_DRIVERS_IMG_ERR_NO_SOURCE ||
        ps2_drivers_img_restage_last() !=
            PS2_DRIVERS_IMG_ERR_NO_SOURCE) {
        fprintf(stderr, "restage unexpectedly succeeded after forgetting the source\n");
        return 200;
    }

    if (check_convenience_staging() != 0)
        return 201;

    return 0;
}
