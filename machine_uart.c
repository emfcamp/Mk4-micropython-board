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

#include <ti/drivers/UART.h>

#define MACHINE_UART_EVEN 2u
#define MACHINE_UART_ODD 1u
#define MACHINE_UART_NONE 0u

#define MACHINE_UART_RTS 256u
#define MACHINE_UART_CTS 512u

#define MACHINE_UART_TEXT 0u
#define MACHINE_UART_BINARY 1u

typedef struct _machine_uart_obj_t {
    mp_obj_base_t base;
    uint32_t id;
    UART_Handle uart;
    uint32_t baudrate;
    uint32_t bits;
    uint32_t timeout;
    uint32_t parity;
    uint32_t stop;
    uint32_t flow;
    uint32_t read_buf_len;
    uint32_t mode;
    bool echo;
} machine_uart_obj_t;

// TODO: figure out how to set this
#define NUM_UARTS 3
static UART_Handle uarts[NUM_UARTS];

void machine_uart_teardown(void) {
    for (uint32_t i = 0; i < NUM_UARTS; i++) {
        if (uarts[i]) {
            UART_close(uarts[i]);
            uarts[i] = NULL;
        }
    }
}

static void uart_init_helper(machine_uart_obj_t * self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_baudrate, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 9600} },
        { MP_QSTR_bits, MP_ARG_INT, {.u_int = 8} },
        { MP_QSTR_parity, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_stop, MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_flow, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_mode, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_echo, MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = 1} },
        { MP_QSTR_timeout, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 1000} },
        { MP_QSTR_read_buf_len, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 64} },
    };
    enum { ARG_baudrate, ARG_bits, ARG_parity, ARG_stop, ARG_flow, ARG_mode,
           ARG_echo, ARG_timeout, ARG_read_buf_len };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args,
        MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    UART_Params params;
    UART_Params_init(&params);

    self->baudrate = args[ARG_baudrate].u_int;
    params.baudRate = self->baudrate;

    self->bits = args[ARG_bits].u_int;
    switch (self->bits) {
    case 8:
        params.dataLength = UART_LEN_8;
        break;
    case 7:
        params.dataLength = UART_LEN_7;
        break;
    case 6:
        params.dataLength = UART_LEN_6;
        break;
    case 5:
        params.dataLength = UART_LEN_5;
        break;
    default:
        mp_raise_OSError(MP_EINVAL);
        break;
    }

    if (args[ARG_parity].u_obj == mp_const_none) {
        self->parity = 0;
        params.parityType = UART_PAR_NONE;
    }
    else {
        uint32_t parity = mp_obj_get_int(args[ARG_parity].u_obj);
        if (parity == MACHINE_UART_ODD) {
            self->parity = MACHINE_UART_ODD;
            params.parityType = UART_PAR_ODD;
        }
        else if (parity == MACHINE_UART_EVEN) {
            self->parity = MACHINE_UART_EVEN;
            params.parityType = UART_PAR_EVEN;
        }
        else {
            mp_raise_OSError(MP_EINVAL);
        }
    }

    self->stop = args[ARG_stop].u_int;
    if (self->stop == 1) {
        params.stopBits = UART_STOP_ONE;
    }
    else if (self->stop == 2) {
        params.stopBits = UART_STOP_TWO;
    }
    else {
        mp_raise_OSError(MP_EINVAL);
    }

    self->flow = args[ARG_flow].u_int;
    if (self->flow != 0) {
        mp_raise_OSError(MP_EOPNOTSUPP);
    }

    self->echo = args[ARG_echo].u_bool;

    self->mode = args[ARG_mode].u_int;
    if (self->mode == MACHINE_UART_TEXT) {
        // defaults from init are what we want
        if (!self->echo) {
            params.readEcho = UART_ECHO_OFF;
        }
    }
    else if (self->mode == MACHINE_UART_BINARY) {
        params.readReturnMode = UART_RETURN_FULL;
        params.readDataMode = UART_DATA_BINARY;
        params.writeDataMode = UART_DATA_BINARY;
    }
    else {
        mp_raise_OSError(MP_EINVAL);
    }

    self->read_buf_len = args[ARG_read_buf_len].u_int;

    self->timeout = args[ARG_timeout].u_int;
    // TODO: validate units
    params.readTimeout = params.writeTimeout = self->timeout;

    if ((self->uart = UART_open(self->id, &params)) == NULL) {
        mp_raise_OSError(MP_ENODEV);
    }
    uarts[self->id] = self->uart;
}

STATIC mp_obj_t machine_uart_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);
    mp_int_t id = mp_obj_get_int(args[0]);
    machine_uart_obj_t *self = m_new_obj(machine_uart_obj_t);
    self->base.type = type;
    self->id = id;
    self->uart = NULL;

    if (n_args > 1 || n_kw > 0) {
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        uart_init_helper(self, n_args - 1, args + 1, &kw_args);
    }

    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t machine_uart_init(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    machine_uart_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    uart_init_helper(self, n_args - 1, pos_args + 1, kw_args);

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_uart_init_obj, 1, machine_uart_init);

STATIC void machine_uart_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_uart_obj_t *self = self_in;
    mp_printf(print, "<machine_uart.%p> id=%d baudrate=%d bits=%d parity=%d "
              "stop=%d flow=0x%x mode=%d timeout=%d read_buf_len=%d",
              self, self->id, self->baudrate, self->bits, self->parity,
              self->stop, self->flow, self->mode, self->timeout,
              self->read_buf_len);
}

STATIC mp_obj_t machine_uart_deinit(mp_obj_t self_in) {
    machine_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);

    UART_close(self->uart);
    self->uart = NULL;

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_uart_deinit_obj, machine_uart_deinit);

STATIC mp_obj_t machine_uart_any(mp_obj_t self_in) {
    machine_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_obj_t result = mp_const_none;
    uint32_t count = 0;

    if (UART_control(self->uart, UART_CMD_GETRXCOUNT, (void *)&count) >= 0) {
        result = MP_OBJ_NEW_SMALL_INT(count);
    }
    else {
        mp_raise_OSError(MP_EIO);
    }

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_uart_any_obj, machine_uart_any);

STATIC mp_obj_t machine_uart_read(size_t n_args, const mp_obj_t *args) {
    machine_uart_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    uint32_t nbytes = self->read_buf_len;
    if (n_args > 1) {
        nbytes = mp_obj_get_int(args[1]);
    }

    vstr_t vstr;
    vstr_init_len(&vstr, nbytes);

    int32_t num_read = UART_read(self->uart, vstr.buf, nbytes);
    if (num_read > 0) {
        vstr.len = num_read;
        return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr);
    }
    else {
        vstr_clear(&vstr);
        return mp_const_none;
    }
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(machine_uart_read_obj, 1, machine_uart_read);

STATIC mp_obj_t machine_uart_readinto(size_t n_args, const mp_obj_t *args) {
    machine_uart_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_buffer_info_t buf_info;
    mp_get_buffer_raise(args[1], &buf_info, MP_BUFFER_WRITE);
    uint32_t nbytes = buf_info.len;
    if (n_args > 2) {
        nbytes = mp_obj_get_int(args[2]);
    }
    int32_t num_read = UART_read(self->uart, buf_info.buf, nbytes);
    mp_obj_t result = mp_const_none;
    if (num_read > 0) {
        result = MP_OBJ_NEW_SMALL_INT(num_read);
    }

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(machine_uart_readinto_obj, 2, machine_uart_readinto);

STATIC mp_obj_t machine_uart_readline(mp_obj_t self_in) {
    machine_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if (self->mode == MACHINE_UART_BINARY) {
        mp_raise_OSError(MP_EINVAL);
    }

    vstr_t vstr;
    vstr_init_len(&vstr, self->read_buf_len);

    int32_t num_read = UART_read(self->uart, vstr.buf, self->read_buf_len);
    if (num_read > 0) {
        vstr.len = num_read;
        return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr);
    }
    else {
        vstr_clear(&vstr);
        return mp_const_none;
    }
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_uart_readline_obj, machine_uart_readline);

STATIC mp_obj_t machine_uart_write(mp_obj_t self_in, mp_obj_t buf_in) {
    machine_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_buffer_info_t buf_info;
    mp_get_buffer_raise(buf_in, &buf_info, MP_BUFFER_READ);

    int32_t num_write = UART_write(self->uart, buf_info.buf, buf_info.len);

    mp_obj_t result = mp_const_none;
    if (num_write > 0) {
        result = MP_OBJ_NEW_SMALL_INT(num_write);
    }

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(machine_uart_write_obj, machine_uart_write);

STATIC mp_obj_t machine_uart_sendbreak(mp_obj_t self_in) {
    mp_raise_OSError(MP_EOPNOTSUPP);
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_uart_sendbreak_obj, machine_uart_sendbreak);

STATIC const mp_rom_map_elem_t machine_uart_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_RTS), MP_ROM_INT(MACHINE_UART_RTS) },
    { MP_ROM_QSTR(MP_QSTR_CTS), MP_ROM_INT(MACHINE_UART_CTS) },
    { MP_ROM_QSTR(MP_QSTR_NONE), MP_ROM_INT(MACHINE_UART_NONE) },
    { MP_ROM_QSTR(MP_QSTR_EVEN), MP_ROM_INT(MACHINE_UART_EVEN) },
    { MP_ROM_QSTR(MP_QSTR_ODD), MP_ROM_INT(MACHINE_UART_ODD) },
    { MP_ROM_QSTR(MP_QSTR_TEXT), MP_ROM_INT(MACHINE_UART_TEXT) },
    { MP_ROM_QSTR(MP_QSTR_BINARY), MP_ROM_INT(MACHINE_UART_BINARY) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_uart_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&machine_uart_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_any), MP_ROM_PTR(&machine_uart_any_obj) },
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&machine_uart_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_readinto), MP_ROM_PTR(&machine_uart_readinto_obj) },
    { MP_ROM_QSTR(MP_QSTR_readline), MP_ROM_PTR(&machine_uart_readline_obj) },
    { MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&machine_uart_write_obj) },
    { MP_ROM_QSTR(MP_QSTR_sendbreak), MP_ROM_PTR(&machine_uart_sendbreak_obj) },
#ifdef MACHINE_UART_IDS
    MACHINE_UART_IDS
#endif
};

STATIC MP_DEFINE_CONST_DICT(machine_uart_locals_dict, machine_uart_locals_dict_table);

const mp_obj_type_t machine_uart_type = {
    { &mp_type_type },
    .name = MP_QSTR_UART,
    .make_new = machine_uart_make_new,
    .print = machine_uart_print,
    .locals_dict = (mp_obj_dict_t*)&machine_uart_locals_dict,
};
