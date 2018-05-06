// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    EACA Colour Genie Parallel Carts

***************************************************************************/

#include "emu.h"
#include "carts.h"
#include "joystick.h"
#include "printer.h"


void cg_parallel_slot_carts(device_slot_interface &device)
{
	device.option_add("joystick", CGENIE_JOYSTICK);
	device.option_add("printer", CGENIE_PRINTER);
}
