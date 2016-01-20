// license:BSD-3-Clause
// copyright-holders:hap
/*

  Sharp SM511 MCU core implementation

*/

#include "sm510.h"
#include "debugger.h"


// MCU types
const device_type SM511 = &device_creator<sm511_device>;
const device_type SM512 = &device_creator<sm512_device>;


// internal memory maps
static ADDRESS_MAP_START(program_4k, AS_PROGRAM, 8, sm510_base_device)
	AM_RANGE(0x0000, 0x0fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(data_96_32x4, AS_DATA, 8, sm510_base_device)
	AM_RANGE(0x00, 0x5f) AM_RAM
	AM_RANGE(0x60, 0x6f) AM_RAM AM_SHARE("lcd_ram_a")
	AM_RANGE(0x70, 0x7f) AM_RAM AM_SHARE("lcd_ram_b")
ADDRESS_MAP_END

static ADDRESS_MAP_START(data_80_48x4, AS_DATA, 8, sm510_base_device)
	AM_RANGE(0x00, 0x4f) AM_RAM
	AM_RANGE(0x50, 0x5f) AM_RAM AM_SHARE("lcd_ram_c")
	AM_RANGE(0x60, 0x6f) AM_RAM AM_SHARE("lcd_ram_a")
	AM_RANGE(0x70, 0x7f) AM_RAM AM_SHARE("lcd_ram_b")
ADDRESS_MAP_END


// disasm
offs_t sm511_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE(sm511);
	return CPU_DISASSEMBLE_NAME(sm511)(this, buffer, pc, oprom, opram, options);
}


// device definitions
sm511_device::sm511_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sm510_base_device(mconfig, SM511, "SM511", tag, owner, clock, 2 /* stack levels */, 12 /* prg width */, ADDRESS_MAP_NAME(program_4k), 7 /* data width */, ADDRESS_MAP_NAME(data_96_32x4), "sm511", __FILE__)
{ }

sm511_device::sm511_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, int stack_levels, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source)
	: sm510_base_device(mconfig, type, name, tag, owner, clock, stack_levels, prgwidth, program, datawidth, data, shortname, source)
{ }

sm512_device::sm512_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sm511_device(mconfig, SM512, "SM512", tag, owner, clock, 2, 12, ADDRESS_MAP_NAME(program_4k), 7, ADDRESS_MAP_NAME(data_80_48x4), "sm512", __FILE__)
{ }



//-------------------------------------------------
//  execute
//-------------------------------------------------

void sm511_device::get_opcode_param()
{
	// XXX?, LBL, PRE, TL, TML and prefix opcodes are 2 bytes
	if (m_op == 0x01 || (m_op >= 0x5f && m_op <= 0x61) || (m_op & 0xf0) == 0x70 || (m_op & 0xfc) == 0x68)
	{
		m_icount--;
		m_param = m_program->read_byte(m_pc);
		increment_pc();
	}
}

void sm511_device::execute_one()
{
	switch (m_op & 0xf0)
	{
		case 0x20: op_lax(); break;
		case 0x30: op_adx(); break;
		case 0x40: op_lb(); break;
		case 0x70: op_tl(); break;

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
		case 0x68: op_tml(); break;

		default:
			switch (m_op)
			{
		case 0x00: op_rot(); break;
//      case 0x01: op_xxx(); break; // ?
		case 0x02: op_sbm(); break;
		case 0x03: op_atpl(); break;
		case 0x08: op_add(); break;
		case 0x09: op_add11(); break;
		case 0x0a: op_coma(); break;
		case 0x0b: op_exbla(); break;

		case 0x50: op_kta(); break;
		case 0x51: op_tb(); break;
		case 0x52: op_tc(); break;
		case 0x53: op_tam(); break;
		case 0x58: op_tis(); break;
		case 0x59: op_atl(); break;
		case 0x5a: op_ta0(); break;
		case 0x5b: op_tabl(); break;
		case 0x5c: op_atx(); break;
//      case 0x5d: op_cend(); break;
		case 0x5e: op_tal(); break;
		case 0x5f: op_lbl(); break;

		case 0x61: op_pre(); break;
		case 0x62: op_wr(); break;
		case 0x63: op_ws(); break;
		case 0x64: op_incb(); break;
//      case 0x65: op_idiv(); break;
		case 0x66: op_rc(); break;
		case 0x67: op_sc(); break;
		case 0x6c: op_decb(); break;
		case 0x6d: op_ptw(); break;
		case 0x6e: op_rtn0(); break;
		case 0x6f: op_rtn1(); break;

		// extended opcodes
		case 0x60:
			m_op = m_op << 8 | m_param;
			switch (m_param)
			{
		case 0x30: op_rme(); break;
		case 0x31: op_sme(); break;
		case 0x32: op_tmel(); break;
		case 0x33: op_atfc(); break;
		case 0x34: op_bdc(); break;
		case 0x35: op_atbp(); break;

		default: op_illegal(); break;
			}
			break; // 0x60

		default: op_illegal(); break;
			}
			break; // 0xff

			}
			break; // 0xfc

	} // big switch
}
