// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell B5000 MCU

TODO:
- is unmapped ram mirrored? (that goes for subdevices too)
- not sure how 7seg zero suppression works, the only documentation for it
  is a block diagram with the logic connected to the 7seg decoder
- fill unknown data in segment decoder, it's not on a neat PLA
- is ATB an unskippable opcode? nothing relies on it

*/

#include "emu.h"
#include "b5000.h"

#include "rw5000d.h"


DEFINE_DEVICE_TYPE(B5000, b5000_cpu_device, "b5000", "Rockwell B5000")


// constructor
b5000_cpu_device::b5000_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data) :
	rw5000_base_device(mconfig, type, tag, owner, clock, prgwidth, program, datawidth, data)
{ }

b5000_cpu_device::b5000_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	b5000_cpu_device(mconfig, B5000, tag, owner, clock, 9, address_map_constructor(FUNC(b5000_cpu_device::program_448x8), this), 6, address_map_constructor(FUNC(b5000_cpu_device::data_45x4), this))
{ }


// internal memory maps
void b5000_cpu_device::program_448x8(address_map &map)
{
	map(0x000, 0x0bf).rom();
	map(0x100, 0x1ff).rom();
}

void b5000_cpu_device::data_45x4(address_map &map)
{
	map(0x00, 0x0b).ram();
	map(0x10, 0x1a).ram();
	map(0x20, 0x2a).ram();
	map(0x30, 0x3a).ram();
}


// disasm
std::unique_ptr<util::disasm_interface> b5000_cpu_device::create_disassembler()
{
	return std::make_unique<b5000_disassembler>();
}


// digit segment decoder
u16 b5000_cpu_device::decode_digit(u8 data)
{
	static const u8 lut_segs[0x10] =
	{
		// 0-9 ok (6 and 9 have tails)
		0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f,

		// ?, ?, newline?, -, ?, ?
		0, 0, 0x00, 0x40, 0, 0
	};

	u8 dp = BIT(data, 4);
	u8 seg = lut_segs[data & 0xf];

	// zero suppression logic is done in hardware
	if (data > 0)
		m_suppress0 = false;
	if (m_suppress0)
		seg = 0;
	if ((data & 0xf) >= 0xc)
		m_suppress0 = true;

	return seg << 1 | dp;
}


//-------------------------------------------------
//  execute
//-------------------------------------------------

void b5000_cpu_device::execute_one()
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

		case 0x20: op_lb(7); break;
		case 0x24: op_lb(10); break;
		case 0x28: op_lb(9); break;
		case 0x2c: op_lb(8); break;
		case 0x3c: op_lb(0); break;

		case 0x30: case 0x34: op_tl(); break;

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
		case 0x39: op_rsc(); break;
		case 0x3b: op_sc(); break;
		case 0x74: op_kseg(); break;
		case 0x77: m_atb_step = 1; break;

		default: op_illegal(); break;
			}
			break; // 0xff

			}
			break; // 0xfc

	} // 0xf0
}

bool b5000_cpu_device::op_is_tl(u8 op)
{
	return ((op & 0xf8) == 0x30);
}

bool b5000_cpu_device::op_is_lb(u8 op)
{
	return ((op & 0xf0) == 0x20) || ((op & 0xfc) == 0x3c);
}

bool b5000_cpu_device::op_is_atb(u8 op)
{
	return (op == 0x77);
}
