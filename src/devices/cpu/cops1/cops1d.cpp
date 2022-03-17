// license:BSD-3-Clause
// copyright-holders:hap
/*

  National Semiconductor COPS(MM57 MCU family) disassembler

*/

#include "emu.h"
#include "cops1d.h"

// constructor

cops1_common_disassembler::cops1_common_disassembler()
{
	// init lfsr pc lut
	for (u32 i = 0, pc = 0; i < 0x40; i++)
	{
		m_l2r[i] = pc;
		m_r2l[pc] = i;
		pc = increment_pc(pc);
	}
}

offs_t cops1_common_disassembler::increment_pc(offs_t pc)
{
	int feed = ((pc & 0x3e) == 0) ? 1 : 0;
	feed ^= (pc >> 1 ^ pc) & 1;
	return (pc & ~0x3f) | (pc >> 1 & 0x1f) | (feed << 5);
}


// common lookup tables

const char *const cops1_common_disassembler::s_name[] =
{
	"?",
	"AD", "ADD", "SUB", "COMP", "0TA", "ADX", "HXA", "TAM", "SC", "RSC", "TC",
	"TIN", "TF", "TKB", "TIR",
	"BTD", "DSPA", "DSPS", "AXO", "LDF", "READ",
	"GO", "CALL", "RET", "RETS", "LG/GO", "LG/CALL", "NOP",
	"EXC", "EXCM", "EXCP", "MTA", "LM",
	"SM", "SM", "SM", "SM", "RSM", "RSM", "RSM", "RSM", "TM",
	"LB", "LBL", "ATB", "BTA", "HXBR"
};

// bitmask for opcode parameter
const u8 cops1_common_disassembler::s_bits[] =
{
	0,
	0, 0, 0, 0, 0, 0x0f, 0, 0, 0, 0, 0,
	0, 0x30, 0, 0,
	0, 0, 0, 0, 0xff, 0,
	0x3f, 0x3f, 0, 0, 0xff, 0xff, 0,
	0x30, 0x30, 0x30, 0x30, 0x0f,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x3f, 0xff, 0, 0, 0
};

const u32 cops1_common_disassembler::s_flags[] =
{
	0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, STEP_OVER, STEP_OUT, STEP_OUT, 0, STEP_OVER, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0
};


// common disasm

offs_t cops1_common_disassembler::common_disasm(const u8 *lut_opmap, std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	// get raw opcode
	u8 op = opcodes.r8(pc);
	u8 instr = lut_opmap[op];
	int len = 1;

	// get parameter
	u8 mask = s_bits[instr];
	u16 param = op & mask;

	if (mask == 0x30)
	{
		mask >>= 4;
		param >>= 4;
	}

	// 2-byte instructions
	else if (mask == 0xff)
	{
		pc = increment_pc(pc);
		u8 arg = params.r8(pc);
		len++;

		if (instr == em_LG)
		{
			// bit 6 indicates GO or CALL
			if (~arg & 0x40)
				instr++;

			param = (~param << 7 & 0x780) | (arg >> 1 & 0x40) | (arg & 0x3f);
		}
		else
			param = arg;
	}

	// disassemble it
	util::stream_format(stream, "%-8s", s_name[instr]);
	if (mask > 0)
	{
		if (mask < 16)
		{
			// SM and RSM param is scrambled
			if (instr >= em_SM1 && instr <= em_SM8)
				param = instr - em_SM1;
			else if (instr >= em_RSM1 && instr <= em_RSM8)
				param = instr - em_RSM1;

			// memory bit opcodes are 1,2,4,8
			if (instr >= em_SM1 && instr <= em_TM)
				param = 1 << param;

			// TF is 1,2,3,4
			else if (instr == em_TF)
				param++;

			// EXC type instructions omit param if it's 0
			if (!(param == 0 && (instr == em_EXC || instr == em_EXCM || instr == em_EXCP || instr == em_MTA)))
				util::stream_format(stream, "%d", param);
		}
		else
		{
			// exception for LG
			if (instr == em_LG || instr == em_LGCALL)
				util::stream_format(stream, "$%02X,$%02X", param >> 6, param & 0x3f);

			// exception for LB/LBL
			else if (instr == em_LB || instr == em_LBL)
			{
				// LB x,10 is 0 by default
				if (instr == em_LB && (param & 0xf) == 10)
					param &= 0x30;

				util::stream_format(stream, "%d,%d", param >> 4, param & 0xf);
			}

			else
				util::stream_format(stream, "$%02X", param);
		}
	}

	return len | s_flags[instr] | SUPPORTED;
}


// MM5799 disasm

const u8 mm5799_disassembler::mm5799_opmap[0x100] =
{
/*  0        1        2        3        4        5        6        7        8        9        A        B        C        D        E        F  */
	em_NOP,  em_HXBR, em_ADD,  em_SC,   em_TF,   em_TIR,  em_MTA,  em_EXC,  em_EXCM, em_EXCP, em_LB,   em_LB,   em_LB,   em_LB,   em_LB,   em_LB,   // 0
	em_DSPA, em_DSPS, em_AD,   em_LBL,  em_TF,   em_TKB,  em_MTA,  em_EXC,  em_EXCM, em_EXCP, em_LB,   em_LB,   em_LB,   em_LB,   em_LB,   em_LB,   // 1
	em_COMP, em_AXO,  em_SUB,  em_RSC,  em_TF,   em_BTD,  em_MTA,  em_EXC,  em_EXCM, em_EXCP, em_LB,   em_LB,   em_LB,   em_LB,   em_LB,   em_LB,   // 2
	em_0TA,  em_HXA,  em_TAM,  em_LDF,  em_READ, em_TIN,  em_MTA,  em_EXC,  em_EXCM, em_EXCP, em_LB,   em_LB,   em_LB,   em_LB,   em_LB,   em_LB,   // 3

	em_RET,  em_RETS, em_RSM8, em_BTA,  em_TM,   em_TM,   em_TM,   em_TM,   em_RSM1, em_SM1,  em_SM8,  em_RSM4, em_RSM2, em_TC,   em_SM2,  em_SM4,  // 4
	em_ATB,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  // 5
	em_LG,   em_LG,   em_LG,   em_LG,   em_LG,   em_LG,   em_LG,   em_LG,   em_LG,   em_LG,   em_LG,   em_LG,   em_LG,   em_LG,   em_LG,   em_LG,   // 6
	em_LM,   em_LM,   em_LM,   em_LM,   em_LM,   em_LM,   em_LM,   em_LM,   em_LM,   em_LM,   em_LM,   em_LM,   em_LM,   em_LM,   em_LM,   em_LM,   // 7

	em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, // 8
	em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, // 9
	em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, // A
	em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, em_CALL, // B

	em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   // C
	em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   // D
	em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   // E
	em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO,   em_GO    // F
};

offs_t mm5799_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	return common_disasm(mm5799_opmap, stream, pc, opcodes, params);
}
