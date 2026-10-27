#include <ps2_hdd_launch.h>

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "HDD launch test failed at line %d: %s\n", __LINE__, #expr); \
    return 1; \
} } while (0)

int main(void)
{
    static const char *valid[] = {
        "-f", "hdd0:__common:pfs:/NJEMU/MVS/BOOT.ELF"
    };
    static const char *alias[] = {
        "hdd:my_partition:pfs:/apps/boot.elf"
    };
    static const char *invalid[] = {
        "pfs:/apps/boot.elf", "hdd0::pfs:/boot.elf",
        "hdd0:__common:pfs:boot.elf", "hdd0:__common:garbage:/boot.elf"
    };
    ps2_hdd_launch_context_t context;
    char argument[FILENAME_MAX + PS2_HDD_LAUNCH_PARTITION_MAX + 2];
    char tiny[8];

    CHECK(ps2_hdd_launch_capture(&context, 2, valid,
          "pfs:/NJEMU/MVS/") == PS2_HDD_LAUNCH_OK);
    CHECK(strcmp(context.partition, "hdd0:__common") == 0);
    CHECK(strcmp(context.directory, "pfs:/NJEMU/MVS/") == 0);
    CHECK(ps2_hdd_launch_format_argument(&context, "pfs:/NJEMU/MVS/MVS",
          argument, sizeof(argument)) == PS2_HDD_LAUNCH_OK);
    CHECK(strcmp(argument, "hdd0:__common:pfs:/NJEMU/MVS/MVS") == 0);
    CHECK(ps2_hdd_launch_format_argument(&context, "pfs:/NJEMU/MVS/MVS",
          tiny, sizeof(tiny)) == PS2_HDD_LAUNCH_ERR_ARGUMENT);
    CHECK(ps2_hdd_launch_capture(&context, 1, alias,
          "pfs:/apps/") == PS2_HDD_LAUNCH_OK);
    CHECK(strcmp(context.partition, "hdd:my_partition") == 0);
    CHECK(ps2_hdd_launch_capture(&context, 1,
          (const char *const[]){"hdd0:__common:pfs0:/apps/BOOT.ELF"},
          "pfs0:/apps/") == PS2_HDD_LAUNCH_OK);
    CHECK(ps2_hdd_launch_format_argument(&context, "pfs0:/apps/MVS",
          argument, sizeof(argument)) == PS2_HDD_LAUNCH_OK);
    CHECK(strcmp(argument, "hdd0:__common:pfs0:/apps/MVS") == 0);
    CHECK(ps2_hdd_launch_capture(&context, 4, invalid,
          "pfs:/apps/") == PS2_HDD_LAUNCH_ERR_PARTITION);
    CHECK(context.partition[0] == '\0');
    CHECK(ps2_hdd_launch_capture(&context, 2, valid,
          "mass:/NJEMU/MVS/") == PS2_HDD_LAUNCH_OK);
    CHECK(strcmp(context.directory, "pfs:/NJEMU/MVS/") == 0);
    CHECK(ps2_hdd_launch_capture(&context, 2, valid,
          "hdd0:__common:pfs:NJEMUMVS") == PS2_HDD_LAUNCH_OK);
    CHECK(strcmp(context.partition, "hdd0:__common") == 0);
    CHECK(strcmp(context.directory, "pfs:/NJEMU/MVS/") == 0);
    CHECK(ps2_hdd_launch_capture(&context, 1,
          (const char *const[]){"hdd0:__common:pfs0:/apps/BOOT.ELF"},
          "") == PS2_HDD_LAUNCH_OK);
    CHECK(strcmp(context.directory, "pfs0:/apps/") == 0);
    CHECK(ps2_hdd_launch_capture(&context, 4, invalid,
          "mass:/other/") == PS2_HDD_LAUNCH_ERR_DIRECTORY);
    CHECK(ps2_hdd_launch_capture(&context, 0, NULL,
          "pfs:/apps/") == PS2_HDD_LAUNCH_ERR_PARTITION);
    CHECK(ps2_hdd_launch_capture(NULL, 2, valid,
          "pfs:/apps/") == PS2_HDD_LAUNCH_ERR_ARGUMENT);
    CHECK(ps2_hdd_launch_capture(&context, 1, NULL,
          "pfs:/apps/") == PS2_HDD_LAUNCH_ERR_ARGUMENT);

    puts("HDD launch context tests passed");
    return 0;
}
