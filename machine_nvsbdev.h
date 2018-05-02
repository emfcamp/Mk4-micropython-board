#ifndef MACHINE_NVSBDEV_H_INC
#define MACHINE_NVSBDEV_H_INC

#if MICROPY_MACHINE_NVSBDEV
extern const mp_obj_type_t machine_nvsbdev_type;
extern void machine_nvsbdev_teardown();

#define MACHINE_NVSBDEV_CLASS { MP_ROM_QSTR(MP_QSTR_nvsbdev), MP_ROM_PTR(&machine_nvsbdev_type) },
#define MACHINE_NVSBDEV_TEARDOWN() machine_nvsbdev_teardown()
#else
#define MACHINE_NVSBDEV_CLASS
#define MACHINE_NVSBDEV_TEARDOWN()
#endif

#endif
