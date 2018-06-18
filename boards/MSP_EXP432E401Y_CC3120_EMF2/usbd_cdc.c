/*
 * This file is part of the Micro Python project, http://micropython.org/
 * And the Electromagnetic Field: TiLDA Mk4 Badge
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Electromagnetic Field Badge Team Authors
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

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

/* DriverLib Includes */
#include <ti/devices/msp432e4/driverlib/driverlib.h>

/* BIOS Header files */
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/MutexP.h>
#include <ti/drivers/dpl/SemaphoreP.h>

/* usblib Header files */
#include <ti/usblib/msp432e4/usblib.h>
#include <ti/usblib/msp432e4/usbcdc.h>
#include <ti/usblib/msp432e4/usb-ids.h>
#include <ti/usblib/msp432e4/device/usbdevice.h>
#include <ti/usblib/msp432e4/device/usbdcdc.h>
#include <ti/usblib/msp432e4/device/usbdcomp.h>

/* board Header files */
#include "usb.h"
#include "usbd_cdc.h"

#define UART_BUFFER_SIZE        256

/* Typedefs */
typedef volatile enum {
    USBCDCD_STATE_IDLE = 0,
    USBCDCD_STATE_INIT,
    USBCDCD_STATE_UNCONFIGURED
} USBCDCD_USBState;

/* Static variables and handles */
static volatile USBCDCD_USBState stateCDC;
static uint8_t receiveBuffer[UART_BUFFER_SIZE];
static uint8_t transmitBuffer[UART_BUFFER_SIZE];
tUSBBuffer txBuffer;
tUSBBuffer rxBuffer;
static tUSBDCDCDevice serialDevice;

static MutexP_Handle mutexTxSerial;
static MutexP_Handle mutexRxSerial;
static SemaphoreP_Handle semTxSerial;
static SemaphoreP_Handle semRxSerial;

static tLineCoding g_sLineCoding = {
    115200,                     /* 115200 baud rate. */
    1,                          /* 1 Stop Bit. */
    0,                          /* No Parity. */
    8                           /* 8 Bits of data. */
};


/* Function prototypes */
uint32_t cbRxHandler(void *cbData, uint32_t event,
                                    uint32_t eventMsg,
                                    void *eventMsgPtr);
uint32_t cbSerialHandler(void *cbData, uint32_t event,
                                        uint32_t eventMsg,
                                        void *eventMsgPtr);
uint32_t cbTxHandler(void *cbData, uint32_t event,
                                    uint32_t eventMsg,
                                    void *eventMsgPtr);

static unsigned int rxData(unsigned char *pStr,
                           unsigned int length,
                           unsigned int timeout);
static unsigned int txData(const unsigned char *pStr,
                           int length, unsigned int timeout);
void USBD_CDC_init(tCompositeEntry *psCompEntry);
unsigned int USBCDCD_receiveData(unsigned char *pStr,
                                 unsigned int length,
                                 unsigned int timeout);
unsigned int USBCDCD_sendData(const unsigned char *pStr,
                              unsigned int length,
                              unsigned int timeout);


//*****************************************************************************
//
// The CDC device initialization and customization structures. In this case,
// we are using USBBuffers between the CDC device class driver and the
// application code. The function pointers and callback data values are set
// to insert a buffer in each of the data channels, transmit and receive.
//
// With the buffer in place, the CDC channel callback is set to the relevant
// channel function and the callback data is set to point to the channel
// instance data. The buffer, in turn, has its callback set to the application
// function and the callback data set to our CDC instance structure.
//
//*****************************************************************************
static tUSBDCDCDevice serialDevice = {
    USB_VID_TI_1CBE,
    USB_PID_SERIAL,
    0,
    USB_CONF_ATTR_SELF_PWR,

    cbSerialHandler,
    NULL,

    USBBufferEventCallback,
    (void *)&rxBuffer,

    USBBufferEventCallback,
    (void *)&txBuffer,

    0,
    0
};

//*****************************************************************************
//
// Receive buffer (from the USB perspective).
//
//*****************************************************************************
tUSBBuffer rxBuffer = {
    false,                      /* This is a receive buffer. */
    cbRxHandler,                /* pfnCallback */
    (void *)&serialDevice,      /* Callback data is our device pointer. */
    USBDCDCPacketRead,          /* pfnTransfer */
    USBDCDCRxPacketAvailable,   /* pfnAvailable */
    (void *)&serialDevice,      /* pvHandle */
    receiveBuffer,              /* pcBuffer */
    UART_BUFFER_SIZE,              /* ulBufferSize */
    {{0, 0, 0, 0}, 0, 0}        /* private data workspace */
};

//*****************************************************************************
//
// Transmit buffer (from the USB perspective).
//
//*****************************************************************************
tUSBBuffer txBuffer = {
    true,                       /* This is a transmit buffer. */
    cbTxHandler,                /* pfnCallback */
    (void *)&serialDevice,      /* Callback data is our device pointer. */
    USBDCDCPacketWrite,         /* pfnTransfer */
    USBDCDCTxPacketAvailable,   /* pfnAvailable */
    (void *)&serialDevice,      /* pvHandle */
    transmitBuffer,             /* pcBuffer */
    UART_BUFFER_SIZE,              /* ulBufferSize */
    {{0, 0, 0, 0}, 0, 0}        /* private data workspace */
};

/*
 *  ======== cbRxHandler ========
 *  Callback handler for the USB stack.
 *
 *  Callback handler call by the USB stack to notify us on what has happened in
 *  regards to the keyboard.
 *
 *  @param(cbData)          A callback pointer provided by the client.
 *
 *  @param(event)           Identifies the event that occurred in regards to
 *                          this device.
 *
 *  @param(eventMsgData)    A data value associated with a particular event.
 *
 *  @param(eventMsgPtr)     A data pointer associated with a particular event.
 *
 */
uint32_t cbRxHandler(void *cbData, uint32_t event,
                                    uint32_t eventMsg,
                                    void *eventMsgPtr)
{
    switch (event) {
        case USB_EVENT_RX_AVAILABLE:
            SemaphoreP_post(semRxSerial);
            break;

        case USB_EVENT_DATA_REMAINING:
            break;

        case USB_EVENT_REQUEST_BUFFER:
            break;

        default:
            break;
    }

    return (0);
}

/*
 *  ======== cbSerialHandler ========
 *  Callback handler for the USB stack.
 *
 *  Callback handler call by the USB stack to notify us on what has happened in
 *  regards to the keyboard.
 *
 *  @param(cbData)          A callback pointer provided by the client.
 *
 *  @param(event)           Identifies the event that occurred in regards to
 *                          this device.
 *
 *  @param(eventMsgData)    A data value associated with a particular event.
 *
 *  @param(eventMsgPtr)     A data pointer associated with a particular event.
 *
 */
uint32_t cbSerialHandler(void *cbData, uint32_t event,
                                        uint32_t eventMsg,
                                        void *eventMsgPtr)
{
    tLineCoding *psLineCoding;

    /* Determine what event has happened */
    switch (event) {
        case USB_EVENT_CONNECTED:
            stateCDC = USBCDCD_STATE_INIT;
            break;

        case USB_EVENT_DISCONNECTED:
            stateCDC = USBCDCD_STATE_UNCONFIGURED;
            break;

        case USBD_CDC_EVENT_GET_LINE_CODING:
            /* Create a pointer to the line coding information. */
            psLineCoding = (tLineCoding *)eventMsgPtr;

            /* Copy the current line coding information into the structure. */
            *(psLineCoding) = g_sLineCoding;
            break;

        case USBD_CDC_EVENT_SET_LINE_CODING:
            /* Create a pointer to the line coding information. */
            psLineCoding = (tLineCoding *)eventMsgPtr;

            /*
             * Copy the line coding information into the current line coding
             * structure.
             */
            g_sLineCoding = *(psLineCoding);
            break;

        case USBD_CDC_EVENT_SET_CONTROL_LINE_STATE:
            break;

        case USBD_CDC_EVENT_SEND_BREAK:
            break;

        case USBD_CDC_EVENT_CLEAR_BREAK:
            break;

        case USB_EVENT_SUSPEND:
            break;

        case USB_EVENT_RESUME:
            break;

        default:
            break;
    }

    return (0);
}

/*
 *  ======== cbTxHandler ========
 *  Callback handler for the USB stack.
 *
 *  Callback handler call by the USB stack to notify us on what has happened in
 *  regards to the keyboard.
 *
 *  @param(cbData)          A callback pointer provided by the client.
 *
 *  @param(event)           Identifies the event that occurred in regards to
 *                          this device.
 *
 *  @param(eventMsgData)    A data value associated with a particular event.
 *
 *  @param(eventMsgPtr)     A data pointer associated with a particular event.
 *
 */
uint32_t cbTxHandler(void *cbData, uint32_t event,
                                    uint32_t eventMsg,
                                    void *eventMsgPtr)
{
    switch (event) {
        case USB_EVENT_TX_COMPLETE:
            /*
             * Data was sent, so there should be some space available on the
             * buffer
             */
            SemaphoreP_post(semTxSerial);
            break;

        default:
            break;
    }

    return (0);
}


/*
 *  ======== rxData ========
 */
static unsigned int rxData(unsigned char *pStr,
                           unsigned int length,
                           unsigned int timeout)
{
    unsigned int read = 0;

    if (USBBufferDataAvailable(&rxBuffer) || !(SemaphoreP_pend(semRxSerial, timeout) == SemaphoreP_TIMEOUT)) {
       read = USBBufferRead(&rxBuffer, pStr, length);
    }

    return (read);
}


/*
 *  ======== txData ========
 */
static unsigned int txData(const unsigned char *pStr,
                           int length, unsigned int timeout)
{
    unsigned int buffAvailSize;
    unsigned int bufferedCount = 0;
    unsigned int sendCount = 0;
    unsigned char *sendPtr;

    while (bufferedCount != length) {
        /* Determine the buffer size available */
        buffAvailSize = USBBufferSpaceAvailable(&txBuffer);

        /* Determine how much needs to be sent */
        if ((length - bufferedCount) > buffAvailSize) {
            sendCount = buffAvailSize;
        }
        else {
            sendCount = length - bufferedCount;
        }

        /* Adjust the pointer to the data */
        sendPtr = (unsigned char *)pStr + bufferedCount;

        /* Place the contents into the USB BUffer */
        bufferedCount += USBBufferWrite(&txBuffer, sendPtr, sendCount);

        /* Pend until some data was sent through the USB*/
        if (SemaphoreP_pend(semTxSerial, timeout) == SemaphoreP_TIMEOUT) {
            break;
        }
    }

    return (bufferedCount);
}


/*
 *  ======== USBD_CDC_init ========
 */
void USBD_CDC_init(tCompositeEntry *psCompEntry)
{

    /* RTOS primitives */
    semTxSerial = SemaphoreP_createBinary(0);
    if (semTxSerial == NULL) {
        // Display_printf(display, 0, 0, "Can't create TX semaphore.\n");
        while(1);
    }

    semRxSerial = SemaphoreP_createBinary(0);
    if (semRxSerial == NULL) {
        // Display_printf(display, 0, 0, "Can't create RX semaphore.\n");
        while(1);
    }

    mutexTxSerial = MutexP_create(NULL);
    if (mutexTxSerial == NULL) {
        // Display_printf(display, 0, 0, "Can't create TX mutex.\n");
        while(1);
    }

    mutexRxSerial = MutexP_create(NULL);
    if (mutexRxSerial == NULL) {
        // Display_printf(display, 0, 0, "Can't create RX mutex.\n");
        while(1);
    }

    /* State specific variables */
    stateCDC = USBCDCD_STATE_UNCONFIGURED;

    /*
     * Pass our device information to the USB HID device class driver,
     * initialize the USB controller and connect the device to the bus.
     */
    if (!USBDCDCCompositeInit(0, &serialDevice, psCompEntry)) {
        //Can't initialize CDC composite component
        // Display_printf(display, 0, 0, "Can't initialize CDC composite component.\n");
        while(1);
    }
}


/*
 *  ======== USBCDCD_receiveData ========
 */
unsigned int USBCDCD_receiveData(unsigned char *pStr,
                                 unsigned int length,
                                 unsigned int timeout)
{
    unsigned int retValue = 0;
    unsigned int key;

    switch (stateCDC) {
        case USBCDCD_STATE_UNCONFIGURED:
            USB_waitForConnect(timeout);
            break;

        case USBCDCD_STATE_INIT:
            /* Acquire lock */
            key = MutexP_lock(mutexRxSerial);

            USBBufferInit(&txBuffer);
            USBBufferInit(&rxBuffer);

            stateCDC = USBCDCD_STATE_IDLE;

            retValue = rxData(pStr, length, timeout);

            /* Release lock */
            MutexP_unlock(mutexRxSerial, key);
            break;

        case USBCDCD_STATE_IDLE:
            /* Acquire lock */
            key = MutexP_lock(mutexRxSerial);

            retValue = rxData(pStr, length, timeout);

            /* Release lock */
            MutexP_unlock(mutexRxSerial, key);
            break;

        default:
            break;
    }

    return (retValue);
}


/*
 *  ======== USBCDCD_sendData ========
 */
unsigned int USBCDCD_sendData(const unsigned char *pStr,
                              unsigned int length,
                              unsigned int timeout)
{
    unsigned int retValue = 0;
    unsigned int key;

    switch (stateCDC) {
        case USBCDCD_STATE_UNCONFIGURED:
            USB_waitForConnect(timeout);
            break;

        case USBCDCD_STATE_INIT:
            /* Acquire lock */
            key = MutexP_lock(mutexTxSerial);

            USBBufferInit(&txBuffer);
            USBBufferInit(&rxBuffer);

            stateCDC = USBCDCD_STATE_IDLE;

            retValue = txData(pStr, length, timeout);

            /* Release lock */
            MutexP_unlock(mutexTxSerial, key);
            break;

        case USBCDCD_STATE_IDLE:
            /* Acquire lock */
            key = MutexP_lock(mutexTxSerial);

            retValue = txData(pStr, length, timeout);

            /* Release lock */
            MutexP_unlock(mutexTxSerial, key);
            break;

        default:
            break;
    }

    return (retValue);
}

