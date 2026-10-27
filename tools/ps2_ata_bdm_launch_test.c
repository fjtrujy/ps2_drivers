#include <stdio.h>
#include <string.h>

#include <ps2_ata_bdm_launch.h>

#define CHECK(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "ATA/BDM launch test failed at line %d: %s\n", __LINE__, #cond); \
        return 1; \
    } \
} while (0)

int main(void)
{
    ps2_ata_bdm_launch_context_t context;
    const char *args[] = { "mass0:/apps/BOOT.ELF",
                           PS2_ATA_BDM_LAUNCH_MARKER, "--game" };
    const char *unmarked[] = { "mass0:/apps/BOOT.ELF", "--game" };
    char oversized[FILENAME_MAX + 20];

    CHECK(ps2_ata_bdm_launch_capture(&context, 3, args, "mass0:/apps") ==
          PS2_ATA_BDM_LAUNCH_OK);
    CHECK(strcmp(context.directory, "mass0:/apps") == 0);
    CHECK(ps2_ata_bdm_launch_capture(&context, 2, unmarked, "mass0:/apps") ==
          PS2_ATA_BDM_LAUNCH_ERR_TRANSPORT);
    CHECK(ps2_ata_bdm_launch_capture(&context, 3, args, "host:/apps") ==
          PS2_ATA_BDM_LAUNCH_ERR_DIRECTORY);
    CHECK(ps2_ata_bdm_launch_capture(&context, 3, args, "massx:/apps") ==
          PS2_ATA_BDM_LAUNCH_ERR_DIRECTORY);
    CHECK(ps2_ata_bdm_launch_capture(&context, 3, args, "mass2:missing-slash") ==
          PS2_ATA_BDM_LAUNCH_ERR_DIRECTORY);
    CHECK(ps2_ata_bdm_launch_capture(&context, 1, NULL, "mass:/") ==
          PS2_ATA_BDM_LAUNCH_ERR_ARGUMENT);
    CHECK(ps2_ata_bdm_launch_capture(&context, 0, NULL, "mass:/") ==
          PS2_ATA_BDM_LAUNCH_ERR_TRANSPORT);
    CHECK(ps2_ata_bdm_launch_capture_explicit(&context, "mass12:/apps") ==
          PS2_ATA_BDM_LAUNCH_OK);
    CHECK(strcmp(context.directory, "mass12:/apps") == 0);
    CHECK(ps2_ata_bdm_launch_capture_explicit(&context, "mass:/") ==
          PS2_ATA_BDM_LAUNCH_OK);
    CHECK(ps2_ata_bdm_launch_capture_explicit(NULL, "mass:/") ==
          PS2_ATA_BDM_LAUNCH_ERR_ARGUMENT);
    memset(oversized, 'a', sizeof(oversized));
    memcpy(oversized, "mass:/", 6);
    oversized[sizeof(oversized) - 1] = '\0';
    CHECK(ps2_ata_bdm_launch_capture_explicit(&context, oversized) ==
          PS2_ATA_BDM_LAUNCH_ERR_DIRECTORY);

    puts("ATA/BDM launch context tests passed");
    return 0;
}
