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

#include <ti/drivers/SPI.h>

// TODO: remove after implementing the empty functions
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

typedef struct _machine_spi_obj_t {
    mp_obj_base_t base;
    SPI_Handle spi;
    uint8_t id;
    uint32_t baudrate;
    uint8_t polarity;
    uint8_t phase;
    uint8_t datasize;
} machine_spi_obj_t;

extern const mp_obj_type_t machine_spi_type;

// TODO: how to size this table?
#define NUM_SPI  3
static machine_spi_obj_t spi_obj[NUM_SPI] = {
    {{&machine_spi_type}, .id = 0},
    {{&machine_spi_type}, .id = 1},
    {{&machine_spi_type}, .id = 2},
};

void machine_spi_teardown(void) {
    for (int i = 0; i < NUM_SPI; i++) {
        if (spi_obj[i].spi) {
            SPI_close(spi_obj[i].spi);
            spi_obj[i].spi = NULL;
        }
    }
}

static void spi_init_helper(machine_spi_obj_t * self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_baudrate, MP_ARG_INT, {.u_int = 1000000} },
        { MP_QSTR_polarity, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_phase, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_bits, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 8} },
        { MP_QSTR_firstbit, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
    };
    enum { ARG_baudrate, ARG_polarity, ARG_phase, ARG_bits, ARG_firstbit};
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args,
        MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_int_t baudrate = args[ARG_baudrate].u_int;
    mp_int_t bits = args[ARG_bits].u_int;
    mp_int_t polarity = args[ARG_polarity].u_int ? 1 : 0;
    mp_int_t phase = args[ARG_phase].u_int ? 1 : 0;

    SPI_Params params;
    SPI_Params_init(&params);
    params.bitRate = baudrate;
    params.dataSize = bits;
    params.frameFormat = (polarity << 1) | phase;

    if (self->spi) {
        SPI_close(self->spi);
    }

    if ((self->spi = SPI_open(self->id, &params)) == NULL) {
        mp_raise_OSError(MP_ENODEV);
    }

    self->baudrate = baudrate;
    self->datasize = bits;
    self->polarity = polarity;
    self->phase = phase;
}

STATIC mp_obj_t machine_spi_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);
    mp_int_t id = mp_obj_get_int(args[0]);
    if (id < 0 || id >= NUM_SPI) {
        mp_raise_OSError(MP_ENODEV);
    }
    machine_spi_obj_t *self = &spi_obj[id];

    if (n_args > 1 || n_kw > 0) {
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        spi_init_helper(self, n_args - 1, args + 1, &kw_args);
    }

    return MP_OBJ_FROM_PTR(self);
}

STATIC void machine_spi_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_spi_obj_t *self = self_in;
    mp_printf(print,
              "<machine_spi.%p> id=%d, baudrate=%u, polarity=%u, phase=%u, bits=%u",
              self, self->id, self->baudrate, self->polarity, self->phase,
              self->datasize);
}

STATIC mp_obj_t machine_spi_init(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    machine_spi_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    spi_init_helper(self, n_args - 1, pos_args + 1, kw_args);

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_spi_init_obj, 1, machine_spi_init);

STATIC mp_obj_t machine_spi_deinit(mp_obj_t self_in) {
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_spi_deinit_obj, machine_spi_deinit);

static void spi_transfer(machine_spi_obj_t *self, size_t len, const uint8_t *src, void *dest) {
    SPI_Transaction trans;

    trans.txBuf = (void *)src;
    trans.rxBuf = dest;
    if (self->datasize > 8 && self->datasize <= 16) {
        trans.count = len / 2;
    }
    else {
        trans.count = len;
    }
    (void)SPI_transfer(self->spi, &trans);
}

STATIC mp_obj_t machine_spi_read(size_t n_args, const mp_obj_t *args) {
    machine_spi_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_int_t nbytes = mp_obj_get_int(args[1]);
    vstr_t vstr;
    vstr_init_len(&vstr, nbytes);

    spi_transfer(self, nbytes, NULL, vstr.buf);

    return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_spi_read_obj, 2, 3, machine_spi_read);

STATIC mp_obj_t machine_spi_readinto(size_t n_args, const mp_obj_t *args) {
    machine_spi_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_buffer_info_t buf_info;
    mp_get_buffer_raise(args[1], &buf_info, MP_BUFFER_WRITE);

    spi_transfer(self, buf_info.len, NULL, buf_info.buf);

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_spi_readinto_obj, 2, 3, machine_spi_readinto);

STATIC mp_obj_t machine_spi_write(mp_obj_t self_in, mp_obj_t buf_in) {
    machine_spi_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_buffer_info_t buf_info;
    mp_get_buffer_raise(buf_in, &buf_info, MP_BUFFER_READ);

    spi_transfer(self, buf_info.len, buf_info.buf, NULL);

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(machine_spi_write_obj, machine_spi_write);

STATIC mp_obj_t machine_spi_write_readinto(mp_obj_t self_in, mp_obj_t write_buf_in, mp_obj_t read_buf_in) {
    machine_spi_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_buffer_info_t write_buf_info;
    mp_get_buffer_raise(write_buf_in, &write_buf_info, MP_BUFFER_READ);
    mp_buffer_info_t read_buf_info;
    mp_get_buffer_raise(read_buf_in, &read_buf_info, MP_BUFFER_WRITE);

    spi_transfer(self, read_buf_info.len, write_buf_info.buf, read_buf_info.buf);

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(machine_spi_write_readinto_obj, machine_spi_write_readinto);

STATIC const mp_rom_map_elem_t machine_spi_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_spi_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&machine_spi_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&machine_spi_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_readinto), MP_ROM_PTR(&machine_spi_readinto_obj) },
    { MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&machine_spi_write_obj) },
    { MP_ROM_QSTR(MP_QSTR_write_readinto), MP_ROM_PTR(&machine_spi_write_readinto_obj) },
    { MP_ROM_QSTR(MP_QSTR_MASTER), MP_ROM_INT(3) },
    { MP_ROM_QSTR(MP_QSTR_MSB), MP_ROM_INT(0) },
    { MP_ROM_QSTR(MP_QSTR_LSB), MP_ROM_INT(1) },
};

STATIC MP_DEFINE_CONST_DICT(machine_spi_locals_dict, machine_spi_locals_dict_table);

const mp_obj_type_t machine_spi_type = {
    { &mp_type_type },
    .name = MP_QSTR_SPI,
    .make_new = machine_spi_make_new,
    .print = machine_spi_print,
    .locals_dict = (mp_obj_dict_t*)&machine_spi_locals_dict,
};
