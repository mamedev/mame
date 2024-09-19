// license:BSD-3-Clause
// copyright-holders:hap
/*

  Sharp SM510 MCU core implementation

*/

#include "emu.h"
#include "sm510.h"
#include "sm510d.h"


// MCU types
DEFINE_DEVICE_TYPE(SM510, sm510_device, "sm510", "Sharp SM510") // 2.7Kx8 ROM, 128x4 RAM(32x4 for LCD)


// internal memory maps
void sm510_device::program_2_7k(address_map &map)
{
	map(0x0000, 0x02bf).rom();
	map(0x0400, 0x06bf).rom();
	map(0x0800, 0x0abf).rom();
	map(0x0c00, 0x0ebf).rom();
}

void sm510_device::data_96_32x4(address_map &map)
{
	map(0x00, 0x5f).ram();
	map(0x60, 0x6f).ram().share("lcd_ram_a");
	map(0x70, 0x7f).ram().share("lcd_ram_b");
}


// device definitions
sm510_device::sm510_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	sm510_base_device(mconfig, SM510, tag, owner, clock, 2 /* stack levels */, 12 /* prg width */, address_map_constructor(FUNC(sm510_device::program_2_7k), this), 7 /* data width */, address_map_constructor(FUNC(sm510_device::data_96_32x4), this))
{ }


// disasm
std::unique_ptr<util::disasm_interface> sm510_device::create_disassembler()
{
	return std::make_unique<sm510_disassembler>();
}


//-------------------------------------------------
//  buzzer controller
//-------------------------------------------------

void sm510_device::clock_melody()
{
	u8 out = 0;

	if (m_r_mask_option == RMASK_DIRECT)
	{
		// direct output
		out = m_r & 3;
	}
	else
	{
		// from divider, R2 inverse phase
		out = m_div >> m_r_mask_option & 1;
		out |= (out << 1 ^ 2);
		out &= m_r;
	}

	// output to R pins
	if (out != m_r_out)
	{
		m_write_r(out);
		m_r_out = out;
	}
}


//-------------------------------------------------
//  execute
//-------------------------------------------------

void sm510_device::execute_one()
{
	switch (m_op & 0xf0)
	{
		case 0x20: op_lax(); break;
		case 0x30: op_adx(); break;
		case 0x40: op_lb(); break;

		case 0x80: case 0x90: case 0xa0: case 0xb0:
			op_t(); break;
		case 0xc0: case 0xd0: case 0xe0: case 0xf0:
			op_tm(); break;

		default:
			switch (m_op & 0xfc)
			{
		case 0x04: op_rm(); break;
		case 0x0c: op_sm(); break;
		case 0x10: op_exc(); break;
		case 0x14: op_exci(); break;
		case 0x18: op_lda(); break;
		case 0x1c: op_excd(); break;
		case 0x54: op_tmi(); break;
		case 0x70: case 0x74: case 0x78: op_tl(); break;
		case 0x7c: op_tml(); break;

		default:
			switch (m_op)
			{
		case 0x00: op_skip(); break;
		case 0x01: op_atbp(); break;
		case 0x02: op_sbm(); break;
		case 0x03: op_atpl(); break;
		case 0x08: op_add(); break;
		case 0x09: op_add11(); break;
		case 0x0a: op_coma(); break;
		case 0x0b: op_exbla(); break;

		case 0x51: op_tb(); break;
		case 0x52: op_tc(); break;
		case 0x53: op_tam(); break;
		case 0x58: op_tis(); break;
		case 0x59: op_atl(); break;
		case 0x5a: op_ta0(); break;
		case 0x5b: op_tabl(); break;
		case 0x5d: op_cend(); break;
		case 0x5e: op_tal(); break;
		case 0x5f: op_lbl(); break;

		case 0x60: op_atfc(); break;
		case 0x61: op_atr(); break;
		case 0x62: op_wr(); break;
		case 0x63: op_ws(); break;
		case 0x64: op_incb(); break;
		case 0x65: op_idiv(); break;
		case 0x66: op_rc(); break;
		case 0x67: op_sc(); break;
		case 0x68: op_tf1(); break;
		case 0x69: op_tf4(); break;
		case 0x6a: op_kta(); break;
		case 0x6b: op_rot(); break;
		case 0x6c: op_decb(); break;
		case 0x6d: op_bdc(); break;
		case 0x6e: op_rtn0(); break;
		case 0x6f: op_rtn1(); break;

		default: op_illegal(); break;
			}
			break; // 0xff

			}
			break; // 0xfc

	} // 0xf0

	// BM high bit is only valid for 1 step
	m_bmask = (m_op == 0x02) ? 0x40 : 0;
}

bool sm510_device::op_argument()
{
	// LBL, TL, TML opcodes are 2 bytes
	return m_op == 0x5f || (m_op & 0xf0) == 0x70;
}
