// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "carts.h"
#include "rom.h"


device_slot_interface &c2color_plain_slot(device_slot_interface &device)
{
	device.option_add_internal("rom", C2COLOR_ROM_PLAIN);
	return device;
}
