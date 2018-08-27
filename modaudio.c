/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Texas Instruments, Incorporated
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

#include "py/runtime.h"

#if MICROPY_HW_AUDIO

#include <ti/devices/msp432e4/inc/msp432.h>
#include <ti/devices/msp432e4/driverlib/interrupt.h>
#include <ti/drivers/dpl/HwiP.h>

#include "sound.h"

static bool init = false;
static uint8_t cur_volume = 128;

static void soundHandler(uint32_t half)
{
    if (half) {
        SoundStop();
    }
}

STATIC mp_obj_t audio_play(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    mp_buffer_info_t source_info;
    mp_get_buffer_raise(pos_args[0], &source_info, MP_BUFFER_READ);
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_wait, MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = true} },
        { MP_QSTR_sample_rate, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 16000} },
    };
    enum { ARG_wait, ARG_sample_rate,};
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
        MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    if (!init) {
        HwiP_Params params;
        HwiP_Params_init(&params);
        params.priority = 0;
        params.enableInt = true;
        HwiP_create(INT_PWM0_1, (HwiP_Fxn)SoundIntHandler, &params);

        SoundInit(120000000u);
        init = true;
    }

    SoundStart(source_info.buf, source_info.len/2, args[ARG_sample_rate].u_int,
               soundHandler);

    if (args[ARG_wait].u_int) {
        while (SoundBusy()) {
            usleep(10000);
        }
    }

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(audio_play_obj, 1, audio_play);

STATIC mp_obj_t audio_volume(size_t n_args, const mp_obj_t *args) {
    mp_obj_t result;
    if (n_args > 0) {
        mp_int_t volume;
        volume = mp_obj_get_int(args[0]);
        volume = volume < 0 ? 0 : volume;
        volume = volume > 255 ? 255 : volume;
        SoundVolumeSet(volume);
        cur_volume = volume;
        result = args[0];
    }
    else {
        result = MP_OBJ_NEW_SMALL_INT(cur_volume);
    }

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(audio_volume_obj, 0, 1, audio_volume);

STATIC const mp_rom_map_elem_t audio_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_audio) },
    { MP_ROM_QSTR(MP_QSTR_play), MP_ROM_PTR(&audio_play_obj) },
    { MP_ROM_QSTR(MP_QSTR_volume), MP_ROM_PTR(&audio_volume_obj) },
};

STATIC MP_DEFINE_CONST_DICT(audio_module_globals, audio_module_globals_table);

const mp_obj_module_t mp_module_audio = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&audio_module_globals,
};

#endif
