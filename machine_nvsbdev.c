/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Texas Instruments, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include "storage.h"
#include "machine_nvsbdev.h"

#include "py/runtime.h"
#include "py/mperrno.h"

#include "py/obj.h"


#if MICROPY_MACHINE_NVSBDEV
#include <ti/drivers/NVS.h>
#include <ti/drivers/dpl/HwiP.h>
#include <ti/devices/msp432e4/driverlib/interrupt.h>
#include <ti/sysbios/knl/Task.h>
#include <xdc/runtime/System.h>

#include "py/mpconfig.h"
#include "py/mphal.h"
#include "py/misc.h"
#include "led.h"

// LWK TODO: fix up once we have pin names via CSV
#include "MSP_EXP432E401Y.h"

static NVS_Handle nvs_handle;
static NVS_Attrs nvs_attrs;

typedef struct {
    uint32_t base_address;
    uint32_t sector_size;
    uint32_t sector_count;
} flash_layout_t;

static const flash_layout_t flash_layout[] = {
    { (uint32_t)0, (uint32_t)0x4000, 20 },
};

STATIC byte flash_cache_mem[0x4000] __attribute__((aligned(4))); // 16k
#define CACHE_MEM_START_ADDR (&flash_cache_mem[0])
#define FLASH_SECTOR_SIZE_MAX (0x4000) // 16k max due to size of cache buffer
#define FLASH_MEM_SEG1_NUM_BLOCKS (640) 
#define FLASH_PART1_START_BLOCK (0x100) // FAT data starts at block 256

// TODO: Use self->attrs.regionSize & self->attrs.sectorSize

uint32_t flash_get_sector_info(uint32_t addr, uint32_t *start_addr, uint32_t *size) {
    if (addr >= flash_layout[0].base_address) {
        uint32_t sector_index = 0;
        for (int i = 0; i < MP_ARRAY_SIZE(flash_layout); ++i) {
            for (int j = 0; j < flash_layout[i].sector_count; ++j) {
                uint32_t sector_start_next = flash_layout[i].base_address
                    + (j + 1) * flash_layout[i].sector_size;
                if (addr < sector_start_next) {
                    if (start_addr != NULL) {
                        *start_addr = flash_layout[i].base_address
                            + j * flash_layout[i].sector_size;
                    }
                    if (size != NULL) {
                        *size = flash_layout[i].sector_size;
                    }
                    return sector_index;
                }
                ++sector_index;
            }
        }
    }
    return 0;
}

static void FLASH_hwiHandler(uintptr_t arg0)
{
    flash_bdev_flush();
}

void flash_bdev_init(void) {
    NVS_Params params;
    NVS_Params_init(&params);
    if ((nvs_handle = NVS_open(0, &params))) {
        NVS_getAttrs(nvs_handle, &nvs_attrs);
    }
    else {
        mp_raise_OSError(MP_ENODEV);
    }

    HwiP_create(INT_FLASH, FLASH_hwiHandler, NULL);
    HwiP_setPriority(INT_FLASH, 0); // LWK TODO: find out what level USB actually runs at and make us just one higher

    pthread_t thread;
    pthread_attr_t attrs;

    pthread_attr_init(&attrs);
    pthread_attr_setstacksize(&attrs, 512); //TODO: Make stacksize a define
    pthread_create(&thread, &attrs, flash_bdev_flush_thread, NULL);
    pthread_attr_destroy(&attrs);
}

void * flash_bdev_flush_thread(void * arg)
{
    while(1) {
        HwiP_post(INT_FLASH);
        // Sleep 5 seconds
        mp_hal_delay_ms(5000);
    }
}


#define FLASH_FLAG_DIRTY        (1)
#define FLASH_FLAG_ERASED       (4)
static volatile uint8_t flash_flags = 0;
static uint32_t flash_cache_sector_id;
static volatile uint32_t flash_cache_sector_start;
static uint32_t flash_cache_sector_size;


int32_t flash_bdev_ioctl(uint32_t op, uint32_t arg) {
    (void)arg;
    switch (op) {
        case BDEV_IOCTL_INIT:
            flash_flags = 0;
            flash_cache_sector_id = -1; // basically set this to max
            flash_bdev_init();
            return 0;

        case BDEV_IOCTL_NUM_BLOCKS:
            return FLASH_MEM_SEG1_NUM_BLOCKS;

        case BDEV_IOCTL_SYNC:
            if (flash_flags & FLASH_FLAG_DIRTY) {
                while (flash_flags & FLASH_FLAG_DIRTY) {
                   HwiP_post(INT_FLASH); // Firing this interrupt should have such a high priority that we should never loop
                }
            }
            return 0;
    }
    return -MP_EINVAL;
}

static uint8_t *flash_cache_get_addr(uint32_t flash_addr, bool write) {
    uint32_t flash_sector_start;
    uint32_t flash_sector_size;
    uint32_t flash_sector_id = flash_get_sector_info(flash_addr, &flash_sector_start, &flash_sector_size);
    if (flash_sector_size > FLASH_SECTOR_SIZE_MAX) {
        flash_sector_size = FLASH_SECTOR_SIZE_MAX;
    }
    if (flash_cache_sector_id != flash_sector_id) {
        flash_bdev_ioctl(BDEV_IOCTL_SYNC, 0); //Sync 
        //NVS_lock(nvs_handle, NVS_LOCK_WAIT_FOREVER);
        int_fast16_t status = NVS_read(nvs_handle, flash_sector_start, (char *)CACHE_MEM_START_ADDR, flash_sector_size);
        //NVS_unlock(nvs_handle);
        if (status != NVS_STATUS_SUCCESS) {
            mp_raise_OSError(MP_EIO);
        }

        flash_cache_sector_id = flash_sector_id;
        flash_cache_sector_start = flash_sector_start;
        flash_cache_sector_size = flash_sector_size;
    }
    if (write) {
      flash_flags |= FLASH_FLAG_DIRTY;
      // indicate a dirty cache with LED on
      led_state(TILDA_LED_RED, 1);

    }
    return (uint8_t*)CACHE_MEM_START_ADDR + flash_addr - flash_sector_start;
}

static uint8_t *flash_cache_get_addr_for_read(uint32_t flash_addr) {
    return flash_cache_get_addr(flash_addr, false);
}

static uint8_t *flash_cache_get_addr_for_write(uint32_t flash_addr) {
    return flash_cache_get_addr(flash_addr, true);
}


static uint32_t convert_block_to_flash_addr(uint32_t block) {
    if (block < FLASH_MEM_SEG1_NUM_BLOCKS) {
        return block * FLASH_BLOCK_SIZE;
    }

    // bad block
    return -1;
}

void flash_bdev_flush(void) {
    if (!(flash_flags & FLASH_FLAG_DIRTY)) {
        return;
    }
    // sync the cache RAM buffer by writing it to the flash page
    //NVS_lock(nvs_handle, NVS_LOCK_WAIT_FOREVER);
    int_fast16_t status = NVS_write(nvs_handle, flash_cache_sector_start,
                       CACHE_MEM_START_ADDR, 0x4000, NVS_WRITE_ERASE | NVS_WRITE_POST_VERIFY);
    //NVS_unlock(nvs_handle);
    if (status != NVS_STATUS_SUCCESS) {
        mp_raise_OSError(MP_EIO);
    }
    // clear the flash flags now that we have a clean cache
    flash_flags = 0;
    //TODO: indicate a clean cache with LED off
    led_state(TILDA_LED_RED, 0);
}

bool flash_bdev_readblock(uint8_t *dest, uint32_t block) {
    // non-MBR block, get data from flash memory, possibly via cache
    uint32_t flash_addr = convert_block_to_flash_addr(block);
    if (flash_addr == -1) {
        // bad block number
        return false;
    }
    uint8_t *src = flash_cache_get_addr_for_read(flash_addr);
    memcpy(dest, src, FLASH_BLOCK_SIZE);
    return true;
}

bool flash_bdev_writeblock(const uint8_t *src, uint32_t block) {
    // non-MBR block, copy to cache
    uint32_t flash_addr = convert_block_to_flash_addr(block);
    if (flash_addr == -1) {
        // bad block number
        return false;
    }
    uint8_t *dest = flash_cache_get_addr_for_write(flash_addr);
    memcpy(dest, src, FLASH_BLOCK_SIZE);
    return true;
}

#endif