/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Texas Instruments, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include "py/runtime.h"

#if MICROPY_PY_NETWORK

extern const mp_obj_type_t network_stalan_type;
extern const mp_obj_type_t network_ndklan_type;

STATIC const mp_map_elem_t mp_module_network_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_network) },
#if MICROPY_PY_NETWORK_WIFI
    { MP_OBJ_NEW_QSTR(MP_QSTR_WLAN), (mp_obj_t)&network_stalan_type },
#endif
#if MICROPY_PY_NETWORK_NDK
    { MP_OBJ_NEW_QSTR(MP_QSTR_LAN), (mp_obj_t)&network_ndklan_type },
#endif
};

STATIC MP_DEFINE_CONST_DICT(mp_module_network_globals, mp_module_network_globals_table);

const mp_obj_module_t mp_module_network = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_network_globals,
};

#endif
