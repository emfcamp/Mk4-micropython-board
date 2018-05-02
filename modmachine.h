#ifndef __MICROPY_INCLUDED_TI_MODMACHINE_H__
#define __MICROPY_INCLUDED_TI_MODMACHINE_H__

#include "py/obj.h"

extern const mp_obj_type_t machine_i2c_type;
extern const mp_obj_type_t machine_pin_type;
extern const mp_obj_type_t machine_sd_type;
extern const mp_obj_type_t machine_spi_type;
extern const mp_obj_type_t machine_uart_type;
//extern const mp_obj_type_t machine_pwm_type;
//extern const mp_obj_type_t machine_nvs_type;

MP_DECLARE_CONST_FUN_OBJ_0(machine_info_obj);

#endif
