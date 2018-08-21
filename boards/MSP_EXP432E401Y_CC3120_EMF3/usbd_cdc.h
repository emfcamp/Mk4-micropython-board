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

#ifndef __USBD_CDC_H
#define __USBD_CDC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ti/usblib/msp432e4/usblib.h>
#include <ti/usblib/msp432e4/device/usbdevice.h>

/*!
 *  ======== USBD_CDC_init ========
 *  Function to initialize the USB serial reference module.
 *
 *  Note: This function is not reentrant safe.
 */
extern void USBD_CDC_init(tCompositeEntry *psCompEntry);

/*!
 *  ======== USBCDCD_sendData ========
 *  A blocking function that sends a specified amount of data.
 *
 *  Function sends size number of bytes from the buffer pointed by pStr.
 *
 *  @param(pStr)    Pointer to a buffer of data
 *
 *  @param(size)    Number of bytes to be sent
 *
 *  @return         Number of bytes added to the USB Buffer for transmission
 */
extern unsigned int USBCDCD_sendData(const unsigned char *pStr,
                                     unsigned int length,
                                     unsigned int timeout);

/*!
 *  ======== USBCDCD_receiveData ========
 *  A blocking function to read from the USB Buffer
 *
 *  Function reads up to size number of bytes from the USB Buffer. The return
 *  value will indicate how many bytes were read from the USB Buffer. If there
 *  is no data in the USB Buffer, this function will block timeout number of
 *  ticks. Note: The BIOS_WAIT_FOREVER macro can be used to block indefinitely.
 *
 *  @param(pStr)    Pointer to a buffer to which data is written to.
 *
 *  @param(size)    Maximum number of bytes to be read
 *
 *  @param(timeout) Number of tick to block if no data is currently available in
 *                  the USB Buffer.
 *
 *  @return         Returns the number of bytes actually read from the USB
 *                  Buffer.
 */
extern unsigned int USBCDCD_receiveData(unsigned char *pStr,
                                        unsigned int length,
                                        unsigned int timeout);


#ifdef __cplusplus
}
#endif

#endif /* __USBD_CDC_H */