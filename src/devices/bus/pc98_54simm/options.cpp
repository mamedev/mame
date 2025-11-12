// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#include "emu.h"
#include "options.h"
#include "simm.h"

void pc9801ux_simm_options(device_slot_interface &device)
{
	device.option_add("2mb",  PC9801_54_2MB);
	device.option_add("4mb",  PC9801_54_4MB);
	device.option_add("7mb",  PC9801_54_7MB);
}

void pc9801vx_simm_options(device_slot_interface &device)
{
	pc9801ux_simm_options(device);
	device.option_add("8mb",  PC9801_54_8MB);
}

void pc9801dx_simm_options(device_slot_interface &device)
{
	pc9801vx_simm_options(device);
	device.option_add("9mb",  PC9801_54_9MB);
	device.option_add("15mb", PC9801_54_15MB);
}

