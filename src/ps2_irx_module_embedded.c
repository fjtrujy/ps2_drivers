#include <stddef.h>
#include <loadfile.h>
#include <ps2_irx_module.h>

int ps2_irx_exec(
    const struct ps2_irx_module *module,
    unsigned int arg_len,
    const char *args,
    int *module_result)
{
    if (module == NULL || module->embedded_data == NULL || module->embedded_size == NULL)
        return -1;

    return SifExecModuleBuffer(
        (void *)module->embedded_data,
        *module->embedded_size,
        (int)arg_len,
        args,
        module_result);
}
