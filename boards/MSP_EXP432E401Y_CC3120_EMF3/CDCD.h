#ifndef CDCD_H_INCLUDE
#define CDCD_H_INCLUDE

#include <stdint.h>
#include <stdbool.h>

typedef struct CDCD_Object * CDCD_Handle;

extern CDCD_Handle CDCD_handles[];
extern const uint32_t CDCD_count;

extern uint32_t CDCD_available(CDCD_Handle cdcd);
extern void CDCD_init(void);
extern CDCD_Handle CDCD_open(uint32_t index);
extern uint32_t CDCD_receiveData(CDCD_Handle cdcd, void * buf, uint32_t length,
                                 uint32_t timeout);
extern uint32_t CDCD_sendData(CDCD_Handle cdcd, const void * buf, uint32_t length,
                              uint32_t timeout);
extern bool CDCD_waitForConnect(CDCD_Handle cdcd, uint32_t timeout);


#endif
