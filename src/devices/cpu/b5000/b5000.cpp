// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell B5000 MCU core implementation

*/

#include "emu.h"
#include "b5000.h"

#include "b5000d.h"


DEFINE_DEVICE_TYPE(B5000, b5000_cpu_device, "b5000", "Rockwell B5000")


// constructor
b5000_cpu_device::b5000_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	b5000_base_device(mconfig, B5000, tag, owner, clock, 9, address_map_constructor(FUNC(b5000_cpu_device::program_map), this), 6, address_map_constructor(FUNC(b5000_cpu_device::data_map), this))
{ }


// internal memory maps
void b5000_cpu_device::program_map(address_map &map)
{
	map(0x000, 0x1ff).rom();
}

void b5000_cpu_device::data_map(address_map &map)
{
	map(0x00, 0x3f).ram();
}


// disasm
std::unique_ptr<util::disasm_interface> b5000_cpu_device::create_disassembler()
{
	return std::make_unique<b5000_disassembler>();
}


// initialize
void b5000_cpu_device::device_start()
{
	b5000_base_device::device_start();
}

void b5000_cpu_device::device_reset()
{
	b5000_base_device::device_reset();
}


//-------------------------------------------------
//  execute
//-------------------------------------------------

void b5000_cpu_device::execute_one()
{
}
