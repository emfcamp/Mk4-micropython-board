#include "py/mphal.h"
#include "tilda_thread.h"

#ifndef _GDISP_LLD_TOGGLE_BOARD_H
#define _GDISP_LLD_TOGGLE_BOARD_H

// The below are example values

#define GINPUT_TOGGLE_NUM_PORTS			Buttons_MAX			// The total number of toggle inputs
#define GINPUT_TOGGLE_CONFIG_ENTRIES	1			// The total number of GToggleConfig entries

// {id, mask, invert, mode}
#define GINPUT_TOGGLE_DECLARE_STRUCTURE()											 \
const GToggleConfig GInputToggleConfigTable[GINPUT_TOGGLE_CONFIG_ENTRIES] = {    \
    {0, 0x003fffff, 0, 0},       \
}

#endif /* _GDISP_LLD_TOGGLE_BOARD_H */
