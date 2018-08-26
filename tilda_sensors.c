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

STATIC mp_obj_t tilda_sensors_get_vbus_connected()
{
    return mp_const_none; //mp_obj_new_bool
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(tilda_sensors_get_vbus_connected_obj, tilda_sensors_get_vbus_connected);

STATIC mp_obj_t tilda_sensors_get_charge_status()
{
    /*
        Charging status:
        00 – Not Charging
        01 – Pre-charge (< VBATLOWV)
        10 – Fast Charging
        11 – Charge Termination
     */
    return mp_const_none;
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
    { MP_ROM_QSTR(MP_QSTR_get_vbus_connected), MP_ROM_PTR(&tilda_sensors_get_vbus_connected_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_charge_status), MP_ROM_PTR(&tilda_sensors_get_charge_status_obj) },
    // { MP_ROM_QSTR(MP_QSTR_get_battery_voltage), MP_ROM_PTR(&tilda_sensors_get_battery_voltage_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_tmp_temperature), MP_ROM_PTR(&tilda_sensors_get_tmp_temperature_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_hdc_temperature), MP_ROM_PTR(&tilda_sensors_get_hdc_temperature_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_hdc_humidity), MP_ROM_PTR(&tilda_sensors_get_hdc_humidity_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_lux), MP_ROM_PTR(&tilda_sensors_get_lux_obj) },
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