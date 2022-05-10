// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/**********************************************************************

    RC2014 Modules

**********************************************************************/

#include "emu.h"
#include "bus/rc2014/rc2014.h"

#include "bus/rc2014/z80cpu.h"
#include "bus/rc2014/clock.h"
#include "bus/rc2014/ram.h"
#include "bus/rc2014/rom.h"
#include "bus/rc2014/romram.h"
#include "bus/rc2014/serial.h"
#include "bus/rc2014/cf.h"

void rc2014_bus_modules(device_slot_interface &device)
{
	device.option_add("z80", RC2014_Z80CPU);
	// Z80 2.1 on standard bus is same as Z80 module
	device.option_add("z80_21_40p", RC2014_Z80CPU);
	device.option_add("clock", RC2014_SINGLE_CLOCK);
	device.option_add("dual_clk_40p", RC2014_DUAL_CLOCK_40P);
	device.option_add("ram32k", RC2014_RAM_32K);
	device.option_add("ram64k_40p", RC2014_RAM_64K_40P);
	device.option_add("sw_rom", RC2014_SWITCHABLE_ROM);
	device.option_add("serial", RC2014_SERIAL_IO);
	device.option_add("sio_40p", RC2014_DUAL_SERIAL_40P);
	device.option_add("cf", RC2014_COMPACT_FLASH);
	device.option_add("rom_ram", RC2014_ROM_RAM_512);
}

void rc2014_ext_bus_modules(device_slot_interface &device)
{
	rc2014_bus_modules(device);
	device.option_add("z80_21", RC2014_Z80CPU_21);
	device.option_add("dual_clk", RC2014_DUAL_CLOCK);
	device.option_add("sio", RC2014_DUAL_SERIAL);
	device.option_add("page_rom", RC2014_PAGABLE_ROM);
	device.option_add("ram64k", RC2014_RAM_64K);
}

void rc2014_rc80_bus_modules(device_slot_interface &device)
{
	rc2014_ext_bus_modules(device);
}
