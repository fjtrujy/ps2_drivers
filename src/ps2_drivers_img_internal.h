#ifndef PS2_DRIVERS_IMG_INTERNAL_H
#define PS2_DRIVERS_IMG_INTERNAL_H

#include <stdint.h>

int ps2_drivers_img_get_staged(uint32_t module_id, void **data, unsigned int *size);
void ps2_drivers_img_release_staged(uint32_t module_id);

#endif
