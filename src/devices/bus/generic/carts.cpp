// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

 Generic ROM / RAM socket slots

 **********************************************************************/


#include "emu.h"
#include "carts.h"
#include "rom.h"
#include "ram.h"


device_slot_interface &generic_plain_slot(device_slot_interface &device)
{
	device.option_add_internal("rom", GENERIC_ROM_PLAIN);
	return device;
}

device_slot_interface &generic_linear_slot(device_slot_interface &device)
{
	device.option_add_internal("rom", GENERIC_ROM_LINEAR);
	return device;
}

device_slot_interface &generic_romram_plain_slot(device_slot_interface &device)
{
	device.option_add_internal("rom", GENERIC_ROMRAM_PLAIN);
	return device;
}
