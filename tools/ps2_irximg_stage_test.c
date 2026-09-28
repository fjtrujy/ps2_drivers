#include <stdio.h>
#include <stdlib.h>

#include <ps2_drivers_img.h>

struct stage_case {
    const char *name;
    uint32_t requirements;
    size_t expected_modules;
};

int main(int argc, char **argv)
{
    static const struct stage_case cases[] = {
        {"joystick", PS2_DRIVER_REQ_JOYSTICK, 3u},
        {"audio", PS2_DRIVER_REQ_AUDIO, 2u},
        {"iopip", PS2_DRIVER_REQ_IOPIP, 5u},
        {"filesystem", PS2_DRIVER_REQ_FILESYSTEM_ALL, 16u},
    };
    size_t i;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <ps2_drivers.irximg>\n", argv[0]);
        return 2;
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
            return 20 + (int)i;
        }

        if (ps2_drivers_img_stage(argv[1], cases[i].requirements) !=
            PS2_DRIVERS_IMG_ERR_ALREADY_STAGED) {
            fprintf(stderr, "%s duplicate staging was not rejected\n", cases[i].name);
            return 30 + (int)i;
        }

        printf("%s: modules=%zu bytes=%zu\n",
               cases[i].name,
               ps2_drivers_img_staged_modules(),
               staged_bytes);

        ps2_drivers_img_discard_staged();
        if (ps2_drivers_img_staged_modules() != 0u ||
            ps2_drivers_img_staged_bytes() != 0u) {
            fprintf(stderr, "%s cleanup did not return to zero\n", cases[i].name);
            return 40 + (int)i;
        }
    }

    return 0;
}
