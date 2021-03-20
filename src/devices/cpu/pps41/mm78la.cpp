// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell MM77LA/MM78LA MCU

*/

#include "emu.h"
#include "mm78la.h"


DEFINE_DEVICE_TYPE(MM78LA, mm78la_device, "mm78la", "Rockwell MM78LA") // MM78L + output PLA and tone generator
DEFINE_DEVICE_TYPE(MM77LA, mm77la_device, "mm77la", "Rockwell MM77LA") // MM77L + output PLA and tone generator


// constructor
mm78la_device::mm78la_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	mm78la_device(mconfig, MM78LA, tag, owner, clock, 11, address_map_constructor(FUNC(mm78la_device::program_2k), this), 7, address_map_constructor(FUNC(mm78la_device::data_128x4), this))
{ }

mm78la_device::mm78la_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data) :
	mm78_device(mconfig, type, tag, owner, clock, prgwidth, program, datawidth, data)
{ }

mm77la_device::mm77la_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	mm78la_device(mconfig, MM77LA, tag, owner, clock, 11, address_map_constructor(FUNC(mm77la_device::program_1_5k), this), 7, address_map_constructor(FUNC(mm77la_device::data_96x4), this))
{ }


// machine config
void mm78la_device::device_add_mconfig(machine_config &config)
{
	PLA(config, "opla", 5, 14, 32).set_format(pla_device::FMT::BERKELEY);
}

void mm77la_device::device_add_mconfig(machine_config &config)
{
	PLA(config, "opla", 4, 10, 16).set_format(pla_device::FMT::BERKELEY);
}


// initialize
void mm78la_device::device_start()
{
	mm78_device::device_start();
	set_r_pins(14);
}

void mm78la_device::device_reset()
{
	mm78_device::device_reset();
}

void mm77la_device::device_start()
{
	mm78la_device::device_start();
	set_r_pins(10);
}

void mm77la_device::device_reset()
{
	mm78la_device::device_reset();
}
