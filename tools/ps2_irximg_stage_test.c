#include <stdio.h>
#include <stdlib.h>

#include <ps2_drivers_img.h>
#include <ps2_irx_image_format.h>

#include "ps2_drivers_img_internal.h"

struct stage_case {
    const char *name;
    uint32_t requirements;
    size_t expected_modules;
};

static void consume_all_staged_modules(void)
{
    uint32_t id;

    for (id = 1u; id <= IRXIMG_MAX_ENTRIES; ++id)
        ps2_drivers_img_release_staged(id);
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

    if (ps2_drivers_img_restage(PS2_DRIVER_REQ_JOYSTICK) !=
        PS2_DRIVERS_IMG_ERR_NO_SOURCE) {
        fprintf(stderr, "restage unexpectedly succeeded without a remembered source\n");
        return 3;
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
        PS2_DRIVERS_IMG_ERR_NO_SOURCE) {
        fprintf(stderr, "restage unexpectedly succeeded after forgetting the source\n");
        return 200;
    }

    return 0;
}
