// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    SAM Coupe Mouse Port modules

***************************************************************************/

#include "emu.h"
#include "modules.h"

#include "mouse.h"

void samcoupe_mouse_modules(device_slot_interface &device)
{
	device.option_add("mouse", SAM_MOUSE);
}
