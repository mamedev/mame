// license:BSD-3-Clause
// copyright-holders:hap, Jonathan Gevaryahu
/*

  Sharp SM590 MCU core implementation

  TODO:
  - finish SM590/SM595 emulation (NES/SNES CIC)

*/

#include "emu.h"
#include "sm590.h"
#include "debugger.h"


// MCU types
DEFINE_DEVICE_TYPE(SM590, sm590_device, "sm590", "SM590") // 512x8 ROM, 32x4 RAM
//DEFINE_DEVICE_TYPE(SM591, sm591_device, "sm591", "SM591") // 1kx8 ROM, 56x4 RAM
//DEFINE_DEVICE_TYPE(SM595, sm595_device, "sm595", "SM595") // 768x8 ROM, 32x4 RAM

// internal memory maps
static ADDRESS_MAP_START(program_1x128x4, AS_PROGRAM, 8, sm510_base_device)
	AM_RANGE(0x000, 0x1ff) AM_ROM
ADDRESS_MAP_END

/*static ADDRESS_MAP_START(program_2x128x4, AS_PROGRAM, 8, sm510_base_device)
    AM_RANGE(0x000, 0x3ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(program_1x128x4_1x128x2, AS_PROGRAM, 8, sm510_base_device)
    AM_RANGE(0x000, 0x2ff) AM_ROM
ADDRESS_MAP_END*/

static ADDRESS_MAP_START(data_16x2x4, AS_DATA, 8, sm510_base_device)
	AM_RANGE(0x00, 0x0f) AM_RAM
	AM_RANGE(0x10, 0x1f) AM_RAM
ADDRESS_MAP_END

/*
static ADDRESS_MAP_START(data_16x3.5x4, AS_DATA, 8, sm510_base_device)
    AM_RANGE(0x00, 0x0f) AM_RAM
    AM_RANGE(0x10, 0x1f) AM_RAM
    AM_RANGE(0x20, 0x2f) AM_RAM
    AM_RANGE(0x30, 0x37) AM_RAM
ADDRESS_MAP_END
*/

// device definitions
sm590_device::sm590_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: sm590_device(mconfig, SM590, tag, owner, clock, 4 /* stack levels */, 9 /* prg width */, ADDRESS_MAP_NAME(program_1x128x4), 5 /* data width */, ADDRESS_MAP_NAME(data_16x2x4))
{
}

//sm591_device::sm591_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
//  : sm510_base_device(mconfig, SM591, tag, owner, clock, 4 /* stack levels */, 10 /* prg width */, ADDRESS_MAP_NAME(program_2x128x4), 6 /* data width */, ADDRESS_MAP_NAME(data_16x3.5x4))
//{
//}

//sm595_device::sm595_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
//  : sm510_base_device(mconfig, SM595, tag, owner, clock, 4 /* stack levels */, 10 /* prg width */, ADDRESS_MAP_NAME(program_1x128x4_1x128x2), 5 /* data width */, ADDRESS_MAP_NAME(data_16x2x4))
//{
//}

sm590_device::sm590_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int stack_levels, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data)
	: sm510_base_device(mconfig, type, tag, owner, clock, stack_levels, prgwidth, program, datawidth, data)
{
}


// disasm
offs_t sm590_device::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params, u32 options)
{
	extern CPU_DISASSEMBLE(sm590);
	return CPU_DISASSEMBLE_NAME(sm590)(this, stream, pc, opcodes, params, options);
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

	m_rports[0] = m_rports[1] = m_rports[2] = m_rports[3] = 0;
	//m_write_r(0, 0, 0xff); // TODO: are the four ports zeroed on reset?
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

offs_t sm590_device::disasm_pc_linear_to_real(offs_t pc) const
{
	static const u8 l2r[128] = {
		0x00, 0x40, 0x60, 0x70, 0x78, 0x7c, 0x7e, 0x3f, 0x5f, 0x6f, 0x77, 0x7b, 0x7d, 0x3e, 0x1f, 0x4f,
		0x67, 0x73, 0x79, 0x3c, 0x5e, 0x2f, 0x57, 0x6b, 0x75, 0x3a, 0x1d, 0x0e, 0x07, 0x43, 0x61, 0x30,
		0x58, 0x6c, 0x76, 0x3b, 0x5d, 0x2e, 0x17, 0x4b, 0x65, 0x32, 0x19, 0x0c, 0x46, 0x23, 0x51, 0x28,
		0x54, 0x6a, 0x35, 0x1a, 0x0d, 0x06, 0x03, 0x41, 0x20, 0x50, 0x68, 0x74, 0x7a, 0x3d, 0x1e, 0x0f,
		0x47, 0x63, 0x71, 0x38, 0x5c, 0x6e, 0x37, 0x5b, 0x6d, 0x36, 0x1b, 0x4d, 0x26, 0x13, 0x49, 0x24,
		0x52, 0x29, 0x14, 0x4a, 0x25, 0x12, 0x09, 0x04, 0x42, 0x21, 0x10, 0x48, 0x64, 0x72, 0x39, 0x1c,
		0x4e, 0x27, 0x53, 0x69, 0x34, 0x5a, 0x2d, 0x16, 0x0b, 0x45, 0x22, 0x11, 0x08, 0x44, 0x62, 0x31,
		0x18, 0x4c, 0x66, 0x33, 0x59, 0x2c, 0x56, 0x2b, 0x55, 0x2a, 0x15, 0x0a, 0x05, 0x02, 0x01, 0x7f,
	};
	return (pc & ~0x7f) | l2r[pc & 0x7f];
}

offs_t sm590_device::disasm_pc_real_to_linear(offs_t pc) const
{
	static const u8 r2l[128] = {
		0x00, 0x7e, 0x7d, 0x36, 0x57, 0x7c, 0x35, 0x1c, 0x6c, 0x56, 0x7b, 0x68, 0x2b, 0x34, 0x1b, 0x3f,
		0x5a, 0x6b, 0x55, 0x4d, 0x52, 0x7a, 0x67, 0x26, 0x70, 0x2a, 0x33, 0x4a, 0x5f, 0x1a, 0x3e, 0x0e,
		0x38, 0x59, 0x6a, 0x2d, 0x4f, 0x54, 0x4c, 0x61, 0x2f, 0x51, 0x79, 0x77, 0x75, 0x66, 0x25, 0x15,
		0x1f, 0x6f, 0x29, 0x73, 0x64, 0x32, 0x49, 0x46, 0x43, 0x5e, 0x19, 0x23, 0x13, 0x3d, 0x0d, 0x07,
		0x01, 0x37, 0x58, 0x1d, 0x6d, 0x69, 0x2c, 0x40, 0x5b, 0x4e, 0x53, 0x27, 0x71, 0x4b, 0x60, 0x0f,
		0x39, 0x2e, 0x50, 0x62, 0x30, 0x78, 0x76, 0x16, 0x20, 0x74, 0x65, 0x47, 0x44, 0x24, 0x14, 0x08,
		0x02, 0x1e, 0x6e, 0x41, 0x5c, 0x28, 0x72, 0x10, 0x3a, 0x63, 0x31, 0x17, 0x21, 0x48, 0x45, 0x09,
		0x03, 0x42, 0x5d, 0x11, 0x3b, 0x18, 0x22, 0x0a, 0x04, 0x12, 0x3c, 0x0b, 0x05, 0x0c, 0x06, 0x7f,
	};
	return (pc & ~0x7f) | r2l[pc & 0x7f];
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
