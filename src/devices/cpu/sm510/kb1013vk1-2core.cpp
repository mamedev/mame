// license:BSD-3-Clause
// copyright-holders:hap, Igor
/*

  KB1013VK1-2 MCU core implementation

*/

#include "kb1013vk1-2.h"
#include "debugger.h"

// MCU types
const device_type KB1013VK12 = &device_creator<kb1013vk12_device>;


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
	AM_RANGE(0x40, 0x4c) AM_RAM
ADDRESS_MAP_END

// device definitions
kb1013vk12_device::kb1013vk12_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sm500_device(mconfig, KB1013VK12, "KB1013VK1-2", tag, owner, clock, 1 /* stack levels */, 11 /* prg width */, ADDRESS_MAP_NAME(program_1_8k), 7 /* data width */, ADDRESS_MAP_NAME(data_5x13x4), "kb1013vk1-2", __FILE__)
{ }


// disasm
offs_t kb1013vk12_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE(kb1013vk12);
	return CPU_DISASSEMBLE_NAME(kb1013vk12)(this, buffer, pc, oprom, opram, options);
}



//-------------------------------------------------
//  execute
//-------------------------------------------------

void kb1013vk12_device::execute_one()
{
	switch (m_op & 0xf0)
	{
		case 0x20: op_lax(); break; // LC
		case 0x30: op_adx(); break; // AS/A10
		case 0x40: op_lb(); break; // LAS
		case 0x70: op_ssr(); break; // LP

		case 0x80: case 0x90: case 0xa0: case 0xb0:
			op_t(); break; // BR (LP+this=JMP)
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
		case 0x02: op_bs1(); break; // *BS1
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
		case 0x6b: op_bs0(); break; // *BS0
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
