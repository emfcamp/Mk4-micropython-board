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
#include <string.h>

#include "py/runtime.h"
#include "py/mperrno.h"

#include <ti/drivers/I2C.h>

// TODO: remove after implementing the empty functions
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

typedef struct _machine_i2c_obj_t {
    mp_obj_base_t base;
    I2C_Handle i2c;
    uint8_t id;
    uint32_t baudrate;
} machine_i2c_obj_t;

extern const mp_obj_type_t machine_i2c_type;

// TODO: how to size this table?
#define NUM_I2C  3
static machine_i2c_obj_t i2c_obj[NUM_I2C] = {
    {{&machine_i2c_type}, .id = 0, .baudrate = 400000},
    {{&machine_i2c_type}, .id = 1, .baudrate = 400000},
    {{&machine_i2c_type}, .id = 2, .baudrate = 400000},
};

void machine_i2c_teardown(void) {
    for (int i = 0; i < NUM_I2C; i++) {
        if (i2c_obj[i].i2c) {
            I2C_close(i2c_obj[i].i2c);
            i2c_obj[i].i2c = NULL;
        }
    }
}

static void i2c_init_helper(machine_i2c_obj_t * self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_baudrate, MP_ARG_INT, {.u_int = ~0u} },
    };
    enum { ARG_baudrate };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args,
        MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    if (args[ARG_baudrate].u_int != ~0u) {
        self->baudrate = args[ARG_baudrate].u_int;
    }

    I2C_Params params;
    I2C_Params_init(&params);
    if (self->baudrate <= 100000u) {
        self->baudrate = 100000u;
        params.bitRate = I2C_100kHz;
    }
    else if (self->baudrate > 100000U && self->baudrate <= 400000U) {
        self->baudrate = 400000u;
        params.bitRate = I2C_400kHz;
    }
    else if (self->baudrate > 400000U) {
        self->baudrate = 1000000u;
        params.bitRate = I2C_1000kHz;
    }

    if (self->i2c) {
        I2C_close(self->i2c);
    }

    if ((self->i2c = I2C_open(self->id, &params)) == NULL) {
        mp_raise_OSError(MP_ENODEV);
    }
}

STATIC mp_obj_t machine_i2c_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);
    mp_int_t id = mp_obj_get_int(args[0]);
    if (id < 0 || id >= NUM_I2C) {
        mp_raise_OSError(MP_ENODEV);
    }
    machine_i2c_obj_t *self = &i2c_obj[id];

    if (n_args > 1 || n_kw > 0 || !self->i2c) {
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        i2c_init_helper(self, n_args - 1, args + 1, &kw_args);
    }

    return MP_OBJ_FROM_PTR(self);
}

STATIC void machine_i2c_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_i2c_obj_t *self = self_in;
    mp_printf(print, "<machine_i2c.%p> id=%u baudrate=%u", self, self->id,
              self->baudrate);
}

STATIC mp_obj_t machine_i2c_init(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    machine_i2c_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    i2c_init_helper(self, n_args - 1, pos_args + 1, kw_args);

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_i2c_init_obj, 1, machine_i2c_init);

STATIC mp_obj_t machine_i2c_scan(mp_obj_t self_in) {
    machine_i2c_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_obj_t list = mp_obj_new_list(0, NULL);
    I2C_Transaction trans;
    uint8_t data = 0;

    trans.writeBuf = &data;
    trans.writeCount = 1;
    trans.readBuf = &data;
    trans.readCount = 0;

    // 7-bit addresses 0b0000xxx and 0b1111xxx are reserved
    for (int addr = 0x08; addr < 0x78; ++addr) {
        trans.slaveAddress = addr;
        if (I2C_transfer(self->i2c, &trans)) {
            mp_obj_list_append(list, MP_OBJ_NEW_SMALL_INT(addr));
        }
    }

    return list;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_i2c_scan_obj, machine_i2c_scan);

static bool i2c_readfrom(machine_i2c_obj_t * self, mp_int_t addr, uint8_t * buf, mp_int_t len) {
    I2C_Transaction trans;

    trans.slaveAddress = addr;
    trans.writeBuf = NULL;
    trans.writeCount = 0;
    trans.readBuf = buf;
    trans.readCount = len;

    return I2C_transfer(self->i2c, &trans);
}

STATIC mp_obj_t machine_i2c_readfrom(size_t n_args, const mp_obj_t *args) {
    machine_i2c_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_int_t addr = mp_obj_get_int(args[1]);
    mp_int_t nbytes = mp_obj_get_int(args[2]);
    vstr_t vstr;
    vstr_init_len(&vstr, nbytes);

    if (!i2c_readfrom(self, addr, (uint8_t *)vstr.buf, nbytes)) {
        vstr_clear(&vstr);
        mp_raise_OSError(MP_EIO);
    }

    return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_i2c_readfrom_obj, 3, 4, machine_i2c_readfrom);

STATIC mp_obj_t machine_i2c_readfrom_into(size_t n_args, const mp_obj_t *args) {
    machine_i2c_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_int_t addr = mp_obj_get_int(args[1]);
    mp_buffer_info_t buf_info;
    mp_get_buffer_raise(args[2], &buf_info, MP_BUFFER_WRITE);

    if (!i2c_readfrom(self, addr, (uint8_t *)buf_info.buf, buf_info.len)) {
        mp_raise_OSError(MP_EIO);
    }

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_i2c_readfrom_into_obj, 3, 4, machine_i2c_readfrom_into);

STATIC mp_obj_t machine_i2c_writeto(size_t n_args, const mp_obj_t *args) {
    machine_i2c_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_int_t addr = mp_obj_get_int(args[1]);
    mp_buffer_info_t buf_info;
    mp_get_buffer_raise(args[2], &buf_info, MP_BUFFER_READ);

    I2C_Transaction trans;
    trans.slaveAddress = addr;
    trans.writeBuf = buf_info.buf;
    trans.writeCount = buf_info.len;
    trans.readBuf = NULL;
    trans.readCount = 0;
    if (!I2C_transfer(self->i2c, &trans)) {
        mp_raise_OSError(MP_EIO);
    }

    return MP_OBJ_NEW_SMALL_INT(buf_info.len);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_i2c_writeto_obj, 3, 4, machine_i2c_writeto);

static bool i2c_readfrom_mem(machine_i2c_obj_t * self, mp_int_t addr, mp_int_t memaddr, uint8_t * buf, mp_int_t len) {
    I2C_Transaction trans;
    uint8_t mem[4];
    mem[0] = (uint8_t)memaddr;

    trans.slaveAddress = addr;
    trans.writeBuf = mem;
    trans.writeCount = 1;
    trans.readBuf = buf;
    trans.readCount = len;

    return I2C_transfer(self->i2c, &trans);
}

STATIC mp_obj_t machine_i2c_readfrom_mem(size_t n_args, const mp_obj_t *args) {
    machine_i2c_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_int_t addr = mp_obj_get_int(args[1]);
    mp_int_t memaddr = mp_obj_get_int(args[2]);
    mp_int_t nbytes = mp_obj_get_int(args[3]);
    vstr_t vstr;
    vstr_init_len(&vstr, nbytes);

    if (!i2c_readfrom_mem(self, addr, memaddr, (uint8_t *)vstr.buf, nbytes)) {
        vstr_clear(&vstr);
        mp_raise_OSError(MP_EIO);
    }

    return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_i2c_readfrom_mem_obj, 4, 4, machine_i2c_readfrom_mem);

STATIC mp_obj_t machine_i2c_readfrom_mem_into(size_t n_args, const mp_obj_t *args) {
    machine_i2c_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_int_t addr = mp_obj_get_int(args[1]);
    mp_int_t memaddr = mp_obj_get_int(args[2]);
    mp_buffer_info_t buf_info;
    mp_get_buffer_raise(args[3], &buf_info, MP_BUFFER_WRITE);

    if (!i2c_readfrom_mem(self, addr, memaddr, (uint8_t *)buf_info.buf,
                          buf_info.len)) {
        mp_raise_OSError(MP_EIO);
    }

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_i2c_readfrom_mem_into_obj, 4, 4, machine_i2c_readfrom_mem_into);

#define MAX_MEMADDR_SIZE (4)
#define BUF_STACK_SIZE (12)

STATIC mp_obj_t machine_i2c_writeto_mem(size_t n_args, const mp_obj_t *args) {
    machine_i2c_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_int_t addr = mp_obj_get_int(args[1]);
    mp_int_t memaddr = mp_obj_get_int(args[2]);
    mp_buffer_info_t buf_info;
    mp_get_buffer_raise(args[3], &buf_info, MP_BUFFER_READ);
    size_t len = buf_info.len;

    // need some memory to create the buffer to send; try to use stack if possible
    uint8_t buf2_stack[MAX_MEMADDR_SIZE + BUF_STACK_SIZE];
    uint8_t *buf2;
    size_t buf2_alloc = 0;
    if (len <= BUF_STACK_SIZE) {
        buf2 = buf2_stack;
    } else {
        buf2_alloc = MAX_MEMADDR_SIZE + len;
        buf2 = m_new(uint8_t, buf2_alloc);
    }

    // create the buffer to send
    uint32_t addrsize = memaddr > 255 ? 16 : 8;
    size_t memaddr_len = 0;
    for (int16_t i = addrsize - 8; i >= 0; i -= 8) {
        buf2[memaddr_len++] = memaddr >> i;
    }
    memcpy(buf2 + memaddr_len, buf_info.buf, len);

    I2C_Transaction trans;
    trans.slaveAddress = addr;
    trans.writeBuf = buf2;
    trans.writeCount = memaddr_len + len;
    trans.readBuf = NULL;
    trans.readCount = 0;
    bool status = I2C_transfer(self->i2c, &trans);

    if (buf2_alloc != 0) {
        m_del(uint8_t, buf2, buf2_alloc);
    }

    if (!status) {
        mp_raise_OSError(MP_EIO);
    }

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_i2c_writeto_mem_obj, 4, 4, machine_i2c_writeto_mem);

STATIC const mp_rom_map_elem_t machine_i2c_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_i2c_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_scan), MP_ROM_PTR(&machine_i2c_scan_obj) },
    { MP_ROM_QSTR(MP_QSTR_readfrom), MP_ROM_PTR(&machine_i2c_readfrom_obj) },
    { MP_ROM_QSTR(MP_QSTR_readfrom_into), MP_ROM_PTR(&machine_i2c_readfrom_into_obj) },
    { MP_ROM_QSTR(MP_QSTR_writeto), MP_ROM_PTR(&machine_i2c_writeto_obj) },
    { MP_ROM_QSTR(MP_QSTR_readfrom_mem), MP_ROM_PTR(&machine_i2c_readfrom_mem_obj) },
    { MP_ROM_QSTR(MP_QSTR_readfrom_mem_into), MP_ROM_PTR(&machine_i2c_readfrom_mem_into_obj) },
    { MP_ROM_QSTR(MP_QSTR_writeto_mem), MP_ROM_PTR(&machine_i2c_writeto_mem_obj) },
#ifdef MACHINE_I2C_IDS
    MACHINE_I2C_IDS
#endif
};

STATIC MP_DEFINE_CONST_DICT(machine_i2c_locals_dict, machine_i2c_locals_dict_table);

const mp_obj_type_t machine_i2c_type = {
    { &mp_type_type },
    .name = MP_QSTR_I2C,
    .make_new = machine_i2c_make_new,
    .print = machine_i2c_print,
    .locals_dict = (mp_obj_dict_t*)&machine_i2c_locals_dict,
};
