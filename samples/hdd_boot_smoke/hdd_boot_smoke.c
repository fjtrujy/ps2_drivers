#define NEWLIB_PORT_AWARE 1

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <debug.h>
#include <fileXio_rpc.h>
#include <io_common.h>
#include <iopcontrol.h>
#include <sifrpc.h>
#include <sbv_patches.h>

#include <ps2_boot_device.h>
#include <ps2_drivers_img.h>
#include <ps2_hdd_launch.h>

#define PROBE_FILENAME "probe.txt"
#define PROBE_CONTENT "PS2_DRIVERS_HDD_SMOKE_V1\n"

static void report(const char *message)
{
    printf("[hdd-smoke] %s\n", message);
    scr_printf("%s\n", message);
}

static void stop_with_result(const char *step, int status)
{
    char message[192];

    snprintf(message, sizeof(message), "FAIL: %s (code %d)", step, status);
    report(message);
    report("See PCSX2 console or ps2link for detailed output.");
    for (;;)
        sleep(1);
}

static void check_probe(const ps2_hdd_launch_context_t *context,
                        const char *phase)
{
    char path[FILENAME_MAX + sizeof(PROBE_FILENAME) + 2];
    char data[64] __attribute__((aligned(64)));
    int fd;
    int count;
    int length;

    length = snprintf(path, sizeof(path), "%s%s%s", context->directory,
                      context->directory[strlen(context->directory) - 1] == '/'
                          ? "" : "/",
                      PROBE_FILENAME);
    if (length <= 0 || (size_t)length >= sizeof(path))
        stop_with_result("probe path too long", length);

    /* fileXio takes IOP flags, not newlib's O_RDONLY (which is zero). */
    fd = fileXioOpen(path, FIO_O_RDONLY, 0);
    if (fd < 0)
        stop_with_result("fileXioOpen(probe.txt)", fd);

    memset(data, 0, sizeof(data));
    count = fileXioRead(fd, data, sizeof(data));
    fileXioClose(fd);
    if (count != (int)sizeof(PROBE_CONTENT) - 1 ||
        memcmp(data, PROBE_CONTENT, sizeof(PROBE_CONTENT) - 1) != 0)
        stop_with_result("probe.txt contents", count);

    printf("[hdd-smoke] %s read OK: %s\n", phase, path);
    scr_printf("%s: probe.txt read OK\n", phase);
}

int main(int argc, char *argv[])
{
    ps2_hdd_launch_context_t context;
    char message[192];
    int status;
    size_t staged_modules;

    SifInitRpc(0);
    init_scr();
    report("PS2_DRIVERS HDD/PFS SMOKE TEST");
    report("No NJEMU, ROMs, or game assets required.");

    status = ps2_hdd_launch_capture_current(&context, argc, argv);
    if (status != PS2_HDD_LAUNCH_OK)
        stop_with_result("HDD launch context (wLaunchELF required)", status);

    snprintf(message, sizeof(message), "Partition: %s", context.partition);
    report(message);
    snprintf(message, sizeof(message), "Directory: %.155s", context.directory);
    report(message);

    check_probe(&context, "Before reset");

    status = ps2_drivers_img_stage_default_for_boot_device(BOOT_DEVICE_HDD0, 0);
    if (status != PS2_DRIVERS_IMG_OK) {
        printf("[hdd-smoke] IRX staging: %s\n",
               ps2_drivers_img_error_string(status));
        stop_with_result("stage ps2_drivers.irximg", status);
    }

    staged_modules = ps2_drivers_img_staged_modules();
    snprintf(message, sizeof(message), "Staged %u modules (%u bytes)",
             (unsigned int)staged_modules,
             (unsigned int)ps2_drivers_img_staged_bytes());
    report(message);
    if (staged_modules != 7u)
        stop_with_result("unexpected HDD staging module count",
                         (int)staged_modules);

    report("Resetting IOP...");
    while (!SifIopReset(NULL, 0)) {}
    while (!SifIopSync()) {}
    SifInitRpc(0);
    sbv_patch_enable_lmb();
    sbv_patch_disable_prefix_check();

    status = ps2_hdd_launch_restore(&context);
    if (status != PS2_HDD_LAUNCH_OK)
        stop_with_result("restore APA/PFS drivers and mount", status);
    report("IOP reset and HDD/PFS restore OK");

    if (ps2_drivers_img_staged_modules() != 0u ||
        ps2_drivers_img_staged_bytes() != 0u)
        stop_with_result("IRX staged payloads not fully consumed",
                         (int)ps2_drivers_img_staged_modules());

    check_probe(&context, "After reset");
    report("PASS: bootstrap, image staging, IOP reset,");
    report("HDD remount, and file read succeeded.");
    report("Test complete. Power off or reset the PS2.");
    for (;;)
        sleep(1);
}
