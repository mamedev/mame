// license:BSD-3-Clause
// copyright-holders:hap, Igor
/*

  Sharp SM5A MCU core implementation

*/

#include "emu.h"
#include "sm500.h"
#include "debugger.h"


// MCU types
DEFINE_DEVICE_TYPE(SM5A, sm5a_device, "sm5a", "SM5A")
DEFINE_DEVICE_TYPE(SM5L, sm5l_device, "sm5l", "SM5L")
DEFINE_DEVICE_TYPE(KB1013VK12, kb1013vk12_device, "kb1013vk1_2", "KB1013VK1-2")


// internal memory maps
static ADDRESS_MAP_START(program_1_8k, AS_PROGRAM, 8, sm510_base_device)
	AM_RANGE(0x000, 0x6ff) AM_ROM
	AM_RANGE(0x700, 0x73f) AM_MIRROR(0x0c0) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(data_5x13x4, AS_DATA, 8, sm510_base_device)
	AM_RANGE(0x00, 0x0c) AM_RAM
	AM_RANGE(0x10, 0x1c) AM_RAM
	AM_RANGE(0x20, 0x2c) AM_RAM
	AM_RANGE(0x30, 0x3c) AM_RAM
	AM_RANGE(0x40, 0x4c) AM_MIRROR(0x30) AM_RAM
ADDRESS_MAP_END


// device definitions
sm5a_device::sm5a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: sm5a_device(mconfig, SM5A, tag, owner, clock, 1 /* stack levels */, 8 /* o mask */, 11 /* prg width */, ADDRESS_MAP_NAME(program_1_8k), 7 /* data width */, ADDRESS_MAP_NAME(data_5x13x4))
{
}

sm5a_device::sm5a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int stack_levels, int o_mask, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data)
	: sm500_device(mconfig, type, tag, owner, clock, stack_levels, o_mask, prgwidth, program, datawidth, data)
{
}

sm5l_device::sm5l_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: sm5a_device(mconfig, SM5L, tag, owner, clock, 1, 8, 11, ADDRESS_MAP_NAME(program_1_8k), 7, ADDRESS_MAP_NAME(data_5x13x4))
{
}

kb1013vk12_device::kb1013vk12_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: sm5a_device(mconfig, KB1013VK12, tag, owner, clock, 1, 8, 11, ADDRESS_MAP_NAME(program_1_8k), 7, ADDRESS_MAP_NAME(data_5x13x4))
{
}


// disasm
offs_t sm5a_device::disasm_disassemble(std::ostream &stream, offs_t pc, const u8 *oprom, const u8 *opram, u32 options)
{
	extern CPU_DISASSEMBLE(sm5a);
	return CPU_DISASSEMBLE_NAME(sm5a)(this, stream, pc, oprom, opram, options);
}



//-------------------------------------------------
//  execute
//-------------------------------------------------

void sm5a_device::execute_one()
{
	switch (m_op & 0xf0)
	{
		case 0x20: op_lax(); break; // LC
		case 0x30: op_adx(); break; // AS/A10
		case 0x40: op_lb(); break; // LAS
		case 0x70: op_ssr(); break; // LP

		case 0x80: case 0x90: case 0xa0: case 0xb0:
			op_tr(); break; // BR (LP+this=JMP)
		case 0xc0: case 0xd0: case 0xe0: case 0xf0:
			op_trs(); break; // CBR/CZP (LP+this=CAL)

		default:
			switch (m_op & 0xfc)
			{
		case 0x04: op_rm(); break; // BM0
		case 0x0c: op_sm(); break; // BM1
		case 0x10: op_exc(); break; // XM/XE
		case 0x14: op_exci(); break; // XI/XEI
		case 0x18: op_lda(); break; // LE
		case 0x1c: op_excd(); break; // XD/XED
		case 0x54: op_tmi(); break; // SM1

		default:
			switch (m_op)
			{
		case 0x00: op_skip(); break; // NOP
		case 0x01: op_atr(); break; // OAR
		case 0x02: op_sbm(); break; // *BS1
		case 0x03: op_atbp(); break;
		case 0x08: op_add(); break; // AM
		case 0x09: op_add11(); break; // AC
		case 0x0a: op_coma(); break; // COM
		case 0x0b: op_exbla(); break; // XL

		case 0x50: op_tal(); break; // SI1
		case 0x51: op_tb(); break; // SI0
		case 0x52: op_tc(); break; // SCO
		case 0x53: op_tam(); break; // SAM
		case 0x58: op_tis(); break; // TIM
		case 0x59: op_ptw(); break;
		case 0x5a: op_ta0(); break; // SAO
		case 0x5b: op_tabl(); break; // SAL
		case 0x5c: op_tw(); break;
		case 0x5d: op_dtw(); break;
		case 0x5f: op_lbl(); break; // LAF

		case 0x60: op_comcn(); break;
		case 0x61: op_pdtw(); break;
		case 0x62: op_wr(); break;
		case 0x63: op_ws(); break;
		case 0x64: op_incb(); break; // INC
		case 0x65: op_idiv(); break; // SYN
		case 0x66: op_rc(); break; // CLC
		case 0x67: op_sc(); break; // STC
		case 0x68: op_rmf(); break; // CLL
		case 0x69: op_smf(); break; // LD0
		case 0x6a: op_kta(); break; // ICD
		case 0x6b: op_rbm(); break; // *BS0
		case 0x6c: op_decb(); break; // DEC
		case 0x6d: op_comcb(); break; // CMS
		case 0x6e: op_rtn0(); break; // RT
		case 0x6f: op_rtn1(); break; // RTS

		// extended opcodes
		case 0x5e:
			m_op = m_op << 8 | m_param;
			switch (m_param)
			{
		case 0x00: op_cend(); break; // HLT
		case 0x04: op_dta(); break; // LDF

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
