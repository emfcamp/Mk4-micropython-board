#include <stdint.h>
#include <stdbool.h>

#include <ti/devices/msp432p4xx/driverlib/reset.h>

#include <SoC.h>

void SoC_reset()
{
    ResetCtl_initiateSoftReset();
}

uint32_t SoC_getResetCause()
{
    uint32_t hardCause = ResetCtl_getHardResetSource();
    uint32_t softCause = ResetCtl_getSoftResetSource();
    uint32_t pssCause = ResetCtl_getPSSSource();
    static uint32_t cause = ~0u;

    if (cause != ~0u) {
        return cause;
    }

    if (pssCause) {
        cause = SOC_RESET_POWER;
    }
    else if (hardCause) {
        cause = SOC_RESET_HARD;
    }
    else if (softCause) {
        cause = SOC_RESET_SOFT;
    }

    ResetCtl_clearSoftResetSource(~0u);
    ResetCtl_clearHardResetSource(~0u);
    ResetCtl_clearPSSFlags();

    return cause;
}
