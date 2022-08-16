// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

 Generic ROM / RAM socket slots

 **********************************************************************/


#include "emu.h"
#include "carts.h"
#include "rom.h"


device_slot_interface &mononcol_plain_slot(device_slot_interface &device)
{
	device.option_add_internal("rom", MONONCOL_ROM_PLAIN);
	return device;
}
