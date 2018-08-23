/*
 * Copyright (c) 2018, Texas Instruments Incorporated
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <ti/drivers/dpl/SemaphoreP.h>

#include <ti/usblib/msp432e4/usblib.h>
#include <ti/usblib/msp432e4/usbcdc.h>
#include <ti/usblib/msp432e4/device/usbdevice.h>
#include <ti/usblib/msp432e4/device/usbdcdc.h>

#include "CDCD.h"

#define CDCD_STATE_UNCONFIGURED 0
#define CDCD_STATE_IDLE 1

typedef struct CDCD_Object {
    uint32_t state;
    tUSBBuffer rxBuf;
    tUSBBuffer txBuf;
    tLineCoding lineCoding;
    SemaphoreP_Handle rxAvail;
    SemaphoreP_Handle connected;
    SemaphoreP_Handle txComplete;
} CDCD_Object;

static uint32_t handleCDC(void * obj, uint32_t event, uint32_t param,
                          void * msgData)
{
    CDCD_Handle cdcd = (CDCD_Handle)obj;

    switch (event) {
        case USB_EVENT_CONNECTED:
            cdcd->state = CDCD_STATE_IDLE;
            USBBufferInit(&cdcd->rxBuf);
            USBBufferInit(&cdcd->txBuf);
            SemaphoreP_post(cdcd->connected);
            break;

        case USB_EVENT_DISCONNECTED:
            cdcd->state = CDCD_STATE_UNCONFIGURED;
            SemaphoreP_pend(cdcd->connected, 0);
            break;

        case USBD_CDC_EVENT_GET_LINE_CODING:
            *(tLineCoding *)msgData = cdcd->lineCoding;
            break;

        case USBD_CDC_EVENT_SET_LINE_CODING:
            cdcd->lineCoding = *(tLineCoding *)msgData;
            break;

        case USBD_CDC_EVENT_SET_CONTROL_LINE_STATE:
        case USBD_CDC_EVENT_SEND_BREAK:
        case USBD_CDC_EVENT_CLEAR_BREAK:
        case USB_EVENT_SUSPEND:
        case USB_EVENT_RESUME:
        default:
            break;
    }

    return (0);
}

static uint32_t handleRx(void * obj, uint32_t event, uint32_t param,
                         void * msgData)
{
    CDCD_Handle cdcd = (CDCD_Handle)obj;

    if (event == USB_EVENT_RX_AVAILABLE) {
        SemaphoreP_post(cdcd->rxAvail);
    }

    return (0);
}

static uint32_t handleTx(void * obj, uint32_t event, uint32_t param,
                         void * msgData)
{
    CDCD_Handle cdcd = (CDCD_Handle)obj;

    if (event == USB_EVENT_TX_COMPLETE) {
        SemaphoreP_post(cdcd->txComplete);
    }

    return (0);
}

static uint32_t rxData(CDCD_Handle cdcd, void * buf, uint32_t length,
                       uint32_t timeout)
{
    uint32_t status = 0;

    if (USBBufferDataAvailable(&cdcd->rxBuf) ||
        (SemaphoreP_pend(cdcd->rxAvail, timeout) != SemaphoreP_TIMEOUT)) {

        status = USBBufferRead(&cdcd->rxBuf, buf, length);
    }

    return (status);
}

static uint32_t txData(CDCD_Handle cdcd, const void * buf, uint32_t length,
                       uint32_t timeout)
{
    uint32_t buffAvailSize;
    uint32_t bufferedCount = 0;
    uint32_t sendCount = 0;
    uint8_t * sendPtr;

    // TODO: cleanup
    while (bufferedCount != length) {
        /* Determine the buffer size available */
        buffAvailSize = USBBufferSpaceAvailable(&cdcd->txBuf);

        /* Determine how much needs to be sent */
        if ((length - bufferedCount) > buffAvailSize) {
            sendCount = buffAvailSize;
        }
        else {
            sendCount = length - bufferedCount;
        }

        /* Adjust the pointer to the data */
        sendPtr = (uint8_t *)buf + bufferedCount;

        /* Place the contents into the USB BUffer */
        bufferedCount += USBBufferWrite(&cdcd->txBuf, sendPtr, sendCount);

        // TODO: pend first - init sem to be +1?
        /* Pend until some data was sent through the USB*/
        if (SemaphoreP_pend(cdcd->txComplete, timeout) == SemaphoreP_TIMEOUT) {
            break;
        }
    }

    return (bufferedCount);
}

bool CDCD_waitForConnect(CDCD_Handle cdcd, uint32_t timeout)
{
    bool status = true;

    if (cdcd->state == CDCD_STATE_UNCONFIGURED) {
        if (SemaphoreP_pend(cdcd->connected, timeout) == SemaphoreP_TIMEOUT) {
            status = false;
        }
    }

    return (status);
}

uint32_t CDCD_available(CDCD_Handle cdcd)
{
    return USBBufferDataAvailable(&cdcd->rxBuf);
}

uint32_t CDCD_receiveData(CDCD_Handle cdcd, void * buf, uint32_t length,
                          uint32_t timeout)
{
    uint32_t status = 0;

    if (cdcd->state == CDCD_STATE_IDLE || CDCD_waitForConnect(cdcd, timeout)) {
        status = rxData(cdcd, buf, length, timeout);
    }

    return (status);
}

uint32_t CDCD_sendData(CDCD_Handle cdcd, const void * buf, uint32_t length,
                       uint32_t timeout)
{
    uint32_t status = 0;

    if (cdcd->state == CDCD_STATE_IDLE || CDCD_waitForConnect(cdcd, timeout)) {
        status = txData(cdcd, buf, length, timeout);
    }

    return (status);
}

static bool setupBuf(tUSBBuffer * usbbuf, CDCD_Handle cdcd,
                     tUSBDCDCDevice * cdc, bool tx, uint32_t bufSize)
{
    usbbuf->bTransmitBuffer = tx;
    usbbuf->pvCBData = cdcd;
    usbbuf->pvHandle = cdc;
    // TODO: check malloc return
    usbbuf->pui8Buffer = calloc(1, bufSize);
    usbbuf->ui32BufferSize = bufSize;

    return true;
}

CDCD_Handle CDCD_create(tUSBDCDCDevice * cdcDevice)
{
    CDCD_Handle cdcd = calloc(1, sizeof(struct CDCD_Object));
    static const tLineCoding DEFLINECODING = {
        .ui32Rate = 115200u,
        .ui8Databits = 8,
        .ui8Parity = 0,
        .ui8Stop = 1,
    };
    static const uint32_t bufSize = 128u;

    if (cdcd) {
        *(tUSBCallback *)&cdcDevice->pfnControlCallback = handleCDC;
        cdcDevice->pvControlCBData = cdcd;
        cdcDevice->pvRxCBData = &cdcd->rxBuf;
        cdcDevice->pvTxCBData = &cdcd->txBuf;

        cdcd->state = CDCD_STATE_UNCONFIGURED;
        cdcd->lineCoding = DEFLINECODING;

        setupBuf(&cdcd->rxBuf, cdcd, cdcDevice, false, bufSize);
        cdcd->rxBuf.pfnCallback = handleRx;
        cdcd->rxBuf.pfnTransfer = USBDCDCPacketRead;
        cdcd->rxBuf.pfnAvailable = USBDCDCRxPacketAvailable;
        USBBufferInit(&cdcd->rxBuf);

        setupBuf(&cdcd->txBuf, cdcd, cdcDevice, true, bufSize);
        cdcd->txBuf.pfnCallback = handleTx;
        cdcd->txBuf.pfnTransfer = USBDCDCPacketWrite;
        cdcd->txBuf.pfnAvailable = USBDCDCTxPacketAvailable;;
        USBBufferInit(&cdcd->txBuf);

        // TODO: check returns
        cdcd->connected = SemaphoreP_createBinary(0);
        cdcd->rxAvail = SemaphoreP_createBinary(0);
        cdcd->txComplete = SemaphoreP_createBinary(0);
    }

    return (cdcd);
}

void CDCD_init(void)
{
}

CDCD_Handle CDCD_open(uint32_t index)
{
    // TODO: open flag?
    return (index < CDCD_count) ? CDCD_handles[index] : NULL;
}
