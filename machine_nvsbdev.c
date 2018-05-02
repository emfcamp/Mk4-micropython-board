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

#if MICROPY_PY_MACHINE_NVSBDEV
#include <ti/drivers/NVS.h>

typedef struct _machine_nvsbdev_obj_t {
    mp_obj_base_t base;
    NVS_Handle nvs;
    uint8_t id;
    NVS_Attrs attrs;
} machine_nvsbdev_obj_t;

STATIC mp_obj_t machine_nvsbdev_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);
    mp_int_t id = mp_obj_get_int(args[0]);
    machine_nvsbdev_obj_t *self = m_new_obj(machine_nvsbdev_obj_t);
    self->base.type = type;
    self->id = id;

    NVS_Params params;
    NVS_Params_init(&params);
    if ((self->nvs = NVS_open(self->id, &params))) {
        NVS_getAttrs(self->nvs, &self->attrs);
    }
    else {
        m_del_obj(machine_nvsbdev_obj_t, self);
        mp_raise_OSError(MP_ENODEV);
    }
    return MP_OBJ_FROM_PTR(self);
}

STATIC void machine_nvsbdev_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_nvsbdev_obj_t *self = self_in;
    mp_printf(print, "<machine_nvsbdev.%p> %u", self, self->id);
    if (self->nvs) {
        mp_printf(print, ", nvs=%p", (void *)self->nvs);
    }
    mp_printf(print, ")");
}

STATIC mp_obj_t machine_nvsbdev_readblocks(mp_obj_t self_in, mp_obj_t offset_in, mp_obj_t buf_in) {
    machine_nvsbdev_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t offset = mp_obj_get_int(offset_in);
    mp_buffer_info_t buf_info;
    mp_get_buffer_raise(buf_in, &buf_info, MP_BUFFER_WRITE);

    uint_fast16_t status = NVS_read(self->nvs, offset * self->attrs.sectorSize,
                                    (char *)buf_info.buf, buf_info.len);
    if (status != NVS_STATUS_SUCCESS) {
        mp_raise_OSError(MP_EIO);
    }

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(machine_nvsbdev_readblocks_obj, machine_nvsbdev_readblocks);

STATIC mp_obj_t machine_nvsbdev_writeblocks(mp_obj_t self_in, mp_obj_t offset_in, mp_obj_t buf_in) {
    machine_nvsbdev_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t offset = mp_obj_get_int(offset_in);
    mp_buffer_info_t buf_info;
    mp_get_buffer_raise(buf_in, &buf_info, MP_BUFFER_READ);

    uint_fast16_t status = NVS_write(self->nvs, offset * self->attrs.sectorSize,
                       buf_info.buf, buf_info.len, NVS_WRITE_POST_VERIFY);

    if (status != NVS_STATUS_SUCCESS) {
        mp_raise_OSError(MP_EIO);
    }

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(machine_nvsbdev_writeblocks_obj, machine_nvsbdev_writeblocks);

STATIC mp_obj_t machine_nvsbdev_ioctl(size_t n_args, const mp_obj_t *args) {
    machine_nvsbdev_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_int_t op = mp_obj_get_int(args[1]);
    // optional args in args[2++]

    switch (op) {
        case 4:
            return MP_OBJ_NEW_SMALL_INT(self->attrs.regionSize / self->attrs.sectorSize);
            break;
        case 5:
            return MP_OBJ_NEW_SMALL_INT(self->attrs.sectorSize);
            break;
        default:
            return mp_const_none;
    }

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(machine_nvsbdev_ioctl_obj, 2, machine_nvsbdev_ioctl);

STATIC const mp_rom_map_elem_t machine_nvsbdev_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_readblocks), MP_ROM_PTR(&machine_nvsbdev_readblocks_obj) },
    { MP_ROM_QSTR(MP_QSTR_writeblocks), MP_ROM_PTR(&machine_nvsbdev_writeblocks_obj) },
    { MP_ROM_QSTR(MP_QSTR_ioctl), MP_ROM_PTR(&machine_nvsbdev_ioctl_obj) },
};

STATIC MP_DEFINE_CONST_DICT(machine_nvsbdev_locals_dict, machine_nvsbdev_locals_dict_table);

const mp_obj_type_t machine_nvsbdev_type = {
    { &mp_type_type },
    .name = MP_QSTR_NVSBdev,
    .make_new = machine_nvsbdev_make_new,
    .print = machine_nvsbdev_print,
    .locals_dict = (mp_obj_dict_t*)&machine_nvsbdev_locals_dict,
};
#endif
