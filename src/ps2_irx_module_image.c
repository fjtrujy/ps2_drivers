#include <stddef.h>
#include <loadfile.h>
#include <ps2_irx_module.h>

#include "ps2_drivers_img_internal.h"

int ps2_irx_exec(
    const struct ps2_irx_module *module,
    unsigned int arg_len,
    const char *args,
    int *module_result)
{
    void *data;
    unsigned int size;
    int module_id;

    if (module == NULL)
        return -1;

    if (ps2_drivers_img_get_staged(module->id, &data, &size) != 0)
        return -1;

    module_id = SifExecModuleBuffer(data, size, (int)arg_len, args, module_result);
    ps2_drivers_img_release_staged(module->id);
    return module_id;
}
