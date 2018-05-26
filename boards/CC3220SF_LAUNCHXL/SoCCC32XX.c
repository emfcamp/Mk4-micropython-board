#include <stdint.h>
#include <stdbool.h>

#include <ti/devices/cc32xx/inc/hw_types.h>
#include <ti/devices/cc32xx/driverlib/prcm.h>

#include <SoC.h>

void SoC_reset()
{
    PRCMMCUReset(true);
}

uint32_t SoC_getResetCause()
{
    uint32_t rawCause = PRCMSysResetCauseGet();
    uint32_t cause;

    switch (rawCause) {
    case PRCM_POWER_ON:
        cause = SOC_RESET_POWER;
        break;

    case PRCM_LPDS_EXIT:
    case PRCM_HIB_EXIT:
        cause = SOC_RESET_SLEEP;
        break;

    case PRCM_CORE_RESET:
    case PRCM_MCU_RESET:
    case PRCM_SOC_RESET:
        cause = SOC_RESET_SOFT;
        break;

    case PRCM_WDT_RESET:
        cause = SOC_RESET_WDT;
        break;

    default:
        cause = SOC_RESET_UNKNOWN;
    }

    return cause;
}
