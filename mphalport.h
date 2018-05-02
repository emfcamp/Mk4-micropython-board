#ifndef TIRTOS_MPHAL_H
#define TIRTOS_MPHAL_H

#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>

#define mp_hal_delay_ms(ms) Task_sleep(ms)

#define mp_hal_delay_us(us) Task_sleep(us / 1000)

#define mp_hal_delay_us_fast(us) Task_sleep(us / 1000)

#define mp_hal_ticks_ms() Clock_getTicks()

#define mp_hal_ticks_us() (Clock_getTicks() * 1000)

#define mp_hal_ticks_cpu() Clock_getTicks()

static inline void mp_hal_set_interrupt_char(char c) {}

#define mp_hal_pin_name(p) "pin"

#define mp_hal_pin_input(p)

#define mp_hal_pin_output(p)

#endif
