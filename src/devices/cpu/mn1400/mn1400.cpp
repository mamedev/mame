// license:BSD-3-Clause
// copyright-holders:hap
/*

  Matsushita MN1400 MCU

TODO:
- stuff

*/

#include "emu.h"
#include "mn1400.h"

#include "mn1400d.h"


DEFINE_DEVICE_TYPE(MN1400, mn1400_cpu_device, "mn1400", "Matsushita MN1400")


// constructor
mn1400_cpu_device::mn1400_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data) :
	mn1400_base_device(mconfig, type, tag, owner, clock, prgwidth, program, datawidth, data)
{ }

mn1400_cpu_device::mn1400_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	mn1400_cpu_device(mconfig, MN1400, tag, owner, clock, 10, address_map_constructor(FUNC(mn1400_cpu_device::program_1024x8), this), 6, address_map_constructor(FUNC(mn1400_cpu_device::data_64x4), this))
{ }


// internal memory maps
void mn1400_cpu_device::program_1024x8(address_map &map)
{
	map(0x000, 0x3ff).rom();
}

void mn1400_cpu_device::data_64x4(address_map &map)
{
	map(0x00, 0x3f).ram();
}


// disasm
std::unique_ptr<util::disasm_interface> mn1400_cpu_device::create_disassembler()
{
	return std::make_unique<mn1400_disassembler>();
}


//-------------------------------------------------
//  execute
//-------------------------------------------------

bool mn1400_cpu_device::op_has_param(u8 op)
{
	return false;
}

void mn1400_cpu_device::execute_one()
{
	op_illegal();
}
