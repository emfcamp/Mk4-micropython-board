#include <stdint.h>
#include <stdbool.h>

#include <ti/devices/cc13x2_cc26x2_v2/driverlib/sys_ctrl.h>

#include <SoC.h>

void SoC_reset()
{
    SysCtrlSystemReset();
}

uint32_t SoC_getResetCause()
{
    uint32_t rawCause = SysCtrlResetSourceGet();
    uint32_t cause = SOC_RESET_UNKNOWN;

    if (rawCause == RSTSRC_PWR_ON || rawCause == RSTSRC_VDDS_LOSS ||
        rawCause == RSTSRC_VDDR_LOSS) {

        cause = SOC_RESET_POWER;
    }
    else if (rawCause == RSTSRC_PIN_RESET || rawCause == 1) {
        cause = SOC_RESET_HARD;
    }
    else if (rawCause == RSTSRC_WARMRESET || rawCause == RSTSRC_SYSRESET) {
        cause = SOC_RESET_SOFT;
    }
    else if (rawCause == RSTSRC_WAKEUP_FROM_SHUTDOWN) {
        cause = SOC_RESET_SLEEP;
    }

    return cause;
}
