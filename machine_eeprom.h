/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Texas Instruments Incorporated
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef MACHINE_EEPROM_H_INC
#define MACHINE_EEPROM_H_INC

#if MICROPY_MACHINE_TI_EEPROM
extern const mp_obj_type_t machine_eeprom_type;
extern void machine_eeprom_teardown(void);

#define MACHINE_EEPROM_CLASS { MP_ROM_QSTR(MP_QSTR_EEPROM), MP_ROM_PTR(&machine_eeprom_type) },
#define MACHINE_EEPROM_TEARDOWN() machine_eeprom

#else
#define MACHINE_EEPROM_CLASS
#define MACHINE_EEPROM_TEARDOWN()
#endif

extern const mp_obj_type_t machine_eeprom_type;
extern void machine_eeprom_teardown(void);

#endif
