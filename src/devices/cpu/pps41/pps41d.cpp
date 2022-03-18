// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell PPS-4/1 disassembler

*/

#include "emu.h"
#include "pps41d.h"

// constructor

pps41_common_disassembler::pps41_common_disassembler()
{
	// init lfsr pc lut
	for (u32 i = 0, pc = 0; i < 0x40; i++)
	{
		m_l2r[i] = pc;
		m_r2l[pc] = i;
		pc = increment_pc(pc);
	}
}

offs_t pps41_common_disassembler::increment_pc(offs_t pc)
{
	int feed = ((pc & 0x3e) == 0) ? 1 : 0;
	feed ^= (pc >> 1 ^ pc) & 1;
	return (pc & ~0x3f) | (pc >> 1 & 0x1f) | (feed << 5);
}


// common lookup tables

const char *const pps41_common_disassembler::s_name[] =
{
	// MM76
	"?",
	"XAB", "LBA", "LB", "EOB",
	"SB", "RB", "SKBF",
	"XAS", "LSA",
	"L", "X", "XDSK", "XNSK",
	"A", "AC", "ACSK", "ASK", "COM", "RC", "SC", "SKNC", "LAI", "AISK",
	"RT", "RTSK", "T", "NOP", "TL", "TM", "TML", "TR",
	"SKMEA", "SKBEI", "SKAEI",
	"SOS", "ROS", "SKISL", "IBM", "OB", "IAM", "OA", "IOS", "I1", "I2C", "INT1H", "DIN1", "INT0L", "DIN0", "SEG1", "SEG2",

	// MM78
	"INT0H", "INT1L", "SAG", "EOB", "TAB",
	"I1SK", "IX", "OX", "LXA", "XAX", "IOA",
	"TLB", "TMLB"
};

// number of bits per opcode parameter
// note: d4 means hex, d5 means inverted, d6 means extended, d7 means double extended
const u8 pps41_common_disassembler::s_bits[] =
{
	// MM76
	0,
	0, 0, 4, 2,
	2, 2, 2,
	0, 0,
	2, 2, 2, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 4, 4,
	0, 0, 0x36, 0, 0x76, 0x36, 0x76, 0x34,
	0, 0x40, 0x60,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	// MM78
	0, 0, 0, 3, 0,
	0, 0, 0, 0, 0, 0,
	0xf6, 0xf6
};

const u32 pps41_common_disassembler::s_flags[] =
{
	// MM76
	0,
	0, 0, 0, 0,
	0, 0, 0,
	0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	STEP_OUT, STEP_OUT, 0, 0, 0, STEP_OVER, STEP_OVER, 0,
	0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	// MM78
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, STEP_OVER
};


// common disasm

offs_t pps41_common_disassembler::common_disasm(const u8 *lut_opmap, std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	// get raw opcode
	u8 op = opcodes.r8(pc);
	u8 instr = lut_opmap[op];

	// get parameter
	u8 bits = s_bits[instr];
	u16 mask = (1 << (bits & 0xf)) - 1;
	bits &= 0xf0;

	u16 param = op & mask;
	if (bits & 0x20)
		param ^= mask;

	// memory bit opcodes are 1,2,3,4
	if (instr == em_SB || instr == em_RB || instr == em_SKBF)
		param++;

	// disassemble it
	util::stream_format(stream, "%-8s", s_name[instr]);
	if (mask > 0)
	{
		if (bits & 0x10)
			util::stream_format(stream, (mask & ~0xf) ? "$%02X" : "$%X", param);
		else
			util::stream_format(stream, "%d", param);
	}

	return 1 | s_flags[instr] | SUPPORTED;
}


// MM76 disasm

const u8 mm76_disassembler::mm76_opmap[0x100] =
{
/*  0        1        2        3        4        5        6        7        8        9        A        B        C        D        E        F  */
	em_NOP,  em_SKNC, em_RT,   em_RTSK, em_INT0L,em_INT1H,em_DIN1, em_DIN0, em_SKBF, em_SKBF, em_SKBF, em_SKBF, em_SC,   em_RC,   em_SEG1, em_SEG2, // 0
	em_SB,   em_SB,   em_SB,   em_SB,   em_RB,   em_RB,   em_RB,   em_RB,   em_OA,   em_OB,   em_IAM,  em_IBM,  em_EOB2, em_EOB2, em_EOB2, em_EOB2, // 1
	em_LB,   em_LB,   em_LB,   em_LB,   em_LB,   em_LB,   em_LB,   em_LB,   em_LB,   em_LB,   em_LB,   em_LB,   em_LB,   em_LB,   em_LB,   em_LB,   // 2
	em_TR,   em_TR,   em_TR,   em_TR,   em_TR,   em_TR,   em_TR,   em_TR,   em_TR,   em_TR,   em_TR,   em_TR,   em_TR,   em_TR,   em_TR,   em_TR,   // 3

	em_AC,   em_ACSK, em_A,    em_ASK,  em_LBA,  em_COM,  em_XAB,  em_SKMEA,em_ILL,  em_ILL,  em_I1,   em_I2C,  em_LSA,  em_IOS,  em_XAS,  em_ILL,  // 4
	em_L,    em_L,    em_L,    em_L,    em_XNSK, em_XNSK, em_XNSK, em_XNSK, em_X,    em_X,    em_X,    em_X,    em_XDSK, em_XDSK, em_XDSK, em_XDSK, // 5
	em_AISK, em_AISK, em_AISK, em_AISK, em_AISK, em_AISK, em_AISK, em_AISK, em_AISK, em_AISK, em_AISK, em_AISK, em_AISK, em_AISK, em_AISK, em_AISK, // 6
	em_LAI,  em_LAI,  em_LAI,  em_LAI,  em_LAI,  em_LAI,  em_LAI,  em_LAI,  em_LAI,  em_LAI,  em_LAI,  em_LAI,  em_LAI,  em_LAI,  em_LAI,  em_LAI,  // 7

	em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   // 8
	em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   // 9
	em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   // A
	em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   // B

	em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    // C
	em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    // D
	em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    // E
	em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T     // F
};

offs_t mm76_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	return common_disasm(mm76_opmap, stream, pc, opcodes, params);
}


// MM78 disasm

const u8 mm78_disassembler::mm78_opmap[0x100] =
{
/*  0        1        2        3        4        5        6        7        8        9        A        B        C        D        E        F  */
	em_NOP,  em_SKISL,em_SKNC, em_INT0H,em_INT1L,em_RC,   em_SC,   em_SAG,  em_EOB3, em_EOB3, em_EOB3, em_EOB3, em_EOB3, em_EOB3, em_EOB3, em_EOB3, // 0
	em_LB,   em_LB,   em_LB,   em_LB,   em_LB,   em_LB,   em_LB,   em_LB,   em_LB,   em_LB,   em_LB,   em_LB,   em_LB,   em_LB,   em_LB,   em_LB,   // 1
	em_SB,   em_SB,   em_SB,   em_SB,   em_RB,   em_RB,   em_RB,   em_RB,   em_SKBF, em_SKBF, em_SKBF, em_SKBF, em_TAB,  em_IOS,  em_RTSK, em_RT,   // 2
	em_TR,   em_TR,   em_TR,   em_TR,   em_TR,   em_TR,   em_TR,   em_TR,   em_TR,   em_TR,   em_TR,   em_TR,   em_TR,   em_TR,   em_TR,   em_TR,   // 3

	em_LAI,  em_LAI,  em_LAI,  em_LAI,  em_LAI,  em_LAI,  em_LAI,  em_LAI,  em_LAI,  em_LAI,  em_LAI,  em_LAI,  em_LAI,  em_LAI,  em_LAI,  em_LAI,  // 4
	em_L,    em_L,    em_L,    em_L,    em_XNSK, em_XNSK, em_XNSK, em_XNSK, em_XDSK, em_XDSK, em_XDSK, em_XDSK, em_X,    em_X,    em_X,    em_X,    // 5
	em_I1SK, em_AISK, em_AISK, em_AISK, em_AISK, em_AISK, em_AISK, em_AISK, em_AISK, em_AISK, em_AISK, em_AISK, em_AISK, em_AISK, em_AISK, em_AISK, // 6
	em_SOS,  em_ROS,  em_IX,   em_OX,   em_XAS,  em_LXA,  em_LBA,  em_COM,  em_I2C,  em_XAX,  em_XAB,  em_IOA,  em_AC,   em_ACSK, em_A,    em_SKMEA,// 7

	em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   // 8
	em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   // 9
	em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   // A
	em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   em_TM,   // B

	em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    // C
	em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    // D
	em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    // E
	em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T,    em_T     // F
};

offs_t mm78_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	return common_disasm(mm78_opmap, stream, pc, opcodes, params);
}
