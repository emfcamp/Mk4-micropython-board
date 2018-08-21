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
#include <ti/usblib/msp432e4/usb-ids.h>
#include <ti/usblib/msp432e4/usblib.h>
#include <ti/usblib/msp432e4/usbcdc.h>
#include <ti/usblib/msp432e4/device/usbdevice.h>
#include <ti/usblib/msp432e4/device/usbdcomp.h>
#include <ti/usblib/msp432e4/device/usbdcdc.h>
#include <ti/usblib/msp432e4/device/usbdmsc.h>

/* board Header files */
#include "mpconfigboard.h"
#include "usb.h"
#include "usbd_cdc.h"
#include "usbd_msc.h"


#if (MICROPY_HW_USB_REPL == 0) && (MICROPY_HW_USB_MSC == 1)
#error "Can not enable USB MSC with out USB REPL"
#endif

#define NUM_COMP_DEVICES        (1 + MICROPY_HW_USB_MSC)
#define DESCRIPTOR_DATA_SIZE    (COMPOSITE_DCDC_SIZE + (COMPOSITE_DMSC_SIZE * MICROPY_HW_USB_MSC))


/* Typedefs */
typedef volatile enum {
    USB_CONNECTED = 0,
    USB_DISCONNECTED
} USB_USBState;

/* Static variables and handles */
static USB_USBState     stateUSB;
static MutexP_Handle mutexUSBWait;
static SemaphoreP_Handle semUSBConnected;

/* Function prototypes */
static void  USB_hwiHandler(uintptr_t arg0);
uint32_t cbCompositeHandler(void *cbData, uint32_t event,
                                uint32_t eventMsg, void *eventMsgPtr);
void USB_Comp_init();
bool USB_waitForConnect(unsigned int timeout);

extern void USB0_IRQDeviceHandler(void);

//*****************************************************************************
//
// The languages supported by this device.
//
//*****************************************************************************
const uint8_t g_pui8LangDescriptor[] =
{
    4,
    USB_DTYPE_STRING,
    USBShort(USB_LANG_EN_US)
};

//*****************************************************************************
//
// The manufacturer string.
//
//*****************************************************************************
const uint8_t g_pui8ManufacturerString[] =
{
    (17 + 1) * 2,
    USB_DTYPE_STRING,
    'T', 0, 'e', 0, 'x', 0, 'a', 0, 's', 0, ' ', 0, 'I', 0, 'n', 0, 's', 0,
    't', 0, 'r', 0, 'u', 0, 'm', 0, 'e', 0, 'n', 0, 't', 0, 's', 0,
};

//*****************************************************************************
//
// The product string.
//
//*****************************************************************************
const uint8_t g_pui8ProductString[] =
{
    2 + (9 * 2),
    USB_DTYPE_STRING,
    'T', 0, 'i', 0, 'L', 0, 'D', 0, 'A', 0, ' ', 0, 'M', 0, 'k', 0,
    0xB4, 0x03
};

//*****************************************************************************
//
// The serial number string.
//
//*****************************************************************************
const uint8_t g_pui8SerialNumberString[] =
{
    (7 + 1) * 2,
    USB_DTYPE_STRING,
    'T', 0, 'i', 0, 'L', 0, 'D', 0, 'A', 0, ' ', 0
};

//*****************************************************************************
//
// The control interface description string.
//
//*****************************************************************************
const uint8_t g_pui8ControlInterfaceString[] =
{
    2 + (21 * 2),
    USB_DTYPE_STRING,
    'T', 0, 'i', 0, 'L', 0, 'D', 0, 'A', 0, ' ', 0, 'I', 0, 'n', 0, 't', 0, 'e', 0,
    'r', 0, 'f', 0, 'a', 0, 'c', 0, 'e', 0
};

//*****************************************************************************
//
// The configuration description string.
//
//*****************************************************************************
const uint8_t g_pui8ConfigString[] =
{
    2 + (26 * 2),
    USB_DTYPE_STRING,
    'S', 0, 'e', 0, 'l', 0, 'f', 0, ' ', 0, 'P', 0, 'o', 0, 'w', 0,
    'e', 0, 'r', 0, 'e', 0, 'd', 0, ' ', 0, 'C', 0, 'o', 0, 'n', 0,
    'f', 0, 'i', 0, 'g', 0, 'u', 0, 'r', 0, 'a', 0, 't', 0, 'i', 0,
    'o', 0, 'n', 0
};

//*****************************************************************************
//
// The descriptor string table.
//
//*****************************************************************************
const uint8_t * const g_pui8StringDescriptors[] =
{
    g_pui8LangDescriptor,
    g_pui8ManufacturerString,
    g_pui8ProductString,
    g_pui8SerialNumberString,
    g_pui8ControlInterfaceString,
    g_pui8ConfigString
};

#define NUM_STRING_DESCRIPTORS (sizeof(g_pui8StringDescriptors) /             \
                                sizeof(uint8_t *))

//****************************************************************************
//
// The memory allocated to hold the composite descriptor that is created by
// the call to USBDCompositeInit().
//
//****************************************************************************
uint8_t g_pui8DescriptorData[DESCRIPTOR_DATA_SIZE];

tCompositeEntry g_psCompEntries[NUM_COMP_DEVICES];

//****************************************************************************
//
// Allocate the Device Data for the top level composite device class.
//
//****************************************************************************
tUSBDCompositeDevice g_sCompDevice =
{
    //
    // Stellaris VID.
    //
    USB_VID_TI_1CBE,

    //
    // Stellaris PID for composite serial device.
    //
    USB_PID_COMP_SERIAL,

    //
    // This is in 2mA increments so 500mA.
    //
    250,

    //
    // Bus powered device.
    //
    USB_CONF_ATTR_BUS_PWR,  // LWK TODO: this should technically be SELF not BUS

    //
    // A composite event handler.
    //
    cbCompositeHandler,

    //
    // The string table.
    //
    g_pui8StringDescriptors,
    NUM_STRING_DESCRIPTORS,

    //
    // The Composite device array.
    //
    NUM_COMP_DEVICES,
    g_psCompEntries
};

/*
 *  ======== USB_hwiHandler ========
 *  This function calls the USB library's device interrupt handler.
 */
static void USB_hwiHandler(uintptr_t arg0)
{
    USB0_IRQDeviceHandler();
}

/*
 *  ======== cbCompositeHandler ========
 *  Callback handler for the USB stack.
 *
 *  Callback handler call by the USB stack to notify us on what has happened in
 *  regards to the usb connection.
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
uint32_t cbCompositeHandler(void *cbData, uint32_t event,
                                uint32_t eventMsg, void *eventMsgPtr)
{
    /* Determine what event has happened */
    switch (event) {
        case USB_EVENT_CONNECTED:
            if (stateUSB != USB_CONNECTED) {
                stateUSB = USB_CONNECTED;

                SemaphoreP_post(semUSBConnected);
            }
            break;

        case USB_EVENT_DISCONNECTED:
            stateUSB = USB_DISCONNECTED;
            break;

        default:
            break;
    }

    return (0);
}

/*
 *  ======== USB_Comp_init ========
 */
void USB_Comp_init()
{
    HwiP_Handle hwi;
    
    /* Install interrupt handler */
    hwi = HwiP_create(INT_USB0, USB_hwiHandler, NULL);
    if (hwi == NULL) {
        // Display_printf(display, 0, 0, "Can't create USB Hwi.\n");
        while(1);
    }

    semUSBConnected = SemaphoreP_createBinary(0);
    if (semUSBConnected == NULL) {
        // Display_printf(display, 0, 0, "Can't create USB semaphore.\n");
        while(1);
    }

    mutexUSBWait = MutexP_create(NULL);
    if (mutexUSBWait == NULL) {
        // Display_printf(display, 0, 0, "Could not create USB Wait mutex.\n");
        while(1);
    }


    /* Forcing device mode so that the VBUS and ID pins are not used or monitored by the USB controller. */
    USBStackModeSet(0, eUSBModeForceDevice, 0);

    #if MICROPY_HW_USB_MSC
    // if we are a CDC+MSC we have to list the MSC first 
    // if enabled init the MSC device as entry 0 in our composite device 
    USBD_MSC_init(&g_psCompEntries[0]);
    // init the CDC device as entry 1 in of our composite device
    USBD_CDC_init(&g_psCompEntries[1]);
    #else
    // for a CDC only device
    // init the CDC device as entry 0 in of our composite device
    USBD_CDC_init(&g_psCompEntries[0]);
    #endif
    


    //
    // Pass the device information to the USB library and place the device
    // on the bus.
    //
    USBDCompositeInit(0, &g_sCompDevice, DESCRIPTOR_DATA_SIZE,
                      g_pui8DescriptorData);

}


/*
 *  ======== USB_waitForConnect ========
 */
bool USB_waitForConnect(unsigned int timeout)
{
    bool ret = true;
    unsigned int key;

    /* Need exclusive access to prevent a race condition */
    key = MutexP_lock(mutexUSBWait);

    if (stateUSB == USB_CONNECTED) {
        if (SemaphoreP_pend(semUSBConnected, timeout)== SemaphoreP_TIMEOUT) {
            ret = false;
        }
    }

    MutexP_unlock(mutexUSBWait, key);

    return (ret);
}
