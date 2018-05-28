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

#include <ti/sysbios/knl/Semaphore.h>
#include <ti/drivers/GPIO.h>

#define PULL_UP     0x4u
#define PULL_DOWN   0x8u

typedef struct _machine_pin_obj_t {
    mp_obj_base_t base;
    uint32_t id;
} machine_pin_obj_t;

void machine_pin_teardown(void) {
    /* nothing to do */
}

STATIC mp_obj_t machine_pin_obj_init_helper(machine_pin_obj_t *self, mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_mode, ARG_pull, ARG_drive, ARG_value };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_mode, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_pull, MP_ARG_OBJ, {.u_obj = mp_const_none}},
        { MP_QSTR_drive, MP_ARG_OBJ, {.u_obj = mp_const_none}},
        { MP_QSTR_value, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args),
                     allowed_args, args);

    uint mode = args[ARG_mode].u_int;

    uint pull = 0;
    if (args[ARG_pull].u_obj != mp_const_none) {
        pull = mp_obj_get_int(args[ARG_pull].u_obj);
    }
    if (pull) {
        if (mode == GPIO_CFG_INPUT) {
            mode = (pull == PULL_UP) ? GPIO_CFG_IN_PU : GPIO_CFG_IN_PD;
        }
        else {
            mode = (pull == PULL_UP) ? GPIO_CFG_OUT_OD_PU : GPIO_CFG_OUT_OD_PD;
        }
    }

    uint drive = 0;
    if (args[ARG_drive].u_obj != mp_const_none) {
        drive = mp_obj_get_int(args[ARG_drive].u_obj);
    }

    int ret = GPIO_setConfig(self->id, mode | drive);
    if (ret) {
        mp_raise_ValueError("invalid pin or settings");
    }

    if (args[ARG_value].u_obj != MP_OBJ_NULL) {
        (void)GPIO_write(self->id, mp_obj_is_true(args[ARG_value].u_obj));
    }

    return mp_const_none;
}

STATIC mp_obj_t machine_pin_call(mp_obj_t self_in, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 1, false);
    machine_pin_obj_t *self = self_in;

    if (n_args == 0) {
        uint32_t pin_val;
        pin_val = GPIO_read(self->id);
        return MP_OBJ_NEW_SMALL_INT(pin_val);
    } else {
        (void)GPIO_write(self->id, mp_obj_is_true(args[0]));
        return mp_const_none;
    }
}

STATIC mp_obj_t machine_pin_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);
    int id = mp_obj_get_int(args[0]);

    machine_pin_obj_t *self = m_new_obj(machine_pin_obj_t);
    self->base.type = type;
    self->id = id;

    if (n_args > 1 || n_kw > 0) {
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        machine_pin_obj_init_helper(self, n_args - 1, args + 1, &kw_args);
    }

    return (mp_obj_t)self;

#if 0
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);
    mp_int_t id = mp_obj_get_int(args[0]);
    mp_int_t mode = n_args > 1 ? mp_obj_get_int(args[1]) : -1;
    mp_int_t pull = n_args > 2 ? mp_obj_get_int(args[2]) : -1;
    // kwargs not handled
    machine_pin_obj_t *self = m_new_obj(machine_pin_obj_t);

    self->base.type = type;
    self->id = id;

    return MP_OBJ_FROM_PTR(self);
#endif
}

STATIC void machine_pin_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_pin_obj_t *self = self_in;
    mp_printf(print, "<machine_pin.%p id=%u>", self, self->id);
}

STATIC mp_obj_t machine_pin_init(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    machine_pin_obj_init_helper(self, n_args - 1, pos_args + 1, kw_args);
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_pin_init_obj, 1, machine_pin_init);

STATIC mp_obj_t machine_pin_value(size_t n_args, const mp_obj_t *args) {
    return machine_pin_call(args[0], n_args - 1, 0, args + 1);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(machine_pin_value_obj, 0, machine_pin_value);

STATIC mp_obj_t machine_pin_on(mp_obj_t self_in) {
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(self_in);

    (void)GPIO_write(self->id, 1);

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_pin_on_obj, machine_pin_on);

STATIC mp_obj_t machine_pin_off(mp_obj_t self_in) {
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(self_in);

    (void)GPIO_write(self->id, 0);

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_pin_off_obj, machine_pin_off);

STATIC mp_obj_t machine_pin_mode(size_t n_args, const mp_obj_t *args) {
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_obj_t result;

    if (n_args > 1) {
        GPIO_setConfig(self->id, mp_obj_get_int(args[1]));
        result = mp_const_none;
    }
    else {
        GPIO_PinConfig cfg;
        GPIO_getConfig(self->id, &cfg);
        result = MP_OBJ_NEW_SMALL_INT(cfg & GPIO_CFG_INPUT);
    }

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(machine_pin_mode_obj, 1, machine_pin_mode);

STATIC mp_obj_t machine_pin_pull(size_t n_args, const mp_obj_t *args) {
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_obj_t result;
    GPIO_PinConfig cfg;
    mp_int_t pull;

    GPIO_getConfig(self->id, &cfg);

    if (n_args > 1) {
        pull = mp_obj_get_int(args[1]);
        if (cfg & GPIO_CFG_INPUT) {
            cfg = (pull == PULL_UP) ? GPIO_CFG_IN_PU : GPIO_CFG_IN_PD;
        }
        else {
            cfg = (pull == PULL_UP) ? GPIO_CFG_OUT_OD_PU : GPIO_CFG_OUT_OD_PD;
        }
        GPIO_setConfig(self->id, cfg);
        result = mp_const_none;
    }
    else {
        switch (cfg & 0x00070000u) {
            case GPIO_CFG_IN_PU:
            case GPIO_CFG_OUT_OD_PU:
                pull = PULL_UP;
                break;
            case GPIO_CFG_IN_PD:
            case GPIO_CFG_OUT_OD_PD:
                pull = PULL_DOWN;
                break;
            default:
                pull = 0;
        }
        result = MP_OBJ_NEW_SMALL_INT(pull);
    }

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(machine_pin_pull_obj, 1, machine_pin_pull);

STATIC mp_obj_t machine_pin_drive(size_t n_args, const mp_obj_t *args) {
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_obj_t result;

    if (n_args > 1) {
        GPIO_setConfig(self->id, mp_obj_get_int(args[1]));
        result = mp_const_none;
    }
    else {
        GPIO_PinConfig cfg;
        GPIO_getConfig(self->id, &cfg);
        result = MP_OBJ_NEW_SMALL_INT(cfg & GPIO_CFG_OUT_STRENGTH_MASK);
    }

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(machine_pin_drive_obj, 1, machine_pin_drive);

static void gpioCallback(uint8_t index) {
    mp_obj_t *cb = &MP_STATE_PORT(pinirq_callback)[index];
    if (*cb != mp_const_none) {
        mp_sched_schedule(*cb, mp_const_none);
    }
    extern Semaphore_Handle machine_sleep_sem;
    Semaphore_post(machine_sleep_sem);
}

STATIC mp_obj_t machine_pin_irq(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_handler, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_trigger, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_priority, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_wake, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
    };
    enum { ARG_handler, ARG_trigger, ARG_priority, ARG_wake };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
        MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_obj_t *cb = &MP_STATE_PORT(pinirq_callback)[self->id];

    GPIO_disableInt(self->id);
    *cb = args[ARG_handler].u_obj;
    GPIO_setCallback(self->id, gpioCallback);
    GPIO_enableInt(self->id);

    // TODO: fix result
    mp_obj_t result = mp_const_none;

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_pin_irq_obj, 1, machine_pin_irq);

STATIC const mp_rom_map_elem_t machine_pin_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_pin_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_value), MP_ROM_PTR(&machine_pin_value_obj) },
    { MP_ROM_QSTR(MP_QSTR_on), MP_ROM_PTR(&machine_pin_on_obj) },
    { MP_ROM_QSTR(MP_QSTR_off), MP_ROM_PTR(&machine_pin_off_obj) },
    { MP_ROM_QSTR(MP_QSTR_mode), MP_ROM_PTR(&machine_pin_mode_obj) },
    { MP_ROM_QSTR(MP_QSTR_pull), MP_ROM_PTR(&machine_pin_pull_obj) },
    { MP_ROM_QSTR(MP_QSTR_drive), MP_ROM_PTR(&machine_pin_drive_obj) },
    { MP_ROM_QSTR(MP_QSTR_irq), MP_ROM_PTR(&machine_pin_irq_obj) },
    { MP_ROM_QSTR(MP_QSTR_IN), MP_ROM_INT(GPIO_CFG_INPUT) },
    { MP_ROM_QSTR(MP_QSTR_OUT), MP_ROM_INT(GPIO_CFG_OUTPUT) },
    { MP_ROM_QSTR(MP_QSTR_PULL_UP), MP_ROM_INT(PULL_UP) },
    { MP_ROM_QSTR(MP_QSTR_PULL_DOWN), MP_ROM_INT(PULL_DOWN) },
    { MP_ROM_QSTR(MP_QSTR_LOW_POWER), MP_ROM_INT(GPIO_CFG_OUT_STR_LOW) },
    { MP_ROM_QSTR(MP_QSTR_MED_POWER), MP_ROM_INT(GPIO_CFG_OUT_STR_MED) },
    { MP_ROM_QSTR(MP_QSTR_HIGH_POWER), MP_ROM_INT(GPIO_CFG_OUT_STR_HIGH) },
};

STATIC MP_DEFINE_CONST_DICT(machine_pin_locals_dict, machine_pin_locals_dict_table);

const mp_obj_type_t machine_pin_type = {
    { &mp_type_type },
    .name = MP_QSTR_Pin,
    .make_new = machine_pin_make_new,
    .print = machine_pin_print,
    .call = machine_pin_call,
    .locals_dict = (mp_obj_dict_t*)&machine_pin_locals_dict,
};
