// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell B6100 MCU (based on A5500+B6000)

*/

#include "emu.h"
#include "b6100.h"

#include "b5000d.h"


DEFINE_DEVICE_TYPE(B6100, b6100_cpu_device, "b6100", "Rockwell B6100")


// constructor
b6100_cpu_device::b6100_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data) :
	b6000_cpu_device(mconfig, type, tag, owner, clock, prgwidth, program, datawidth, data)
{ }

b6100_cpu_device::b6100_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	b6100_cpu_device(mconfig, B6100, tag, owner, clock, 10, address_map_constructor(FUNC(b6100_cpu_device::program_896x8), this), 6, address_map_constructor(FUNC(b6100_cpu_device::data_48x4), this))
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
	return std::make_unique<b5500_disassembler>();
}


// digit segment decoder
u16 b6100_cpu_device::decode_digit(u8 data)
{
	static u16 lut_segs[0x10] =
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
	switch (m_op & 0xf0)
	{
		case 0x40: op_lax(); break;
		case 0x60:
			if (m_op != 0x6f)
				op_adx();
			else
				op_read();
			break;

		case 0x80: case 0x90: case 0xa0: case 0xb0:
		case 0xc0: case 0xd0: case 0xe0: case 0xf0:
			m_tra_step = 1; break;

		default:
			switch (m_op & 0xfc)
			{
		case 0x04: op_tdin(); break;
		case 0x08: op_tm(); break;
		case 0x10: op_sm(); break;
		case 0x14: op_rsm(); break;
		case 0x18: m_ret_step = 1; break;

		case 0x1c: op_lb(11); break;
		case 0x20: op_lb(7); break;
		case 0x24: op_lb(10); break;
		case 0x28: op_lb(9); break;
		case 0x2c: op_lb(8); break;
		case 0x3c: op_lb(0); break;

		case 0x30: case 0x34: case 0x38: op_tl(); break;

		case 0x50: op_lda(); break;
		case 0x54: op_excp(); break;
		case 0x58: op_exc0(); break;
		case 0x5c: op_excm(); break;
		case 0x70: op_add(); break;
		case 0x78: op_comp(); break;
		case 0x7c: op_tam(); break;

		default:
			switch (m_op)
			{
		case 0x00: op_nop(); break;
		case 0x01: op_tc(); break;
		case 0x02: op_tkb(); break;
		case 0x03: op_tkbs(); break;
		case 0x0c: op_sc(); break;
		case 0x0d: op_rsc(); break;
		case 0x74: op_kseg(); break;
		case 0x76: op_atbz(); break;
		case 0x77: op_atb(); break;

		default: op_illegal(); break;
			}
			break; // 0xff

			}
			break; // 0xfc
	}

	update_speaker();
}

bool b6100_cpu_device::op_canskip(u8 op)
{
	// TL is unskippable
	return ((op & 0xf8) != 0x30) && ((op & 0xfc) != 0x38);
}

bool b6100_cpu_device::op_is_lb(u8 op)
{
	return ((op & 0xfc) == 0x1c) || ((op & 0xf0) == 0x20) || ((op & 0xfc) == 0x3c);
}


//-------------------------------------------------
//  changed opcodes (no need for separate file)
//-------------------------------------------------

void b6100_cpu_device::op_atb()
{
	// ATB: load Bl from A (delayed)
	m_bl = m_a;
	m_bl_delay = true;
}

void b6100_cpu_device::op_read()
{
	// READ: add KB to A, skip next on no overflow
	m_a += (m_read_kb() & 0xf);
	m_skip = !BIT(m_a, 4);
	m_a &= 0xf;
}
