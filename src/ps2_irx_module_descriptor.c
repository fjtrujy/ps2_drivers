#include <stddef.h>
#include <ps2_irx_module.h>

#ifndef PS2_IRX_MODULE_ID
#error "PS2_IRX_MODULE_ID must be defined"
#endif

#ifndef PS2_IRX_DESCRIPTOR_SYMBOL
#error "PS2_IRX_DESCRIPTOR_SYMBOL must be defined"
#endif

#ifndef PS2_IRX_EMBEDDED_SYMBOL
#error "PS2_IRX_EMBEDDED_SYMBOL must be defined"
#endif

#ifndef PS2_IRX_SIZE_SYMBOL
#error "PS2_IRX_SIZE_SYMBOL must be defined"
#endif

extern unsigned char PS2_IRX_EMBEDDED_SYMBOL[] __attribute__((aligned(16)));
extern unsigned int PS2_IRX_SIZE_SYMBOL;

const struct ps2_irx_module PS2_IRX_DESCRIPTOR_SYMBOL = {
    PS2_IRX_MODULE_ID,
    PS2_IRX_EMBEDDED_SYMBOL,
    &PS2_IRX_SIZE_SYMBOL,
};
