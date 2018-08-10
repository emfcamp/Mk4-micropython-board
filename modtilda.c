/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdint.h>
#include <stdio.h>

#include "py/runtime.h"
#include "py/mphal.h"
#include "lib/utils/pyexec.h"
#include "led.h"

#include "modmachine.h"


MP_DECLARE_CONST_FUN_OBJ_KW(tilda_main_obj); // defined in mpmain.c

STATIC const mp_rom_map_elem_t tilda_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_tilda) },
    { MP_ROM_QSTR(MP_QSTR_main), MP_ROM_PTR(&tilda_main_obj) },

    #if MICROPY_HW_ENABLE_USB
    { MP_ROM_QSTR(MP_QSTR_usb_mode), MP_ROM_PTR(&pyb_usb_mode_obj) },
    { MP_ROM_QSTR(MP_QSTR_hid_mouse), MP_ROM_PTR(&pyb_usb_hid_mouse_obj) },
    { MP_ROM_QSTR(MP_QSTR_hid_keyboard), MP_ROM_PTR(&pyb_usb_hid_keyboard_obj) },
    { MP_ROM_QSTR(MP_QSTR_USB_VCP), MP_ROM_PTR(&pyb_usb_vcp_type) },
    { MP_ROM_QSTR(MP_QSTR_USB_HID), MP_ROM_PTR(&pyb_usb_hid_type) },
    #endif

    #if MICROPY_HW_HAS_FLASH
    { MP_ROM_QSTR(MP_QSTR_Flash), MP_ROM_PTR(&pyb_flash_type) },
    #endif

    #if defined(MICROPY_HW_LED1)
    { MP_ROM_QSTR(MP_QSTR_LED), MP_ROM_PTR(&tilda_led_type) },
    #endif
};

STATIC MP_DEFINE_CONST_DICT(tilda_module_globals, tilda_module_globals_table);

const mp_obj_module_t mp_module_tilda = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&tilda_module_globals,
};
