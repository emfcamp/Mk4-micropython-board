/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Texas Instruments Incorporated
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>
#include <stdbool.h>

#include "py/runtime.h"

#if MICROPY_MACHINE_TI_EEPROM

#include <ti/devices/msp432e4/driverlib/eeprom.h>

#define SYSCTL_PERIPH_EEPROM0   0xf0005800  // EEPROM 0

typedef struct _machine_eeprom_obj_t {
    mp_obj_base_t base;
} machine_eeprom_obj_t;

void machine_eeprom_teardown(void) {
}

STATIC mp_obj_t machine_eeprom_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, MP_OBJ_FUN_ARGS_MAX, true);
    // kwargs not handled
    machine_eeprom_obj_t *self = m_new_obj(machine_eeprom_obj_t);
    self->base.type = type;

    static bool init = false;
    if (!init) {
        // TODO: should be a Power dependency
        extern void SysCtlPeripheralEnable(uint32_t);
        SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
        if (EEPROMInit()) {
            mp_raise_msg(&mp_type_OSError, "EEPROM initialization failed");
        }
        init = true;
    }

    return MP_OBJ_FROM_PTR(self);
}

STATIC void machine_eeprom_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_eeprom_obj_t *self = self_in;
    mp_printf(print, "<machine_eeprom.%p>", self);
}

STATIC mp_obj_t machine_eeprom_erase(mp_obj_t self_in) {
    (void)self_in;
    uint32_t status;
    if ((status = EEPROMMassErase())) {
        mp_raise_msg(&mp_type_OSError, "EEPROM mass erase failed");
    }
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_eeprom_erase_obj, machine_eeprom_erase);

STATIC mp_obj_t machine_eeprom_get_block_count(mp_obj_t self_in) {
    (void)self_in;
    return MP_OBJ_NEW_SMALL_INT(EEPROMBlockCountGet());
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_eeprom_get_block_count_obj, machine_eeprom_get_block_count);

STATIC mp_obj_t machine_eeprom_get_block_size(mp_obj_t self_in) {
    (void)self_in;
    return MP_OBJ_NEW_SMALL_INT(EEPROMSizeGet());
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_eeprom_get_block_size_obj, machine_eeprom_get_block_size);

STATIC mp_obj_t machine_eeprom_read(size_t n_args, const mp_obj_t *args) {
    /* self not used in args[0]  */
    mp_int_t address = mp_obj_get_int(args[1]);
    mp_int_t size = 1;
    if (n_args > 2) {
        size = mp_obj_get_int(args[2]);
    };

    if (address & 0x3) {
        mp_raise_msg(&mp_type_OSError, "address must be multiple of 4");
    }

    if (size & 0x3) {
        mp_raise_msg(&mp_type_OSError, "size must be multiple of 4");
    }

    vstr_t vstr;
    vstr_init_len(&vstr, size);
    EEPROMRead((uint32_t *)vstr.buf, address, size);
    return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_eeprom_read_obj, 2, 3, machine_eeprom_read);

STATIC mp_obj_t machine_eeprom_write(mp_obj_t self_in, mp_obj_t src_in, mp_obj_t address_in) {
    /* self not used in args[0]  */
    mp_buffer_info_t src_info;
    mp_get_buffer_raise(src_in, &src_info, MP_BUFFER_READ);
    mp_int_t address = mp_obj_get_int(address_in);

    if (address & 0x3) {
        mp_raise_msg(&mp_type_OSError, "address must be multiple of 4");
    }

    if (src_info.len & 0x3) {
        mp_raise_msg(&mp_type_OSError, "size of buffer must be multiple of 4");
    }

    EEPROMProgram(src_info.buf, address, src_info.len);

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(machine_eeprom_write_obj, machine_eeprom_write);

STATIC const mp_rom_map_elem_t machine_eeprom_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_erase), MP_ROM_PTR(&machine_eeprom_erase_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_block_count), MP_ROM_PTR(&machine_eeprom_get_block_count_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_block_size), MP_ROM_PTR(&machine_eeprom_get_block_size_obj) },
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&machine_eeprom_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&machine_eeprom_write_obj) },
};

STATIC MP_DEFINE_CONST_DICT(machine_eeprom_locals_dict, machine_eeprom_locals_dict_table);

const mp_obj_type_t machine_eeprom_type = {
    { &mp_type_type },
    .name = MP_QSTR_EEPROM,
    .make_new = machine_eeprom_make_new,
    .print = machine_eeprom_print,
    .locals_dict = (mp_obj_dict_t*)&machine_eeprom_locals_dict,
};

#endif
