// license:BSD-3-Clause
// copyright-holders:hap, Jonathan Gevaryahu
/*

  Sharp SM590 MCU core implementation

  TODO:
  - finish SM590/SM595 emulation (NES/SNES CIC)

  http://bitsavers.informatik.uni-stuttgart.de/pdf/sharp/_dataBooks/1990_Sharp_Microcomputers_Data_Book.pdf
  pdf page 35/doc page 26 thru pdf page 44/doc page 35
*/

#include "emu.h"
#include "sm590.h"
#include "sm510d.h"
#include "debugger.h"


// MCU types
DEFINE_DEVICE_TYPE(SM590, sm590_device, "sm590", "Sharp SM590") // 512x8 ROM, 32x4 RAM
//DEFINE_DEVICE_TYPE(SM591, sm591_device, "sm591", "Sharp SM591") // 1kx8 ROM, 56x4 RAM
//DEFINE_DEVICE_TYPE(SM595, sm595_device, "sm595", "Sharp SM595") // 768x8 ROM, 32x4 RAM

// internal memory maps
void sm590_device::program_1x128x4(address_map &map)
{
	map(0x000, 0x1ff).rom();
}

void sm590_device::data_16x2x4(address_map &map)
{
	map(0x00, 0x0f).ram();
	map(0x10, 0x1f).ram();
}

// device definitions
sm590_device::sm590_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	sm590_device(mconfig, SM590, tag, owner, clock, 4 /* stack levels */, 9 /* prg width */, address_map_constructor(FUNC(sm590_device::program_1x128x4), this), 5 /* data width */, address_map_constructor(FUNC(sm590_device::data_16x2x4), this))
{ }

//sm591_device::sm591_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
//	sm510_base_device(mconfig, SM591, tag, owner, clock, 4 /* stack levels */, 10 /* prg width */, address_map_constructor(FUNC(sm591_device::program_2x128x4), this), 6 /* data width */, address_map_constructor(FUNC(sm591_device::data_16x3.5x4), this))
//{ }

//sm595_device::sm595_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
//	sm510_base_device(mconfig, SM595, tag, owner, clock, 4 /* stack levels */, 10 /* prg width */, address_map_constructor(FUNC(sm595_device::program_1x128x4_1x128x2), this), 5 /* data width */, address_map_constructor(FUNC(sm595_device::data_16x2x4), this))
//{ }

sm590_device::sm590_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int stack_levels, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data) :
	sm510_base_device(mconfig, type, tag, owner, clock, stack_levels, prgwidth, program, datawidth, data)
{ }


std::unique_ptr<util::disasm_interface> sm590_device::create_disassembler()
{
	return std::make_unique<sm590_disassembler>();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------
void sm590_device::device_reset()
{
	// ACL
	m_skip = false;
	m_halt = false;
	m_sbm = false; // needed?
	m_op = m_prev_op = 0;
	reset_vector();
	m_prev_pc = m_pc;
	m_clk_div = 4; // 4 clock oscillations per cycle on SM59x, see datasheet page 30/pdf page 39

	m_rports[0] = m_rports[1] = m_rports[2] = m_rports[3] = 0;
	//m_write_r(0, 0, 0xff); // TODO: are the four ports zeroed on reset?
}

//-------------------------------------------------
//  wake from suspend mode
//-------------------------------------------------
bool sm590_device::wake_me_up()
{
	// in halt mode, wake up after R2.2 goes high
	if (m_rports[2]&0x4)
	{
		m_halt = false;
		do_branch(0, 1, 0); // field 0, page 1, step 0

		standard_irq_callback(0);
		return true;
	}
	else
		return false;
}

//-------------------------------------------------
//  execute
//-------------------------------------------------
void sm590_device::increment_pc()
{
	// PL(program counter low 7 bits) is a simple LFSR: newbit = (bit0==bit1)
	// PU,PM(high bits) specify page, PL specifies steps within page
	int feed = ((m_pc >> 1 ^ m_pc) & 1) ? 0 : 0x40;
	m_pc = feed | (m_pc >> 1 & 0x3f) | (m_pc & ~0x7f);
}

void sm590_device::get_opcode_param()
{
	// TL, TLS(TML) opcodes are 2 bytes
	if ((m_op & 0xf8) == 0x78)
	{
		m_icount--;
		m_param = m_program->read_byte(m_pc);
		increment_pc();
	}
}

void sm590_device::execute_one()
{
	switch (m_op & 0xf0) // opcodes with 4 bit params
	{
		case 0x00: op_adx(); break;
		case 0x10: op_tax(); break;
		case 0x20: op_lblx(); break;
		case 0x30: op_lax(); break;
		case 0x80: case 0x90: case 0xa0: case 0xb0:
		case 0xc0: case 0xd0: case 0xe0: case 0xf0:
		op_t(); break; // aka tr

		default: // opcodes with 2 bit params
			switch (m_op & 0xfc)
			{
		case 0x60: op_tmi(); break; // aka tm
		case 0x64: op_tba(); break;
		case 0x68: op_rm(); break;
		case 0x6c: op_sm(); break;
		case 0x74: op_lbmx(); break;
		case 0x78: op_tl(); break;
		case 0x7c: op_tml(); break; // aka tls

		default: // everything else
			switch (m_op)
			{
		case 0x40: op_lda(); break;
		case 0x41: op_exc(); break;
		case 0x42: op_exci(); break;
		case 0x43: op_excd(); break;
		case 0x44: op_coma(); break;
		case 0x45: op_tam(); break;
		case 0x46: op_atr(); break;
		case 0x47: op_mtr(); break;
		case 0x48: op_rc(); break;
		case 0x49: op_sc(); break;
		case 0x4a: op_str(); break;
		case 0x4b: op_cend(); break; // aka cctrl
		case 0x4c: op_rtn0(); break; // aka rtn
		case 0x4d: op_rtn1(); break; // aka rtns
		// 4e, 4f illegal
		case 0x50: op_inbm(); break;
		case 0x51: op_debm(); break;
		case 0x52: op_incb(); break; // aka inbl
		case 0x53: op_decb(); break; // aka debl
		case 0x54: op_tc(); break;
		case 0x55: op_rta(); break;
		case 0x56: op_blta(); break;
		case 0x57: op_exbla(); break; // aka xbla
		// 58, 59, 5a, 5b illegal
		case 0x5c: op_atx(); break;
		case 0x5d: op_exax(); break;
		// 5e is illegal???
		// 5f is illegal
		case 0x70: op_add(); break;
		case 0x71: op_ads(); break;
		case 0x72: op_adc(); break;
		case 0x73: op_add11(); break; // aka adcs

		default: op_illegal(); break;
			}
			break; // 0xff

			}
			break; // 0xfc

	} // big switch
}
