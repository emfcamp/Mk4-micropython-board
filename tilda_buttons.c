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

#include "py/nlr.h"
#include "py/runtime.h"

#include "i2c_thread.h"
#include "tilda_buttons.h"

// ***LWK*** a lot of this should have the same functions as buttons.py
// https://github.com/emfcamp/Mk4-Apps/blob/master/lib/buttons.py


STATIC mp_obj_t tilda_buttons_is_pressed() //(button)
{

    // if id ison GPIO
        // direct gpio read
    // if id ison expander
        // read from i2cSharedStates

    return mp_const_none; //true/false
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(tilda_buttons_is_pressed_obj, tilda_buttons_is_pressed);

STATIC mp_obj_t tilda_buttons_is_triggered() //(button)
{
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(tilda_buttons_is_triggered_obj, tilda_buttons_is_triggered);

STATIC mp_obj_t tilda_buttons_has_interrupt() //(button)
{
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(tilda_buttons_has_interrupt_obj, tilda_buttons_has_interrupt);


STATIC mp_obj_t tilda_buttons_enable_interrupt() //(button, interrupt, on_press = True, on_release = False)
{
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(tilda_buttons_enable_interrupt_obj, tilda_buttons_enable_interrupt);


STATIC mp_obj_t tilda_buttons_disable_interrupt() //(button)
{
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(tilda_buttons_disable_interrupt_obj, tilda_buttons_disable_interrupt);


STATIC mp_obj_t tilda_buttons_disable_all_interrupt()
{
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(tilda_buttons_disable_all_interrupt_obj, tilda_buttons_disable_all_interrupt);


STATIC mp_obj_t tilda_buttons_enable_menu_reset()
{
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(tilda_buttons_enable_menu_reset_obj, tilda_buttons_enable_menu_reset);


STATIC mp_obj_t tilda_buttons_disable_menu_reset()
{
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(tilda_buttons_disable_menu_reset_obj, tilda_buttons_disable_menu_reset);


STATIC const mp_rom_map_elem_t tilda_buttons_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_is_pressed), MP_ROM_PTR(&tilda_buttons_is_pressed_obj) },
    { MP_ROM_QSTR(MP_QSTR_is_triggered), MP_ROM_PTR(&tilda_buttons_is_triggered_obj) },
    { MP_ROM_QSTR(MP_QSTR_has_interrupt), MP_ROM_PTR(&tilda_buttons_has_interrupt_obj) },
    { MP_ROM_QSTR(MP_QSTR_enable_interrupt), MP_ROM_PTR(&tilda_buttons_enable_interrupt_obj) },
    { MP_ROM_QSTR(MP_QSTR_disable_interrupt), MP_ROM_PTR(&tilda_buttons_disable_interrupt_obj) },
    { MP_ROM_QSTR(MP_QSTR_disable_all_interrupt), MP_ROM_PTR(&tilda_buttons_disable_all_interrupt_obj) },
    { MP_ROM_QSTR(MP_QSTR_enable_menu_reset), MP_ROM_PTR(&tilda_buttons_enable_menu_reset_obj) },
    { MP_ROM_QSTR(MP_QSTR_disable_menu_reset), MP_ROM_PTR(&tilda_buttons_disable_menu_reset_obj) },

    { MP_ROM_QSTR(MP_QSTR_BTN_1), MP_ROM_INT(0) },
    { MP_ROM_QSTR(MP_QSTR_BTN_2), MP_ROM_INT(1) },
    { MP_ROM_QSTR(MP_QSTR_BTN_3), MP_ROM_INT(2) },
    { MP_ROM_QSTR(MP_QSTR_BTN_4), MP_ROM_INT(3) },
    { MP_ROM_QSTR(MP_QSTR_BTN_5), MP_ROM_INT(4) },
    { MP_ROM_QSTR(MP_QSTR_BTN_6), MP_ROM_INT(5) },
    { MP_ROM_QSTR(MP_QSTR_BTN_Call), MP_ROM_INT(6) },

};

STATIC MP_DEFINE_CONST_DICT (
    tilda_buttons_locals_dict,
    tilda_buttons_locals_dict_table
);

const mp_obj_type_t tilda_buttons_type = {
    { &mp_type_type },
    .name = MP_QSTR_Buttons,
    .locals_dict = (mp_obj_dict_t*)&tilda_buttons_locals_dict,
};