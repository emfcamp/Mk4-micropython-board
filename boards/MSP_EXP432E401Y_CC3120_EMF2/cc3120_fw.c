/*
 * Copyright (c) 2015-2017, Texas Instruments Incorporated
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

/*
 *  ======== cc3120_fw.c ========
 */
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>

#include <time.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>

#include <ti/display/Display.h>

#include <ti/drivers/net/wifi/simplelink.h>

/* Example/Board Header files */
#include "MSP_EXP432E401Y.h"

#include "cc3120_fw.h"

#ifndef __MSP432E401Y__
#define __MSP432E401Y__
#endif

/*
 *  =============================== UART ===============================
 */
#include <ti/drivers/UART.h>
#include <ti/drivers/uart/UARTMSP432E4.h>

#include <ti/devices/msp432e4/driverlib/uart.h>

void setBreak(UART_Handle uart, int state)
{
    UARTMSP432E4_HWAttrs const *hwAttrs = uart->hwAttrs;

    UARTBreakCtl(hwAttrs->baseAddr, state);
}

uint8_t chunk[4096];

static Display_Handle display;
static UART_Handle uart;

typedef struct slWifiBldr_TxPacket {
    uint16_t len;
    uint8_t chksum;
    uint8_t opcode;
    uint8_t data[(4096+24)]; // Max is 4080 (4096-16)for SRAM/SFLASH patches, but buffer room for 4096 data + key + sizes/flags
} slWifiBldr_TxPacket;

typedef struct slWifiBldr_RxPacket {
    uint8_t data[(4096)];
} slWifiBldr_RxPacket;

static slWifiBldr_TxPacket txPacket;
static slWifiBldr_RxPacket rxPacket;

static int debugVerbose = 0;

void slWifiBldr_sendAck(void)
{
    uint8_t buf[2] = {0x00, 0xcc};
    UART_write(uart, buf, sizeof(buf));
}

void slWifiBldr_sendCmd(void)
{
    uint16_t packetLen = txPacket.len + 1;

    uint8_t len1 = (txPacket.len & 0xff00) >> 8;
    uint8_t len2 = (txPacket.len & 0x00ff);
    txPacket.len = (len2 << 8) + len1;

    if (debugVerbose) {
        int i;
        for(i = 0; i < packetLen; i++) {
            Display_printf(display, 0, 0, "-> 0x%02x", ((uint8_t *)&txPacket)[i]);
        }
    }
    UART_write(uart, (uint8_t *) &txPacket, packetLen);
}

void slWifiBldr_recvAck(void)
{
    UART_read(uart, (uint8_t *) &rxPacket, 2);

    if (debugVerbose) {
        int i;
        for(i = 0; i < 2; i++) {
            Display_printf(display, 0, 0, "<- 0x%02x", ((uint8_t *)&rxPacket)[i]);
        }
    }

    if ( (rxPacket.data[0] == 0x00) && (rxPacket.data[1] == 0xcc) ) {
        if (debugVerbose) {
            Display_printf(display, 0, 0, "ACK Good");
        }
    }
    else {
        Display_printf(display, 0, 0, "ACK error - ALERT !!!!");
        if (debugVerbose) {
            while(1);
        }
    }
}

void slWifiBldr_recvResponse(uint32_t len)
{
    if (debugVerbose) {
        Display_printf(display, 0, 0, "Receiving %d bytes", len);
    }

    UART_read(uart, (uint8_t *) &rxPacket, len);

    if (debugVerbose) {
        int i;
        for(i = 0; i < len; i++) {
            Display_printf(display, 0, 0, "<- 0x%02x", ((uint8_t *)&rxPacket)[i]);
        }
    }
}


void slWifiBldr_genChksum(void)
{
    int i;
    int len = txPacket.len - sizeof(txPacket.len) - sizeof(txPacket.opcode);

    txPacket.chksum = 0;

    for(i = 0; i < len; i++) {
        txPacket.chksum += txPacket.data[i];
    }
    txPacket.chksum += txPacket.opcode;
}


void slWifiBldr_getStatus(void)
{
#define slWifiBldr_CMD_GETSTATUS 0x23
#define slWifiBldr_CMD_GETSTATUS_LEN    (sizeof(txPacket.len) + sizeof(txPacket.opcode))
#define slWifiBldr_CMD_GETSTATUS_RESP_LASTSTATUS_LEN 0x4

    txPacket.len = slWifiBldr_CMD_GETSTATUS_LEN;
    txPacket.opcode = slWifiBldr_CMD_GETSTATUS;

    slWifiBldr_genChksum();
    slWifiBldr_sendCmd();
    slWifiBldr_recvAck();

    slWifiBldr_recvResponse(slWifiBldr_CMD_GETSTATUS_RESP_LASTSTATUS_LEN);

    // 0/1 is length
    // 2 is chksum
    // 3 is status
    if (debugVerbose) {
        Display_printf(display, 0, 0, "Status Received 0x%02x", rxPacket.data[3]);
    }

    if (rxPacket.data[3] != 0x40) {
        Display_printf(display, 0, 0, "Status Received 0x%02x - ERROR!!! ", rxPacket.data[3]);
        if (debugVerbose) {
            while(1);
        }
    }

    slWifiBldr_sendAck();
}


void slWifiBldr_getStorageList(void)
{
#define slWifiBldr_CMD_GETSTORAGELIST    0x27
#define slWifiBldr_CMD_GETSTORAGELIST_LEN    (sizeof(txPacket.len) + sizeof(txPacket.opcode))
#define slWifiBldr_CMD_GETSTORAGELIST_RESP_STORAGELIST_LEN 0x1

    txPacket.len = slWifiBldr_CMD_GETSTORAGELIST_LEN;
    txPacket.opcode = slWifiBldr_CMD_GETSTORAGELIST;

    slWifiBldr_genChksum();
    slWifiBldr_sendCmd();
    slWifiBldr_recvAck();

    slWifiBldr_recvResponse(slWifiBldr_CMD_GETSTORAGELIST_RESP_STORAGELIST_LEN);

    Display_printf(display, 0, 0, ">> Storage List 0x%02x", rxPacket.data[0]);
}

void slWifiBldr_rawStorageWrite(uint32_t id, uint32_t offset, uint32_t len, uint8_t *data)
{
#define slWifiBldr_CMD_RAWSTORAGEWRITE    0x2D
#define slWifiBldr_CMD_RAWSTORAGEWRITE_LEN    (sizeof(txPacket.len) + sizeof(txPacket.opcode) + sizeof(id) + sizeof(offset) + sizeof(len) + len)

    uint32_t i;

    if (len > 4080) {
        Display_printf(display, 0, 0, "slWifiBldr_rawStorageWrite size > 4080 - ALERT !!!");
        if (debugVerbose) {
            while(1);
        }
    }

    txPacket.len = slWifiBldr_CMD_RAWSTORAGEWRITE_LEN;
    txPacket.opcode = slWifiBldr_CMD_RAWSTORAGEWRITE;

    // uint32 storageId
    txPacket.data[0] = (id & 0xff000000) >> 24;
    txPacket.data[1] = (id & 0x00ff0000) >> 16;
    txPacket.data[2] = (id & 0x0000ff00) >> 8;
    txPacket.data[3] = (id & 0x000000ff) >> 0;

    // uint32 offset
    txPacket.data[4] = (offset & 0xff000000) >> 24;
    txPacket.data[5] = (offset & 0x00ff0000) >> 16;
    txPacket.data[6] = (offset & 0x0000ff00) >> 8;
    txPacket.data[7] = (offset & 0x000000ff) >> 0;

    // uint32 length
    txPacket.data[8] = (len & 0xff000000) >> 24;
    txPacket.data[9] = (len & 0x00ff0000) >> 16;
    txPacket.data[10] = (len & 0x0000ff00) >> 8;
    txPacket.data[11] = (len & 0x000000ff) >> 0;

    for (i = 0; i < len; i++) {
        txPacket.data[i+12] = data[i];
    }

    Display_printf(display, 0, 0, "Writing to %s at 0x%08x, Length 0x%08x",
                   (id == 0 ? "SRAM" : "SFLASH"),
                   offset, len);

    slWifiBldr_genChksum();
    slWifiBldr_sendCmd();
    slWifiBldr_recvAck();
}


void slWifiBldr_getVersionInfo(void)
{
#define slWifiBldr_CMD_GETVERSIONINFO    0x2F
#define slWifiBldr_CMD_GETVERSIONINFO_LEN    (sizeof(txPacket.len) + sizeof(txPacket.opcode))
#define slWifiBldr_CMD_GETVERSIONINFO_RESP_VERSIONINFO_LEN 31

    txPacket.len = slWifiBldr_CMD_GETVERSIONINFO_LEN;
    txPacket.opcode = slWifiBldr_CMD_GETVERSIONINFO;

    slWifiBldr_genChksum();
    slWifiBldr_sendCmd();
    slWifiBldr_recvAck();

    slWifiBldr_recvResponse(slWifiBldr_CMD_GETVERSIONINFO_RESP_VERSIONINFO_LEN);
    // verify checksum?
    slWifiBldr_sendAck();

    Display_printf(display, 0, 0, ">> Version Info");
    Display_printf(display, 0, 0, ">> Version Info - Bootloader %d.%d.%d.%d",
                   rxPacket.data[4],
                   rxPacket.data[5],
                   rxPacket.data[6],
                   rxPacket.data[7]
                   );

    if (debugVerbose) {
        Display_printf(display, 0, 0, ">> Version Info - NWP %d.%d.%d.%d",
                       rxPacket.data[8],
                       rxPacket.data[9],
                       rxPacket.data[10],
                       rxPacket.data[11]
                       );
        Display_printf(display, 0, 0, ">> Version Info - MAC %d.%d.%d.%d",
                       rxPacket.data[12],
                       rxPacket.data[13],
                       rxPacket.data[14],
                       rxPacket.data[15]
                       );
        Display_printf(display, 0, 0, ">> Version Info - PHY %d.%d.%d.%d",
                       rxPacket.data[16],
                       rxPacket.data[17],
                       rxPacket.data[18],
                       rxPacket.data[19]
                       );
        Display_printf(display, 0, 0, ">> Version Info - CHIP (0x%02x)%d.%d.%d.%d",
                       rxPacket.data[20],
                       rxPacket.data[20],
                       rxPacket.data[21],
                       rxPacket.data[22],
                       rxPacket.data[23]
                       );
    }
}

void slWifiBldr_rawStorageErase(uint32_t id, uint32_t offset, uint32_t numblocks)
{
#define slWifiBldr_CMD_RAWSTORAGEERASE    0x30
#define slWifiBldr_CMD_RAWSTORAGEERASE_LEN    (sizeof(txPacket.len) + sizeof(txPacket.opcode) + sizeof(id) + sizeof(offset) + sizeof(numblocks))

    txPacket.len = slWifiBldr_CMD_RAWSTORAGEERASE_LEN;
    txPacket.opcode = slWifiBldr_CMD_RAWSTORAGEERASE;

    // uint32 storageId
    txPacket.data[0] = (id & 0xff000000) >> 24;
    txPacket.data[1] = (id & 0x00ff0000) >> 16;
    txPacket.data[2] = (id & 0x0000ff00) >> 8;
    txPacket.data[3] = (id & 0x000000ff) >> 0;

    // uint32 offset
    txPacket.data[4] = (offset & 0xff000000) >> 24;
    txPacket.data[5] = (offset & 0x00ff0000) >> 16;
    txPacket.data[6] = (offset & 0x0000ff00) >> 8;
    txPacket.data[7] = (offset & 0x000000ff) >> 0;

    // uint32 numblocks
    txPacket.data[8] = (numblocks & 0xff000000) >> 24;
    txPacket.data[9] = (numblocks & 0x00ff0000) >> 16;
    txPacket.data[10] = (numblocks & 0x0000ff00) >> 8;
    txPacket.data[11] = (numblocks & 0x000000ff) >> 0;

    slWifiBldr_genChksum();
    slWifiBldr_sendCmd();
    slWifiBldr_recvAck();

    // doc says to send Ack() 5.3.5 step 3, but this doesnt work
}

void slWifiBldr_getStorageInfo(uint32_t id)
{
#define slWifiBldr_CMD_GETSTORAGEINFO    0x31
#define slWifiBldr_CMD_GETSTORAGEINFO_LEN    (sizeof(txPacket.len) + sizeof(txPacket.opcode) + sizeof(id))
#define slWifiBldr_CMD_GETSTORAGEINFO_RESP_STORAGEINFO_LEN 11 // docs say 8!

    txPacket.len = slWifiBldr_CMD_GETSTORAGEINFO_LEN;
    txPacket.opcode = slWifiBldr_CMD_GETSTORAGEINFO;

    // uint32 storageId
    txPacket.data[0] = (id & 0xff000000) >> 24;
    txPacket.data[1] = (id & 0x00ff0000) >> 16;
    txPacket.data[2] = (id & 0x0000ff00) >> 8;
    txPacket.data[3] = (id & 0x000000ff) >> 0;

    slWifiBldr_genChksum();
    slWifiBldr_sendCmd();
    slWifiBldr_recvAck();

    slWifiBldr_recvResponse(slWifiBldr_CMD_GETSTORAGEINFO_RESP_STORAGEINFO_LEN);

    slWifiBldr_sendAck();

    // docs bad,
    // data[0], data[1] = len;
    // data[2] = chksum

    Display_printf(display, 0, 0, ">> Storage Info %s", (id == 0 ? "SRAM" : "SFLASH"));
    Display_printf(display, 0, 0, ">> Storage Info - Block Size 0x%02x%02x (%d)",
                   rxPacket.data[3],
                   rxPacket.data[4],
                   ((rxPacket.data[3] << 8) + rxPacket.data[4])
                   );
    Display_printf(display, 0, 0, ">> Storage Info - Num Blocks 0x%02x%02x (%d)",
                   rxPacket.data[5],
                   rxPacket.data[6],
                   ((rxPacket.data[5] << 8) + rxPacket.data[6])
                   );
    //    other 4 == reserved
}

void slWifiBldr_execFromRam(void)
{
#define slWifiBldr_CMD_EXECFROMRAM    0x32
#define slWifiBldr_CMD_EXECFROMRAM_LEN    (sizeof(txPacket.len) + sizeof(txPacket.opcode))

    txPacket.len = slWifiBldr_CMD_EXECFROMRAM_LEN;
    txPacket.opcode = slWifiBldr_CMD_EXECFROMRAM;

    slWifiBldr_genChksum();
    slWifiBldr_sendCmd();
    slWifiBldr_recvAck();

    // Recv second ACK indicated initialisation is complete from new bootloader image
    slWifiBldr_recvAck();
    if (debugVerbose) {
        Display_printf(display, 0, 0, "Exec From RAM, restarted from RAM");
    }
}

void slWifiBldr_switchUart(void)
{
    uint32_t delay = 26666667;

#define slWifiBldr_CMD_SWITCHUART    0x33
#define slWifiBldr_CMD_SWITCHUART_LEN    (sizeof(txPacket.len) + sizeof(txPacket.opcode) + sizeof(delay))

    txPacket.len = slWifiBldr_CMD_SWITCHUART_LEN;
    txPacket.opcode = slWifiBldr_CMD_SWITCHUART;

    // uint32 delay
    txPacket.data[0] = (delay & 0xff000000) >> 24;
    txPacket.data[1] = (delay & 0x00ff0000) >> 16;
    txPacket.data[2] = (delay & 0x0000ff00) >> 8;
    txPacket.data[3] = (delay & 0x000000ff) >> 0;

    slWifiBldr_genChksum();
    slWifiBldr_sendCmd();
    slWifiBldr_recvAck();
}

void slWifiBldr_fsProgram(uint16_t keySize, uint16_t len, uint32_t flags, uint8_t *key, uint8_t *data)
{
#define slWifiBldr_CMD_FSPROGRAM    0x34
#define slWifiBldr_CMD_FSPROGRAM_LEN    ( sizeof(txPacket.len) + sizeof(txPacket.opcode) + sizeof(keySize) + sizeof(len) + sizeof(flags) + keySize + len)
#define slWifiBldr_CMD_FSPROGRAM_RESP_STATUS_LEN 0x4

    uint32_t i;

    if ((flags != 0) || (keySize > 16) || (len > 4096)) {
        if (debugVerbose) {
            while(1);
        }
    }

    txPacket.len = slWifiBldr_CMD_FSPROGRAM_LEN;
    txPacket.opcode = slWifiBldr_CMD_FSPROGRAM;

    // uint16 keySize
    txPacket.data[0] = (keySize & 0xff00) >> 8;
    txPacket.data[1] = (keySize & 0x00ff) >> 0;

    // uint16 chunkSize
    txPacket.data[2] = (len & 0xff00) >> 8;
    txPacket.data[3] = (len & 0x00ff) >> 0;

    // uint32 flags
    txPacket.data[4] = (flags & 0xff000000) >> 24;
    txPacket.data[5] = (flags & 0x00ff0000) >> 16;
    txPacket.data[6] = (flags & 0x0000ff00) >> 8;
    txPacket.data[7] = (flags & 0x000000ff) >> 0;

    for (i = 0; i < keySize; i++) {
        txPacket.data[i+8] = key[i];
    }

    for (i = 0; i < len; i++) {
        txPacket.data[i+8+keySize] = data[i];
    }

    slWifiBldr_genChksum();
    slWifiBldr_sendCmd();
    slWifiBldr_recvAck();

    slWifiBldr_recvResponse(slWifiBldr_CMD_FSPROGRAM_RESP_STATUS_LEN);

//    if (debugVerbose) {
        Display_printf(display, 0, 0, "Program Status : %d",
                       (rxPacket.data[0] << 24) +
                       (rxPacket.data[1] << 16) +
                       (rxPacket.data[2] << 8) +
                       (rxPacket.data[3] << 0)
                       );
//    }
}

int CC3120_checkIfUpdateNeeded(void)
{
    _i16 status;

    status = sl_Start(NULL, NULL, NULL);

    if ((status !=  ROLE_STA) &&
        (status !=  ROLE_AP) &&
        (status !=  ROLE_P2P)) {

        return(1);
    }

    SlDeviceVersion_t ver;
    _u8 pConfigOpt;
    _u16 pConfigLen;
    pConfigLen = sizeof(ver);
    pConfigOpt = SL_DEVICE_GENERAL_VERSION;
    status = sl_DeviceGet(SL_DEVICE_GENERAL,&pConfigOpt,&pConfigLen,(_u8 *)(&ver));
    if (status < 0) {
        Display_printf(display, 0, 0,
                       "Failed to get FW Version\n");
        return 1;
    }
    else {
        Display_printf(display, 0, 0,
                       "CC3120 Versions : \n"
                       "- CHIP %d\n"
                       "- MAC  31.%d.%d.%d.%d\n"
                       "- PHY  %d.%d.%d.%d\n"
                       "- NWP  %d.%d.%d.%d\n"
                       "- ROM  %d\n"
                       "- HOST %d.%d.%d.%d",
                     ver.ChipId,
                     ver.FwVersion[0],
                     ver.FwVersion[1],
                     ver.FwVersion[2],
                     ver.FwVersion[3],
                     ver.PhyVersion[0],
                     ver.PhyVersion[1],
                     ver.PhyVersion[2],
                     ver.PhyVersion[3],
                     ver.NwpVersion[0],
                     ver.NwpVersion[1],
                     ver.NwpVersion[2],
                     ver.NwpVersion[3],
                     ver.RomVersion,
                     SL_MAJOR_VERSION_NUM,SL_MINOR_VERSION_NUM,SL_VERSION_NUM,SL_SUB_VERSION_NUM);
    }
    sl_Stop(0x200);

    _u32 nwpVersion = ((ver.NwpVersion[0] << 24) +
                       (ver.NwpVersion[1] << 16) +
                       (ver.NwpVersion[2] << 8) +
                       (ver.NwpVersion[3] << 0));

    if ( nwpVersion < CC3120_FW_VERSION_NWP ) {
        return 1;
    }

    return 0;
 }

void CC3120_doUpdate(void)
{
    UART_Params uartParams;

    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.readReturnMode = UART_RETURN_FULL;
    uartParams.readEcho = UART_ECHO_OFF;
    uartParams.baudRate = 921600;

    Display_printf(display, 0, 0, "Opening CC3120 UART");
    uart = UART_open(MSP_EXP432E401Y_UART3, &uartParams);

    if (uart == NULL) {
        /* UART_open() failed */
        Display_printf(display, 0, 0, "Failed to open CC3120 UART - STOP!! <spin>");
        while(1);
    }

    Display_printf(display, 0, 0, "Entering Bootloader");
    Display_printf(display, 0, 0, "Setting Break");

    setBreak(uart, 1);

    Display_printf(display, 0, 0, "Toggle Reset Low");
    GPIO_write(MSP_EXP432E401Y_CC_nHIB_pin, 0);
    usleep(50000);
    Display_printf(display, 0, 0, "Toggle Reset High");
    GPIO_write(MSP_EXP432E401Y_CC_nHIB_pin, 1);

    Display_printf(display, 0, 0, "Waiting for ACK");
    slWifiBldr_recvAck();

    Display_printf(display, 0, 0, "Setting Break False");
    setBreak(uart, 0);

    usleep(1000000);

    Display_printf(display, 0, 0, "Sending Get Storage List Command");
    slWifiBldr_getStorageList();

    Display_printf(display, 0, 0, "Sending Get Version Info Command");
    slWifiBldr_getVersionInfo();

    Display_printf(display, 0, 0, "Sending Get Storage Info Command - SRAM");
    slWifiBldr_getStorageInfo(0);

    Display_printf(display, 0, 0, "Sending Get Storage Info Command - SFLASH");
    slWifiBldr_getStorageInfo(2);

    Display_printf(display, 0, 0, "Sending Raw Storage Erase, SRAM, 0 offset, 3 blocks");
    slWifiBldr_rawStorageErase(0, 0, 3);

    Display_printf(display, 0, 0, "Sending Get Status");
    slWifiBldr_getStatus();

    Display_printf(display, 0, 0, "Updating Bootloader in SRAM");
    // Load Bootloader Patches to SRAM
    {
        uint32_t len = 0;
        uint32_t remain = BTL_ram_ptc_len;

        while (remain > 0) {
            uint32_t offset = BTL_ram_ptc_len - remain;

            if (remain < 4080) {
                len = remain;
            }
            else {
                len = 4080;
            }

            // Write to SRAM
//          Display_printf(display, 0, 0, "Sending Raw Storage Write to SRAM");
            slWifiBldr_rawStorageWrite(0, offset, len, &BTL_ram_ptc[offset]);

//          Display_printf(display, 0, 0, "Sending Get Status");
            slWifiBldr_getStatus();

            remain -= len;
        }
    }

    Display_printf(display, 0, 0, "Sending Get Version Info Command");
    slWifiBldr_getVersionInfo();

    Display_printf(display, 0, 0, "Sending Execute From RAM");
    slWifiBldr_execFromRam();

    Display_printf(display, 0, 0, "Sending Get Version Info Command");
    slWifiBldr_getVersionInfo();

    Display_printf(display, 0, 0, "Sending Get Storage Info Command - SFLASH");
    slWifiBldr_getStorageInfo(2);

    Display_printf(display, 0, 0, "Sending Raw Storage Erase, SFLASH, 33 offset, 2 blocks");
    slWifiBldr_rawStorageErase(2, 33, 2);

    Display_printf(display, 0, 0, "Sending Get Status");
    slWifiBldr_getStatus();

    Display_printf(display, 0, 0, "Updating Bootloader in SFLASH");
    // Load Bootloader Patches to SFLASH (for return to factory)
    {
        uint32_t len = 0;
        uint32_t remain = BTL_sflash_ptc_len;

        while (remain > 0) {
            uint32_t offset = BTL_sflash_ptc_len - remain;

            if (remain < 4080) {
                len = remain;
            }
            else {
                len = 4080;
            }

            // Write to SFLASH
            if (debugVerbose) {
                Display_printf(display, 0, 0, "Sending Raw Storage Write to SFLASH");
            }

            // Must be written at offset block 33 + 8 bytes
            slWifiBldr_rawStorageWrite(2, ((33*4096)+8) + offset, len, &BTL_sflash_ptc[offset]);

            if (debugVerbose) {
                Display_printf(display, 0, 0, "Sending Get Status");
            }
            slWifiBldr_getStatus();

            remain -= len;
        }
    }

    Display_printf(display, 0, 0, "Sending Get Version Info Command");
    slWifiBldr_getVersionInfo();

    // Load Programming Image
    {
        uint32_t len = 0;
        uint32_t remain = cc3120r_prod_sdk210sp_Programming_ucf_len;

        while (remain > 0) {
            uint32_t offset = cc3120r_prod_sdk210sp_Programming_ucf_len - remain;

            if (remain < 4096) {
                len = remain;
            }
            else {
                len = 4096;
            }

            // Write to SFLASH
            if (debugVerbose) {
                Display_printf(display, 0, 0, "Sending FS Programming Command");
            }
            // Must be written as 4096 byte chunks
            slWifiBldr_fsProgram(0, len, 0, NULL, &cc3120r_prod_sdk210sp_Programming_ucf[offset]);

            remain -= len;
        }
    }
    Display_printf(display, 0, 0, "Programming Done");

    Display_printf(display, 0, 0, "Closing CC3120 UART");
    UART_close(uart);
}


/*
 *  ======== CC3120_fwUpdate ========
 */
void CC3120_fwUpdate(void)
{
    Display_init();
    UART_init();

    display = Display_open(Display_Type_UART, NULL);
    if (display == NULL) {
        while (1);
    }

    Display_printf(display, 0, 0, "\n\nCC3120 FW Check ...");

    if (CC3120_checkIfUpdateNeeded() || GPIO_read(MSP_EXP432E401Y_GPIO_JOYD)) {
        Display_printf(display, 0, 0, "CC3120 FW needs update ...");
        CC3120_doUpdate();

        if (CC3120_checkIfUpdateNeeded()) {
            Display_printf(display, 0, 0, "CC3120 FW update failed... ALERT !!!");
        }
    }
    Display_printf(display, 0, 0, "CC3120 FW Check Done ...\n");

    Display_close(display);
}
