// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell A5500 MCU

*/

#include "emu.h"
#include "a5500.h"

#include "rw5000d.h"


DEFINE_DEVICE_TYPE(A5500, a5500_cpu_device, "a5500", "Rockwell A5500")


// constructor
a5500_cpu_device::a5500_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data) :
	a5000_cpu_device(mconfig, type, tag, owner, clock, prgwidth, program, datawidth, data)
{ }

a5500_cpu_device::a5500_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	a5500_cpu_device(mconfig, A5500, tag, owner, clock, 10, address_map_constructor(FUNC(a5500_cpu_device::program_768x8), this), 6, address_map_constructor(FUNC(a5500_cpu_device::data_48x4), this))
{ }


// internal memory maps
void a5500_cpu_device::program_768x8(address_map &map)
{
	map(0x000, 0x27f).rom();
	map(0x380, 0x3ff).rom();
}

void a5500_cpu_device::data_48x4(address_map &map)
{
	map(0x00, 0x0b).ram();
	map(0x10, 0x1b).ram();
	map(0x20, 0x2b).ram();
	map(0x30, 0x3b).ram();
}


// disasm
std::unique_ptr<util::disasm_interface> a5500_cpu_device::create_disassembler()
{
	return std::make_unique<a5500_disassembler>();
}


//-------------------------------------------------
//  execute
//-------------------------------------------------

void a5500_cpu_device::execute_one()
{
	switch (m_op)
	{
		case 0x1c: case 0x1d: case 0x1e: case 0x1f: op_lb(11); break;
		case 0x38: case 0x39: case 0x3a: case 0x3b: op_tl(); break;

		case 0x0c: op_sc(); break;
		case 0x0d: op_rsc(); break;

		// rest is same as A5000
		default: a5000_cpu_device::execute_one(); break;
	}
}

bool a5500_cpu_device::op_is_tl(u8 op)
{
	return a5000_cpu_device::op_is_tl(op) || ((op & 0xfc) == 0x38);
}

bool a5500_cpu_device::op_is_lb(u8 op)
{
	return a5000_cpu_device::op_is_lb(op) || ((op & 0xfc) == 0x1c);
}
