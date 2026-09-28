#ifndef PS2_BOOT_DEVICE_H
#define PS2_BOOT_DEVICE_H

#ifdef __cplusplus
extern "C" {
#endif

enum BootDeviceIDs {
    BOOT_DEVICE_UNKNOWN = -1,
    BOOT_DEVICE_MC0 = 0,
    BOOT_DEVICE_MC1,
    BOOT_DEVICE_CDROM,
    BOOT_DEVICE_CDFS,
    BOOT_DEVICE_MASS,
    BOOT_DEVICE_MASS0,
    BOOT_DEVICE_MASS1,
    BOOT_DEVICE_MX4SIO,
    BOOT_DEVICE_MX4SIO0,
    BOOT_DEVICE_MX4SIO1,
    BOOT_DEVICE_HDD,
    BOOT_DEVICE_HDD0,
    BOOT_DEVICE_HOST,
    BOOT_DEVICE_HOST0,
    BOOT_DEVICE_HOST1,
    BOOT_DEVICE_COUNT,
};

char *rootDevicePath(enum BootDeviceIDs device_id);
enum BootDeviceIDs getBootDeviceID(const char *path);

#ifdef __cplusplus
}
#endif

#endif
