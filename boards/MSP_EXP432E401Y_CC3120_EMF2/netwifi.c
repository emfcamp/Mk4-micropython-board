/*
 * Copyright (c) 2017, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdbool.h>

#include <ti/drivers/GPIO.h>

#include <ti/display/Display.h>
#include <ti/drivers/net/wifi/netapp.h>
#include <ti/drivers/net/wifi/simplelink.h>
#include <ti/drivers/net/wifi/wlan.h>

#include <unistd.h>

#include "Board.h"
#include "wificonfig.h"

SlNetCfgIpV4Args_t ipV4;

static uint32_t deviceConnected = false;
static uint32_t ipAcquired = false;
static uint32_t currButton, prevButton;

//extern Display_Handle AWSIOT_display;

//extern void startNTP(void);

void SimpleLinkHttpServerEventHandler(
        SlNetAppHttpServerEvent_t *pSlHttpServerEvent,
        SlNetAppHttpServerResponse_t *pSlHttpServerResponse)
{
}

void SimpleLinkNetAppRequestEventHandler(SlNetAppRequest_t *pNetAppRequest,
        SlNetAppResponse_t *pNetAppResponse)
{
}

void SimpleLinkNetAppRequestMemFreeEventHandler(uint8_t *buffer)
{
}

/*
 *  ======== SimpleLinkWlanEventHandler ========
 *  SimpleLink Host Driver callback for handling WLAN connection or
 *  disconnection events.
 */
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pArgs)
{
    switch (pArgs->Id) {
        case SL_WLAN_EVENT_CONNECT:
            deviceConnected = true;
            break;

        case SL_WLAN_EVENT_DISCONNECT:
            deviceConnected = false;
            break;

        default:
            break;
    }
}

//*****************************************************************************
//
//! The Function Handles the Fatal errors
//!
//! \param[in]  slFatalErrorEvent - Pointer to Fatl Error Event info
//!
//! \return     None
//!
//*****************************************************************************
void SimpleLinkFatalErrorEventHandler(SlDeviceFatal_t *slFatalErrorEvent)
{
#if 0
    switch (slFatalErrorEvent->Id)
    {
        case SL_DEVICE_EVENT_FATAL_DEVICE_ABORT:
            Display_printf(AWSIOT_display, 0, 0,
                    "[ERROR] - FATAL ERROR: Abort NWP event detected: "
                    "AbortType=%d, AbortData=0x%x\n\r",
                    slFatalErrorEvent->Data.DeviceAssert.Code,
                    slFatalErrorEvent->Data.DeviceAssert.Value);
            break;

        case SL_DEVICE_EVENT_FATAL_DRIVER_ABORT:
            Display_printf(AWSIOT_display, 0, 0,
                    "[ERROR] - FATAL ERROR: Driver Abort detected. \n\r");
            break;

        case SL_DEVICE_EVENT_FATAL_NO_CMD_ACK:
            Display_printf(AWSIOT_display, 0, 0,
                    "[ERROR] - FATAL ERROR: No Cmd Ack detected "
                    "[cmd opcode = 0x%x] \n\r",
                    slFatalErrorEvent->Data.NoCmdAck.Code);
            break;

        case SL_DEVICE_EVENT_FATAL_SYNC_LOSS:
            Display_printf(AWSIOT_display, 0, 0,
                    "[ERROR] - FATAL ERROR: Sync loss detected n\r");
            break;

        case SL_DEVICE_EVENT_FATAL_CMD_TIMEOUT:
            Display_printf(AWSIOT_display, 0, 0,
                    "[ERROR] - FATAL ERROR: Async event timeout detected "
                    "[event opcode =0x%x]  \n\r",
                    slFatalErrorEvent->Data.CmdTimeout.Code);
            break;

        default:
            Display_printf(AWSIOT_display, 0, 0,
                    "[ERROR] - FATAL ERROR: Unspecified error detected \n\r");
            break;
    }
#endif
}

/*
 *  ======== SimpleLinkNetAppEventHandler ========
 *  SimpleLink Host Driver callback for asynchoronous IP address events.
 */
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pArgs)
{
    switch (pArgs->Id) {
        case SL_NETAPP_EVENT_IPV4_ACQUIRED:
            ipAcquired = true;
            break;

        default:
            break;
    }
}

/*
 *  ======== SimpleLinkSockEventHandler ========
 */
void SimpleLinkSockEventHandler(SlSockEvent_t *pArgs)
{

}

/*
 *  ======== SimpleLinkGeneralEventHandler ========
 */
void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *sockEvent)
{

}
#if 0
/*
 *  ======== smartConfigFxn ========
 */
void smartConfigFxn()
{
  uint8_t policyVal;

  /* Set auto connect policy */
  sl_WlanPolicySet(SL_WLAN_POLICY_CONNECTION,
          SL_WLAN_CONNECTION_POLICY(1, 0, 0, 0), &policyVal,
          sizeof(policyVal));

  /* Start SmartConfig using unsecured method. */
  sl_WlanProvisioning(SL_WLAN_PROVISIONING_CMD_START_MODE_SC, ROLE_STA, 30,
          NULL, 0);
}

/*
 *  ======== setStationMode ========
 *  Sets the SimpleLink Wi-Fi in station mode and enables DHCP client
 */
void setStationMode(void)
{
    int           mode;
    int           response;

    mode = sl_Start(0, 0, 0);
    if (mode < 0) {
//        Display_printf(AWSIOT_display, 0, 0,
//                "sl_Start error (%d): "
//                "Could not initialize SimpleLink Wi-Fi\n", mode);
        while(1);
    }

    /* Change network processor to station mode */
    if (mode != ROLE_STA) {
        sl_WlanSetMode(ROLE_STA);

        /* Restart network processor */
        sl_Stop(0);
        mode = sl_Start(0, 0, 0);
        if (mode < 0) {
//            Display_printf(AWSIOT_display, 0, 0,
//                    "Failed to set SimpleLink Wi-Fi to Station mode (%d)\n",
//                    mode);
            while(1);
        }
    }

    sl_WlanDisconnect();
    /* Set auto connect policy */
    response = sl_WlanPolicySet(SL_WLAN_POLICY_CONNECTION,
            SL_WLAN_CONNECTION_POLICY(1, 0, 0, 0), NULL, 0);
    if (response < 0) {
//        Display_printf(AWSIOT_display, 0, 0,
//                "Failed to set connection policy to auto connect (%d)\n",
//                response);
    }

    /* Enable DHCP client */
    response = sl_NetCfgSet(SL_NETCFG_IPV4_STA_ADDR_MODE, SL_NETCFG_ADDR_DHCP,
            0, 0);

    if (response < 0) {
//        Display_printf(AWSIOT_display, 0, 0,
//                "Could not enable DHCP client (%d)\n", response);
    }

    sl_Stop(0);

    /* Set connection variables to initial values */
    deviceConnected = false;
    ipAcquired = false;
}

/*
 *  ======== wlanConnect =======
 *  Secure connection parameters
 */
static int wlanConnect()
{
    SlWlanSecParams_t secParams = {0};
    int ret = 0;

    // return 0;

    secParams.Key = (signed char *)SECURITY_KEY;
    if (secParams.Key) {
        secParams.KeyLen = strlen((const char *)secParams.Key);
    }
    else {
        secParams.KeyLen = 0;
    }
    secParams.Type = SECURITY_TYPE;

    ret = sl_WlanConnect((signed char*)SSID, strlen((const char*)SSID),
            NULL, &secParams, NULL);

    return (ret);
}

/*
 *  ======== NetWifi_init =======
 *  Initialize Wifi
 */
void NetWiFi_init()
{
    uint16_t           len = sizeof(ipV4);
    uint16_t           dhcpIsOn;

    /* Turn LED OFF. It will be used as a connection indicator */
//    GPIO_write(Board_LED0, Board_LED_OFF);
//    GPIO_write(Board_LED1, Board_LED_OFF);
//    GPIO_write(Board_LED2, Board_LED_OFF);

    setStationMode();

    /* Host driver starts the network processor */
    if (sl_Start(NULL, NULL, NULL) < 0) {
        //Display_printf(AWSIOT_display, 0, 0, "Could not initialize WiFi\n");
    }

    if (wlanConnect() < 0) {
        //Display_printf(AWSIOT_display, 0, 0, "Could not connect to WiFi AP\n");
    }

    /*
     * Wait for the WiFi to connect to an AP. If a profile for the AP in
     * use has not been stored yet, press Board_BUTTON0 to start SmartConfig.
     */
    int timeOut = 2000;  // 2000 ms
    while (((deviceConnected != true) || (ipAcquired != true)) && timeOut > 0) {
        /*
         *  Start SmartConfig if a button is pressed. This could be done with
         *  GPIO interrupts, but for simplicity polling is used to check the
         *  button.
         */
        currButton = GPIO_read(Board_GPIO_BUTTON0);
        if ((currButton == 0) && (prevButton != 0)) {
            smartConfigFxn();
        }
        prevButton = currButton;
        usleep(50000);
        timeOut -= 50;
    }

    /* Retrieve & print the IP address */
    sl_NetCfgGet(SL_NETCFG_IPV4_STA_ADDR_MODE, &dhcpIsOn, &len,
            (unsigned char *)&ipV4);
//    Display_printf(AWSIOT_display, 0, 0,
//            "CC3220 has connected to AP and acquired an IP address.\n");
//    Display_printf(AWSIOT_display, 0, 0,
//            "IP Address: %d.%d.%d.%d\n", SL_IPV4_BYTE(ipV4.Ip, 3),
//            SL_IPV4_BYTE(ipV4.Ip, 2), SL_IPV4_BYTE(ipV4.Ip, 1),
//            SL_IPV4_BYTE(ipV4.Ip, 0));

    /* Use NTP to get the current time, as needed for SSL authentication */
//    startNTP();

    // GPIO_write(Board_LED0, Board_LED_ON);
}

/*
 *  ======== NetWifi_isConnected =======
 */
uint32_t NetWiFi_isConnected()
{
    return (deviceConnected);
}

/*
 *  ======== NetWifi_exit =======
 */
void NetWiFi_exit()
{
}

#endif



