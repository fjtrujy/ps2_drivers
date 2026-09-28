#ifndef PS2_IRX_MODULE_H
#define PS2_IRX_MODULE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ps2_irx_module {
    uint32_t id;
    const unsigned char *embedded_data;
    const unsigned int *embedded_size;
};

int ps2_irx_exec(
    const struct ps2_irx_module *module,
    unsigned int arg_len,
    const char *args,
    int *module_result);

#define EXTERN_PS2_IRX_MODULE(name) \
    extern const struct ps2_irx_module ps2_irx_##name

#ifdef __cplusplus
}
#endif

#endif
