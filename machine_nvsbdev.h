#ifndef MICROPY_INCLUDED_TI_MACHINE_NVSBDEV_H
#define MICROPY_INCLUDED_TI_MACHINE_NVSBDEV_H
void * flash_bdev_flush_thread(void * arg);
uint32_t flash_get_sector_info(uint32_t addr, uint32_t *start_addr, uint32_t *size);
void flash_bdev_init(void);
void flash_bdev_flush(void);
#endif