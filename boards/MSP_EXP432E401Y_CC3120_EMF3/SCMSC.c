/*
 * Adapter functions for "storage" usage with MSC device
 */

#include "SCMSC.h"

/* storage.c function declarations, can't include storage.h due to mp paths */
extern void storage_init(void);
extern uint32_t storage_get_block_size(void);
extern uint32_t storage_get_block_count(void);
extern void storage_flush(void);

// these return 0 on success, non-zero on error
extern unsigned storage_read_blocks(uint8_t *dest, uint32_t block_num, uint32_t num_blocks);
extern unsigned storage_write_blocks(const uint8_t *src, uint32_t block_num, uint32_t num_blocks);

static uint32_t blockSize;

void * SCMSC_open(uint32_t region)
{
    storage_init();
    blockSize = storage_get_block_size();

    return (void *)0xdeadbeef;
}

void SCMSC_close(void * drive)
{
    storage_flush();
}

uint32_t SCMSC_read(void * drive, uint8_t * buf, uint32_t sector, uint32_t numBlocks)
{
#if 1
    storage_read_blocks(buf, sector, numBlocks);
    return numBlocks * blockSize;
#else
    if (storage_read_blocks(buf, sector, numBlocks) == 0) {
        return numBlocks * blockSize;
    }
    else {
        return 0;
    }
#endif
}

uint32_t SCMSC_write(void * drive, uint8_t * buf, uint32_t sector, uint32_t numBlocks)
{
    if (storage_write_blocks(buf, sector, numBlocks) == 0) {
        return numBlocks * blockSize;
    }
    else {
        return 0;
    }
}

uint32_t SCMSC_getNumBlocks(void * drive)
{
    return storage_get_block_count();
}

uint32_t SCMSC_getBlockSize(void * drive)
{
    return blockSize;
}
