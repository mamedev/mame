// license:BSD-3-Clause
// copyright-holders:Carl

#include "emu.h"
#include "options.h"

#include "basic_joy.h"
#include "mssw.h"

void pc_joysticks(device_slot_interface &device)
{
	device.option_add("basic_joy",     PC_BASIC_JOY);
	device.option_add("magnum6_stick", PC_MAGNUM6_STICK);
	device.option_add("pc_gamepad",    PC_GRAVIS_GAMEPAD);
	device.option_add("mssw_pad",      PC_MSSW_PAD);
}
