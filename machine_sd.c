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
#include "py/nlr.h"
#include "py/mperrno.h"
#include "extmod/vfs.h"

#if MICROPY_MACHINE_SD

#include <ti/drivers/SD.h>

typedef struct _machine_sd_obj_t {
    mp_obj_base_t base;
    SD_Handle sd;
    uint8_t id;
    uint32_t numSectors;
    uint32_t sectorSize;
} machine_sd_obj_t;

static machine_sd_obj_t sd_obj;

void machine_sd_teardown(void) {
    if (sd_obj.sd) {
        SD_close(sd_obj.sd);
        sd_obj.sd = NULL;
    }
}

STATIC mp_obj_t machine_sd_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);
    mp_int_t id = mp_obj_get_int(args[0]);
    // kwargs not handled
    machine_sd_obj_t *self = &sd_obj; // m_new_obj(machine_sd_obj_t);
    self->base.type = type;

    if ((self->sd = SD_open(id, NULL))) {
        self->id = id;
        SD_initialize(self->sd);
        self->numSectors = SD_getNumSectors(self->sd);
        self->sectorSize = SD_getSectorSize(self->sd);
    }
    else {
        mp_raise_OSError(MP_ENODEV);
    }

    return MP_OBJ_FROM_PTR(self);
}

STATIC void machine_sd_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_sd_obj_t *self = self_in;
    mp_printf(print, "<machine_sd.%p>", self);
}

STATIC mp_obj_t machine_sd_init(mp_obj_t self_in) {
    // machine_sd_obj_t *self = MP_OBJ_TO_PTR(self_in);

    mp_obj_t result = mp_const_none;

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_sd_init_obj, machine_sd_init);

STATIC mp_obj_t machine_sd_deinit(mp_obj_t self_in) {
    // machine_sd_obj_t *self = MP_OBJ_TO_PTR(self_in);

    mp_obj_t result = mp_const_none;

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_sd_deinit_obj, machine_sd_deinit);

STATIC mp_obj_t machine_sd_readblocks(mp_obj_t self_in, mp_obj_t offset_in, mp_obj_t len_or_buf) {
    machine_sd_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t offset = mp_obj_get_int(offset_in);
    mp_int_t len;
    byte *buf;
    bool alloc_buf = MP_OBJ_IS_INT(len_or_buf);

    if (alloc_buf) {
        len = mp_obj_get_int(len_or_buf);
        buf = m_new(byte, len);
    }
    else {
        mp_buffer_info_t buf_info;
        mp_get_buffer_raise(len_or_buf, &buf_info, MP_BUFFER_WRITE);
        len = buf_info.len;
        buf = buf_info.buf;
    }

    int_fast16_t status = SD_read(self->sd, buf, offset,
                                  len / self->sectorSize);

    mp_obj_t result = mp_const_none;
    if (status == SD_STATUS_SUCCESS) {
        if (alloc_buf) {
            result = mp_obj_new_bytes(buf, len);
        }
    }
    else {
        if (alloc_buf) {
            m_del(byte, buf, len);
        }
        mp_raise_OSError(MP_EIO);
    }

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(machine_sd_readblocks_obj, machine_sd_readblocks);

STATIC mp_obj_t machine_sd_eraseblock(mp_obj_t self_in, mp_obj_t block_in) {
    /* TODO: is this needed */
    mp_raise_OSError(MP_EIO);

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(machine_sd_eraseblock_obj, machine_sd_eraseblock);

STATIC mp_obj_t machine_sd_writeblocks(mp_obj_t self_in, mp_obj_t offset_in, mp_obj_t buf_in) {
    machine_sd_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t offset = mp_obj_get_int(offset_in);
    mp_buffer_info_t buf_info;
    mp_get_buffer_raise(buf_in, &buf_info, MP_BUFFER_READ);

    if (buf_info.len & 0x3) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError,
                                           "len must be multiple of 4"));
    }

    int_fast16_t status = SD_write(self->sd, buf_info.buf, offset,
                                   buf_info.len / self->sectorSize);

    if (status != SD_STATUS_SUCCESS) {
        mp_raise_OSError(MP_EIO);
    }

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(machine_sd_writeblocks_obj, machine_sd_writeblocks);

STATIC mp_obj_t machine_sd_ioctl(size_t n_args, const mp_obj_t *args) {
    machine_sd_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_int_t op = mp_obj_get_int(args[1]);
    // optional args in args[2++]
    mp_obj_t result = mp_const_none;

    switch (op) {
        case BP_IOCTL_SEC_COUNT:
            result = MP_OBJ_NEW_SMALL_INT(self->numSectors);
            break;
        case BP_IOCTL_SEC_SIZE:
            result = MP_OBJ_NEW_SMALL_INT(self->sectorSize);
            break;
        case BP_IOCTL_INIT:
        case BP_IOCTL_DEINIT:
        case BP_IOCTL_SYNC:
        default:
            break;
    }

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(machine_sd_ioctl_obj, 2, machine_sd_ioctl);

STATIC const mp_rom_map_elem_t machine_sd_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_sd_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&machine_sd_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_readblocks), MP_ROM_PTR(&machine_sd_readblocks_obj) },
    { MP_ROM_QSTR(MP_QSTR_eraseblock), MP_ROM_PTR(&machine_sd_eraseblock_obj) },
    { MP_ROM_QSTR(MP_QSTR_writeblocks), MP_ROM_PTR(&machine_sd_writeblocks_obj) },
    { MP_ROM_QSTR(MP_QSTR_ioctl), MP_ROM_PTR(&machine_sd_ioctl_obj) },
};

STATIC MP_DEFINE_CONST_DICT(machine_sd_locals_dict, machine_sd_locals_dict_table);

const mp_obj_type_t machine_sd_type = {
    { &mp_type_type },
    .name = MP_QSTR_SD,
    .make_new = machine_sd_make_new,
    .print = machine_sd_print,
    .locals_dict = (mp_obj_dict_t*)&machine_sd_locals_dict,
};

#endif
