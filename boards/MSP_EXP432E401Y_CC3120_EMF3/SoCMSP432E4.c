#include <stdint.h>
#include <stdbool.h>

#include <ti/devices/msp432e4/driverlib/sysctl.h>

#include "SoC.h"

void SoC_reset()
{
    SysCtlReset();
}

uint32_t SoC_getResetCause()
{
    uint32_t rawCause = SysCtlResetCauseGet();
    static uint32_t cause = ~0u;

    // used the saved value since we clear HW the first time called
    if (cause != ~0u) {
        return cause;
    }

    if (rawCause & SYSCTL_CAUSE_POR || rawCause & SYSCTL_CAUSE_BOR) {
        cause = SOC_RESET_POWER;
    }
    else if (rawCause & SYSCTL_CAUSE_WDOG0 || rawCause & SYSCTL_CAUSE_WDOG1) {
        cause = SOC_RESET_WDT;
    }
    else if (rawCause & SYSCTL_CAUSE_EXT || rawCause & SYSCTL_CAUSE_HSRVREQ) {
        cause = SOC_RESET_HARD;
    }
    else if (rawCause & SYSCTL_CAUSE_SW) {
        cause = SOC_RESET_SOFT;
    }
    else if (rawCause & SYSCTL_CAUSE_HIB) {
        cause = SOC_RESET_SLEEP;
    }

    SysCtlResetCauseClear(~0u);

    return cause;
}
