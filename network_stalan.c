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
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "py/runtime.h"
#include "py/mperrno.h"
#include "lib/netutils/netutils.h"

#if MICROPY_PY_NETWORK_WIFI

#include <simplelink.h>

#define STA_DHCP_ADDR 42

typedef struct _network_stalan_obj_t {
    mp_obj_base_t base;
} network_stalan_obj_t;

static char * formatMAC(char * fmt, const uint8_t * mac)
{
    snprintf(fmt, 18, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2],
             mac[3], mac[4], mac[5]);
    return fmt;
}

STATIC mp_obj_t network_stalan_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, MP_OBJ_FUN_ARGS_MAX, true);
    if (n_args > 0) {
        if (mp_obj_get_int(args[0]) != 0) {
            mp_raise_OSError(MP_ENODEV);
        }
    }
    network_stalan_obj_t *self = m_new_obj(network_stalan_obj_t);
    self->base.type = type;

    return MP_OBJ_FROM_PTR(self);
}

STATIC void network_stalan_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    network_stalan_obj_t *self = self_in;
    mp_printf(print, "<network_stalan.%p>", self);
}

STATIC mp_obj_t network_stalan_active(size_t n_args, const mp_obj_t *args) {
    uint8_t configOpt = SL_DEVICE_EVENT_CLASS_WLAN;
    uint32_t wlanStatus;
    uint16_t configLen = sizeof(wlanStatus);
    int16_t status = sl_DeviceGet(SL_DEVICE_STATUS, &configOpt, &configLen,
                                  (uint8_t *)&wlanStatus);
    mp_obj_t result;

    if (n_args > 1) {
        bool activate = mp_obj_is_true(args[1]);
        if (activate && status != 0) {
            if (sl_Start(0, 0, 0) != ROLE_STA) {
                sl_WlanSetMode(ROLE_STA);
                if ((status = sl_Stop(20)) < 0) {
                    mp_printf(MP_PYTHON_PRINTER, "stop: %d\n", status);
                }

                if ((status = sl_Start(0, 0, 0)) != ROLE_STA) {
                    mp_printf(MP_PYTHON_PRINTER, "start: %d\n", status);
                }
            }
            result = mp_const_true;
        }
        if (!activate) {
            sl_Stop(0);
            result = mp_const_false;
        }
    }
    else {
        uint8_t configOpt = SL_DEVICE_EVENT_CLASS_WLAN;
        uint32_t wlanStatus;
        uint16_t configLen = sizeof(wlanStatus);

        int16_t status = sl_DeviceGet(SL_DEVICE_STATUS, &configOpt, &configLen,
                                      (uint8_t *)&wlanStatus);
        result = (status == 0) ? mp_const_true : mp_const_false;
    }

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(network_stalan_active_obj, 1, 2, network_stalan_active);

STATIC mp_obj_t network_stalan_connect(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_service_id, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_key, MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_ssid, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_bssid, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_password, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_autoconnect, MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = false} },
    };
    enum { ARG_service_id, ARG_key, ARG_ssid, ARG_bssid, ARG_password, ARG_autoconnect };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
        MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    size_t len;
    const char *p;
    char ssid[32] = {0};
    char password[32] = {0};
    char bssid[18] = {0};
    bool autoconnect = false;

    // set parameters based on given args
    if (args[ARG_ssid].u_obj != MP_OBJ_NULL) {
        p = mp_obj_str_get_data(args[ARG_ssid].u_obj, &len);
        len = MIN(len, sizeof(ssid));
        memcpy(ssid, p, len);
    }
    if (args[ARG_password].u_obj != MP_OBJ_NULL) {
        p = mp_obj_str_get_data(args[ARG_password].u_obj, &len);
        len = MIN(len, sizeof(password));
        memcpy(password, p, len);
    }
    if (args[ARG_bssid].u_obj != MP_OBJ_NULL) {
        p = mp_obj_str_get_data(args[ARG_bssid].u_obj, &len);
        if (len != sizeof(bssid)) {
            mp_raise_ValueError(NULL);
        }
        memcpy(bssid, p, sizeof(bssid));
    }
    autoconnect = mp_obj_get_int(args[ARG_autoconnect].u_obj);

    if (n_args > 1) {
        mp_uint_t len = 0;
        SlWlanSecParams_t secParams = {0};

        sl_WlanDisconnect();

        /* TODO: adjust when config() method is implemented */
        sl_WlanPolicySet(SL_WLAN_POLICY_CONNECTION,
                         SL_WLAN_CONNECTION_POLICY(0, 1, 0, 0), NULL, 0);
        sl_NetCfgSet(SL_NETCFG_IPV4_STA_ADDR_MODE, SL_NETCFG_ADDR_DHCP, 0, 0);

        len = strlen(password);
        if (len) {
            secParams.Key = (signed char *)password;
            secParams.KeyLen = len;
            secParams.Type = SL_WLAN_SEC_TYPE_WPA_WPA2;
        }
        else {
            secParams.Key = NULL;
            secParams.KeyLen = 0;
            secParams.Type = SL_WLAN_SEC_TYPE_OPEN;
        }

        if (sl_WlanConnect((signed char *)ssid, strlen((const char *)ssid),
                           NULL, &secParams, NULL) < 0) {
            mp_raise_msg(&mp_type_OSError, "Connect failed");
        }

        if (autoconnect) {
            sl_WlanProfileAdd((const int8_t *)ssid, strlen(ssid), NULL,
                              &secParams, NULL, 15u, 0u);
            sl_WlanPolicySet(SL_WLAN_POLICY_CONNECTION,
                             SL_WLAN_CONNECTION_POLICY(1, 0, 0, 0), NULL, 0);
        }
        else {
            sl_WlanPolicySet(SL_WLAN_POLICY_CONNECTION,
                             SL_WLAN_CONNECTION_POLICY(0, 0, 0, 0), NULL, 0);
        }
    }

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(network_stalan_connect_obj, 1, network_stalan_connect);

STATIC mp_obj_t network_stalan_disconnect(mp_obj_t self_in) {
    if (sl_WlanDisconnect() < 0) {
        mp_raise_msg(&mp_type_OSError, "disconnect failed");
    }

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(network_stalan_disconnect_obj, network_stalan_disconnect);

STATIC mp_obj_t network_stalan_isconnected(mp_obj_t self_in) {
    uint8_t configOpt = SL_DEVICE_EVENT_CLASS_WLAN;
    uint32_t wlanStatus = 0;
    uint16_t configLen = sizeof(wlanStatus);
    (void)sl_DeviceGet(SL_DEVICE_STATUS, &configOpt, &configLen,
                       (uint8_t *)&wlanStatus);
    return (wlanStatus & SL_DEVICE_STATUS_WLAN_STA_CONNECTED) ? mp_const_true :
        mp_const_false;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(network_stalan_isconnected_obj, network_stalan_isconnected);

STATIC mp_obj_t network_stalan_scan(mp_obj_t self_in) {
    SlWlanNetworkEntry_t entries[10];

    memset(entries, 0, sizeof(entries));
    int status = sl_WlanGetNetworkList(0, 10, entries);
    if (status < 0) {
        usleep(1000000u);
        status = sl_WlanGetNetworkList(0, 10, entries);
    }
    if (status > 0) {
        mp_obj_t list = mp_obj_new_list(0, NULL);
        for (int i = 0; i < status; i++) {
            char mac[18];
            mp_obj_tuple_t * tuple = MP_OBJ_TO_PTR(mp_obj_new_tuple(5, NULL));
            tuple->items[0] = MP_OBJ_NEW_QSTR(qstr_from_str((const char *)entries[i].Ssid));
            tuple->items[1] = MP_OBJ_NEW_QSTR(qstr_from_str((const char *)formatMAC(mac, entries[i].Bssid)));
            tuple->items[2] = MP_OBJ_NEW_SMALL_INT(entries[i].Channel);
            tuple->items[3] = MP_OBJ_NEW_SMALL_INT(entries[i].Rssi);
            tuple->items[4] = MP_OBJ_NEW_SMALL_INT(entries[i].SecurityInfo);
            mp_obj_list_append(list, MP_OBJ_FROM_PTR(tuple));
        }
        return list;
    }
    else {
        return mp_const_none;
    }
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(network_stalan_scan_obj, network_stalan_scan);

STATIC mp_obj_t network_stalan_status(size_t n_args, const mp_obj_t *args) {
    const char *param = NULL;
    if (n_args > 1) {
        param = mp_obj_str_get_str(args[1]);
    };

    SlWlanGetRxStatResponse_t stat;
    mp_obj_t result = mp_const_false;
    if (sl_WlanRxStatGet(&stat, 0) == 0) {
        if (param) {
            if (strncmp(param, "rssi", 4) == 0) {
                result = MP_OBJ_NEW_SMALL_INT(stat.AvarageDataCtrlRssi);
            }
            else {
                mp_raise_OSError(MP_EINVAL);
            }
        }
        else {
            result = mp_const_true;
        }
    }

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(network_stalan_status_obj, 1, 2, network_stalan_status);

STATIC mp_obj_t network_stalan_ifconfig(size_t n_args, const mp_obj_t *args) {
    if (n_args > 1 && MP_OBJ_IS_SMALL_INT(args[1])) {
        if (mp_obj_get_int(args[1]) != STA_DHCP_ADDR) {
            mp_raise_OSError(MP_EINVAL);
        }

        sl_NetCfgSet(SL_NETCFG_IPV4_STA_ADDR_MODE, SL_NETCFG_ADDR_DHCP,
                     0, NULL);
        sl_Stop(0);
        sl_Start(0, 0, 0);

        return mp_const_none;
    }

    if (n_args > 1) {
        mp_obj_t * config;
        size_t config_len;
        mp_obj_get_array(args[1], &config_len, &config);

        if (config_len != 4) {
            mp_raise_OSError(MP_EINVAL);
        }
        SlNetCfgIpV4Args_t addrCfg;

        netutils_parse_ipv4_addr(config[0], (uint8_t *)&addrCfg.Ip, NETUTILS_LITTLE);
        netutils_parse_ipv4_addr(config[1], (uint8_t *)&addrCfg.IpMask, NETUTILS_LITTLE);
        netutils_parse_ipv4_addr(config[2], (uint8_t *)&addrCfg.IpGateway, NETUTILS_LITTLE);
        netutils_parse_ipv4_addr(config[3], (uint8_t *)&addrCfg.IpDnsServer, NETUTILS_LITTLE);

        sl_NetCfgSet(SL_NETCFG_IPV4_STA_ADDR_MODE, SL_NETCFG_ADDR_STATIC,
                     sizeof(addrCfg), (uint8_t *)&addrCfg);
        sl_Stop(0);
        sl_Start(0, 0, 0);

        return mp_const_none;
    }
    else {
        uint16_t len = sizeof(SlNetCfgIpV4Args_t);
        uint16_t ConfigOpt = 0;
        SlNetCfgIpV4Args_t ipV4 = {0};
        sl_NetCfgGet(SL_NETCFG_IPV4_STA_ADDR_MODE, &ConfigOpt, &len,
                     (uint8_t*)&ipV4);

        if (ipV4.Ip == 0) {
            return mp_const_none;
        }
        else {
            mp_obj_t tuple[4] = {
                netutils_format_ipv4_addr((uint8_t*)&ipV4.Ip, NETUTILS_LITTLE),
                netutils_format_ipv4_addr((uint8_t*)&ipV4.IpMask,
                                          NETUTILS_LITTLE),
                netutils_format_ipv4_addr((uint8_t*)&ipV4.IpGateway,
                                          NETUTILS_LITTLE),
                netutils_format_ipv4_addr((uint8_t*)&ipV4.IpDnsServer,
                                          NETUTILS_LITTLE),
            };
            return mp_obj_new_tuple(4, tuple);
        }
    }
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(network_stalan_ifconfig_obj, 1, 2, network_stalan_ifconfig);

STATIC const mp_rom_map_elem_t network_stalan_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_active), MP_ROM_PTR(&network_stalan_active_obj) },
    { MP_ROM_QSTR(MP_QSTR_connect), MP_ROM_PTR(&network_stalan_connect_obj) },
    { MP_ROM_QSTR(MP_QSTR_disconnect), MP_ROM_PTR(&network_stalan_disconnect_obj) },
    { MP_ROM_QSTR(MP_QSTR_isconnected), MP_ROM_PTR(&network_stalan_isconnected_obj) },
    { MP_ROM_QSTR(MP_QSTR_scan), MP_ROM_PTR(&network_stalan_scan_obj) },
    { MP_ROM_QSTR(MP_QSTR_status), MP_ROM_PTR(&network_stalan_status_obj) },
    { MP_ROM_QSTR(MP_QSTR_ifconfig), MP_ROM_PTR(&network_stalan_ifconfig_obj) },
    { MP_ROM_QSTR(MP_QSTR_DCHP), MP_ROM_INT(STA_DHCP_ADDR) },
};

STATIC MP_DEFINE_CONST_DICT(network_stalan_locals_dict, network_stalan_locals_dict_table);

const mp_obj_type_t network_stalan_type = {
    { &mp_type_type },
    .name = MP_QSTR_STALAN,
    .make_new = network_stalan_make_new,
    .print = network_stalan_print,
    .locals_dict = (mp_obj_dict_t*)&network_stalan_locals_dict,
};

#endif
