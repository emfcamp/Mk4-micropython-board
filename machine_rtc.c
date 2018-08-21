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
#include "lib/timeutils/timeutils.h"

#include <ti/sysbios/hal/Seconds.h>

#define OFFSET_1970 (((100-70) * 31536000U) + ((100-69)/4) * 86400U)

typedef struct _machine_rtc_obj_t {
    mp_obj_base_t base;
} machine_rtc_obj_t;

void machine_rtc_teardown(void) {
}

static void init_rtc(mp_obj_t datetime_arg) {
    mp_obj_t *datetime;
    size_t datetime_len;
    mp_obj_get_array(datetime_arg, &datetime_len, &datetime);
    if (datetime_len < 3) {
        mp_raise_OSError(MP_EINVAL);
    }
    mp_uint_t sec = timeutils_mktime(mp_obj_get_int(datetime[0]),
                                     mp_obj_get_int(datetime[1]),
                                     mp_obj_get_int(datetime[2]),
                                     datetime_len > 3 ? mp_obj_get_int(datetime[3]) : 0,
                                     datetime_len > 4 ? mp_obj_get_int(datetime[4]) : 0,
                                     datetime_len > 5 ? mp_obj_get_int(datetime[5]) : 0);

    Seconds_set(sec + OFFSET_1970);
}

STATIC mp_obj_t machine_rtc_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, MP_OBJ_FUN_ARGS_MAX, true);
    // mp_int_t id = n_args > 0 ? mp_obj_get_int(args[0]) : 0;

    machine_rtc_obj_t *self = m_new_obj(machine_rtc_obj_t);
    self->base.type = type;

    if (n_args == 2) {
        init_rtc(args[1]);
    }

    return MP_OBJ_FROM_PTR(self);
}

STATIC void machine_rtc_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_rtc_obj_t *self = self_in;
    mp_printf(print, "<machine_rtc.%p>", self);
}

STATIC mp_obj_t machine_rtc_init(mp_obj_t self_in, mp_obj_t datetime_in) {
    (void)self_in;
    init_rtc(datetime_in);
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(machine_rtc_init_obj, machine_rtc_init);

STATIC mp_obj_t machine_rtc_now(mp_obj_t self_in) {
    (void)self_in;
    uint32_t sec = Seconds_get() - OFFSET_1970;
    timeutils_struct_time_t time;
    timeutils_seconds_since_2000_to_struct_time(sec, &time);
    mp_obj_t tuple[6] = {
        MP_OBJ_NEW_SMALL_INT(time.tm_year),
        MP_OBJ_NEW_SMALL_INT(time.tm_mon),
        MP_OBJ_NEW_SMALL_INT(time.tm_mday),
        MP_OBJ_NEW_SMALL_INT(time.tm_hour),
        MP_OBJ_NEW_SMALL_INT(time.tm_min),
        MP_OBJ_NEW_SMALL_INT(time.tm_sec),
    };
    mp_obj_tuple_t * t = MP_OBJ_TO_PTR(mp_obj_new_tuple(6, tuple));
    return MP_OBJ_FROM_PTR(t);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_rtc_now_obj, machine_rtc_now);

STATIC mp_obj_t machine_rtc_deinit(mp_obj_t self_in) {
    (void)self_in;
    Seconds_set(timeutils_mktime(2015, 1, 1, 0, 0, 0) + OFFSET_1970);
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_rtc_deinit_obj, machine_rtc_deinit);

STATIC const mp_rom_map_elem_t machine_rtc_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_rtc_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_now), MP_ROM_PTR(&machine_rtc_now_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&machine_rtc_deinit_obj) },
#ifdef MACHINE_RTC_IDS
    MACHINE_RTC_IDS
#endif
};

STATIC MP_DEFINE_CONST_DICT(machine_rtc_locals_dict, machine_rtc_locals_dict_table);

const mp_obj_type_t machine_rtc_type = {
    { &mp_type_type },
    .name = MP_QSTR_RTC,
    .make_new = machine_rtc_make_new,
    .print = machine_rtc_print,
    .locals_dict = (mp_obj_dict_t*)&machine_rtc_locals_dict,
};
