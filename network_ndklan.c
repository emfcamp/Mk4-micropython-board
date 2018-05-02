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
#include <string.h>

#include "py/runtime.h"
#include "py/mperrno.h"
#include "lib/netutils/netutils.h"

#if MICROPY_PY_NETWORK_NDK

#include <ti/ndk/inc/usertype.h>
#include <ti/ndk/inc/socket.h>
#include <ti/ndk/inc/nettools/nettools.h>

// TODO: remove after implementing the empty functions
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

typedef struct _network_ndklan_obj_t {
    mp_obj_base_t base;
} network_ndklan_obj_t;

STATIC mp_obj_t network_ndklan_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);
    mp_int_t if_idx = mp_obj_get_int(args[0]);
    // kwargs not handled
    network_ndklan_obj_t *self = m_new_obj(network_ndklan_obj_t);
    self->base.type = type;

    return MP_OBJ_FROM_PTR(self);
}

STATIC void network_ndklan_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    network_ndklan_obj_t *self = self_in;
    mp_printf(print, "<network_ndklan.%p>", self);
}

STATIC mp_obj_t network_ndklan_active(size_t n_args, const mp_obj_t *args) {
    network_ndklan_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    bool activate = true;
    if (n_args > 1) {
        activate = mp_obj_is_true(args[1]);
    };

    mp_obj_t result = mp_obj_new_bool(1u);

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(network_ndklan_active_obj, 1, 2, network_ndklan_active);

STATIC mp_obj_t network_ndklan_connect(size_t n_args, const mp_obj_t *args) {
    network_ndklan_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_int_t service_id = 0;
    if (n_args > 1) {
        service_id = mp_obj_get_int(args[1]);
    };

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(network_ndklan_connect_obj, 1, 2, network_ndklan_connect);

STATIC mp_obj_t network_ndklan_disconnect(mp_obj_t self_in) {
    network_ndklan_obj_t *self = MP_OBJ_TO_PTR(self_in);

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(network_ndklan_disconnect_obj, network_ndklan_disconnect);

STATIC mp_obj_t network_ndklan_isconnected(mp_obj_t self_in) {
    network_ndklan_obj_t *self = MP_OBJ_TO_PTR(self_in);

    mp_obj_t result = mp_obj_new_bool(1u);

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(network_ndklan_isconnected_obj, network_ndklan_isconnected);

STATIC mp_obj_t network_ndklan_status(mp_obj_t self_in) {
    network_ndklan_obj_t *self = MP_OBJ_TO_PTR(self_in);

    mp_obj_t result = MP_OBJ_NEW_SMALL_INT(100);

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(network_ndklan_status_obj, network_ndklan_status);

STATIC mp_obj_t network_ndklan_ifconfig(size_t n_args, const mp_obj_t *args) {
    if (n_args > 1) {
        mp_obj_t * config;
        size_t config_len;
        mp_obj_get_array(args[1], &config_len, &config);

        if (config_len != 4) {
            mp_raise_OSError(MP_EINVAL);
        }

        uint32_t addr;
        netutils_parse_ipv4_addr(config[0], (uint8_t *)&addr, NETUTILS_LITTLE);
        uint32_t mask;
        netutils_parse_ipv4_addr(config[1], (uint8_t *)&mask, NETUTILS_LITTLE);
        uint32_t gateway;
        netutils_parse_ipv4_addr(config[2], (uint8_t *)&gateway, NETUTILS_LITTLE);
        uint32_t dns;
        netutils_parse_ipv4_addr(config[3], (uint8_t *)&dns, NETUTILS_LITTLE);

        mp_raise_OSError(MP_EOPNOTSUPP);
        return mp_const_none;
    };

    if (n_args == 1) {
        CI_IPNET addr;

        memset(&addr, 0, sizeof(addr));
        if (CfgGetImmediate(0, CFGTAG_IPNET, 0, 1, sizeof(addr),
                            (unsigned char *)&addr) == sizeof(addr)) {

            CI_ROUTE route;
            CfgGetImmediate(0, CFGTAG_ROUTE, 0,
                            1, sizeof(route), (uint8_t *)&route);

            uint32_t dns;
            CfgGetImmediate(0, CFGTAG_SYSINFO, CFGITEM_DHCP_DOMAINNAMESERVER,
                            1, sizeof(dns), (uint8_t *)&dns);

            mp_obj_t tuple[4] = {
                netutils_format_ipv4_addr((uint8_t*)&addr.IPAddr, NETUTILS_BIG),
                netutils_format_ipv4_addr((uint8_t*)&addr.IPMask, NETUTILS_BIG),
                netutils_format_ipv4_addr((uint8_t*)&route.IPGateAddr, NETUTILS_BIG),
                netutils_format_ipv4_addr((uint8_t*)&dns, NETUTILS_BIG),
            };
            return mp_obj_new_tuple(4, tuple);
        }
    }

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(network_ndklan_ifconfig_obj, 1, 2, network_ndklan_ifconfig);

STATIC const mp_rom_map_elem_t network_ndklan_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_active), MP_ROM_PTR(&network_ndklan_active_obj) },
    { MP_ROM_QSTR(MP_QSTR_connect), MP_ROM_PTR(&network_ndklan_connect_obj) },
    { MP_ROM_QSTR(MP_QSTR_disconnect), MP_ROM_PTR(&network_ndklan_disconnect_obj) },
    { MP_ROM_QSTR(MP_QSTR_isconnected), MP_ROM_PTR(&network_ndklan_isconnected_obj) },
    { MP_ROM_QSTR(MP_QSTR_status), MP_ROM_PTR(&network_ndklan_status_obj) },
    { MP_ROM_QSTR(MP_QSTR_ifconfig), MP_ROM_PTR(&network_ndklan_ifconfig_obj) },
};

STATIC MP_DEFINE_CONST_DICT(network_ndklan_locals_dict, network_ndklan_locals_dict_table);

const mp_obj_type_t network_ndklan_type = {
    { &mp_type_type },
    .name = MP_QSTR_NDKLAN,
    .make_new = network_ndklan_make_new,
    .print = network_ndklan_print,
    .locals_dict = (mp_obj_dict_t*)&network_ndklan_locals_dict,
};

#endif
