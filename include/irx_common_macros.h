#ifndef COMMON_MACROS_H
#define COMMON_MACROS_H

/// declare IRX ID and return value variables
#define DECL_IRX_VARS(irx) \
    int32_t __##irx##_id = -1; \
    int32_t __##irx##_ret = -1

/// extern IRX ID and return value variables
#define EXTERN_IRX_VARS(irx) \
    extern int32_t __##irx##_id; \
    extern int32_t __##irx##_ret

#define RESET_IRX_VARS(irx) \
    __##irx##_id = -1; \
    __##irx##_ret = -1

/// condition for irx startup errors
#define CHECK_IRX_ERR(irx) (__##irx##_id < 0 || __##irx##_ret == 1)

/// check if this irx module is successfully loaded and supports unloading
#define CHECK_IRX_UNLOAD(irx) (__##irx##_id >= 0 && __##irx##_ret == 2)
/// extern an embedded irx file buffer and size
#define EXTERN_IRX(_IRX) \
extern unsigned char _IRX[] __attribute__((aligned(16))); \
extern unsigned int size_##_IRX

#endif
