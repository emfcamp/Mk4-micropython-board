/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2015 Damien P. George
 * Copyright (c) 2018 Texas Instruments, Inc.
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

#include "py/obj.h"
#include "py/runtime.h"

#include <ti/sysbios/BIOS.h>
#include <xdc/runtime/Memory.h>

#include "modmachine.h"

#if MICROPY_PY_MACHINE

extern void machine_pin_teardown(void);
extern void machine_i2c_teardown(void);
extern void machine_spi_teardown(void);
extern void machine_uart_teardown(void);
extern void machine_sd_teardown(void);
extern void machine_pwm_teardown(void);

void machine_teardown(void) {
    machine_pin_teardown();
    machine_i2c_teardown();
    machine_spi_teardown();
    machine_uart_teardown();
    machine_sd_teardown();
    machine_pwm_teardown();
}

STATIC mp_obj_t machine_info(size_t n_args, const mp_obj_t *args) {
    xdc_runtime_Types_FreqHz freqHz;
    BIOS_getCpuFreq(&freqHz);
    mp_printf(&mp_plat_print, "CPU freq: %d\n", freqHz.lo);

    Memory_Stats mem;
    Memory_getStats(0, &mem);
    mp_printf(&mp_plat_print, "Heap: total=%d, free=%d, largest=%d\n",
              mem.totalSize, mem.totalFreeSize, mem.largestFreeSize);

    return mp_const_none;
}

MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_info_obj, 0, 1, machine_info);

STATIC mp_obj_t machine_freq(void) {
    xdc_runtime_Types_FreqHz freqHz;
    BIOS_getCpuFreq(&freqHz);

    return MP_OBJ_NEW_SMALL_INT(freqHz.lo);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(machine_freq_obj, machine_freq);

STATIC mp_obj_t machine_reset(void) {
    //DEBUG_printf("Warning: %s is not implemented\n", __func__);
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(machine_reset_obj, machine_reset);

STATIC mp_obj_t machine_reset_cause(void) {
    //DEBUG_printf("Warning: %s is not implemented\n", __func__);
    return MP_OBJ_NEW_SMALL_INT(42);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(machine_reset_cause_obj, machine_reset_cause);

STATIC mp_obj_t machine_unique_id(void) {
    byte id[12] = {0xde, 0xad, 0xbe, 0xef, 0xba, 0xbe, 0xca, 0xfe, 0x1e, 0xe7, 0xfe, 0xed};
    return mp_obj_new_bytes(id, 12);
}

MP_DEFINE_CONST_FUN_OBJ_0(machine_unique_id_obj, machine_unique_id);

STATIC const mp_rom_map_elem_t machine_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_umachine) },
    { MP_ROM_QSTR(MP_QSTR_freq), MP_ROM_PTR(&machine_freq_obj) },
    { MP_ROM_QSTR(MP_QSTR_info), MP_ROM_PTR(&machine_info_obj) },
    { MP_ROM_QSTR(MP_QSTR_reset), MP_ROM_PTR(&machine_reset_obj) },
    { MP_ROM_QSTR(MP_QSTR_reset_cause), MP_ROM_PTR(&machine_reset_cause_obj) },
    { MP_ROM_QSTR(MP_QSTR_unique_id), MP_ROM_PTR(&machine_unique_id_obj) },

    { MP_ROM_QSTR(MP_QSTR_Pin), MP_ROM_PTR(&machine_pin_type) },
    { MP_ROM_QSTR(MP_QSTR_I2C), MP_ROM_PTR(&machine_i2c_type) },
    { MP_ROM_QSTR(MP_QSTR_SPI), MP_ROM_PTR(&machine_spi_type) },
    { MP_ROM_QSTR(MP_QSTR_UART), MP_ROM_PTR(&machine_uart_type) },
    { MP_ROM_QSTR(MP_QSTR_SD), MP_ROM_PTR(&machine_sd_type) },
    { MP_ROM_QSTR(MP_QSTR_PWM), MP_ROM_PTR(&machine_pwm_type) },
#if MICROPY_PY_MACHINE_NVSBDEV
    { MP_ROM_QSTR(MP_QSTR_NVSBdev), MP_ROM_PTR(&machine_nvsbdev_type) },
#endif

    // reset causes
    /*{ MP_ROM_QSTR(MP_QSTR_PWRON_RESET), MP_ROM_INT(REASON_DEFAULT_RST) },*/
};

STATIC MP_DEFINE_CONST_DICT(machine_module_globals, machine_module_globals_table);

const mp_obj_module_t machine_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&machine_module_globals,
};

#endif // MICROPY_PY_MACHINE
