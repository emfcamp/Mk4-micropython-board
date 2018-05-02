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

// TODO: remove after implementing the empty functions
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

typedef struct _machine_uart_obj_t {
    mp_obj_base_t base;
} machine_uart_obj_t;

void machine_uart_teardown(void) {
}

STATIC mp_obj_t machine_uart_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 5, MP_OBJ_FUN_ARGS_MAX, true);
    mp_int_t id = mp_obj_get_int(args[0]);
    mp_int_t baudrate = mp_obj_get_int(args[1]);
    mp_int_t bits = mp_obj_get_int(args[2]);
    bool parity = mp_obj_is_true(args[3]);
    mp_int_t stopbits = mp_obj_get_int(args[4]);
    // kwargs not handled
    machine_uart_obj_t *self = m_new_obj(machine_uart_obj_t);
    self->base.type = type;

    return MP_OBJ_FROM_PTR(self);
}

STATIC void machine_uart_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_uart_obj_t *self = self_in;
    mp_printf(print, "<machine_uart.%p>", self);
}

STATIC mp_obj_t machine_uart_deinit(mp_obj_t self_in) {
    machine_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);

    mp_obj_t result = mp_const_none;

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_uart_deinit_obj, machine_uart_deinit);

STATIC mp_obj_t machine_uart_any(mp_obj_t self_in) {
    machine_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);

    mp_obj_t result = MP_OBJ_NEW_SMALL_INT(0);

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_uart_any_obj, machine_uart_any);

STATIC mp_obj_t machine_uart_read(size_t n_args, const mp_obj_t *args) {
    machine_uart_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    // optional args in args[1++]

    mp_obj_t result = mp_obj_new_bytes(NULL, 0);

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(machine_uart_read_obj, 1, machine_uart_read);

STATIC mp_obj_t machine_uart_readinto(size_t n_args, const mp_obj_t *args) {
    machine_uart_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_buffer_info_t buf_info;
    mp_get_buffer_raise(args[1], &buf_info, MP_BUFFER_WRITE);
    // optional args in args[2++]

    mp_obj_t result = MP_OBJ_NEW_SMALL_INT(0);

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(machine_uart_readinto_obj, 2, machine_uart_readinto);

STATIC mp_obj_t machine_uart_readline(mp_obj_t self_in) {
    machine_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);

    mp_obj_t result = mp_obj_new_bytes(NULL, 0);

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_uart_readline_obj, machine_uart_readline);

STATIC mp_obj_t machine_uart_write(mp_obj_t self_in, mp_obj_t buf_in) {
    machine_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_buffer_info_t buf_info;
    mp_get_buffer_raise(buf_in, &buf_info, MP_BUFFER_READ);

    mp_obj_t result = MP_OBJ_NEW_SMALL_INT(0);

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(machine_uart_write_obj, machine_uart_write);

STATIC mp_obj_t machine_uart_sendbreak(mp_obj_t self_in) {
    machine_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);

    mp_obj_t result = mp_const_none;

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_uart_sendbreak_obj, machine_uart_sendbreak);

STATIC const mp_rom_map_elem_t machine_uart_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&machine_uart_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_any), MP_ROM_PTR(&machine_uart_any_obj) },
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&machine_uart_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_readinto), MP_ROM_PTR(&machine_uart_readinto_obj) },
    { MP_ROM_QSTR(MP_QSTR_readline), MP_ROM_PTR(&machine_uart_readline_obj) },
    { MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&machine_uart_write_obj) },
    { MP_ROM_QSTR(MP_QSTR_sendbreak), MP_ROM_PTR(&machine_uart_sendbreak_obj) },
};

STATIC MP_DEFINE_CONST_DICT(machine_uart_locals_dict, machine_uart_locals_dict_table);

const mp_obj_type_t machine_uart_type = {
    { &mp_type_type },
    .name = MP_QSTR_UART,
    .make_new = machine_uart_make_new,
    .print = machine_uart_print,
    .locals_dict = (mp_obj_dict_t*)&machine_uart_locals_dict,
};
