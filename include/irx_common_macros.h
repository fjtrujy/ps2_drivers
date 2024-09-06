#ifndef COMMON_MACROS_H
#define COMMON_MACROS_H

/// declare IRX ID and return value variables
#define DECL_IRX_VARS(irx) \
    int32_t __##irx##_id = -1; \
    int __##irx##_ret = -1

/// extern IRX ID and return value variables
#define EXTERN_IRX_VARS(irx) \
    extern int32_t __##irx##_id; \
    extern int __##irx##_ret

#define RESET_IRX_VARS(irx) \
    __##irx##_id = -1; \
    __##irx##_ret = -1

/* Values taken from PS2SDK
* MODULE_RESIDENT_END		0
* MODULE_NO_RESIDENT_END	1
* MODULE_REMOVABLE_END	2 // Only for 'modload' > v1.2
*/

/// condition to perform irx load
#define CHECK_IRX_LOAD(irx) (__##irx##_id == -1) && (__##irx##_ret == -1)

/// condition for irx startup errors
#define CHECK_IRX_ERR(irx) (__##irx##_id < 0 || __##irx##_ret == 1)

/// check if this irx module is successfully loaded and supports unloading.
/// It should use MODULE_REMOVABLE_END however in PS2SDK it is not used, so we will use MODULE_RESIDENT_END
#define CHECK_IRX_UNLOAD(irx) (__##irx##_id >= 0 && __##irx##_ret == 0)
/// extern an embedded irx file buffer and size
#define EXTERN_IRX(_IRX) \
extern unsigned char _IRX[] __attribute__((aligned(16))); \
extern unsigned int size_##_IRX

#endif
