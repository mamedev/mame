// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#include "emu.h"
#include "options.h"
#include "simm.h"

void pc9821_simm_options(device_slot_interface &device)
{
	device.option_add("2mb",  PC9801_61_2MB);
	device.option_add("4mb",  PC9801_61_4MB);
	device.option_add("8mb",  PC9801_61_8MB);
	device.option_add("16mb", PC9801_61_16MB);
}

void pc9801bx2_simm_options(device_slot_interface &device)
{
	pc9821_simm_options(device);
	device.option_add("20mb",  PC9801_61_20MB);
}

void pc9821ap2_simm_options(device_slot_interface &device)
{
	pc9821_simm_options(device);
	device.option_add("32mb", PC9801_61_32MB);
	device.option_add("64mb", PC9801_61_64MB);
}

