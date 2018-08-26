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

#include "tilda_thread.h"
#include "tilda_sensors.h"

STATIC mp_obj_t tilda_sensors_sample_rate(size_t n_args, const mp_obj_t *args)
{
    if (n_args == 0) {
        return MP_OBJ_NEW_SMALL_INT(tildaSharedStates.sampleRate);
    }

    int newSampleRate = mp_obj_get_int(args[0]);
    tildaSharedStates.sampleRate = newSampleRate;

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(tilda_sensors_sample_rate_obj, 0, tilda_sensors_sample_rate);

#define vbus_state(r) (((r) & 0xe0u) >> 5u)
#define NO_INPUT 0u
#define USB_HOST 1u
#define ADAPTER_24 2u
#define OTG 7u

STATIC mp_obj_t tilda_sensors_get_vbus_connected()
{
    mp_obj_t result = mp_const_none;
    uint8_t reg08 = tildaSharedStates.bqRegs[8];

    switch (vbus_state(reg08)) {
    case NO_INPUT:
        result = MP_OBJ_NEW_SMALL_INT(NO_INPUT);
        break;

    case USB_HOST:
        result = MP_OBJ_NEW_SMALL_INT(USB_HOST);
        break;

    case ADAPTER_24:
        result = MP_OBJ_NEW_SMALL_INT(ADAPTER_24);
        break;

    case OTG:
        result = MP_OBJ_NEW_SMALL_INT(OTG);
        break;
    }

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(tilda_sensors_get_vbus_connected_obj, tilda_sensors_get_vbus_connected);

STATIC mp_obj_t tilda_sensors_raw_bq()
{
    return mp_obj_new_bytes((const byte *)tildaSharedStates.bqRegs,
                            sizeof(tildaSharedStates.bqRegs));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(tilda_sensors_raw_bq_obj, tilda_sensors_raw_bq);

#define charge_state(r) (((r) & 0x18u) >> 3u)
#define NOT_CHARGING 0u
#define PRE_CHARGING 1u
#define FAST_CHARGING 2u
#define DONE_CHARGING 3u

STATIC mp_obj_t tilda_sensors_get_charge_status()
{
    mp_obj_t result = mp_const_none;
    uint8_t reg08 = tildaSharedStates.bqRegs[8];

    switch (charge_state(reg08)) {
    case NOT_CHARGING:
        result = MP_OBJ_NEW_SMALL_INT(NOT_CHARGING);
        break;

    case PRE_CHARGING:
        result = MP_OBJ_NEW_SMALL_INT(PRE_CHARGING);
        break;

    case FAST_CHARGING:
        result = MP_OBJ_NEW_SMALL_INT(FAST_CHARGING);
        break;

    case DONE_CHARGING:
        result = MP_OBJ_NEW_SMALL_INT(DONE_CHARGING);
        break;
    }

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(tilda_sensors_get_charge_status_obj, tilda_sensors_get_charge_status);


// STATIC mp_obj_t tilda_sensors_get_battery_voltage()
// {
//     return mp_const_none;
// }
// STATIC MP_DEFINE_CONST_FUN_OBJ_0(tilda_sensors_get_battery_voltage_obj, tilda_sensors_get_battery_voltage);


STATIC mp_obj_t tilda_sensors_get_tmp_temperature()
{
    return mp_obj_new_float(tildaSharedStates.tmpTemperature);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(tilda_sensors_get_tmp_temperature_obj, tilda_sensors_get_tmp_temperature);


STATIC mp_obj_t tilda_sensors_get_hdc_temperature()
{
    return mp_obj_new_float(tildaSharedStates.hdcTemperature);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(tilda_sensors_get_hdc_temperature_obj, tilda_sensors_get_hdc_temperature);


STATIC mp_obj_t tilda_sensors_get_hdc_humidity()
{
    return mp_obj_new_float(tildaSharedStates.hdcHumidity);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(tilda_sensors_get_hdc_humidity_obj, tilda_sensors_get_hdc_humidity);


STATIC mp_obj_t tilda_sensors_get_lux()
{
    return mp_obj_new_float(tildaSharedStates.optLux);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(tilda_sensors_get_lux_obj, tilda_sensors_get_lux);


STATIC const mp_rom_map_elem_t tilda_sensors_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_sample_rate), MP_ROM_PTR(&tilda_sensors_sample_rate_obj) },
    { MP_ROM_QSTR(MP_QSTR__raw_bq), MP_ROM_PTR(&tilda_sensors_raw_bq_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_vbus_connected), MP_ROM_PTR(&tilda_sensors_get_vbus_connected_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_charge_status), MP_ROM_PTR(&tilda_sensors_get_charge_status_obj) },
    // { MP_ROM_QSTR(MP_QSTR_get_battery_voltage), MP_ROM_PTR(&tilda_sensors_get_battery_voltage_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_tmp_temperature), MP_ROM_PTR(&tilda_sensors_get_tmp_temperature_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_hdc_temperature), MP_ROM_PTR(&tilda_sensors_get_hdc_temperature_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_hdc_humidity), MP_ROM_PTR(&tilda_sensors_get_hdc_humidity_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_lux), MP_ROM_PTR(&tilda_sensors_get_lux_obj) },

    { MP_ROM_QSTR(MP_QSTR_BAT_NO_INPUT), MP_ROM_INT(NO_INPUT) },
    { MP_ROM_QSTR(MP_QSTR_BAT_USB_HOST), MP_ROM_INT(USB_HOST) },
    { MP_ROM_QSTR(MP_QSTR_BAT_ADAPTER_24), MP_ROM_INT(ADAPTER_24) },
    { MP_ROM_QSTR(MP_QSTR_BAT_OTG), MP_ROM_INT(OTG) },
    { MP_ROM_QSTR(MP_QSTR_BAT_NOT_CHARGING), MP_ROM_INT(NOT_CHARGING) },
    { MP_ROM_QSTR(MP_QSTR_BAT_PRE_CHARGING), MP_ROM_INT(PRE_CHARGING) },
    { MP_ROM_QSTR(MP_QSTR_BAT_FAST_CHARGING), MP_ROM_INT(FAST_CHARGING) },
    { MP_ROM_QSTR(MP_QSTR_BAT_DONE_CHARGING), MP_ROM_INT(DONE_CHARGING) },
};

STATIC MP_DEFINE_CONST_DICT (
    tilda_sensors_locals_dict,
    tilda_sensors_locals_dict_table
);

const mp_obj_type_t tilda_sensors_type = {
    { &mp_type_type },
    .name = MP_QSTR_Sensors,
    .locals_dict = (mp_obj_dict_t*)&tilda_sensors_locals_dict,
};
