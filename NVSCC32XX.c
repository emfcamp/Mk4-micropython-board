/*
 * Copyright (c) 2016, Texas Instruments Incorporated
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
 *  ======== NVSCC32XX.c ========
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <ti/drivers/NVS.h>
#include "NVSCC32XX.h"

#include <simplelink.h>

/* value from fs.h */ //(4096 - 512)
#define SECTOR_SIZE 2048  /* FAT FS requires 2^N sector size */

#define NUM_SECTORS 64

#define PATHFORMAT "ti_drivers_nvs_%d_%d"

extern NVS_Config NVS_config[];

static void NVSCC32XX_close(NVS_Handle handle);
static int_fast16_t NVSCC32XX_control(NVS_Handle handle, uint_fast16_t cmd,
                                      uintptr_t arg);
static int_fast16_t NVSCC32XX_erase(NVS_Handle handle, size_t offset, size_t size);
static void NVSCC32XX_getAttrs(NVS_Handle handle, NVS_Attrs *attrs);
static void NVSCC32XX_init(void);
static int_fast16_t NVSCC32XX_lock(NVS_Handle handle, uint32_t timeout);
static NVS_Handle NVSCC32XX_open(uint_least8_t index, NVS_Params *params);
static int_fast16_t NVSCC32XX_read(NVS_Handle handle, size_t offset, void *buffer,
                                   size_t bufferSize);
static void NVSCC32XX_unlock(NVS_Handle handle);
static int_fast16_t NVSCC32XX_write(NVS_Handle handle, size_t offset, void *buffer,
                                    size_t bufferSize, uint_fast16_t flags);

const NVS_FxnTable NVSCC32XX_fxnTable = {
    NVSCC32XX_close,
    NVSCC32XX_control,
    NVSCC32XX_erase,
    NVSCC32XX_getAttrs,
    NVSCC32XX_init,
    NVSCC32XX_lock,
    NVSCC32XX_open,
    NVSCC32XX_read,
    NVSCC32XX_unlock,
    NVSCC32XX_write
};

/* TODO: not re-entrant */
static uint8_t copyBlock[SECTOR_SIZE];

typedef int (*IOFxn)(NVSCC32XX_Object * nvs, size_t sector, size_t offset,
                     void * buf, size_t len);

static int performIO(NVS_Handle handle, size_t offset, void *buffer,
                     size_t bufferSize, IOFxn doIO)
{
    NVSCC32XX_Object * nvs = handle->object;
    size_t first = offset % SECTOR_SIZE;
    size_t startSector = offset / SECTOR_SIZE;
    int status = NVS_STATUS_SUCCESS;

    if (first) {
        size_t len;
        if ((len = SECTOR_SIZE - first) > bufferSize) {
            len = bufferSize;
        }
        status = doIO(nvs, first, startSector++, buffer, len);
        buffer = (uint8_t *)buffer + len;
        bufferSize -= len;
    }

    if (bufferSize && !status) {
        for (uint32_t i = bufferSize / SECTOR_SIZE; i > 0; i--) {
            if ((status = doIO(nvs, startSector++, 0, buffer, SECTOR_SIZE))) {
                break;
            }
            buffer = (uint8_t *)buffer + SECTOR_SIZE;
            bufferSize -= SECTOR_SIZE;
        }

        if (!status && bufferSize) {
            doIO(nvs, startSector++, 0, buffer, bufferSize);
        }
    }

    return status;
}

static int readSector(NVSCC32XX_Object * nvs, size_t sector, size_t offset,
                       void * buf, size_t len)
{
    char filename[sizeof(PATHFORMAT)];
    int status = NVS_STATUS_ERROR;
    long file;

    snprintf(filename, sizeof(filename), PATHFORMAT, nvs->id, sector);

    if ((file = sl_FsOpen((unsigned char *)filename, SL_FS_READ, NULL)) >= 0) {
        if (sl_FsRead(file, offset, buf, len) == len) {
            status = NVS_STATUS_SUCCESS;
        }
        sl_FsClose(file, NULL, NULL, 0);
    }

    return status;
}

static int writeSector(NVSCC32XX_Object * nvs, size_t sector, size_t offset,
                       void * buf, size_t len)
{
    char filename[sizeof(PATHFORMAT)];
    int status = NVS_STATUS_SUCCESS;
    long file;

    snprintf(filename, sizeof(filename), PATHFORMAT, nvs->id, sector);

    if (len < SECTOR_SIZE) {
        if ((file = sl_FsOpen((unsigned char *)filename, SL_FS_READ, NULL)) >= 0) {
            if (sl_FsRead(file, 0, copyBlock, SECTOR_SIZE) == SECTOR_SIZE) {
                memcpy(copyBlock + offset, buf, len);
                buf = copyBlock;
            }
            else {
                status = -1;
            }
            sl_FsClose(file, NULL, NULL, 0);
        }
    }

    if (status == NVS_STATUS_SUCCESS &&
        (file = sl_FsOpen((unsigned char *)filename,
                          SL_FS_CREATE | SL_FS_OVERWRITE |
                          SL_FS_CREATE_MAX_SIZE(SECTOR_SIZE), NULL)) >= 0) {

        if (sl_FsWrite(file, 0, buf, SECTOR_SIZE) == SECTOR_SIZE) {
            status = NVS_STATUS_SUCCESS;
        }
        sl_FsClose(file, NULL, NULL, 0);
    }

    return status;
}

/*
 *  ======== NVSCC32XX_close ========
 */
static void NVSCC32XX_close(NVS_Handle handle)
{
    ((NVSCC32XX_Object *)(handle->object))->opened = false;
}

/*
 *  ======== NVSCC32XX_control ========
 */
static int_fast16_t NVSCC32XX_control(NVS_Handle handle, uint_fast16_t cmd, uintptr_t arg)
{
    return 0;
}

/*
 *  ======== NVSCC32XX_erase ========
 */
static int_fast16_t NVSCC32XX_erase(NVS_Handle handle, size_t offset, size_t size)
{
    return 0;
}

/*
 *  ======== NVSCC32XX_getAttrs ========
 */
static void NVSCC32XX_getAttrs(NVS_Handle handle, NVS_Attrs *attrs)
{
    attrs->sectorSize = SECTOR_SIZE;
    attrs->regionSize = NUM_SECTORS * attrs->sectorSize;
    attrs->regionBase = NVS_REGION_NOT_ADDRESSABLE;
}

/*
 *  ======== NVSCC32XX_init ========
 */
static void NVSCC32XX_init(void)
{
    /* TODO: remove? */
    sl_Stop(0);
    sl_Start(0, 0, 0);
}

/*
 * ======== NVSCC32XX_lock ========
 */
static int_fast16_t NVSCC32XX_lock(NVS_Handle handle, uint32_t timeout)
{
    return 0;
}

/*
 *  ======== NVSCC32XX_open =======
 */
static NVS_Handle NVSCC32XX_open(uint_least8_t index, NVS_Params *params)
{
    NVSCC32XX_Object *object = NULL;
    NVS_Handle handle;

    (void)params;

    handle = &NVS_config[index];
    object = NVS_config[index].object;

    if (object->opened == true) {
        return (NULL);
    }

    object->opened = true;
    object->id = index;

    return (handle);
}

/*
 *  ======== NVSCC32XX_read =======
 */
static int_fast16_t NVSCC32XX_read(NVS_Handle handle, size_t offset, void *buffer,
                                   size_t bufferSize)
{
#if 0
    NVSCC32XX_Object * nvs = handle->object;
    size_t first = offset % SECTOR_SIZE;
    size_t startSector = offset / SECTOR_SIZE;

    if (first) {
        size_t len;
        if ((len = SECTOR_SIZE - first) > bufferSize) {
            len = bufferSize;
        }
        readSector(nvs, first, startSector++, buffer, len);
        buffer += len;
        bufferSize -= len;
    }

    if (bufferSize) {
        for (uint32_t i = bufferSize / SECTOR_SIZE; i > 0; i--) {
            readSector(nvs, startSector++, 0, buffer, SECTOR_SIZE);
            buffer += SECTOR_SIZE;
            bufferSize -= SECTOR_SIZE;
        }

        if (bufferSize) {
            readSector(nvs, startSector++, 0, buffer, bufferSize);
        }
    }
#endif
    return performIO(handle, offset, buffer, bufferSize, readSector);
}

/*
 * ======== NVSCC32XX_unlock ========
 */
static void NVSCC32XX_unlock(NVS_Handle handle)
{
    (void)handle;
}

/*
 *  ======== NVSCC32XX_write =======
 */
static int_fast16_t NVSCC32XX_write(NVS_Handle handle, size_t offset, void *buffer,
                                    size_t bufferSize, uint_fast16_t flags)
{
#if 0
    NVSCC32XX_Object * nvs = handle->object;
    size_t first = offset % SECTOR_SIZE;
    size_t startSector = offset / SECTOR_SIZE;

    if (first) {
        size_t len;
        if ((len = SECTOR_SIZE - first) > bufferSize) {
            len = bufferSize;
        }
        writeSector(nvs, first, startSector++, buffer, len);
        buffer += len;
        bufferSize -= len;
    }

    if (bufferSize) {
        for (uint32_t i = bufferSize / SECTOR_SIZE; i > 0; i--) {
            writeSector(nvs, startSector++, 0, buffer, SECTOR_SIZE);
            buffer += SECTOR_SIZE;
            bufferSize -= SECTOR_SIZE;
        }

        if (bufferSize) {
            writeSector(nvs, startSector++, 0, buffer, bufferSize);
        }
    }
#endif
    return performIO(handle, offset, buffer, bufferSize, writeSector);
}
