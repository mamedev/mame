// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/**********************************************************************

    RC2014 Modules

**********************************************************************/

#include "emu.h"
#include "bus/rc2014/rc2014.h"

#include "bus/rc2014/cf.h"
#include "bus/rc2014/clock.h"
#include "bus/rc2014/fdc.h"
#include "bus/rc2014/ide.h"
#include "bus/rc2014/micro.h"
#include "bus/rc2014/ram.h"
#include "bus/rc2014/rom.h"
#include "bus/rc2014/romram.h"
#include "bus/rc2014/rtc.h"
#include "bus/rc2014/serial.h"
#include "bus/rc2014/sound.h"
#include "bus/rc2014/z80cpu.h"

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
	device.option_add("rtc", RC2014_DS1302_RTC);
	device.option_add("ym_sound", RC2014_YM2149_SOUND);
	device.option_add("ay_sound", RC2014_AY8190_SOUND);
	device.option_add("82c55_ide", RC2014_82C55_IDE);
	device.option_add("ide_hdd", RC2014_IDE_HDD);
	device.option_add("fdc_smc", RC2014_FDC9266);
	device.option_add("fdc_wdc", RC2014_WD37C65);
	device.option_add("micro", RC2014_MICRO);
}

// Mini mezzanine boards
void rc2014_mini_bus_modules(device_slot_interface &device)
{
	rc2014_bus_modules(device);
	device.option_add("mini", RC2014_MICRO);
	device.option_add("mini_cpm", RC2014_MINI_CPM);
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
