/* --COPYRIGHT--,BSD
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
 * --/COPYRIGHT--*/

/*
 *  ======== USBCDCD.c ========
 */

#include <stdbool.h>

/* Header files */
#include <ti/display/Display.h>


/* BIOS Header files */
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/MutexP.h>
#include <ti/drivers/dpl/SemaphoreP.h>


/* driverlib Header files */
#include "ti/devices/msp432e4/driverlib/driverlib.h"


/* usblib Header files */
#include <ti/usblib/msp432e4/usb-ids.h>
#include <ti/usblib/msp432e4/usblib.h>
#include <ti/usblib/msp432e4/usbcdc.h>
#include <ti/usblib/msp432e4/device/usbdevice.h>
#include <ti/usblib/msp432e4/device/usbdcdc.h>

/* Example/Board Header files */
#include "USBCDCD.h"

typedef uint32_t            USBCDCDEventType;

/* Defines */
#define USBBUFFERSIZE   256

/* Typedefs */
typedef volatile enum {
    USBCDCD_STATE_IDLE = 0,
    USBCDCD_STATE_INIT,
    USBCDCD_STATE_UNCONFIGURED
} USBCDCD_USBState;

/* Static variables and handles */
static volatile USBCDCD_USBState state;
static unsigned char receiveBuffer[USBBUFFERSIZE];
static unsigned char transmitBuffer[USBBUFFERSIZE];

static MutexP_Handle mutexTxSerial;
static MutexP_Handle mutexRxSerial;
static MutexP_Handle mutexUSBWait;
static SemaphoreP_Handle semTxSerial;
static SemaphoreP_Handle semRxSerial;
static SemaphoreP_Handle semUSBConnected;

/* Function prototypes */
static USBCDCDEventType cbRxHandler(void *cbData, USBCDCDEventType event,
                                    USBCDCDEventType eventMsg,
                                    void *eventMsgPtr);
static USBCDCDEventType cbSerialHandler(void *cbData, USBCDCDEventType event,
                                        USBCDCDEventType eventMsg,
                                        void *eventMsgPtr);
static USBCDCDEventType cbTxHandler(void *cbData, USBCDCDEventType event,
                                    USBCDCDEventType eventMsg,
                                    void *eventMsgPtr);
static void USBCDCD_hwiHandler(uintptr_t arg0);
static unsigned int rxData(unsigned char *pStr,
                           unsigned int length,
                           unsigned int timeout);
static unsigned int txData(const unsigned char *pStr,
                           int length, unsigned int timeout);
void USBCDCD_init(bool usbInternal);
unsigned int USBCDCD_receiveData(unsigned char *pStr,
                                 unsigned int length,
                                 unsigned int timeout);
unsigned int USBCDCD_sendData(const unsigned char *pStr,
                              unsigned int length,
                              unsigned int timeout);
bool USBCDCD_waitForConnect(unsigned int timeout);

/* The languages supported by this device. */
const unsigned char langDescriptor[] = {
    4,
    USB_DTYPE_STRING,
    USBShort(USB_LANG_EN_US)
};

/* The manufacturer string. */
const unsigned char manufacturerString[] = {
    (17 + 1) * 2,
    USB_DTYPE_STRING,
    'T', 0, 'e', 0, 'x', 0, 'a', 0, 's', 0, ' ', 0, 'I', 0, 'n', 0, 's', 0,
    't', 0, 'r', 0, 'u', 0, 'm', 0, 'e', 0, 'n', 0, 't', 0, 's', 0,
};

/* The product string. */
const unsigned char productString[] = {
    2 + (16 * 2),
    USB_DTYPE_STRING,
    'V', 0, 'i', 0, 'r', 0, 't', 0, 'u', 0, 'a', 0, 'l', 0, ' ', 0,
    'C', 0, 'O', 0, 'M', 0, ' ', 0, 'P', 0, 'o', 0, 'r', 0, 't', 0
};

/* The serial number string. */
const unsigned char serialNumberString[] = {
    (7 + 1) * 2,
    USB_DTYPE_STRING,
    'T', 0, 'i', 0, 'L', 0, 'D', 0, 'A', 0, ' ', 0
};

/* The interface description string. */
const unsigned char controlInterfaceString[] = {
    2 + (21 * 2),
    USB_DTYPE_STRING,
    'A', 0, 'C', 0, 'M', 0, ' ', 0, 'C', 0, 'o', 0, 'n', 0, 't', 0,
    'r', 0, 'o', 0, 'l', 0, ' ', 0, 'I', 0, 'n', 0, 't', 0, 'e', 0,
    'r', 0, 'f', 0, 'a', 0, 'c', 0, 'e', 0
};

/* The configuration description string. */
const unsigned char configString[] = {
    2 + (26 * 2),
    USB_DTYPE_STRING,
    'S', 0, 'e', 0, 'l', 0, 'f', 0, ' ', 0, 'P', 0, 'o', 0, 'w', 0,
    'e', 0, 'r', 0, 'e', 0, 'd', 0, ' ', 0, 'C', 0, 'o', 0, 'n', 0,
    'f', 0, 'i', 0, 'g', 0, 'u', 0, 'r', 0, 'a', 0, 't', 0, 'i', 0,
    'o', 0, 'n', 0
};

/* The descriptor string table. */
const unsigned char * const stringDescriptors[] = {
    langDescriptor,
    manufacturerString,
    productString,
    serialNumberString,
    controlInterfaceString,
    configString
};

#define STRINGDESCRIPTORSCOUNT (sizeof(stringDescriptors) / \
                                sizeof(unsigned char *))

tUSBBuffer txBuffer;
tUSBBuffer rxBuffer;
static tUSBDCDCDevice serialDevice;

tUSBBuffer rxBuffer = {
    false,                      /* This is a receive buffer. */
    cbRxHandler,                /* pfnCallback */
    (void *)&serialDevice,      /* Callback data is our device pointer. */
    USBDCDCPacketRead,          /* pfnTransfer */
    USBDCDCRxPacketAvailable,   /* pfnAvailable */
    (void *)&serialDevice,      /* pvHandle */
    receiveBuffer,              /* pcBuffer */
    USBBUFFERSIZE,              /* ulBufferSize */
    {{0, 0, 0, 0}, 0, 0}        /* private data workspace */
};

tUSBBuffer txBuffer = {
    true,                       /* This is a transmit buffer. */
    cbTxHandler,                /* pfnCallback */
    (void *)&serialDevice,      /* Callback data is our device pointer. */
    USBDCDCPacketWrite,         /* pfnTransfer */
    USBDCDCTxPacketAvailable,   /* pfnAvailable */
    (void *)&serialDevice,      /* pvHandle */
    transmitBuffer,             /* pcBuffer */
    USBBUFFERSIZE,              /* ulBufferSize */
    {{0, 0, 0, 0}, 0, 0}        /* private data workspace */
};

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

    stringDescriptors,
    STRINGDESCRIPTORSCOUNT
};

static tLineCoding g_sLineCoding = {
    115200,                     /* 115200 baud rate. */
    1,                          /* 1 Stop Bit. */
    0,                          /* No Parity. */
    8                           /* 8 Bits of data. */
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
static USBCDCDEventType cbRxHandler(void *cbData, USBCDCDEventType event,
                                    USBCDCDEventType eventMsg,
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
static USBCDCDEventType cbSerialHandler(void *cbData, USBCDCDEventType event,
                                        USBCDCDEventType eventMsg,
                                        void *eventMsgPtr)
{
    tLineCoding *psLineCoding;

    /* Determine what event has happened */
    switch (event) {
        case USB_EVENT_CONNECTED:
            state = USBCDCD_STATE_INIT;
            SemaphoreP_post(semUSBConnected);
            break;

        case USB_EVENT_DISCONNECTED:
            state = USBCDCD_STATE_UNCONFIGURED;
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
static USBCDCDEventType cbTxHandler(void *cbData, USBCDCDEventType event,
                                    USBCDCDEventType eventMsg,
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
 *  ======== USBCDCD_hwiHandler ========
 *  This function calls the USB library's device interrupt handler.
 */
static void USBCDCD_hwiHandler(uintptr_t arg0)
{
    USB0_IRQDeviceHandler();
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
 *  ======== USBCDCD_init ========
 */
void USBCDCD_init(bool usbInternal)
{
    HwiP_Handle hwi;
    uint32_t ui32ULPI;

    // LWK TODO: fix these to just go out over the repl uart? mp_hal_stdout_tx_str()??
    // Display_Handle display;

    // Display_init();

    // /* Open the display for output */
    // display = Display_open(Display_Type_UART, NULL);

    // if (display == NULL) {
    //     /* Failed to open display driver */
    //     while (1);
    // }

    /* Install interrupt handler */
    hwi = HwiP_create(INT_USB0, USBCDCD_hwiHandler, NULL);
    if (hwi == NULL) {
        // Display_printf(display, 0, 0, "Can't create USB Hwi.\n");
        while(1);
    }

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

    semUSBConnected = SemaphoreP_createBinary(0);
    if (semUSBConnected == NULL) {
        // Display_printf(display, 0, 0, "Can't create USB semaphore.\n");
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

    mutexUSBWait = MutexP_create(NULL);
    if (mutexUSBWait == NULL) {
        // Display_printf(display, 0, 0, "Could not create USB Wait mutex.\n");
        while(1);
    }

    /* State specific variables */
    state = USBCDCD_STATE_UNCONFIGURED;

    /* Check if the ULPI mode is to be used or not */
    if(!(usbInternal)) {
        ui32ULPI = USBLIB_FEATURE_ULPI_HS;
        USBDCDFeatureSet(0, USBLIB_FEATURE_USBULPI, &ui32ULPI);
    }

    // LWK TODO: need to check this to see if its looking at PB1 and or ignoreing it
    // we should handle via PQ4 or selves if needed
    
    /* Set the USB stack mode to Device mode with VBUS monitoring */
    USBStackModeSet(0, eUSBModeForceDevice, 0);

    /*
     * Pass our device information to the USB HID device class driver,
     * initialize the USB controller and connect the device to the bus.
     */
    if (!USBDCDCInit(0, &serialDevice)) {
        // Display_printf(display, 0, 0, "Error initializing the serial device.\n");
        while(1);
    }
    // Display_close(display);
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

    switch (state) {
        case USBCDCD_STATE_UNCONFIGURED:
            USBCDCD_waitForConnect(timeout);
            break;

        case USBCDCD_STATE_INIT:
            /* Acquire lock */
            key = MutexP_lock(mutexRxSerial);

            USBBufferInit(&txBuffer);
            USBBufferInit(&rxBuffer);

            state = USBCDCD_STATE_IDLE;

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

    switch (state) {
        case USBCDCD_STATE_UNCONFIGURED:
            USBCDCD_waitForConnect(timeout);
            break;

        case USBCDCD_STATE_INIT:
            /* Acquire lock */
            key = MutexP_lock(mutexTxSerial);

            USBBufferInit(&txBuffer);
            USBBufferInit(&rxBuffer);

            state = USBCDCD_STATE_IDLE;

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

/*
 *  ======== USBCDCD_waitForConnect ========
 */
bool USBCDCD_waitForConnect(unsigned int timeout)
{
    bool ret = true;
    unsigned int key;

    /* Need exclusive access to prevent a race condition */
    key = MutexP_lock(mutexUSBWait);

    if (state == USBCDCD_STATE_UNCONFIGURED) {
        if (SemaphoreP_pend(semUSBConnected, timeout)== SemaphoreP_TIMEOUT) {
            ret = false;
        }
    }

    MutexP_unlock(mutexUSBWait, key);

    return (ret);
}
