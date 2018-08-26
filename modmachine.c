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

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <xdc/runtime/Memory.h>

#include <SoC.h>

#include "extmod/machine_mem.h"

#include "machine_adc.h"
#include "machine_i2c.h"
#include "machine_pin.h"
#include "machine_pwm.h"
#include "machine_rtc.h"
#include "machine_spi.h"
#include "machine_uart.h"

#include "machine_nvsbdev.h"
#include "machine_sd.h"

#if MICROPY_HW_HAS_NEOPIX
#include "neopix.h"
#endif

#define MACHINE_IDLE (1)
#define MACHINE_SLEEP (2)
#define MACHINE_DEEPSLEEP (3)
#define MACHINE_PWRON_RESET (10)
#define MACHINE_HARD_RESET (11)
#define MACHINE_WDT_RESET (12)
#define MACHINE_DEEPSLEEP_RESET (13)
#define MACHINE_SOFT_RESET (14)
#define MACHINE_WLAN_WAKE (20)
#define MACHINE_PIN_WAKE (21)
#define MACHINE_RTC_WAKE (22)

#if MICROPY_PY_MACHINE

// Reset functions not in SDKs yet so provide dummy implementations
__attribute__(( weak )) void SoC_reset(void) {
}

__attribute__(( weak )) uint32_t SoC_getResetCause(void) {
    return 0u;
}

Semaphore_Handle machine_sleep_sem;

/* helper function used in mp_main() to cleanup on a soft MP reboot */
void machine_teardown(void) {
    machine_pin_teardown();
    machine_i2c_teardown();
    machine_spi_teardown();
    machine_uart_teardown();
    machine_pwm_teardown();
    machine_adc_teardown();
    machine_rtc_teardown();
//TODO: Teardown flash_bdev
    MACHINE_SD_TEARDOWN();

    if (machine_sleep_sem) {
        Semaphore_delete(&machine_sleep_sem);
    }
}

void machine_setup(void) {
    if (!machine_sleep_sem) {
        Semaphore_Params params;
        Semaphore_Params_init(&params);
        params.mode = Semaphore_Mode_BINARY;
        machine_sleep_sem = Semaphore_create(0, &params, NULL);
    }
}

STATIC mp_obj_t machine_sleep() {
    Semaphore_pend(machine_sleep_sem, BIOS_WAIT_FOREVER);
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(machine_sleep_obj, machine_sleep);

STATIC mp_obj_t machine_reset_cause() {
    uint32_t cause = SoC_getResetCause();

    if (cause == 0) {
        mp_raise_OSError(MP_EOPNOTSUPP);
    }
    else {
        switch (cause) {
        case SOC_RESET_POWER:
            cause = MACHINE_PWRON_RESET;
            break;
        case SOC_RESET_SLEEP:
            cause = MACHINE_DEEPSLEEP_RESET;
            break;
        case SOC_RESET_HARD:
            cause = MACHINE_HARD_RESET;
            break;
        case SOC_RESET_SOFT:
            cause = MACHINE_SOFT_RESET;
            break;
        case SOC_RESET_WDT:
            cause = MACHINE_WDT_RESET;
            break;
        default:
            cause = ~0u;
        }
    }

    mp_obj_t result = MP_OBJ_NEW_SMALL_INT(cause);

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(machine_reset_cause_obj, machine_reset_cause);

STATIC mp_obj_t machine_enable_irq(mp_obj_t state_in) {
    mp_raise_OSError(MP_EOPNOTSUPP);
    // mp_int_t state = mp_obj_get_int(state_in);

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_enable_irq_obj, machine_enable_irq);

STATIC mp_obj_t machine_freq() {
    xdc_runtime_Types_FreqHz freqHz;
    BIOS_getCpuFreq(&freqHz);

    return mp_obj_new_int(freqHz.lo);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(machine_freq_obj, machine_freq);

STATIC mp_obj_t machine_time_pulse_us(size_t n_args, const mp_obj_t *args) {
    // TODO: missing or unhandled arg type
    /*
    mp_obj_t pin = args[0];
    mp_int_t pulse_level = mp_obj_get_int(args[1]);
    mp_int_t timeout_us = 1000000;
    if (n_args > 2) {
        timeout_us = mp_obj_get_int(args[2]);
    };
    */

    mp_raise_OSError(MP_EOPNOTSUPP);

    mp_obj_t result = MP_OBJ_NEW_SMALL_INT(0);

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_time_pulse_us_obj, 2, 3, machine_time_pulse_us);

STATIC mp_obj_t machine_disable_irq() {
    mp_raise_OSError(MP_EOPNOTSUPP);

    mp_obj_t result = MP_OBJ_NEW_SMALL_INT(0);

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(machine_disable_irq_obj, machine_disable_irq);

STATIC mp_obj_t machine_heap_info() {
    Memory_Stats mem;
    Memory_getStats(0, &mem);
    mp_obj_t heap[3] = {
        mp_obj_new_int(mem.totalSize),
        mp_obj_new_int(mem.totalFreeSize),
        mp_obj_new_int(mem.largestFreeSize),
    };

    return MP_OBJ_TO_PTR(mp_obj_new_tuple(3, heap));
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(machine_heap_info_obj, machine_heap_info);

STATIC mp_obj_t machine_reset() {
    SoC_reset();

    /* if reset() is not implemented, falls thru to here */
    mp_raise_OSError(MP_EOPNOTSUPP);

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(machine_reset_obj, machine_reset);

STATIC mp_obj_t machine_unique_id() {
    mp_obj_t result = mp_obj_new_str("result", 6);

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(machine_unique_id_obj, machine_unique_id);

STATIC mp_obj_t machine_deepsleep() {
    mp_raise_OSError(MP_EOPNOTSUPP);

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(machine_deepsleep_obj, machine_deepsleep);

STATIC mp_obj_t machine_idle() {
    mp_raise_OSError(MP_EOPNOTSUPP);

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(machine_idle_obj, machine_idle);

STATIC const mp_rom_map_elem_t machine_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_machine) },
    { MP_ROM_QSTR(MP_QSTR_sleep), MP_ROM_PTR(&machine_sleep_obj) },
    { MP_ROM_QSTR(MP_QSTR_reset_cause), MP_ROM_PTR(&machine_reset_cause_obj) },
    { MP_ROM_QSTR(MP_QSTR_enable_irq), MP_ROM_PTR(&machine_enable_irq_obj) },
    { MP_ROM_QSTR(MP_QSTR_freq), MP_ROM_PTR(&machine_freq_obj) },
    { MP_ROM_QSTR(MP_QSTR_time_pulse_us), MP_ROM_PTR(&machine_time_pulse_us_obj) },
    { MP_ROM_QSTR(MP_QSTR_disable_irq), MP_ROM_PTR(&machine_disable_irq_obj) },
    { MP_ROM_QSTR(MP_QSTR_heap_info), MP_ROM_PTR(&machine_heap_info_obj) },
    { MP_ROM_QSTR(MP_QSTR_reset), MP_ROM_PTR(&machine_reset_obj) },
    { MP_ROM_QSTR(MP_QSTR_unique_id), MP_ROM_PTR(&machine_unique_id_obj) },
    { MP_ROM_QSTR(MP_QSTR_deepsleep), MP_ROM_PTR(&machine_deepsleep_obj) },
    { MP_ROM_QSTR(MP_QSTR_idle), MP_ROM_PTR(&machine_idle_obj) },
    { MP_ROM_QSTR(MP_QSTR_RTC_WAKE), MP_ROM_INT(MACHINE_RTC_WAKE) },
    { MP_ROM_QSTR(MP_QSTR_PIN_WAKE), MP_ROM_INT(MACHINE_PIN_WAKE) },
    { MP_ROM_QSTR(MP_QSTR_PWRON_RESET), MP_ROM_INT(MACHINE_PWRON_RESET) },
    { MP_ROM_QSTR(MP_QSTR_HARD_RESET), MP_ROM_INT(MACHINE_HARD_RESET) },
    { MP_ROM_QSTR(MP_QSTR_WDT_RESET), MP_ROM_INT(MACHINE_WDT_RESET) },
    { MP_ROM_QSTR(MP_QSTR_IDLE), MP_ROM_INT(MACHINE_IDLE) },
    { MP_ROM_QSTR(MP_QSTR_DEEPSLEEP), MP_ROM_INT(MACHINE_DEEPSLEEP) },
    { MP_ROM_QSTR(MP_QSTR_DEEPSLEEP_RESET), MP_ROM_INT(MACHINE_DEEPSLEEP_RESET) },
    { MP_ROM_QSTR(MP_QSTR_WLAN_WAKE), MP_ROM_INT(MACHINE_WLAN_WAKE) },
    { MP_ROM_QSTR(MP_QSTR_SOFT_RESET), MP_ROM_INT(MACHINE_SOFT_RESET) },
    { MP_ROM_QSTR(MP_QSTR_SLEEP), MP_ROM_INT(MACHINE_SLEEP) },

    { MP_ROM_QSTR(MP_QSTR_mem8), MP_ROM_PTR(&machine_mem8_obj) },
    { MP_ROM_QSTR(MP_QSTR_mem16), MP_ROM_PTR(&machine_mem16_obj) },
    { MP_ROM_QSTR(MP_QSTR_mem32), MP_ROM_PTR(&machine_mem32_obj) },
    
    { MP_ROM_QSTR(MP_QSTR_Pin), MP_ROM_PTR(&machine_pin_type) },
    { MP_ROM_QSTR(MP_QSTR_I2C), MP_ROM_PTR(&machine_i2c_type) },
    { MP_ROM_QSTR(MP_QSTR_SPI), MP_ROM_PTR(&machine_spi_type) },
    { MP_ROM_QSTR(MP_QSTR_UART), MP_ROM_PTR(&machine_uart_type) },
    { MP_ROM_QSTR(MP_QSTR_PWM), MP_ROM_PTR(&machine_pwm_type) },
    { MP_ROM_QSTR(MP_QSTR_ADC), MP_ROM_PTR(&machine_adc_type) },
    { MP_ROM_QSTR(MP_QSTR_RTC), MP_ROM_PTR(&machine_rtc_type) },
    
#if MICROPY_HW_HAS_NEOPIX
    { MP_OBJ_NEW_QSTR(MP_QSTR_Neopix), (mp_obj_t)&pyb_neopix_type },
#endif
    MACHINE_SD_CLASS
};

STATIC MP_DEFINE_CONST_DICT(machine_module_globals, machine_module_globals_table);

const mp_obj_module_t machine_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&machine_module_globals,
};

#else

void machine_teardown(void)
{
}

#endif
