#ifndef SCMSC_H_INC
#define SCMSC_H_INC

#include <stdint.h>

void * SCMSC_open(uint32_t region);
void SCMSC_close(void * drive);
uint32_t SCMSC_read(void * drive, uint8_t * buf, uint32_t sector, uint32_t numBlocks);
uint32_t SCMSC_write(void * drive, uint8_t * buf, uint32_t sector, uint32_t numBlocks);
uint32_t SCMSC_getNumBlocks(void * drive);
uint32_t SCMSC_getBlockSize(void * drive);

#endif
