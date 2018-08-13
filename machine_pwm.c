/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) Texas Instruments, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>
#include <stdbool.h>

#include "py/runtime.h"
#include "py/mperrno.h"

#include <ti/drivers/PWM.h>

typedef struct _machine_pwm_obj_t {
    mp_obj_base_t base;
    PWM_Handle pwm;
    uint8_t id;
    uint32_t freq;
    uint32_t duty;
} machine_pwm_obj_t;

extern const mp_obj_type_t machine_pwm_type;

// TODO: how to set this?
#define NUM_PWM 4
static machine_pwm_obj_t pwm_obj[NUM_PWM] = {
    {{&machine_pwm_type}, .id = 0, .freq = 1000, .duty = 0},
    {{&machine_pwm_type}, .id = 1, .freq = 1000, .duty = 0},
    {{&machine_pwm_type}, .id = 2, .freq = 1000, .duty = 0},
    {{&machine_pwm_type}, .id = 3, .freq = 1000, .duty = 0},
};

void machine_pwm_teardown(void) {
    for (int i = 0; i < NUM_PWM; i++) {
        if (pwm_obj[i].pwm) {
            PWM_close(pwm_obj[i].pwm);
            pwm_obj[i].pwm = NULL;
        }
    }
}

static uint32_t scale_duty(uint32_t duty)
{
    return duty * (PWM_DUTY_FRACTION_MAX / 100);
}

static void pwm_init_helper(machine_pwm_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_freq, ARG_duty };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_freq, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_duty, MP_ARG_INT, {.u_int = ~0} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args,
        MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    int freq = args[ARG_freq].u_int;
    if (freq > 0) {
        self->freq = freq;
    }

    int duty = args[ARG_duty].u_int;
    if (duty != ~0) {
        duty = (duty > 100) ? 100 : duty;
        self->duty = duty;
    }

    PWM_Params params;
    PWM_Params_init(&params);
    params.periodUnits = PWM_PERIOD_HZ;
    params.periodValue = self->freq;
    params.dutyUnits = PWM_DUTY_FRACTION;
    params.dutyValue = scale_duty(self->duty);

    if (self->pwm) {
        PWM_close(self->pwm);
    }

    if ((self->pwm = PWM_open(self->id, &params)) == NULL) {
        mp_raise_OSError(MP_ENODEV);
    }

    PWM_start(self->pwm);
}

STATIC mp_obj_t machine_pwm_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);
    mp_int_t id = mp_obj_get_int(args[0]);

    if (id < 0 || id >= NUM_PWM) {
        mp_raise_OSError(MP_ENODEV);
    }

    machine_pwm_obj_t *self = &pwm_obj[id]; // m_new_obj(machine_pwm_obj_t);

    if (n_args > 1 || n_kw > 0 || self->pwm == NULL) {
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        pwm_init_helper(self, n_args - 1, args + 1, &kw_args);
    }

    return MP_OBJ_FROM_PTR(self);
}

STATIC void machine_pwm_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_pwm_obj_t *self = self_in;
    mp_printf(print, "<machine_pwm.%p> id=%u, freq=%u, duty=%u", self, self->id, self->freq, self->duty);
}

STATIC mp_obj_t machine_pwm_init(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    machine_pwm_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    pwm_init_helper(self, n_args - 1, pos_args + 1, kw_args);
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_pwm_init_obj, 1, machine_pwm_init);

STATIC mp_obj_t machine_pwm_freq(size_t n_args, const mp_obj_t *args) {
    machine_pwm_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    if (n_args == 1) {
        return MP_OBJ_NEW_SMALL_INT(self->freq);
    }

    int tval = mp_obj_get_int(args[1]);
    if (PWM_setPeriod(self->pwm, tval) < 0) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
            "Bad frequency %d", tval));
    }
    self->freq = tval;

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(machine_pwm_freq_obj, 1, machine_pwm_freq);

STATIC mp_obj_t machine_pwm_duty(size_t n_args, const mp_obj_t *args) {
    machine_pwm_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    if (n_args == 1) {
        return MP_OBJ_NEW_SMALL_INT(self->duty);
    }

    int duty = mp_obj_get_int(args[1]);
    duty = (duty > 100) ? 100 : duty;
    PWM_setDuty(self->pwm, scale_duty(duty));
    self->duty = duty;

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(machine_pwm_duty_obj, 1, machine_pwm_duty);

STATIC const mp_rom_map_elem_t machine_pwm_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_pwm_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_freq), MP_ROM_PTR(&machine_pwm_freq_obj) },
    { MP_ROM_QSTR(MP_QSTR_duty), MP_ROM_PTR(&machine_pwm_duty_obj) },
#ifdef MACHINE_PWM_IDS
    MACHINE_PWM_IDS
#endif
};

STATIC MP_DEFINE_CONST_DICT(machine_pwm_locals_dict, machine_pwm_locals_dict_table);

const mp_obj_type_t machine_pwm_type = {
    { &mp_type_type },
    .name = MP_QSTR_PWM,
    .make_new = machine_pwm_make_new,
    .print = machine_pwm_print,
    .locals_dict = (mp_obj_dict_t*)&machine_pwm_locals_dict,
};
