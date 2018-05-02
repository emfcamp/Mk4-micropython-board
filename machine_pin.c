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

#include <ti/drivers/GPIO.h>

// TODO: remove after implementing the empty functions
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

typedef struct _machine_pin_obj_t {
    mp_obj_base_t base;
    uint32_t pin;
} machine_pin_obj_t;

STATIC mp_obj_t machine_pin_call(mp_obj_t self_in, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 1, false);
    machine_pin_obj_t *self = self_in;

    if (n_args == 0) {
        uint32_t pin_val;
        pin_val = GPIO_read(self->pin);
        return MP_OBJ_NEW_SMALL_INT(pin_val);
    } else {
        (void)GPIO_write(self->pin, mp_obj_is_true(args[0]));
        return mp_const_none;
    }
}

STATIC mp_obj_t machine_pin_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);
    mp_int_t id = mp_obj_get_int(args[0]);
    mp_int_t mode = n_args > 1 ? mp_obj_get_int(args[1]) : -1;
    mp_int_t pull = n_args > 2 ? mp_obj_get_int(args[2]) : -1;
    // kwargs not handled
    machine_pin_obj_t *self = m_new_obj(machine_pin_obj_t);

    self->base.type = type;
    self->pin = id;

    return MP_OBJ_FROM_PTR(self);
}

STATIC void machine_pin_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_pin_obj_t *self = self_in;
    mp_printf(print, "<machine_pin.%p>", self);
}

STATIC mp_obj_t machine_pin_init(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_mode, MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_pull, MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_value, MP_ARG_KW_ONLY | MP_ARG_INT, },
        { MP_QSTR_drive, MP_ARG_KW_ONLY | MP_ARG_INT, },
        { MP_QSTR_alt, MP_ARG_KW_ONLY | MP_ARG_INT, },
    };
    enum { ARG_mode, ARG_pull, ARG_value, ARG_drive, ARG_alt,};
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
        MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    mp_obj_t result = mp_const_none;

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_pin_init_obj, 1, machine_pin_init);

STATIC mp_obj_t machine_pin_value(size_t n_args, const mp_obj_t *args) {
    return machine_pin_call(args[0], n_args - 1, 0, args + 1);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(machine_pin_value_obj, 0, machine_pin_value);

STATIC mp_obj_t machine_pin_on(mp_obj_t self_in) {
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(self_in);

    (void)GPIO_write(self->pin, 1);

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_pin_on_obj, machine_pin_on);

STATIC mp_obj_t machine_pin_off(mp_obj_t self_in) {
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(self_in);

    (void)GPIO_write(self->pin, 0);

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_pin_off_obj, machine_pin_off);

STATIC mp_obj_t machine_pin_mode(size_t n_args, const mp_obj_t *args) {
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    // optional args in args[1++]

    mp_obj_t result = MP_OBJ_NEW_SMALL_INT(0);

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(machine_pin_mode_obj, 1, machine_pin_mode);

STATIC mp_obj_t machine_pin_pull(size_t n_args, const mp_obj_t *args) {
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    // optional args in args[1++]

    mp_obj_t result = MP_OBJ_NEW_SMALL_INT(0);

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(machine_pin_pull_obj, 1, machine_pin_pull);

STATIC mp_obj_t machine_pin_drive(size_t n_args, const mp_obj_t *args) {
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    // optional args in args[1++]

    mp_obj_t result = MP_OBJ_NEW_SMALL_INT(0);

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(machine_pin_drive_obj, 1, machine_pin_drive);

STATIC const mp_rom_map_elem_t machine_pin_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_pin_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_value), MP_ROM_PTR(&machine_pin_value_obj) },
    { MP_ROM_QSTR(MP_QSTR_on), MP_ROM_PTR(&machine_pin_on_obj) },
    { MP_ROM_QSTR(MP_QSTR_off), MP_ROM_PTR(&machine_pin_off_obj) },
    { MP_ROM_QSTR(MP_QSTR_mode), MP_ROM_PTR(&machine_pin_mode_obj) },
    { MP_ROM_QSTR(MP_QSTR_pull), MP_ROM_PTR(&machine_pin_pull_obj) },
    { MP_ROM_QSTR(MP_QSTR_drive), MP_ROM_PTR(&machine_pin_drive_obj) },
    { MP_ROM_QSTR(MP_QSTR_IN), MP_ROM_INT(GPIO_CFG_INPUT) },
    { MP_ROM_QSTR(MP_QSTR_OUT), MP_ROM_INT(GPIO_CFG_OUTPUT) },
    { MP_ROM_QSTR(MP_QSTR_PULL_UP), MP_ROM_INT(GPIO_CFG_OUT_OD_PU) },
    { MP_ROM_QSTR(MP_QSTR_PULL_DOWN), MP_ROM_INT(GPIO_CFG_OUT_OD_PD) },
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
