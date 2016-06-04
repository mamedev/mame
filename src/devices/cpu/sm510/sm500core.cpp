// license:BSD-3-Clause
// copyright-holders:hap
/*

  Sharp SM500 MCU core implementation

*/

#include "sm500.h"
#include "debugger.h"


// MCU types
const device_type SM500 = &device_creator<sm500_device>;


// internal memory maps
static ADDRESS_MAP_START(program_1_2k, AS_PROGRAM, 8, sm510_base_device)
	AM_RANGE(0x000, 0x4bf) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(data_4x10x4, AS_DATA, 8, sm510_base_device)
	AM_RANGE(0x00, 0x09) AM_RAM
	AM_RANGE(0x10, 0x19) AM_RAM
	AM_RANGE(0x20, 0x29) AM_RAM
	AM_RANGE(0x30, 0x39) AM_RAM
ADDRESS_MAP_END


// device definitions
sm500_device::sm500_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sm510_base_device(mconfig, SM500, "SM500", tag, owner, clock, 1 /* stack levels */, 11 /* prg width */, ADDRESS_MAP_NAME(program_1_2k), 6 /* data width */, ADDRESS_MAP_NAME(data_4x10x4), "sm500", __FILE__)
{ }

sm500_device::sm500_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, int stack_levels, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source)
	: sm510_base_device(mconfig, type, name, tag, owner, clock, stack_levels, prgwidth, program, datawidth, data, shortname, source)
{ }


// disasm
offs_t sm500_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE(sm500);
	return CPU_DISASSEMBLE_NAME(sm500)(this, buffer, pc, oprom, opram, options);
}



//-------------------------------------------------
//  execute
//-------------------------------------------------

void sm500_device::get_opcode_param()
{
	// LBL and prefix opcodes are 2 bytes
	if (m_op == 0x5e || m_op == 0x5f)
	{
		m_icount--;
		m_param = m_program->read_byte(m_pc);
		increment_pc();
	}
}

void sm500_device::execute_one()
{
	switch (m_op & 0xf0)
	{
		case 0x20: op_lax(); break;
		case 0x30:
			if (m_op == 0x30) op_ats(); // !
			else op_adx();
			break;
		case 0x40: op_lb(); break;
		case 0x70: op_ssr(); break;

		case 0x80: case 0x90: case 0xa0: case 0xb0:
			op_t(); break; // aka tr
		case 0xc0: case 0xd0: case 0xe0: case 0xf0:
			op_trs(); break;

		default:
			switch (m_op & 0xfc)
			{
		case 0x04: op_rm(); break;
		case 0x0c: op_sm(); break;
		case 0x10: op_exc(); break;
		case 0x14: op_exci(); break;
		case 0x18: op_lda(); break;
		case 0x1c: op_excd(); break;
		case 0x54: op_tmi(); break; // aka tm

		default:
			switch (m_op)
			{
		case 0x00: op_skip(); break;
		case 0x01: op_atr(); break;
		case 0x02: op_exksa(); break;
		case 0x03: op_atbp(); break;
		case 0x08: op_add(); break;
		case 0x09: op_add11(); break; // aka addc
		case 0x0a: op_coma(); break;
		case 0x0b: op_exbla(); break;

		case 0x50: op_tal(); break; // aka ta: test alpha
		case 0x51: op_tb(); break;
		case 0x52: op_tc(); break;
		case 0x53: op_tam(); break;
		case 0x58: op_tis(); break; // aka tg: test gamma
		case 0x59: op_ptw(); break;
		case 0x5a: op_ta0(); break;
		case 0x5b: op_tabl(); break;
		case 0x5c: op_tw(); break;
		case 0x5d: op_dtw(); break;
		case 0x5f: op_lbl(); break;

		case 0x60: op_comcn(); break;
		case 0x61: op_pdtw(); break;
		case 0x62: op_wr(); break;
		case 0x63: op_ws(); break;
		case 0x64: op_incb(); break;
		case 0x65: op_idiv(); break;
		case 0x66: op_rc(); break;
		case 0x67: op_sc(); break;
		case 0x68: op_rmf(); break;
		case 0x69: op_smf(); break;
		case 0x6a: op_kta(); break;
		case 0x6b: op_exkfa(); break;
		case 0x6c: op_decb(); break;
		case 0x6d: op_comcb(); break;
		case 0x6e: op_rtn0(); break; // aka rtn
		case 0x6f: op_rtn1(); break; // aka rtns

		// extended opcodes
		case 0x5e:
			m_op = m_op << 8 | m_param;
			switch (m_param)
			{
		case 0x00: op_cend(); break;
		case 0x04: op_dta(); break;

		default: op_illegal(); break;
			}
			break; // 0x5e

		default: op_illegal(); break;
			}
			break; // 0xff

			}
			break; // 0xfc

	} // big switch
}
