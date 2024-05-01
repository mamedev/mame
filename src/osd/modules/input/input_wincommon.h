// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  input_wincommon.h - Common code used by Windows input modules
//
//============================================================
#ifndef MAME_OSD_INPUT_INPUT_WINCOMMON_H
#define MAME_OSD_INPUT_INPUT_WINCOMMON_H

#pragma once

#include "input_common.h"

#include <windows.h>


namespace osd {

// state information for a keyboard
struct keyboard_state
{
	uint8_t state[MAX_KEYS];
	int8_t  oldkey[MAX_KEYS];
	int8_t  currkey[MAX_KEYS];
};

// state information for a mouse
struct mouse_state
{
	LONG    lX;
	LONG    lY;
	LONG    lV;
	LONG    lH;
	BYTE    rgbButtons[8];
};

} // namespace osd

#endif // MAME_OSD_INPUT_INPUT_WINCOMMON_H
