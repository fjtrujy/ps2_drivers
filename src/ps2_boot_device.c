#include <string.h>

#include <ps2_boot_device.h>

#define DEVICE_SLASH "/"

#define DEVICE_MC0     "mc0:"
#define DEVICE_MC1     "mc1:"
#define DEVICE_CDROM   "cdrom0:"
#define DEVICE_CDFS    "cdfs:"
#define DEVICE_MASS    "mass:"
#define DEVICE_MASS0   "mass0:"
#define DEVICE_MASS1   "mass1:"
#define DEVICE_MX4SIO  "mx4sio:"
#define DEVICE_MX4SIO0 "mx4sio0:"
#define DEVICE_MX4SIO1 "mx4sio1:"
#define DEVICE_HDD     "hdd:"
#define DEVICE_HDD0    "hdd0:"
#define DEVICE_HOST    "host:"
#define DEVICE_HOST0   "host0:"
#define DEVICE_HOST1   "host1:"

#define DEVICE_MC0_PATH     DEVICE_MC0 DEVICE_SLASH
#define DEVICE_MC1_PATH     DEVICE_MC1 DEVICE_SLASH
#define DEVICE_CDFS_PATH    DEVICE_CDFS DEVICE_SLASH
#define DEVICE_CDROM_PATH   DEVICE_CDROM DEVICE_SLASH
#define DEVICE_MASS_PATH    DEVICE_MASS DEVICE_SLASH
#define DEVICE_MASS0_PATH   DEVICE_MASS0 DEVICE_SLASH
#define DEVICE_MASS1_PATH   DEVICE_MASS1 DEVICE_SLASH
#define DEVICE_MX4SIO_PATH  DEVICE_MX4SIO DEVICE_SLASH
#define DEVICE_MX4SIO0_PATH DEVICE_MX4SIO0 DEVICE_SLASH
#define DEVICE_MX4SIO1_PATH DEVICE_MX4SIO1 DEVICE_SLASH
#define DEVICE_HDD_PATH     DEVICE_HDD DEVICE_SLASH
#define DEVICE_HDD0_PATH    DEVICE_HDD0 DEVICE_SLASH
#define DEVICE_HOST_PATH    DEVICE_HOST DEVICE_SLASH
#define DEVICE_HOST0_PATH   DEVICE_HOST0 DEVICE_SLASH
#define DEVICE_HOST1_PATH   DEVICE_HOST1 DEVICE_SLASH

static int has_device_prefix(const char *path, const char *device)
{
    return path != NULL && strncmp(path, device, strlen(device)) == 0;
}

char *rootDevicePath(enum BootDeviceIDs device_id)
{
    switch (device_id) {
        case BOOT_DEVICE_MC0:
            return DEVICE_MC0_PATH;
        case BOOT_DEVICE_MC1:
            return DEVICE_MC1_PATH;
        case BOOT_DEVICE_CDROM:
            return DEVICE_CDROM_PATH;
        case BOOT_DEVICE_CDFS:
            return DEVICE_CDFS_PATH;
        case BOOT_DEVICE_MASS:
            return DEVICE_MASS_PATH;
        case BOOT_DEVICE_MASS0:
            return DEVICE_MASS0_PATH;
        case BOOT_DEVICE_MASS1:
            return DEVICE_MASS1_PATH;
        case BOOT_DEVICE_MX4SIO:
            return DEVICE_MX4SIO_PATH;
        case BOOT_DEVICE_MX4SIO0:
            return DEVICE_MX4SIO0_PATH;
        case BOOT_DEVICE_MX4SIO1:
            return DEVICE_MX4SIO1_PATH;
        case BOOT_DEVICE_HDD:
            return DEVICE_HDD_PATH;
        case BOOT_DEVICE_HDD0:
            return DEVICE_HDD0_PATH;
        case BOOT_DEVICE_HOST:
            return DEVICE_HOST_PATH;
        case BOOT_DEVICE_HOST0:
            return DEVICE_HOST0_PATH;
        case BOOT_DEVICE_HOST1:
            return DEVICE_HOST1_PATH;
        default:
            return "";
    }
}

enum BootDeviceIDs getBootDeviceID(const char *path)
{
    if (has_device_prefix(path, DEVICE_MC0))
        return BOOT_DEVICE_MC0;
    if (has_device_prefix(path, DEVICE_MC1))
        return BOOT_DEVICE_MC1;
    if (has_device_prefix(path, DEVICE_CDROM))
        return BOOT_DEVICE_CDROM;
    if (has_device_prefix(path, DEVICE_CDFS))
        return BOOT_DEVICE_CDFS;
    if (has_device_prefix(path, DEVICE_MASS))
        return BOOT_DEVICE_MASS;
    if (has_device_prefix(path, DEVICE_MASS0))
        return BOOT_DEVICE_MASS0;
    if (has_device_prefix(path, DEVICE_MASS1))
        return BOOT_DEVICE_MASS1;
    if (has_device_prefix(path, DEVICE_MX4SIO))
        return BOOT_DEVICE_MX4SIO;
    if (has_device_prefix(path, DEVICE_MX4SIO0))
        return BOOT_DEVICE_MX4SIO0;
    if (has_device_prefix(path, DEVICE_MX4SIO1))
        return BOOT_DEVICE_MX4SIO1;
    if (has_device_prefix(path, DEVICE_HDD))
        return BOOT_DEVICE_HDD;
    if (has_device_prefix(path, DEVICE_HDD0))
        return BOOT_DEVICE_HDD0;
    if (has_device_prefix(path, DEVICE_HOST))
        return BOOT_DEVICE_HOST;
    if (has_device_prefix(path, DEVICE_HOST0))
        return BOOT_DEVICE_HOST0;
    if (has_device_prefix(path, DEVICE_HOST1))
        return BOOT_DEVICE_HOST1;

    return BOOT_DEVICE_UNKNOWN;
}
