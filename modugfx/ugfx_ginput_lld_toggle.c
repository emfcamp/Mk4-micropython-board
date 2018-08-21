#include "gfx.h"

#if (GFX_USE_GINPUT && GINPUT_NEED_TOGGLE)
#include "py/mphal.h"
#include "src/ginput/ginput_driver_toggle.h"
#include <ti/drivers/GPIO.h>
#include "tilda_thread.h"

GINPUT_TOGGLE_DECLARE_STRUCTURE();

void ginput_lld_toggle_init(const GToggleConfig *ptc) {
}

unsigned ginput_lld_toggle_getbits(const GToggleConfig *ptc) {
	return getAllButtonStates();
}

#endif /* GFX_USE_GINPUT && GINPUT_NEED_TOGGLE */
