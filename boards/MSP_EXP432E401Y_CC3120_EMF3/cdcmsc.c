#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <ti/sysbios/hal/Hwi.h>

#include <ti/devices/msp432e4/driverlib/driverlib.h>
#include <ti/usblib/msp432e4/usblib.h>
#include <ti/usblib/msp432e4/usb-ids.h>
#include <ti/usblib/msp432e4/usbcdc.h>
#include <ti/usblib/msp432e4/device/usbdevice.h>
#include <ti/usblib/msp432e4/device/usbdcomp.h>
#include <ti/usblib/msp432e4/device/usbdcdc.h>
#include <ti/usblib/msp432e4/device/usbdmsc.h>

#include "SCMSC.h"

static const unsigned char langDescriptor[] = {
    4,
    USB_DTYPE_STRING,
    USBShort(USB_LANG_EN_US)
};

static const unsigned char manufacturerString[] = {
    (17 + 1) * 2,
    USB_DTYPE_STRING,
    'T', 0, 'e', 0, 'x', 0, 'a', 0, 's', 0, ' ', 0, 'I', 0, 'n', 0, 's', 0,
    't', 0, 'r', 0, 'u', 0, 'm', 0, 'e', 0, 'n', 0, 't', 0, 's', 0,
};

//
// The product string.
//
//*****************************************************************************
const uint8_t productString[] =
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
const uint8_t serialNumberString[] =
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
const uint8_t controlInterfaceString[] =
{
    2 + (21 * 2),
    USB_DTYPE_STRING,
    'T', 0, 'i', 0, 'L', 0, 'D', 0, 'A', 0, ' ', 0, 'I', 0, 'n', 0, 't', 0, 'e', 0,
    'r', 0, 'f', 0, 'a', 0, 'c', 0, 'e', 0
};

#if 0
static const unsigned char productString[] = {
    2 + (16 * 2),
    USB_DTYPE_STRING,
    'V', 0, 'i', 0, 'r', 0, 't', 0, 'u', 0, 'a', 0, 'l', 0, ' ', 0,
    'C', 0, 'O', 0, 'M', 0, ' ', 0, 'P', 0, 'o', 0, 'r', 0, 't', 0
};

static const unsigned char serialNumberString[] = {
    (8 + 1) * 2,
    USB_DTYPE_STRING,
    '1', 0, '2', 0, '3', 0, '4', 0, '5', 0, '6', 0, '7', 0, '8', 0
};

static const unsigned char controlInterfaceString[] = {
    2 + (21 * 2),
    USB_DTYPE_STRING,
    'A', 0, 'C', 0, 'M', 0, ' ', 0, 'C', 0, 'o', 0, 'n', 0, 't', 0,
    'r', 0, 'o', 0, 'l', 0, ' ', 0, 'I', 0, 'n', 0, 't', 0, 'e', 0,
    'r', 0, 'f', 0, 'a', 0, 'c', 0, 'e', 0
};
#endif
static const unsigned char configString[] = {
    2 + (26 * 2),
    USB_DTYPE_STRING,
    'S', 0, 'e', 0, 'l', 0, 'f', 0, ' ', 0, 'P', 0, 'o', 0, 'w', 0,
    'e', 0, 'r', 0, 'e', 0, 'd', 0, ' ', 0, 'C', 0, 'o', 0, 'n', 0,
    'f', 0, 'i', 0, 'g', 0, 'u', 0, 'r', 0, 'a', 0, 't', 0, 'i', 0,
    'o', 0, 'n', 0
};

static const unsigned char * const stringDescriptors[] = {
    langDescriptor,
    manufacturerString,
    productString,
    serialNumberString,
    controlInterfaceString,
    configString
};

#define STRINGDESCRIPTORSCOUNT (sizeof(stringDescriptors) / \
                                sizeof(unsigned char *))

extern uint32_t CDCD_serialHandler(void *cbData, uint32_t event,
                                      uint32_t eventMsg,
                                      void *eventMsgPtr);

static tUSBDCDCDevice serialDevice = {
    .ui16VID = USB_VID_TI_1CBE,
    .ui16PID = USB_PID_SERIAL,
    .ui16MaxPowermA = 0,
    .ui8PwrAttributes = USB_CONF_ATTR_SELF_PWR,
    .pvControlCBData = NULL,
    .pfnTxCallback = USBBufferEventCallback,
    .pfnRxCallback = USBBufferEventCallback,
    .ppui8StringDescriptors = NULL, // stringDescriptors,
    .ui32NumStringDescriptors = 0, // STRINGDESCRIPTORSCOUNT,
};

static tUSBDCDCDevice uartDevice = {
    .ui16VID = USB_VID_TI_1CBE,
    .ui16PID = USB_PID_SERIAL + 1,
    .ui16MaxPowermA = 0,
    .ui8PwrAttributes = USB_CONF_ATTR_SELF_PWR,
    .pvControlCBData = NULL,
    .pfnTxCallback = USBBufferEventCallback,
    .pfnRxCallback = USBBufferEventCallback,
    .ppui8StringDescriptors = NULL, // stringDescriptors,
    .ui32NumStringDescriptors = 0, // STRINGDESCRIPTORSCOUNT,
};

static tUSBDMSCDevice mscDevice = {
    .ui16VID = USB_VID_TI_1CBE,
    .ui16PID = USB_PID_MSC,
    .pui8Vendor = "EMF",
    .pui8Product = "TiLDA MK4",
    .pui8Version = {1, 2, 3, 4},
    .ui16MaxPowermA = 0,
    .ui8PwrAttributes = USB_CONF_ATTR_SELF_PWR,
    .ppui8StringDescriptors = NULL, //stringDescriptors,
    .ui32NumStringDescriptors = 0, // STRINGDESCRIPTORSCOUNT,
    .sMediaFunctions = {
        .pfnOpen = SCMSC_open,
        .pfnClose = SCMSC_close,
        .pfnBlockRead = SCMSC_read,
        .pfnBlockWrite = SCMSC_write,
        .pfnNumBlocks = SCMSC_getNumBlocks,
        .pfnBlockSize = SCMSC_getBlockSize
    },
    .pfnEventCallback = NULL,
};

static uint32_t compositeHandler(void * data, uint32_t event, uint32_t param,
                                 void * msg)
{
    return 0;
}

#define NUM_CDC 1

static uint8_t descriptorData[COMPOSITE_DMSC_SIZE + (NUM_CDC * COMPOSITE_DCDC_SIZE)];

extern bool MSCD_setup(tUSBDMSCDevice * dev);

extern void * CDCD_create(tUSBDCDCDevice * dev);

static tCompositeEntry compositeDevices[1 + NUM_CDC];

static tUSBDCompositeDevice compDevice =
{
    .ui16VID = USB_VID_TI_1CBE,
    .ui16PID = USB_PID_COMP_SERIAL,
    .ui16MaxPowermA = 250u,
    .ui8PwrAttributes = USB_CONF_ATTR_BUS_PWR,
    .pfnCallback = compositeHandler,
    .ppui8StringDescriptors = stringDescriptors,
    .ui32NumStringDescriptors = STRINGDESCRIPTORSCOUNT,
    .ui32NumDevices = 1 + NUM_CDC,
    .psDevices = compositeDevices
};

const uint32_t CDCD_count = NUM_CDC;
void * CDCD_handles[NUM_CDC];

bool CDCMSC_setup()
{
    Hwi_create(INT_USB0, (Hwi_FuncPtr)USB0_IRQDeviceHandler, NULL, NULL);

    CDCD_handles[0] = CDCD_create(&serialDevice);
    // CDCD_handles[1] = CDCD_create(&uartDevice);

    MSCD_setup(&mscDevice);

    USBStackModeSet(0, eUSBModeDevice, 0);

    USBDMSCCompositeInit(0, &mscDevice, &compositeDevices[0]);

    USBDCDCCompositeInit(0, &serialDevice, &compositeDevices[1]);
    // USBDCDCCompositeInit(0, &uartDevice, &compositeDevices[2]);

    USBDCompositeInit(0, &compDevice, sizeof(descriptorData), descriptorData);

    return true;
}
