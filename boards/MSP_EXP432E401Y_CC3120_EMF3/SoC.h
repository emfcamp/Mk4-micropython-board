#ifndef SOC_H_INCLUDE
#define SOC_H_INCLUDE

#define SOC_RESET_POWER 1u
#define SOC_RESET_SLEEP 2u
#define SOC_RESET_HARD 4u
#define SOC_RESET_WDT 8u
#define SOC_RESET_SOFT 16u
#define SOC_RESET_UNKNOWN 128u

extern void SoC_reset(void);
extern uint32_t SoC_getResetCause(void);

#endif
