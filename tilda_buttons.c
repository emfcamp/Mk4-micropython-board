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
#include "py/mperrno.h"

#include "tilda_thread.h"
#include "tilda_buttons.h"

// ***LWK*** a lot of this should have the same functions as buttons.py
// https://github.com/emfcamp/Mk4-Apps/blob/master/lib/buttons.py

STATIC mp_obj_t tilda_buttons_get_all_states()
{
    return mp_obj_new_int(getAllButtonStates());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(tilda_buttons_get_all_states_obj, tilda_buttons_get_all_states);


STATIC mp_obj_t tilda_buttons_is_pressed(mp_obj_t button_in) //(button)
{
    TILDA_BUTTONS_Names button = mp_obj_get_int(button_in);
    if (button >= Buttons_MAX) {
        mp_raise_OSError(MP_ENODEV);
    }
    return getButtonState(button) ? mp_const_true : mp_const_false;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(tilda_buttons_is_pressed_obj, tilda_buttons_is_pressed);

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

//(button, interrupt, on_press = True, on_release = False)
STATIC mp_obj_t tilda_buttons_enable_interrupt(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    TILDA_BUTTONS_Names button = mp_obj_get_int(pos_args[0]);

    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_interrupt, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_on_press, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_on_release, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
    };
    enum { ARG_interrupt, ARG_on_press, ARG_on_release};
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
        MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    registerButtonCallback(button, args[ARG_interrupt].u_obj, args[ARG_on_press].u_int, args[ARG_on_release].u_int);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(tilda_buttons_enable_interrupt_obj, 2, tilda_buttons_enable_interrupt);


STATIC mp_obj_t tilda_buttons_disable_interrupt(mp_obj_t button_in) //(button)
{
    TILDA_BUTTONS_Names button = mp_obj_get_int(button_in);
    if (button >= Buttons_MAX) {
        mp_raise_OSError(MP_ENODEV);
    }
    unregisterButtonCallback(button);

    return mp_const_none;

}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(tilda_buttons_disable_interrupt_obj, tilda_buttons_disable_interrupt);


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
    { MP_ROM_QSTR(MP_QSTR_get_all_states), MP_ROM_PTR(&tilda_buttons_get_all_states_obj) },
    { MP_ROM_QSTR(MP_QSTR_is_pressed), MP_ROM_PTR(&tilda_buttons_is_pressed_obj) },
    { MP_ROM_QSTR(MP_QSTR_is_triggered), MP_ROM_PTR(&tilda_buttons_is_triggered_obj) },
    { MP_ROM_QSTR(MP_QSTR_has_interrupt), MP_ROM_PTR(&tilda_buttons_has_interrupt_obj) },
    { MP_ROM_QSTR(MP_QSTR_enable_interrupt), MP_ROM_PTR(&tilda_buttons_enable_interrupt_obj) },
    { MP_ROM_QSTR(MP_QSTR_disable_interrupt), MP_ROM_PTR(&tilda_buttons_disable_interrupt_obj) },
    { MP_ROM_QSTR(MP_QSTR_disable_all_interrupt), MP_ROM_PTR(&tilda_buttons_disable_all_interrupt_obj) },
    { MP_ROM_QSTR(MP_QSTR_enable_menu_reset), MP_ROM_PTR(&tilda_buttons_enable_menu_reset_obj) },
    { MP_ROM_QSTR(MP_QSTR_disable_menu_reset), MP_ROM_PTR(&tilda_buttons_disable_menu_reset_obj) },

    { MP_ROM_QSTR(MP_QSTR_JOY_Center), MP_ROM_INT(Buttons_JOY_Center) },
    { MP_ROM_QSTR(MP_QSTR_JOY_Up), MP_ROM_INT(Buttons_JOY_Up) },
    { MP_ROM_QSTR(MP_QSTR_JOY_Down), MP_ROM_INT(Buttons_JOY_Down) },
    { MP_ROM_QSTR(MP_QSTR_JOY_Left), MP_ROM_INT(Buttons_JOY_Left) },
    { MP_ROM_QSTR(MP_QSTR_JOY_Right), MP_ROM_INT(Buttons_JOY_Right) },
    { MP_ROM_QSTR(MP_QSTR_BTN_Menu), MP_ROM_INT(Buttons_BTN_Menu) },
    { MP_ROM_QSTR(MP_QSTR_BTN_1), MP_ROM_INT(Buttons_BTN_1) },
    { MP_ROM_QSTR(MP_QSTR_BTN_End), MP_ROM_INT(Buttons_BTN_End) },
    { MP_ROM_QSTR(MP_QSTR_BTN_2), MP_ROM_INT(Buttons_BTN_2) },
    { MP_ROM_QSTR(MP_QSTR_BTN_3), MP_ROM_INT(Buttons_BTN_3) },
    { MP_ROM_QSTR(MP_QSTR_BTN_6), MP_ROM_INT(Buttons_BTN_6) },
    { MP_ROM_QSTR(MP_QSTR_BTN_5), MP_ROM_INT(Buttons_BTN_5) },
    { MP_ROM_QSTR(MP_QSTR_BTN_4), MP_ROM_INT(Buttons_BTN_4) },
    { MP_ROM_QSTR(MP_QSTR_BTN_7), MP_ROM_INT(Buttons_BTN_7) },
    { MP_ROM_QSTR(MP_QSTR_BTN_8), MP_ROM_INT(Buttons_BTN_8) },
    { MP_ROM_QSTR(MP_QSTR_BTN_9), MP_ROM_INT(Buttons_BTN_9) },
    { MP_ROM_QSTR(MP_QSTR_BTN_Hash), MP_ROM_INT(Buttons_BTN_Hash) },
    { MP_ROM_QSTR(MP_QSTR_BTN_0), MP_ROM_INT(Buttons_BTN_0) },
    { MP_ROM_QSTR(MP_QSTR_BTN_Star), MP_ROM_INT(Buttons_BTN_Star) },
    { MP_ROM_QSTR(MP_QSTR_BTN_Call), MP_ROM_INT(Buttons_BTN_Call) },
    { MP_ROM_QSTR(MP_QSTR_BTN_A), MP_ROM_INT(Buttons_BTN_A) },
    { MP_ROM_QSTR(MP_QSTR_BTN_B), MP_ROM_INT(Buttons_BTN_B) },

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