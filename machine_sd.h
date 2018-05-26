#ifndef MACHINE_SD_H_INC
#define MACHINE_SD_H_INC

#if MICROPY_MACHINE_SD
extern const mp_obj_type_t machine_sd_type;
extern void machine_sd_teardown(void);

#define MACHINE_SD_CLASS { MP_ROM_QSTR(MP_QSTR_SD), MP_ROM_PTR(&machine_sd_type) },
#define MACHINE_SD_TEARDOWN() machine_sd_teardown()
#else
#define MACHINE_SD_CLASS
#define MACHINE_SD_TEARDOWN()
#endif

#endif
