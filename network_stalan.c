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
#include "py/objstr.h"
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
            status = sl_Start(0, 0, 0);
            if (status < 0) {
                mp_printf(MP_PYTHON_PRINTER, "network_stalan_active error sl_start: %d\n", status);
                return (mp_const_false);
            }
            else if (status != ROLE_STA) {
                status = sl_WlanSetMode(ROLE_STA);
                if (status < 0) {
                    mp_printf(MP_PYTHON_PRINTER, "network_stalan_active error sl_wlansetmode error: %d\n", status);
                    return (mp_const_false);
                }

                mp_printf(MP_PYTHON_PRINTER, "network_stalan_active sl_stop for role change: %d\n", 0);
                if ((status = sl_Stop(200)) < 0) {
                    mp_printf(MP_PYTHON_PRINTER, "stop: %d\n", status);
                    return (mp_const_false);
                }

                if ((status = sl_Start(0, 0, 0)) != ROLE_STA) {
                    mp_printf(MP_PYTHON_PRINTER, "start: %d\n", status);
                    return (mp_const_false);
                }
            }
            result = mp_const_true;
        }
        if (!activate) {
            mp_printf(MP_PYTHON_PRINTER, "network_stalan_active sl_stop deactivate: %d\n", 0);
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
        { MP_QSTR_service_id,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_key,                           MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_bssid,       MP_ARG_KW_ONLY  | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_autoconnect, MP_ARG_KW_ONLY  | MP_ARG_BOOL,{.u_bool = false} },
        /* TODO - Add enterprise security config - review */
        { MP_QSTR_enterprise,  MP_ARG_KW_ONLY  | MP_ARG_BOOL,{.u_bool = false} },
        { MP_QSTR_entuser,     MP_ARG_KW_ONLY  | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_entmethod,   MP_ARG_KW_ONLY  | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_entserverauth,MP_ARG_KW_ONLY  | MP_ARG_BOOL,{.u_bool = true} },
    };
    enum { ARG_service_id, ARG_key, ARG_bssid, ARG_autoconnect,
           ARG_enterprise, ARG_entuser, ARG_entmethod, ARG_entserverauth };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
                     MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // TODO: Big stack usage here..
    size_t len;
    const char *p;
    char ssid[33] = {0};
    char password[33] = {0};
    char bssid[18] = {0};
    uint8_t mac[6];
    bool autoconnect = false;

    bool enterprise = false;
    char entuser[65] = {0};
    uint32_t entmethod = SL_WLAN_ENT_EAP_METHOD_TLS;
    bool entserverauth = true;

    // set parameters based on given args
    if (args[ARG_service_id].u_obj != MP_OBJ_NULL) {
        p = mp_obj_str_get_data(args[ARG_service_id].u_obj, &len);
        // TODO : review throw error if SSID > 32 instead of truncate?
        len = MIN(len, (sizeof(ssid) - 1));
        memcpy(ssid, p, len);
    }
    if (args[ARG_key].u_obj != MP_OBJ_NULL) {
        p = mp_obj_str_get_data(args[ARG_key].u_obj, &len);
        // TODO : review throw error if PSWD > 32 instead of truncate?
        len = MIN(len, (sizeof(password) - 1));
        memcpy(password, p, len);
    }
    if (args[ARG_bssid].u_obj != MP_OBJ_NULL) {
        unsigned int i;
        unsigned int uimac[6];
        p = mp_obj_str_get_data(args[ARG_bssid].u_obj, &len);
        if (len != (sizeof(bssid) - 1)) {
            mp_raise_ValueError("bssid bad len, format = xx:xx:xx:xx:xx:xx");
        }
        // TODO : remove intermediate string buffer
        memcpy(bssid, p, sizeof(bssid));
        sscanf(bssid, "%x:%x:%x:%x:%x:%x",
            &uimac[0], &uimac[1], &uimac[2], &uimac[3], &uimac[4], &uimac[5]);
        for(i = 0; i < sizeof(mac); i++) {
            mac[i] = (unsigned char) uimac[i];
        }
    }
    autoconnect = args[ARG_autoconnect].u_bool;

    enterprise = args[ARG_enterprise].u_bool;
    if (args[ARG_entuser].u_obj != MP_OBJ_NULL) {
        p = mp_obj_str_get_data(args[ARG_entuser].u_obj, &len);
        // TODO : review throw error if ENTUSER > 64 instead of truncate?
        len = MIN(len, (sizeof(entuser) - 1));
        memcpy(entuser, p, len);
    }
    if (args[ARG_entmethod].u_int != 0) {
        // TODO : should we validate that the int is one of the valid methods?
        entmethod = args[ARG_entmethod].u_int;
    }
    entserverauth = args[ARG_entserverauth].u_bool;

    SlWlanSecParams_t secParams = {0};
    SlWlanSecParamsExt_t secExtParams = {0};
    int16_t ret;

    ret = sl_WlanDisconnect();
    if ((ret < 0) && (ret != SL_ERROR_WLAN_WIFI_ALREADY_DISCONNECTED)) {
        mp_raise_msg(&mp_type_OSError, "disconnect failed");
    }

    /* TODO: adjust when config() method is implemented */
    /* TODO: do we want to enable fast connect? - review */
    if (sl_WlanPolicySet(SL_WLAN_POLICY_CONNECTION,
                     SL_WLAN_CONNECTION_POLICY(0, 1, 0, 0), NULL, 0) < 0) {
        mp_raise_msg(&mp_type_OSError, "Failed to enable fast connect mode");
    }
    if (sl_NetCfgSet(SL_NETCFG_IPV4_STA_ADDR_MODE, SL_NETCFG_ADDR_DHCP, 0, 0) < 0) {
        mp_raise_msg(&mp_type_OSError, "Failed to set DHCP mode");
    }

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

    if (autoconnect) {
        /* TODO: manage profiles - support only single stored profile for now */
        if (sl_WlanProfileDel(0xff) < 0) {
            mp_raise_msg(&mp_type_OSError, "Failed to clear wifi profiles");
        }

        if (sl_WlanProfileAdd((const int8_t *)ssid, strlen((const char *)ssid),
                              (const uint8_t *)((strlen(bssid) > 0) ? mac : NULL),
                              &secParams, NULL, 15u, 0u) < 0 ) {
            mp_raise_msg(&mp_type_OSError, "Failed to add wifi profile");
        }

        if (sl_WlanPolicySet(SL_WLAN_POLICY_CONNECTION,
                             SL_WLAN_CONNECTION_POLICY(1, 0, 0, 0), NULL, 0) < 0) {
            mp_raise_msg(&mp_type_OSError, "Failed to enable autoconnect mode");
        }
    }
    else {
        if (sl_WlanPolicySet(SL_WLAN_POLICY_CONNECTION,
                             SL_WLAN_CONNECTION_POLICY(0, 0, 0, 0), NULL, 0) < 0) {
            mp_raise_msg(&mp_type_OSError, "Failed to disable autoconnect mode");
        }
    }

    if (enterprise) {
        secParams.Type = SL_WLAN_SEC_TYPE_WPA_ENT;
        secExtParams.User = (signed char *) entuser;
        secExtParams.UserLen = strlen((const char *) secExtParams.User);
        secExtParams.AnonUser = NULL;
        secExtParams.AnonUserLen = 0;
        secExtParams.CertIndex = 0;
        secExtParams.EapMethod = entmethod;

	// configure server certificate authentication
        _u8 param = entserverauth;
        if (sl_WlanSet(SL_WLAN_CFG_GENERAL_PARAM_ID,
                       SL_WLAN_GENERAL_PARAM_DISABLE_ENT_SERVER_AUTH, 1, &param) < 0) {
            mp_raise_msg(&mp_type_OSError, "Failed to set enterprise server auth mode");
        }

        if (sl_WlanConnect((const int8_t *)ssid, strlen((const char *)ssid),
                           (const uint8_t *)((strlen(bssid) > 0) ? mac : NULL),
                           &secParams, &secExtParams) < 0) {
            mp_raise_msg(&mp_type_OSError, "Connect failed");
        }
    }
    else {
        if (sl_WlanConnect((const int8_t *)ssid, strlen((const char *)ssid),
                           (const uint8_t *)((strlen(bssid) > 0) ? mac : NULL),
                           &secParams, NULL) < 0) {
            mp_raise_msg(&mp_type_OSError, "Connect failed");
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
    SlWlanConnStatusParam_t connectionParams;
    uint16_t opt = 0;
    int32_t status;
    uint16_t size = sizeof(connectionParams);

    memset(&connectionParams, 0, sizeof(connectionParams));
    status = sl_WlanGet(SL_WLAN_CONNECTION_INFO, &opt, &size, (uint8_t *)&connectionParams);
    if (status < 0) {
        mp_raise_OSError(MP_EIO);
    }
    return connectionParams.ConnStatus ? mp_const_true : mp_const_false;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(network_stalan_isconnected_obj, network_stalan_isconnected);

STATIC mp_obj_t network_stalan_scan(mp_obj_t self_in) {
    SlWlanNetworkEntry_t entries[30];
    int retryCount = 5;

    memset(entries, 0, sizeof(entries));
    int status = sl_WlanGetNetworkList(0, 30, entries);

    if (status == SL_RET_CODE_DEV_NOT_STARTED) {
        mp_raise_msg(&mp_type_OSError, "WiFi Not Started, use .active(True)");
        return mp_const_none;
    }

    while ((status == SL_ERROR_WLAN_GET_NETWORK_LIST_EAGAIN) &&
           retryCount--) {
        //usleep(1000000u); // usleep max is 1000000 - this would have returned error..
	//keep comment for now since later we'll want to replace with smaller usleep()
        sleep(1);
	//mp_printf(MP_PYTHON_PRINTER, "WlanGetNetworkList retry: %d\n", retryCount);
        status = sl_WlanGetNetworkList(0, 30, entries);
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

    if (status < 0) {
	mp_printf(MP_PYTHON_PRINTER, "WlanGetNetworkList error: %d\n", status);
    }

    return mp_const_none;
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
	//mp_printf(MP_PYTHON_PRINTER, "network_stalan_ifconfig set DHCP sl_stop: %d\n", 0);
        sl_Stop(200);
	//mp_printf(MP_PYTHON_PRINTER, "network_stalan_ifconfig set DHCP sl_start: %d\n", 0);
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
	//mp_printf(MP_PYTHON_PRINTER, "network_stalan_ifconfig set STATIC sl_stop: %d\n", 0);
        sl_Stop(200);
	//mp_printf(MP_PYTHON_PRINTER, "network_stalan_ifconfig set STATIC sl_stop: %d\n", 0);
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


STATIC mp_obj_t network_stalan_version(mp_obj_t self_in) {

    SlDeviceVersion_t ver;
    _u8 status;
    _u8 option = SL_DEVICE_GENERAL_VERSION;
    _u16 configLen = sizeof(SlDeviceVersion_t);

    status = sl_DeviceGet(SL_DEVICE_GENERAL,&option,&configLen,(_u8 *)(&ver));

    if (status < 0) {
        mp_raise_OSError(MP_EIO);
        return mp_const_none;
    }
    else {
        mp_obj_t list = mp_obj_new_list(0, NULL);
        mp_obj_tuple_t * tuple;

        tuple = MP_OBJ_TO_PTR(mp_obj_new_tuple(1, NULL));
        tuple->items[0] = MP_OBJ_NEW_SMALL_INT(ver.ChipId);
        mp_obj_list_append(list, MP_OBJ_FROM_PTR(tuple));

        tuple = MP_OBJ_TO_PTR(mp_obj_new_tuple(5, NULL));
        tuple->items[0] = MP_OBJ_NEW_SMALL_INT(31);
        tuple->items[1] = MP_OBJ_NEW_SMALL_INT(ver.FwVersion[0]);
        tuple->items[2] = MP_OBJ_NEW_SMALL_INT(ver.FwVersion[1]);
        tuple->items[3] = MP_OBJ_NEW_SMALL_INT(ver.FwVersion[2]);
        tuple->items[4] = MP_OBJ_NEW_SMALL_INT(ver.FwVersion[3]);
        mp_obj_list_append(list, MP_OBJ_FROM_PTR(tuple));

        tuple = MP_OBJ_TO_PTR(mp_obj_new_tuple(4, NULL));
        tuple->items[0] = MP_OBJ_NEW_SMALL_INT(ver.PhyVersion[0]);
        tuple->items[1] = MP_OBJ_NEW_SMALL_INT(ver.PhyVersion[1]);
        tuple->items[2] = MP_OBJ_NEW_SMALL_INT(ver.PhyVersion[2]);
        tuple->items[3] = MP_OBJ_NEW_SMALL_INT(ver.PhyVersion[3]);
        mp_obj_list_append(list, MP_OBJ_FROM_PTR(tuple));

        tuple = MP_OBJ_TO_PTR(mp_obj_new_tuple(4, NULL));
        tuple->items[0] = MP_OBJ_NEW_SMALL_INT(ver.NwpVersion[0]);
        tuple->items[1] = MP_OBJ_NEW_SMALL_INT(ver.NwpVersion[1]);
        tuple->items[2] = MP_OBJ_NEW_SMALL_INT(ver.NwpVersion[2]);
        tuple->items[3] = MP_OBJ_NEW_SMALL_INT(ver.NwpVersion[3]);
        mp_obj_list_append(list, MP_OBJ_FROM_PTR(tuple));

        tuple = MP_OBJ_TO_PTR(mp_obj_new_tuple(1, NULL));
        tuple->items[0] = MP_OBJ_NEW_SMALL_INT(ver.RomVersion);
        mp_obj_list_append(list, MP_OBJ_FROM_PTR(tuple));

        tuple = MP_OBJ_TO_PTR(mp_obj_new_tuple(4, NULL));
        tuple->items[0] = MP_OBJ_NEW_SMALL_INT(SL_MAJOR_VERSION_NUM);
        tuple->items[1] = MP_OBJ_NEW_SMALL_INT(SL_MINOR_VERSION_NUM);
        tuple->items[2] = MP_OBJ_NEW_SMALL_INT(SL_VERSION_NUM);
        tuple->items[3] = MP_OBJ_NEW_SMALL_INT(SL_SUB_VERSION_NUM);
        mp_obj_list_append(list, MP_OBJ_FROM_PTR(tuple));

        return list;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(network_stalan_version_obj, network_stalan_version);

STATIC mp_obj_t network_stalan_rtc_now(mp_obj_t self_in) {

    SlDateTime_t slDateTime;
    _u8 status;
    _u8 option = SL_DEVICE_GENERAL_DATE_TIME;
    _u16 configLen = sizeof(SlDateTime_t);

    status = sl_DeviceGet(SL_DEVICE_GENERAL,&option,&configLen,(_u8 *)(&slDateTime));

    if (status < 0) {
        mp_raise_OSError(MP_EIO);
        return mp_const_none;
    }
    else {
        mp_obj_t tuple[6] = {
            MP_OBJ_NEW_SMALL_INT(slDateTime.tm_year),
            MP_OBJ_NEW_SMALL_INT(slDateTime.tm_mon),
            MP_OBJ_NEW_SMALL_INT(slDateTime.tm_day),
            MP_OBJ_NEW_SMALL_INT(slDateTime.tm_hour),
            MP_OBJ_NEW_SMALL_INT(slDateTime.tm_min),
            MP_OBJ_NEW_SMALL_INT(slDateTime.tm_sec),
        };
        mp_obj_tuple_t * t = MP_OBJ_TO_PTR(mp_obj_new_tuple(6, tuple));
        return MP_OBJ_FROM_PTR(t);
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(network_stalan_rtc_now_obj, network_stalan_rtc_now);

STATIC mp_obj_t network_stalan_rtc_set(mp_obj_t self_in, mp_obj_t datetime_in) {
    (void)self_in;
    mp_obj_t *datetime;
    size_t datetime_len;
    mp_obj_get_array(datetime_in, &datetime_len, &datetime);
    if (datetime_len < 3) {
        mp_raise_OSError(MP_EINVAL);
    }

    SlDateTime_t slDateTime = { 0 };
    _u8 status;
    _u8 option = SL_DEVICE_GENERAL_DATE_TIME;
    _u16 configLen = sizeof(SlDateTime_t);

    slDateTime.tm_year = (_u32)mp_obj_get_int(datetime[0]);
    slDateTime.tm_mon  = (_u32)mp_obj_get_int(datetime[1]);
    slDateTime.tm_day  = (_u32)mp_obj_get_int(datetime[2]);
    slDateTime.tm_hour = (_u32)datetime_len > 3 ? mp_obj_get_int(datetime[3]) : 0;
    slDateTime.tm_min  = (_u32)datetime_len > 4 ? mp_obj_get_int(datetime[4]) : 0;
    slDateTime.tm_sec  = (_u32)datetime_len > 5 ? mp_obj_get_int(datetime[5]) : 0;

    status = sl_DeviceSet(SL_DEVICE_GENERAL,option,configLen,(_u8 *)(&slDateTime));

    if (status < 0) {
        mp_raise_OSError(MP_EIO);
        return mp_const_none;
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(network_stalan_rtc_set_obj, network_stalan_rtc_set);

STATIC mp_obj_t network_stalan_fsinfo(mp_obj_t self_in) {

    SlFsControlGetStorageInfoResponse_t storageInfo;
    int status;

    // Create list to return back to MP - check return?
    mp_obj_t list = mp_obj_new_list(0, NULL);
    mp_obj_tuple_t * tuple;

    // Returns number for file entries fetched or error (<0)
    status = sl_FsCtl((SlFsCtl_e)SL_FS_CTL_GET_STORAGE_INFO,
                0,
                NULL,
                NULL,
                0,
                (_u8 *)&storageInfo,
                sizeof(SlFsControlGetStorageInfoResponse_t),
                NULL);

    if (status < 0) {
        mp_raise_msg(&mp_type_OSError, "filesystem storage info fetch error");
        // Not sure if this is needed?
        return mp_const_none;
    }

    tuple = MP_OBJ_TO_PTR(mp_obj_new_tuple(7, NULL));
    tuple->items[0] = MP_OBJ_NEW_SMALL_INT(storageInfo.DeviceUsage.DeviceBlockSize);
    tuple->items[1] = MP_OBJ_NEW_SMALL_INT(storageInfo.DeviceUsage.DeviceBlocksCapacity);
    tuple->items[2] = MP_OBJ_NEW_SMALL_INT(storageInfo.DeviceUsage.NumOfAllocatedBlocks);
    tuple->items[3] = MP_OBJ_NEW_SMALL_INT(storageInfo.DeviceUsage.NumOfReservedBlocks);
    tuple->items[4] = MP_OBJ_NEW_SMALL_INT(storageInfo.DeviceUsage.NumOfReservedBlocksForSystemfiles);
    tuple->items[5] = MP_OBJ_NEW_SMALL_INT(storageInfo.DeviceUsage.LargestAllocatedGapInBlocks);
    tuple->items[6] = MP_OBJ_NEW_SMALL_INT(storageInfo.DeviceUsage.NumOfAvailableBlocksForUserFiles);
    mp_obj_list_append(list, MP_OBJ_FROM_PTR(tuple));

    tuple = MP_OBJ_TO_PTR(mp_obj_new_tuple(9, NULL));
    tuple->items[0] = MP_OBJ_NEW_SMALL_INT(storageInfo.FilesUsage.MaxFsFiles);
    tuple->items[1] = MP_OBJ_NEW_SMALL_INT(storageInfo.FilesUsage.IsDevlopmentFormatType);
    tuple->items[2] = MP_OBJ_NEW_SMALL_INT(storageInfo.FilesUsage.Bundlestate);
    tuple->items[3] = MP_OBJ_NEW_SMALL_INT(storageInfo.FilesUsage.MaxFsFilesReservedForSysFiles);
    tuple->items[4] = MP_OBJ_NEW_SMALL_INT(storageInfo.FilesUsage.ActualNumOfUserFiles);
    tuple->items[5] = MP_OBJ_NEW_SMALL_INT(storageInfo.FilesUsage.ActualNumOfSysFiles);
    tuple->items[6] = MP_OBJ_NEW_SMALL_INT(storageInfo.FilesUsage.NumOfAlerts);
    tuple->items[7] = MP_OBJ_NEW_SMALL_INT(storageInfo.FilesUsage.NumOfAlertsThreshold);
    tuple->items[8] = MP_OBJ_NEW_SMALL_INT(storageInfo.FilesUsage.FATWriteCounter);
    mp_obj_list_append(list, MP_OBJ_FROM_PTR(tuple));

    return list;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(network_stalan_fsinfo_obj, network_stalan_fsinfo);

typedef struct
{
    SlFileAttributes_t attribute;
    char fileName[SL_FS_MAX_FILE_NAME_LENGTH];
} slGetfileList_t;

STATIC mp_obj_t network_stalan_fslist(mp_obj_t self_in) {

#define FSFETCHCOUNT 5
    slGetfileList_t fileList[FSFETCHCOUNT];
    int status = 1;
    _i32 index = -1;  // Initialise with -1 to start from start of FAT

    // Create list to return back to MP - check return?
    mp_obj_t list = mp_obj_new_list(0, NULL);

    while (status > 0) {
        int i;

        // Returns number for file entries fetched or error (<0)
        status = sl_FsGetFileList(&index, FSFETCHCOUNT,
                     (_u8) (SL_FS_MAX_FILE_NAME_LENGTH + sizeof(SlFileAttributes_t)),
                     (_u8 *) fileList,
                     SL_FS_GET_FILE_ATTRIBUTES);

        if (status < 0) {
            mp_raise_msg(&mp_type_OSError, "filesystem fetch error");
            // Not sure if this is needed?
            return mp_const_none;
        }

        // For each file retured
        for (i = 0; i < status; i++) {
            mp_obj_tuple_t * tuple = MP_OBJ_TO_PTR(mp_obj_new_tuple(4, NULL));
            tuple->items[0] = MP_OBJ_NEW_QSTR(qstr_from_str((const char *)fileList[i].fileName));
            tuple->items[1] = MP_OBJ_NEW_SMALL_INT(fileList[i].attribute.FileAllocatedBlocks);
            tuple->items[2] = MP_OBJ_NEW_SMALL_INT(fileList[i].attribute.FileMaxSize);
            tuple->items[3] = MP_OBJ_NEW_SMALL_INT(fileList[i].attribute.Properties);
            mp_obj_list_append(list, MP_OBJ_FROM_PTR(tuple));
	}
    }

    return list;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(network_stalan_fslist_obj, network_stalan_fslist);

// TODO : Temp enum, since SLFS defines are u32 and can't find way to pass that as QSTR without
//        getting converted to signed
enum {
    TEMP_SL_FS_CREATE = 1,
    TEMP_SL_FS_WRITE = 2,
    TEMP_SL_FS_OVERWRITE = 4,
    TEMP_SL_FS_READ = 8
};

STATIC mp_obj_t network_stalan_fsopen(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_filename,    MP_ARG_REQUIRED | MP_ARG_OBJ,  {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_mode,                          MP_ARG_INT,  {.u_int = TEMP_SL_FS_READ} },
        { MP_QSTR_token,                         MP_ARG_INT,  {.u_int = 0 } },
        // create flags
        { MP_QSTR_maxsize,     MP_ARG_KW_ONLY  | MP_ARG_INT,  {.u_int = 0 } },
        { MP_QSTR_failsafe,    MP_ARG_KW_ONLY  | MP_ARG_BOOL, {.u_bool = false } },
        { MP_QSTR_secure,      MP_ARG_KW_ONLY  | MP_ARG_BOOL, {.u_bool = false } },
        { MP_QSTR_nosignature, MP_ARG_KW_ONLY  | MP_ARG_BOOL, {.u_bool = false } },
        { MP_QSTR_statictoken, MP_ARG_KW_ONLY  | MP_ARG_BOOL, {.u_bool = false } },
        { MP_QSTR_vendortoken, MP_ARG_KW_ONLY  | MP_ARG_BOOL, {.u_bool = false } },
        { MP_QSTR_publicwrite, MP_ARG_KW_ONLY  | MP_ARG_BOOL, {.u_bool = false } },
        { MP_QSTR_publicread,  MP_ARG_KW_ONLY  | MP_ARG_BOOL, {.u_bool = false } },
        // write flags
        { MP_QSTR_mustcommit,  MP_ARG_KW_ONLY  | MP_ARG_BOOL, {.u_bool = false } },
        { MP_QSTR_bundle,      MP_ARG_KW_ONLY  | MP_ARG_BOOL, {.u_bool = false } },
        { MP_QSTR_encrypted,   MP_ARG_KW_ONLY  | MP_ARG_BOOL, {.u_bool = false } },
    };
    enum { ARG_filename, ARG_mode, ARG_token,
           ARG_maxsize, ARG_failsafe, ARG_secure, ARG_nosignature,
           ARG_statictoken, ARG_vendortoken, ARG_publicwrite, ARG_publicread,
           ARG_mustcommit, ARG_bundle, ARG_encrypted };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
                     MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // TODO : should we hide the creation mode, by first checking if file exists?
    //        - depends where we want the logic to live for this...

    _i32 filehandle;
    // TODO : replace when able to pass u32 from QSTR INT?
    _u32 accessModeAndMaxSize = 0;//args[ARG_mode].u_int;
    _u32 token = args[ARG_token].u_int;

    // TODO : also might be better to pass as string "wr"
    if (args[ARG_mode].u_int & TEMP_SL_FS_CREATE) {
        accessModeAndMaxSize |= SL_FS_CREATE;
    }
    if (args[ARG_mode].u_int & TEMP_SL_FS_WRITE) {
        accessModeAndMaxSize |= SL_FS_WRITE;
    }
    if (args[ARG_mode].u_int & TEMP_SL_FS_OVERWRITE) {
        accessModeAndMaxSize |= SL_FS_OVERWRITE;
    }
    if (args[ARG_mode].u_int & TEMP_SL_FS_READ) {
        accessModeAndMaxSize |= SL_FS_READ;
    }

    // Don't think this can happen since it's required arg?
    if (args[ARG_filename].u_obj == NULL) {
        mp_raise_msg(&mp_type_OSError, "bad filename");
        return mp_const_none;
    }

    if (accessModeAndMaxSize & SL_FS_CREATE) {
        // TODO : check maxfile size spec
        if (args[ARG_maxsize].u_int == 0 /*|| args[ARG_maxsize].u_int > MAX */) {
            mp_raise_msg(&mp_type_OSError, "bad file size");
            return mp_const_none;
        }

        accessModeAndMaxSize |=
            (args[ARG_failsafe   ].u_bool ? SL_FS_CREATE_FAILSAFE : 0) |
            (args[ARG_secure     ].u_bool ? SL_FS_CREATE_SECURE : 0 ) |
            (args[ARG_nosignature].u_bool ? SL_FS_CREATE_NOSIGNATURE : 0) |
            (args[ARG_statictoken].u_bool ? SL_FS_CREATE_STATIC_TOKEN : 0) |
            (args[ARG_vendortoken].u_bool ? SL_FS_CREATE_VENDOR_TOKEN : 0) |
            (args[ARG_publicwrite].u_bool ? SL_FS_CREATE_PUBLIC_WRITE : 0) |
            (args[ARG_publicread ].u_bool ? SL_FS_CREATE_PUBLIC_READ : 0) |
            SL_FS_CREATE_MAX_SIZE(args[ARG_maxsize].u_int);
    }

    if (accessModeAndMaxSize & SL_FS_WRITE) {
        accessModeAndMaxSize |=
            (args[ARG_mustcommit ].u_bool ? SL_FS_WRITE_MUST_COMMIT : 0) |
            (args[ARG_bundle     ].u_bool ? SL_FS_WRITE_BUNDLE_FILE : 0) |
            (args[ARG_encrypted  ].u_bool ? SL_FS_WRITE_ENCRYPTED : 0);
    }

    filehandle = sl_FsOpen((const _u8 *) mp_obj_str_get_str(args[ARG_filename].u_obj),
                           accessModeAndMaxSize,
                           &token);
    if (filehandle < 0) {
        // TODO : which debug printer should we use to pass error back
        mp_printf(MP_PYTHON_PRINTER, "sl_FsOpen error: %d\n", filehandle);
//        mp_raise_msg(&mp_type_OSError, "sl_FsOpen error");
        return mp_const_none;
    }

    // Return tuple with filehandle and fileToken
    mp_obj_t tuple[2] = {
        mp_obj_new_int(filehandle),
        mp_obj_new_int(token)
//        MP_OBJ_NEW_SMALL_INT(filehandle),
//        MP_OBJ_NEW_SMALL_INT(token)
    };
    mp_obj_tuple_t * t = MP_OBJ_TO_PTR(mp_obj_new_tuple(2, tuple));
    return MP_OBJ_FROM_PTR(t);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(network_stalan_fsopen_obj, 1, network_stalan_fsopen);

STATIC mp_obj_t network_stalan_fsclose(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_filehandle,  MP_ARG_REQUIRED | MP_ARG_INT,  {.u_int = 0 } },
        { MP_QSTR_certfile,                      MP_ARG_OBJ,  {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_signature,                     MP_ARG_OBJ,  {.u_obj = MP_OBJ_NULL} },
        // Use signature = 'A' to abort
    };
    enum { ARG_filehandle, ARG_certfile, ARG_signature };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
                     MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    _i16 status;
    _i32 filehandle = args[ARG_filehandle].u_int;
    _u8 *certfile = NULL;
    _u8 *signature = NULL;
    _u32 signatureLen = 0;

    if (filehandle == 0) {
        mp_raise_msg(&mp_type_OSError, "sl_FsClose error: bad file handle");
        return mp_const_none;
    }

    if (args[ARG_certfile].u_obj != NULL) {
        certfile = (_u8 *) mp_obj_str_get_str(args[ARG_certfile].u_obj);
    }

    if (args[ARG_signature].u_obj != MP_OBJ_NULL) {
        mp_buffer_info_t bufinfo;
        mp_get_buffer_raise(args[ARG_signature].u_obj, &bufinfo, MP_BUFFER_READ);
        signature = bufinfo.buf;
        signatureLen = bufinfo.len;
    }

    status = sl_FsClose(filehandle, certfile, signature, signatureLen);

    if (status < 0) {
        // TODO : which debug printer should we use to pass error back
        mp_printf(MP_PYTHON_PRINTER, "sl_FsClose error: %d\n", status);
        //mp_raise_msg(&mp_type_OSError, "sl_FsClose error");
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(network_stalan_fsclose_obj, 1, network_stalan_fsclose);

// Probably doesn't need to be kwargs func
STATIC mp_obj_t network_stalan_fsread(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_filehandle,  MP_ARG_REQUIRED | MP_ARG_INT,  {.u_int = 0 } },
        { MP_QSTR_offset,      MP_ARG_REQUIRED | MP_ARG_INT,  {.u_int = 0 } },
        { MP_QSTR_len,         MP_ARG_REQUIRED | MP_ARG_INT,  {.u_int = 0 } },
    };
    enum { ARG_filehandle, ARG_offset, ARG_len };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
                     MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    _i32 status;
    _i32 filehandle = args[ARG_filehandle].u_int;
    _u32 offset = args[ARG_offset].u_int;
    _u32 len = args[ARG_len].u_int;

    if (filehandle == 0) {
        mp_raise_msg(&mp_type_OSError, "bad file handle");
        return mp_const_none;
    }

    if (len == 0) {
        mp_raise_msg(&mp_type_OSError, "bad length");
        return mp_const_none;
    }

    // Alloc a byte buffer for the read
    byte *buf = m_new(byte, len);

    if (buf == NULL) {
        mp_raise_msg(&mp_type_OSError, "byte buffer alloc failed");
        return mp_const_none;
    }

    status = sl_FsRead(filehandle, offset, buf, len);

    if (status < 0) {
        m_del(char, buf, len);

        // TODO : which debug printer should we use to pass error back
        mp_printf(MP_PYTHON_PRINTER, "sl_FsRead error: %d\n", status);
        //mp_raise_msg(&mp_type_OSError, "sl_FsClose error");
        return mp_const_none;
    }

    mp_obj_t ret = mp_obj_new_str_of_type(&mp_type_bytes, buf, status);
    m_del(char, buf, len);

    return ret;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(network_stalan_fsread_obj, 1, network_stalan_fsread);

// Probably doesn't need to be kwargs func
STATIC mp_obj_t network_stalan_fswrite(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_filehandle,  MP_ARG_REQUIRED | MP_ARG_INT,  {.u_int = 0 } },
        { MP_QSTR_offset,      MP_ARG_REQUIRED | MP_ARG_INT,  {.u_int = 0 } },
        { MP_QSTR_data,        MP_ARG_REQUIRED | MP_ARG_OBJ,  {.u_obj = MP_OBJ_NULL } },
    };
    enum { ARG_filehandle, ARG_offset, ARG_data };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
                     MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    _i32 status;
    _i32 filehandle = args[ARG_filehandle].u_int;
    _u32 offset = args[ARG_offset].u_int;

    if (filehandle == 0) {
        mp_raise_msg(&mp_type_OSError, "bad file handle");
        return mp_const_none;
    }

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[ARG_data].u_obj, &bufinfo, MP_BUFFER_READ);

    status = sl_FsWrite(filehandle, offset, bufinfo.buf, bufinfo.len);

    if (status < 0) {
        // TODO : which debug printer should we use to pass error back
        mp_printf(MP_PYTHON_PRINTER, "sl_FsWrite error: %d\n", status);
        //mp_raise_msg(&mp_type_OSError, "sl_FsClose error");
        return mp_const_none;
    }

    // Return bytes written
    return MP_OBJ_NEW_SMALL_INT(status);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(network_stalan_fswrite_obj, 1, network_stalan_fswrite);

STATIC mp_obj_t network_stalan_fsdelete(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_filename,    MP_ARG_REQUIRED | MP_ARG_OBJ,  {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_token,                         MP_ARG_INT,  {.u_int = 0 } },
    };
    enum { ARG_filename, ARG_token };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
                     MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    _i16 status;
    _u32 token = args[ARG_token].u_int;

    // Don't think this can happen since it's required arg?
    if (args[ARG_filename].u_obj == NULL) {
        mp_raise_msg(&mp_type_OSError, "bad filename");
        return mp_const_none;
    }

    status = sl_FsDel((const _u8 *) mp_obj_str_get_str(args[ARG_filename].u_obj),
                       token);

    if (status < 0) {
        // TODO : which debug printer should we use to pass error back
        mp_printf(MP_PYTHON_PRINTER, "sl_FsDel error: %d\n", status);
        //mp_raise_msg(&mp_type_OSError, "sl_FsOpen error");
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(network_stalan_fsdelete_obj, 1, network_stalan_fsdelete);

STATIC const mp_rom_map_elem_t network_stalan_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_active), MP_ROM_PTR(&network_stalan_active_obj) },
    { MP_ROM_QSTR(MP_QSTR_connect), MP_ROM_PTR(&network_stalan_connect_obj) },
    { MP_ROM_QSTR(MP_QSTR_disconnect), MP_ROM_PTR(&network_stalan_disconnect_obj) },
    { MP_ROM_QSTR(MP_QSTR_isconnected), MP_ROM_PTR(&network_stalan_isconnected_obj) },
    { MP_ROM_QSTR(MP_QSTR_scan), MP_ROM_PTR(&network_stalan_scan_obj) },
    { MP_ROM_QSTR(MP_QSTR_status), MP_ROM_PTR(&network_stalan_status_obj) },
    { MP_ROM_QSTR(MP_QSTR_ifconfig), MP_ROM_PTR(&network_stalan_ifconfig_obj) },
    { MP_ROM_QSTR(MP_QSTR_version), MP_ROM_PTR(&network_stalan_version_obj) },

    { MP_ROM_QSTR(MP_QSTR_rtc_now), MP_ROM_PTR(&network_stalan_rtc_now_obj) },
    { MP_ROM_QSTR(MP_QSTR_rtc_set), MP_ROM_PTR(&network_stalan_rtc_set_obj) },

    { MP_ROM_QSTR(MP_QSTR_fsinfo), MP_ROM_PTR(&network_stalan_fsinfo_obj) },
    { MP_ROM_QSTR(MP_QSTR_fslist), MP_ROM_PTR(&network_stalan_fslist_obj) },
    { MP_ROM_QSTR(MP_QSTR_fsopen), MP_ROM_PTR(&network_stalan_fsopen_obj) },
    { MP_ROM_QSTR(MP_QSTR_fsclose), MP_ROM_PTR(&network_stalan_fsclose_obj) },
    { MP_ROM_QSTR(MP_QSTR_fsread), MP_ROM_PTR(&network_stalan_fsread_obj) },
    { MP_ROM_QSTR(MP_QSTR_fswrite), MP_ROM_PTR(&network_stalan_fswrite_obj) },
    { MP_ROM_QSTR(MP_QSTR_fsdelete), MP_ROM_PTR(&network_stalan_fsdelete_obj) },

    // TODO : where does this constant come from - due to u32?
    { MP_ROM_QSTR(MP_QSTR_DHCP), MP_ROM_INT(STA_DHCP_ADDR) },

    // TODO : review these
    { MP_ROM_QSTR(MP_QSTR_FS_CREATE), MP_ROM_INT(TEMP_SL_FS_CREATE) },
    { MP_ROM_QSTR(MP_QSTR_FS_WRITE), MP_ROM_INT(TEMP_SL_FS_WRITE) },
    { MP_ROM_QSTR(MP_QSTR_FS_OVERWRITE), MP_ROM_INT(TEMP_SL_FS_OVERWRITE) },
    { MP_ROM_QSTR(MP_QSTR_FS_READ), MP_ROM_INT(TEMP_SL_FS_READ) },

    // WPA Enterprise Methods
    { MP_ROM_QSTR(MP_QSTR_EAP_METHOD_TLS), MP_ROM_INT(SL_WLAN_ENT_EAP_METHOD_TLS) },
    { MP_ROM_QSTR(MP_QSTR_EAP_METHOD_TTLS_TLS), MP_ROM_INT(SL_WLAN_ENT_EAP_METHOD_TTLS_TLS) },
    { MP_ROM_QSTR(MP_QSTR_EAP_METHOD_TTLS_MSCHAPv2), MP_ROM_INT(SL_WLAN_ENT_EAP_METHOD_TTLS_MSCHAPv2) },
    { MP_ROM_QSTR(MP_QSTR_EAP_METHOD_TTLS_PSK), MP_ROM_INT(SL_WLAN_ENT_EAP_METHOD_TTLS_PSK) },
    { MP_ROM_QSTR(MP_QSTR_EAP_METHOD_PEAP0_TLS), MP_ROM_INT(SL_WLAN_ENT_EAP_METHOD_PEAP0_TLS) },
    { MP_ROM_QSTR(MP_QSTR_EAP_METHOD_PEAP0_MSCHAPv2), MP_ROM_INT(SL_WLAN_ENT_EAP_METHOD_PEAP0_MSCHAPv2) },
    { MP_ROM_QSTR(MP_QSTR_EAP_METHOD_PEAP0_PSK), MP_ROM_INT(SL_WLAN_ENT_EAP_METHOD_PEAP0_PSK) },
    { MP_ROM_QSTR(MP_QSTR_EAP_METHOD_PEAP1_TLS), MP_ROM_INT(SL_WLAN_ENT_EAP_METHOD_PEAP1_TLS) },
    { MP_ROM_QSTR(MP_QSTR_EAP_METHOD_PEAP1_PSK), MP_ROM_INT(SL_WLAN_ENT_EAP_METHOD_PEAP1_PSK) },
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
