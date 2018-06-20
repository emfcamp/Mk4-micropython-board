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
#include <ti/usblib/msp432e4/usbmsc.h>
#include <ti/usblib/msp432e4/usb-ids.h>
#include <ti/usblib/msp432e4/device/usbdevice.h>
#include <ti/usblib/msp432e4/device/usbdmsc.h>
#include <ti/usblib/msp432e4/device/usbdcomp.h>

/* board Header files */
#include "usb.h"
#include "usbd_msc.h"

/* storage.c function declarations, can't include storage.h due to mp paths */
void storage_init(void);
uint32_t storage_get_block_size(void);
uint32_t storage_get_block_count(void);
void storage_irq_handler(void);
void storage_flush(void);
bool storage_read_block(uint8_t *dest, uint32_t block);
bool storage_write_block(const uint8_t *src, uint32_t block);

// these return 0 on success, non-zero on error
unsigned storage_read_blocks(uint8_t *dest, uint32_t block_num, uint32_t num_blocks);
unsigned storage_write_blocks(const uint8_t *src, uint32_t block_num, uint32_t num_blocks);

/* Typedefs */
typedef volatile enum {
    USBMSC_STATE_IDLE = 0,
    USBMSC_STATE_INIT,
    USBMSC_STATE_DISCONNECTED,
    USBMSC_STATE_READ,
    USBMSC_STATE_WRITE
} USBMSC_USBState;

/* Static variables and handles */
static volatile USBMSC_USBState stateMSC;

static tUSBDMSCDevice mscDevice;

static int deviceID = 1;

/* Function prototypes */

uint32_t USBDMSCEventCallback(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgParam,
                     void *pvMsgData);
void USBD_MSC_init(tCompositeEntry *psCompEntry);



//*****************************************************************************
//
// The bulk device initialization and customization structures. In this case,
// we are using USBBuffers between the bulk device class driver and the
// application code. The function pointers and callback data values are set
// to insert a buffer in each of the data channels, transmit and receive.
//
// With the buffer in place, the bulk channel callback is set to the relevant
// channel function and the callback data is set to point to the channel
// instance data. The buffer, in turn, has its callback set to the application
// function and the callback data set to our bulk instance structure.
//
//*****************************************************************************
static tUSBDMSCDevice mscDevice = {
    USB_VID_TI_1CBE,
    USB_PID_MSC,
    "TI      ",
    "Mass Storage    ",
    "1.00",
    500,
    USB_CONF_ATTR_SELF_PWR,
    // Composite device so no descriptors
    0,
    0,
    // tMSCDMedia functions
    {
        USBDMSCStorageOpen,
        USBDMSCStorageClose,
        USBDMSCStorageRead,
        USBDMSCStorageWrite,
        USBDMSCStorageNumBlocks,
        USBDMSCStorageBlockSize
    },
    USBDMSCEventCallback
};


//*****************************************************************************
//
// This function is the call back notification function provided to the USB
// library's mass storage class.
//
//*****************************************************************************
uint32_t
USBDMSCEventCallback(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgParam,
                     void *pvMsgData)
{
    //
    // Reset the time out every time an event occurs.
    //
    // g_ui32IdleTimeout = USBMSC_ACTIVITY_TIMEOUT;

    switch (ui32Event) {
        case USB_EVENT_CONNECTED:
            stateMSC = USBMSC_STATE_IDLE;
            break;

        case USBD_MSC_EVENT_WRITING:
            if(stateMSC != USBMSC_STATE_WRITE) {
                stateMSC = USBMSC_STATE_WRITE;
            }
            break;

        case USBD_MSC_EVENT_READING:
            if(stateMSC != USBMSC_STATE_READ) {
                stateMSC = USBMSC_STATE_READ;
            }
            break;

        case USB_EVENT_DISCONNECTED:
            stateMSC = USBMSC_STATE_DISCONNECTED;
            break;


        case USBD_MSC_EVENT_IDLE:
        default:
            break;
    }

    return(0);
}



/*
 *  ======== USBD_MSC_init ========
 */
void USBD_MSC_init(tCompositeEntry *psCompEntry)
{
    /* State specific variables */
    stateMSC = USBMSC_STATE_DISCONNECTED;

    /* Install the composite instances */
    if (!USBDMSCCompositeInit(0, &mscDevice, psCompEntry)) {
        //Can't initialize CDC composite component
        // Display_printf(display, 0, 0, "Can't initialize MSC composite component.\n");
        while(1);
    }
}


//*****************************************************************************
//
// This function opens the drive number and prepares it for use by the Mass
// storage class device.
//
// \param ulDrive is the driver number to open.
//
// This function is used to initialize and open the physical drive number
// associated with the parameter \e ulDrive.  The function will return zero if
// the drive has already been opened.
//
// \return Returns a pointer to data that should be passed to other APIs or it
// will return 0 if no drive was found.
//
//*****************************************************************************
void *
USBDMSCStorageOpen(uint32_t ulDrive)
{
    // ASSERT(ulDrive == 0);

    // //
    // // Return if already in use.
    // //
    // if(g_sDriveInformation.ulFlags & SPIFLASH_IN_USE)
    // {
    //     return(0);
    // }

    // //
    // // Flash is in use.
    // //
    // g_sDriveInformation.ulFlags = SPIFLASH_IN_USE;

    // return((void *)&g_sDriveInformation);
    
    // LWK
    storage_init();
    return (void *)(&deviceID);
}

//*****************************************************************************
//
// This function close the drive number in use by the mass storage class device.
//
// \param pvDrive is the pointer that was returned from a call to
// USBDMSCStorageOpen().
//
// This function is used to close the physical drive number associated with the
// parameter \e pvDrive.  This function will return 0 if the drive was closed
// successfully and any other value will indicate a failure.
//
// \return Returns 0 if the drive was successfully closed or non-zero for a
// failure.
//
//*****************************************************************************
void
USBDMSCStorageClose(void *pvDrive)
{
    // ASSERT(pvDrive != 0);

    // //
    // // Clear all flags.
    // //
    // g_sDriveInformation.ulFlags = 0;

    // LWK
    // ?????
    storage_flush();
}

//*****************************************************************************
//
// This function will read a block from a device opened by the
// USBDMSCStorageOpen() call.
//
// \param pvDrive is the pointer that was returned from a call to
// USBDMSCStorageOpen().
// \param pucData is the buffer that data will be written into.
// \param ui32NumBlocks is the number of blocks to read.
//
// This function is use to read blocks from a physical device and return them
// in the \e pucData buffer.  The data area pointed to by \e pucData should be
// at least \e ui32NumBlocks * Block Size bytes to prevent overwriting data.
//
// \return Returns the number of bytes that were read from the device.
//
//*****************************************************************************
uint32_t
USBDMSCStorageRead(void *pvDrive, uint8_t *pui8Data, uint32_t ui32Sector,
                   uint32_t ui32NumBlocks)
{
    // ASSERT(pvDrive != 0);

    // g_ui32ReadCount += ui32NumBlocks * MX66L51235F_BLOCK_SIZE;

    // MX66L51235FRead(ui32Sector * MX66L51235F_BLOCK_SIZE, pui8Data, 
    //                 ui32NumBlocks * MX66L51235F_BLOCK_SIZE);

    // return(ui32NumBlocks * MX66L51235F_BLOCK_SIZE);
    
    // LWK
    storage_read_blocks(pui8Data, ui32Sector, ui32NumBlocks);
    return (ui32NumBlocks * storage_get_block_size());
    // return (0);
}

//*****************************************************************************
//
// This function will write a block to a device opened by the
// USBDMSCStorageOpen() call.
//
// \param pvDrive is the pointer that was returned from a call to
// USBDMSCStorageOpen().
// \param pucData is the buffer that data will be used for writing.
// \param ui32NumBlocks is the number of blocks to write.
//
// This function is use to write blocks to a physical device from the buffer
// pointed to by the \e pucData buffer.  If the number of blocks is greater
// than one then the block address will increment and write to the next block
// until \e ui32NumBlocks * Block Size bytes have been written.
//
// \return Returns the number of bytes that were written to the device.
//
//*****************************************************************************
uint32_t
USBDMSCStorageWrite(void *pvDrive, uint8_t *pui8Data, uint32_t ui32Sector,
                    uint32_t ui32NumBlocks)
{
    // uint32_t ui32Idx, ui32BlockAddr;
    // uint32_t ui32PageIdx;

    // ASSERT(pvDrive != 0);

    // g_ui32WriteCount += ui32NumBlocks * MX66L51235F_BLOCK_SIZE;

    // for(ui32Idx = 0; ui32Idx < ui32NumBlocks; ui32Idx++)
    // {
    //     //
    //     // each block is 4K(0x1000) bytes
    //     //
    //     ui32BlockAddr = (ui32Sector + ui32Idx) * MX66L51235F_BLOCK_SIZE;

    //     //
    //     // erase the block
    //     // 
    //     MX66L51235FSectorErase(ui32BlockAddr);

    //     //
    //     // program the block one page(256 bytes) a time
    //     // 
    //     for(ui32PageIdx = 0; ui32PageIdx < (MX66L51235F_BLOCK_SIZE / 256);
    //         ui32PageIdx++)
    //     {
    //         MX66L51235FPageProgram((ui32BlockAddr + (ui32PageIdx * 256)), 
    //                                (pui8Data +
    //                                 (ui32Idx * MX66L51235F_BLOCK_SIZE) +
    //                                 (ui32PageIdx * 256)), 256);
    //     }
    // }

    // return(ui32NumBlocks * MX66L51235F_BLOCK_SIZE);
    
    // LWK
    storage_write_blocks(pui8Data, ui32Sector, ui32NumBlocks);
    return (ui32NumBlocks * storage_get_block_size());
    // return (0);
}

//*****************************************************************************
//
// This function will return the number of blocks present on a device.
//
// \param pvDrive is the pointer that was returned from a call to
// USBDMSCStorageOpen().
//
// This function is used to return the total number of blocks on a physical
// device based on the \e pvDrive parameter.
//
// \return Returns the number of blocks that are present in a device.
//
//*****************************************************************************
uint32_t
USBDMSCStorageNumBlocks(void *pvDrive)
{
    // LWK
    return storage_get_block_count();
    // return (0);
}

//*****************************************************************************
//
// This function will return the block size on a device.
//
// \param pvDrive is the pointer that was returned from a call to
// USBDMSCStorageOpen().
//
// This function is used to return the block size on a physical
// device based on the \e pvDrive parameter.
//
// \return Returns the block size for a device.
//
//*****************************************************************************
uint32_t
USBDMSCStorageBlockSize(void *pvDrive)
{
    // LWK
    return storage_get_block_size();
    // return (0);
}


