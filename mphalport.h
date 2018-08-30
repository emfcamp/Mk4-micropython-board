#ifndef TIRTOS_MPHAL_H
#define TIRTOS_MPHAL_H

#include <stdint.h>

#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <xdc/runtime/Timestamp.h>

static inline void sleep_scaled(uint32_t ms) {
    if (Clock_tickPeriod == 1000) {
        Task_sleep(ms);
    }
    else {
        Task_sleep((ms * 1000) / Clock_tickPeriod);
    }
}

#define mp_hal_delay_ms(ms) sleep_scaled(ms)

#define mp_hal_delay_us(us) sleep_scaled(us / 1000)

#define mp_hal_delay_us_fast(us) sleep_scaled(us / 1000)

static inline uint32_t ticks_scaled() {
    if (Clock_tickPeriod == 1000) {
        return Clock_getTicks();
    }
    else {
        return (Clock_getTicks() * Clock_tickPeriod) / 1000;
    }
}

#define mp_hal_ticks_ms() ticks_scaled()

#define mp_hal_ticks_us() (ticks_scaled() * 1000)

#define mp_hal_ticks_cpu() Timestamp_get32()

#define mp_hal_pin_name(p) "pin"

#define mp_hal_pin_input(p)

#define mp_hal_pin_output(p)

extern void mp_hal_set_interrupt_char(int c);

#endif
