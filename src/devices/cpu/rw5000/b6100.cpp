// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell B6100 MCU (based on B5500+B6000)

*/

#include "emu.h"
#include "b6100.h"

#include "rw5000d.h"


DEFINE_DEVICE_TYPE(B6100, b6100_cpu_device, "b6100", "Rockwell B6100")


// constructor
b6100_cpu_device::b6100_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	b6000_cpu_device(mconfig, B6100, tag, owner, clock, 10, address_map_constructor(FUNC(b6100_cpu_device::program_896x8), this), 6, address_map_constructor(FUNC(b6100_cpu_device::data_48x4), this))
{ }


// internal memory maps
void b6100_cpu_device::program_896x8(address_map &map)
{
	map(0x000, 0x2ff).rom();
	map(0x380, 0x3ff).rom();
}

void b6100_cpu_device::data_48x4(address_map &map)
{
	map(0x00, 0x0b).ram();
	map(0x10, 0x1b).ram();
	map(0x20, 0x2b).ram();
	map(0x30, 0x3b).ram();
}


// disasm
std::unique_ptr<util::disasm_interface> b6100_cpu_device::create_disassembler()
{
	return std::make_unique<b6100_disassembler>();
}


// digit segment decoder
u16 b6100_cpu_device::decode_digit(u8 data)
{
	static const u16 lut_segs[0x10] =
	{
		// 0-9 same as B6000
		0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f,

		// EFG, BCG, none, SEG8, SEG9, SEG10
		0x70, 0x46, 0x00, 0x80, 0x100, 0x200
	};
	return lut_segs[data & 0xf];
}


//-------------------------------------------------
//  execute
//-------------------------------------------------

void b6100_cpu_device::execute_one()
{
	switch (m_op)
	{
		case 0x1c: case 0x1d: case 0x1e: case 0x1f: op_lb(11); break;
		case 0x38: case 0x39: case 0x3a: case 0x3b: op_tl(); break;

		case 0x0c: op_sc(); break;
		case 0x0d: op_rsc(); break;

		// rest is same as B6000
		default: b6000_cpu_device::execute_one(); break;
	}

	// instead of with TKBS, carry flag directly outputs to SPK
	if (m_c != m_prev_c)
		m_write_spk(m_c);
}

bool b6100_cpu_device::op_is_tl(u8 op)
{
	return b6000_cpu_device::op_is_tl(op) || ((op & 0xfc) == 0x38);
}

bool b6100_cpu_device::op_is_lb(u8 op)
{
	return b6000_cpu_device::op_is_lb(op) || ((op & 0xfc) == 0x1c);
}


//-------------------------------------------------
//  changed opcodes (no need for separate file)
//-------------------------------------------------

void b6100_cpu_device::op_read()
{
	// READ: add KB to A, skip next on no overflow
	m_a += (m_read_kb() & 0xf);
	m_skip = !BIT(m_a, 4);
	m_a &= 0xf;
}
