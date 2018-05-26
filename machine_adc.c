/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Texas Instruments, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>
#include <stdbool.h>

#include "py/runtime.h"
#include "py/mperrno.h"

#include <ti/drivers/ADC.h>

typedef struct _machine_adc_obj_t {
    mp_obj_base_t base;
    uint32_t id;
    ADC_Handle adc;
} machine_adc_obj_t;

extern const mp_obj_type_t machine_adc_type;

// TODO: how to size this table?
#define NUM_ADC  4
static machine_adc_obj_t adc_obj[NUM_ADC] = {
    {{&machine_adc_type}, .id = 0, .adc = NULL},
    {{&machine_adc_type}, .id = 1, .adc = NULL},
    {{&machine_adc_type}, .id = 2, .adc = NULL},
    {{&machine_adc_type}, .id = 3, .adc = NULL},
};

void machine_adc_teardown(void) {
    for (int i = 0; i < NUM_ADC; i++) {
        if (adc_obj[i].adc) {
            ADC_close(adc_obj[i].adc);
            adc_obj[i].adc = NULL;
        }
    }
}

STATIC mp_obj_t machine_adc_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);
    mp_int_t channel = mp_obj_get_int(args[0]);

    if (channel < 0 || channel >= NUM_ADC) {
        mp_raise_OSError(MP_ENODEV);
    }
    machine_adc_obj_t *self = &adc_obj[channel];
    if (self->adc == NULL) {
        if ((self->adc = ADC_open(channel, NULL)) == NULL) {
            mp_raise_OSError(MP_ENODEV);
        }
    }

    return MP_OBJ_FROM_PTR(self);
}

STATIC void machine_adc_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_adc_obj_t *self = self_in;
    mp_printf(print, "<machine_adc.%p> id=%d", self, self->id);
}

STATIC mp_obj_t machine_adc_convert(mp_obj_t self_in) {
    machine_adc_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint16_t sample;
    if (ADC_convert(self->adc, &sample) == ADC_STATUS_ERROR) {
        mp_raise_OSError(MP_EIO);
    }

    mp_obj_t result = MP_OBJ_NEW_SMALL_INT(sample);

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_adc_convert_obj, machine_adc_convert);

STATIC const mp_rom_map_elem_t machine_adc_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_convert), MP_ROM_PTR(&machine_adc_convert_obj) },
};

STATIC MP_DEFINE_CONST_DICT(machine_adc_locals_dict, machine_adc_locals_dict_table);

const mp_obj_type_t machine_adc_type = {
    { &mp_type_type },
    .name = MP_QSTR_ADC,
    .make_new = machine_adc_make_new,
    .print = machine_adc_print,
    .locals_dict = (mp_obj_dict_t*)&machine_adc_locals_dict,
};
