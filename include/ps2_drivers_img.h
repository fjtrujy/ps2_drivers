#ifndef PS2_DRIVERS_IMG_H
#define PS2_DRIVERS_IMG_H

#include <stddef.h>
#include <stdint.h>

#include <ps2_boot_device.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PS2_DRIVERS_IMG_DEFAULT_PATH "ps2_drivers.irximg"

enum ps2_driver_requirement {
    PS2_DRIVER_REQ_SIO2MAN  = 1u << 0,
    PS2_DRIVER_REQ_FILEXIO  = 1u << 1,
    PS2_DRIVER_REQ_MEMCARD  = 1u << 2,
    PS2_DRIVER_REQ_BDM      = 1u << 3,
    PS2_DRIVER_REQ_USBD     = 1u << 4,
    PS2_DRIVER_REQ_USB      = 1u << 5,
    PS2_DRIVER_REQ_MX4SIO   = 1u << 6,
    PS2_DRIVER_REQ_CDFS     = 1u << 7,
    PS2_DRIVER_REQ_DEV9     = 1u << 8,
    PS2_DRIVER_REQ_HDD      = 1u << 9,
    PS2_DRIVER_REQ_JOYSTICK = 1u << 10,
    PS2_DRIVER_REQ_AUDIO    = 1u << 11,
    PS2_DRIVER_REQ_POWEROFF = 1u << 12,
    PS2_DRIVER_REQ_MOUSE    = 1u << 13,
    PS2_DRIVER_REQ_KEYBOARD = 1u << 14,
    PS2_DRIVER_REQ_CAMERA   = 1u << 15,
    PS2_DRIVER_REQ_NETMAN   = 1u << 16,
    PS2_DRIVER_REQ_SMAP     = 1u << 17,
    PS2_DRIVER_REQ_EEIP     = 1u << 18,
    PS2_DRIVER_REQ_IOPIP    = 1u << 19,
};

#define PS2_DRIVER_REQ_FILESYSTEM_ALL \
    (PS2_DRIVER_REQ_POWEROFF | PS2_DRIVER_REQ_FILEXIO | PS2_DRIVER_REQ_MEMCARD | \
     PS2_DRIVER_REQ_USB | PS2_DRIVER_REQ_MX4SIO | PS2_DRIVER_REQ_CDFS | \
     PS2_DRIVER_REQ_DEV9 | PS2_DRIVER_REQ_HDD)

enum ps2_drivers_img_error {
    PS2_DRIVERS_IMG_OK = 0,
    PS2_DRIVERS_IMG_ERR_ARGUMENT = -1,
    PS2_DRIVERS_IMG_ERR_ALREADY_STAGED = -2,
    PS2_DRIVERS_IMG_ERR_OPEN = -3,
    PS2_DRIVERS_IMG_ERR_IO = -4,
    PS2_DRIVERS_IMG_ERR_FORMAT = -5,
    PS2_DRIVERS_IMG_ERR_MISSING_MODULE = -6,
    PS2_DRIVERS_IMG_ERR_CRC = -7,
    PS2_DRIVERS_IMG_ERR_MEMORY = -8,
    PS2_DRIVERS_IMG_ERR_NO_SOURCE = -9,
    PS2_DRIVERS_IMG_ERR_CWD = -10,
};

const char *ps2_drivers_img_error_string(int error);

uint32_t ps2_drivers_img_requirements_for_boot_device(enum BootDeviceIDs boot_device);
int ps2_drivers_img_requirements_for_path(
    const char *path,
    uint32_t additional_requirements,
    uint32_t *requirements);
int ps2_drivers_img_requirements_for_current_boot(
    uint32_t additional_requirements,
    uint32_t *requirements);

/*
 * Low-level staging API. Use the convenience helpers below when possible.
 */
int ps2_drivers_img_stage(const char *image_path, uint32_t driver_requirements);

/* Stage an explicit requirement set from PS2_DRIVERS_IMG_DEFAULT_PATH. */
int ps2_drivers_img_stage_default(uint32_t driver_requirements);

/*
 * Stage the drivers required by init_only_boot_ps2_filesystem_driver() for a
 * known launch device/path/current working directory, plus application-specific
 * requirements such as AUDIO or JOYSTICK.
 */
int ps2_drivers_img_stage_for_boot_device(
    const char *image_path,
    enum BootDeviceIDs boot_device,
    uint32_t additional_requirements);
int ps2_drivers_img_stage_for_path(
    const char *image_path,
    const char *path,
    uint32_t additional_requirements);
int ps2_drivers_img_stage_for_current_boot(
    const char *image_path,
    uint32_t additional_requirements);

/*
 * Stage everything required by init_ps2_filesystem_driver(), plus any
 * application-specific requirements.
 */
int ps2_drivers_img_stage_for_all_filesystems(
    const char *image_path,
    uint32_t additional_requirements);
int ps2_drivers_img_stage_default_for_boot_device(
    enum BootDeviceIDs boot_device,
    uint32_t additional_requirements);
int ps2_drivers_img_stage_default_for_path(
    const char *path,
    uint32_t additional_requirements);
int ps2_drivers_img_stage_default_for_current_boot(uint32_t additional_requirements);
int ps2_drivers_img_stage_default_for_all_filesystems(uint32_t additional_requirements);

/* Restage an explicit set, or repeat the last successful staging request. */
int ps2_drivers_img_restage(uint32_t driver_requirements);
int ps2_drivers_img_restage_last(void);
void ps2_drivers_img_discard_staged(void);
void ps2_drivers_img_forget_source(void);
size_t ps2_drivers_img_staged_bytes(void);
size_t ps2_drivers_img_staged_modules(void);

#ifdef __cplusplus
}
#endif

#endif
