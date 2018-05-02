/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * Development of the code in this file was sponsored by Microbric Pty Ltd
 * and Mnemote Pty Ltd
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Texas Instruments
 *
 * Based on esp8266/modnetwork.c which is Copyright (c) 2015 Paul Sokolovsky
 * And the ESP IDF example code which is Public Domain / CC0
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
#include <string.h>

#include <pthread.h>

#include "py/nlr.h"
#include "py/objlist.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mperrno.h"
#include "lib/netutils/netutils.h"

#if MICROPY_PY_NETWORK_NDK

#include <ti/ndk/inc/usertype.h>
#include <ti/ndk/inc/socket.h>
#include <ti/ndk/inc/nettools/nettools.h>

typedef struct ndk_if_obj_t_ {
    mp_obj_base_t base;
    int if_id;
} ndk_if_obj_t;

const mp_obj_type_t ndk_if_type;
/* TODO: NDK can have multiple interfaces */
STATIC const ndk_if_obj_t ndk_obj = {{&ndk_if_type}, 0};

extern int fdOpenSession(void * task);

STATIC mp_obj_t get_lan(size_t n_args, const mp_obj_t *args) {
    /* TODO: handle multiple interfaces */
    return MP_OBJ_FROM_PTR(&ndk_obj);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(get_lan_obj, 0, 1, get_lan);

STATIC mp_obj_t slndk_initialize() {
    static int initialized = 0;
    if (!initialized) {
        fdOpenSession(pthread_self());
        initialized = 1;
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(slndk_initialize_obj, slndk_initialize);


STATIC mp_obj_t slndk_active(size_t n_args, const mp_obj_t *args) {

    ndk_if_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    (void)self;
    uint8_t mode = 0;
    uint8_t bit = 0;

    return (mode & bit) ? mp_const_true : mp_const_false;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(slndk_active_obj, 1, 2, slndk_active);

STATIC mp_obj_t slndk_connect(size_t n_args, const mp_obj_t *args) {
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(slndk_connect_obj, 1, 7, slndk_connect);

STATIC mp_obj_t slndk_disconnect(mp_obj_t self_in) {
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(slndk_disconnect_obj, slndk_disconnect);

STATIC mp_obj_t slndk_status(mp_obj_t self_in) {
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(slndk_status_obj, slndk_status);

STATIC mp_obj_t slndk_scan(mp_obj_t self_in) {
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(slndk_scan_obj, slndk_scan);

STATIC mp_obj_t slndk_isconnected(mp_obj_t self_in) {
    return mp_obj_new_bool(1);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(slndk_isconnected_obj, slndk_isconnected);

STATIC mp_obj_t slndk_ifconfig(size_t n_args, const mp_obj_t *args) {
    ndk_if_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    if (n_args == 1) {
        CI_IPNET addr;

        memset(&addr, 0, sizeof(addr));
        if (CfgGetImmediate(0, CFGTAG_IPNET, self->if_id, 1, sizeof(addr),
                            (unsigned char *)&addr) == sizeof(addr)) {

            mp_obj_t tuple[2] = {
                netutils_format_ipv4_addr((uint8_t*)&addr.IPAddr, NETUTILS_BIG),
                netutils_format_ipv4_addr((uint8_t*)&addr.IPMask, NETUTILS_BIG),
            };
            return mp_obj_new_tuple(2, tuple);
        }
    }

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(slndk_ifconfig_obj, 1, 2, slndk_ifconfig);

STATIC mp_obj_t slndk_config(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(slndk_config_obj, 1, slndk_config);

STATIC const mp_map_elem_t ndk_if_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_active), (mp_obj_t)&slndk_active_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_connect), (mp_obj_t)&slndk_connect_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_disconnect), (mp_obj_t)&slndk_disconnect_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_status), (mp_obj_t)&slndk_status_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_scan), (mp_obj_t)&slndk_scan_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_isconnected), (mp_obj_t)&slndk_isconnected_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_config), (mp_obj_t)&slndk_config_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ifconfig), (mp_obj_t)&slndk_ifconfig_obj },
};

STATIC MP_DEFINE_CONST_DICT(ndk_if_locals_dict, ndk_if_locals_dict_table);

const mp_obj_type_t ndk_if_type = {
    { &mp_type_type },
    .name = MP_QSTR_NDK,
    .locals_dict = (mp_obj_t)&ndk_if_locals_dict,
};

STATIC const mp_map_elem_t mp_module_network_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_network) },
    { MP_OBJ_NEW_QSTR(MP_QSTR___init__), (mp_obj_t)&slndk_initialize_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_LAN), (mp_obj_t)&get_lan_obj },
};

STATIC MP_DEFINE_CONST_DICT(mp_module_network_globals, mp_module_network_globals_table);

const mp_obj_module_t mp_module_network = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_network_globals,
};

#endif
