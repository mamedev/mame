// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell B5500 MCU

*/

#include "emu.h"
#include "b5500.h"

#include "rw5000d.h"


DEFINE_DEVICE_TYPE(B5500, b5500_cpu_device, "b5500", "Rockwell B5500")


// constructor
b5500_cpu_device::b5500_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	a5500_cpu_device(mconfig, B5500, tag, owner, clock, 10, address_map_constructor(FUNC(b5500_cpu_device::program_768x8), this), 6, address_map_constructor(FUNC(b5500_cpu_device::data_48x4), this))
{ }


// disasm
std::unique_ptr<util::disasm_interface> b5500_cpu_device::create_disassembler()
{
	return std::make_unique<b5500_disassembler>();
}


//-------------------------------------------------
//  execute
//-------------------------------------------------

void b5500_cpu_device::execute_one()
{
	switch (m_op)
	{
		case 0x1c: case 0x1d: case 0x1e: case 0x1f: op_lb(11); break;
		case 0x38: case 0x39: case 0x3a: case 0x3b: op_tl(); break;

		case 0x0c: op_sc(); break;
		case 0x0d: op_rsc(); break;

		// rest is same as B5000
		default: b5000_cpu_device::execute_one(); break;
	}
}
