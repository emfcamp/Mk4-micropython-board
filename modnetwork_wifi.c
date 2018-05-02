/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * Development of the code in this file was sponsored by Microbric Pty Ltd
 * and Mnemote Pty Ltd
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016, 2017 Nick Moore @mnemote
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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "py/nlr.h"
#include "py/objlist.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mperrno.h"
#include "lib/netutils/netutils.h"

#if MICROPY_PY_NETWORK_WIFI

#include <simplelink.h>

#define MODNETWORK_INCLUDE_CONSTANTS (0)

#define WIFI_IF_STA 0
#define WIFI_IF_AP 1

#define WIFI_MODE_STA 0
#define WIFI_MODE_AP 1

typedef uint32_t wifi_mode_t;

typedef struct _wlan_if_obj_t {
    mp_obj_base_t base;
    int if_id;
} wlan_if_obj_t;

const mp_obj_type_t wlan_if_type;
STATIC const wlan_if_obj_t wlan_sta_obj = {{&wlan_if_type}, WIFI_IF_STA};
STATIC const wlan_if_obj_t wlan_ap_obj = {{&wlan_if_type}, WIFI_IF_AP};

STATIC mp_obj_t get_wlan(size_t n_args, const mp_obj_t *args) {
    int idx = (n_args > 0) ? mp_obj_get_int(args[0]) : WIFI_IF_STA;
    if (idx == WIFI_IF_STA) {
        return MP_OBJ_FROM_PTR(&wlan_sta_obj);
    } else if (idx == WIFI_IF_AP) {
        return MP_OBJ_FROM_PTR(&wlan_ap_obj);
    } else {
        mp_raise_msg(&mp_type_ValueError, "invalid WLAN interface identifier");
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(get_wlan_obj, 0, 1, get_wlan);

STATIC mp_obj_t slwlan_initialize() {
    static int initialized = 0;
    if (!initialized) {
        initialized = 1;
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(slwlan_initialize_obj, slwlan_initialize);

STATIC mp_obj_t slwlan_active(size_t n_args, const mp_obj_t *args) {
    uint8_t configOpt = SL_DEVICE_EVENT_CLASS_WLAN;
    uint32_t wlanStatus;
    uint16_t configLen = sizeof(wlanStatus);
    int16_t status = sl_DeviceGet(SL_DEVICE_STATUS, &configOpt, &configLen,
                                  (uint8_t *)&wlanStatus);
    mp_obj_t active;

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
            active = mp_const_true;
        }
        if (!activate) {
            sl_Stop(0);
            active = mp_const_false;
        }
    }
    else {
        active = (status == 0) ? mp_const_true : mp_const_false;
    }

    return active;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(slwlan_active_obj, 1, 2, slwlan_active);

STATIC mp_obj_t slwlan_connect(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_ssid, ARG_password, ARG_bssid, ARG_autoconnect };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_bssid, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_autoconnect, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };

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
    if (args[ARG_ssid].u_obj != mp_const_none) {
        p = mp_obj_str_get_data(args[ARG_ssid].u_obj, &len);
        len = MIN(len, sizeof(ssid));
        memcpy(ssid, p, len);
    }
    if (args[ARG_password].u_obj != mp_const_none) {
        p = mp_obj_str_get_data(args[ARG_password].u_obj, &len);
        len = MIN(len, sizeof(password));
        memcpy(password, p, len);
    }
    if (args[ARG_bssid].u_obj != mp_const_none) {
        p = mp_obj_str_get_data(args[ARG_bssid].u_obj, &len);
        if (len != sizeof(bssid)) {
            mp_raise_ValueError(NULL);
        }
        memcpy(bssid, p, sizeof(bssid));
    }
    if (args[ARG_autoconnect].u_obj != mp_const_none) {
        autoconnect = mp_obj_get_int(args[ARG_autoconnect].u_obj);
    }

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

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(slwlan_connect_obj, 1, slwlan_connect);

STATIC mp_obj_t slwlan_disconnect(mp_obj_t self_in) {
    if (sl_WlanDisconnect() < 0) {
        mp_raise_msg(&mp_type_OSError, "disconnect failed");
    }
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(slwlan_disconnect_obj, slwlan_disconnect);

STATIC mp_obj_t slwlan_status(mp_obj_t self_in) {
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(slwlan_status_obj, slwlan_status);

static char * formatMAC(char * fmt, const uint8_t * mac)
{
    snprintf(fmt, 18, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2],
             mac[3], mac[4], mac[5]);
    return fmt;
}

STATIC mp_obj_t slwlan_scan(mp_obj_t self_in) {
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

STATIC MP_DEFINE_CONST_FUN_OBJ_1(slwlan_scan_obj, slwlan_scan);

STATIC mp_obj_t slwlan_isconnected(mp_obj_t self_in) {
    uint8_t configOpt = SL_DEVICE_EVENT_CLASS_WLAN;
    uint32_t wlanStatus = 0;
    uint16_t configLen = sizeof(wlanStatus);
    (void)sl_DeviceGet(SL_DEVICE_STATUS, &configOpt, &configLen,
                       (uint8_t *)&wlanStatus);
    return (wlanStatus & SL_DEVICE_STATUS_WLAN_STA_CONNECTED) ? mp_const_true :
        mp_const_false;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(slwlan_isconnected_obj, slwlan_isconnected);

STATIC mp_obj_t slwlan_ifconfig(size_t n_args, const mp_obj_t *args) {
    if (n_args > 1) {
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

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(slwlan_ifconfig_obj, 1, 2, slwlan_ifconfig);

STATIC mp_obj_t slwlan_config(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(slwlan_config_obj, 1, slwlan_config);

STATIC const mp_map_elem_t wlan_if_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_active), (mp_obj_t)&slwlan_active_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_connect), (mp_obj_t)&slwlan_connect_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_disconnect), (mp_obj_t)&slwlan_disconnect_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_status), (mp_obj_t)&slwlan_status_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_scan), (mp_obj_t)&slwlan_scan_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_isconnected), (mp_obj_t)&slwlan_isconnected_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_config), (mp_obj_t)&slwlan_config_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ifconfig), (mp_obj_t)&slwlan_ifconfig_obj },
};

STATIC MP_DEFINE_CONST_DICT(wlan_if_locals_dict, wlan_if_locals_dict_table);

const mp_obj_type_t wlan_if_type = {
    { &mp_type_type },
    .name = MP_QSTR_WLAN,
    .locals_dict = (mp_obj_t)&wlan_if_locals_dict,
};


STATIC const mp_map_elem_t mp_module_network_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_network) },
    { MP_OBJ_NEW_QSTR(MP_QSTR___init__), (mp_obj_t)&slwlan_initialize_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_WLAN), (mp_obj_t)&get_wlan_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_LAN), (mp_obj_t)&get_wlan_obj },

#if MODNETWORK_INCLUDE_CONSTANTS
    { MP_OBJ_NEW_QSTR(MP_QSTR_STA_IF),
        MP_OBJ_NEW_SMALL_INT(WIFI_IF_STA)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_AP_IF),
        MP_OBJ_NEW_SMALL_INT(WIFI_IF_AP)},

    { MP_OBJ_NEW_QSTR(MP_QSTR_MODE_11B),
        MP_OBJ_NEW_SMALL_INT(WIFI_PROTOCOL_11B) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_MODE_11G),
        MP_OBJ_NEW_SMALL_INT(WIFI_PROTOCOL_11G) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_MODE_11N),
        MP_OBJ_NEW_SMALL_INT(WIFI_PROTOCOL_11N) },

    { MP_OBJ_NEW_QSTR(MP_QSTR_AUTH_OPEN),
        MP_OBJ_NEW_SMALL_INT(WIFI_AUTH_OPEN) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_AUTH_WEP),
        MP_OBJ_NEW_SMALL_INT(WIFI_AUTH_WEP) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_AUTH_WPA_PSK),
        MP_OBJ_NEW_SMALL_INT(WIFI_AUTH_WPA_PSK) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_AUTH_WPA2_PSK),
        MP_OBJ_NEW_SMALL_INT(WIFI_AUTH_WPA2_PSK) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_AUTH_WPA_WPA2_PSK),
        MP_OBJ_NEW_SMALL_INT(WIFI_AUTH_WPA_WPA2_PSK) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_AUTH_MAX),
        MP_OBJ_NEW_SMALL_INT(WIFI_AUTH_MAX) },
#endif
};

STATIC MP_DEFINE_CONST_DICT(mp_module_network_globals, mp_module_network_globals_table);

const mp_obj_module_t mp_module_network = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_network_globals,
};

#endif
