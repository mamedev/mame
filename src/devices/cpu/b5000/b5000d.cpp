// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell B5000 family MCU disassembler

*/

#include "emu.h"
#include "b5000d.h"

// constructor

b5000_common_disassembler::b5000_common_disassembler()
{
	// init lfsr pc lut
	for (u32 i = 0, pc = 0; i < 0x40; i++)
	{
		m_l2r[i] = pc;
		m_r2l[pc] = i;
		pc = increment_pc(pc);
	}
}

offs_t b5000_common_disassembler::increment_pc(offs_t pc)
{
	int feed = ((pc & 0x3e) == 0) ? 1 : 0;
	feed ^= (pc >> 1 ^ pc) & 1;
	return (pc & ~0x3f) | (pc >> 1 & 0x1f) | (feed << 5);
}


// common lookup tables

const char *const b5000_common_disassembler::s_name[] =
{
	"?",
	"NOP", "RSC", "SC", "TC", "TAM",
	"LAX", "ADX", "COMP", "ATB", "ATBZ",
	"LDA", "EXC", "EXC", "EXC", "ADD",
	"LB", "LB", "LB", "LB", "LB", "LB",
	"RSM", "SM", "TM",
	"TL", "TRA", "TRA", "RET",
	"TKB", "TKBS", "TDIN", "READ", "KSEG", "MTD"
};

// number of bits per opcode parameter
// note: d4 means bitmask param, d5 means inverted
const u8 b5000_common_disassembler::s_bits[] =
{
	0,
	0, 0, 0, 0, 0,
	0x24, 0x24, 0, 0, 0,
	2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2,
	0x12, 0x12, 0x12,
	4, 6, 6, 0,
	0, 0, 2, 0, 0, 0
};

const u32 b5000_common_disassembler::s_flags[] =
{
	0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0,
	0, STEP_OVER, 0, STEP_OUT,
	0, 0, 0, 0, 0, 0
};


// common disasm

offs_t b5000_common_disassembler::common_disasm(const u8 *lut_opmap, std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	// get raw opcode
	u8 op = opcodes.r8(pc);
	u8 instr = lut_opmap[op];

	// get parameter
	u8 bits = s_bits[instr];
	u8 mask = (1 << (bits & 0xf)) - 1;
	u8 param = (bits & 0x20) ? (~op & mask) : (op & mask);
	if (bits & 0x10)
		param = 1 << param;

	// TDIN 0 is 4
	if (instr == em_TDIN && param == 0)
		param = 4;

	// disassemble it
	util::stream_format(stream, "%-6s", s_name[instr]);

	if (bits > 0)
	{
		// exceptions for opcodes with 2 params
		if (instr >= em_EXC0 && instr <= em_EXCM)
		{
			switch (instr)
			{
				case em_EXC0: util::stream_format(stream, "%d,0", param); break;
				case em_EXCP: util::stream_format(stream, "%d,+1", param); break;
				case em_EXCM: util::stream_format(stream, "%d,-1", param); break;
				default: break;
			}
		}
		else if (instr == em_ADD)
		{
			switch (param ^ 2)
			{
				case 1: stream << "S"; break; // 0,1
				case 2: stream << "C"; break; // 1,0
				case 3: stream << "C,S"; break; // 1,1
				default: break;
			}
		}
		else if (instr >= em_LB0 && instr <= em_LB11)
		{
			int param2 = (instr == em_LB0) ? 0 : (6 + instr - em_LB0);
			util::stream_format(stream, "%d,%d", param, param2);
		}
		else if (instr == em_TRA0 || instr == em_TRA1)
		{
			int param2 = (instr == em_TRA1) ? 1 : 0;
			util::stream_format(stream, "%d,$%02X", param2, param);
		}
		else
			util::stream_format(stream, "%d", param);
	}

	return 1 | s_flags[instr] | SUPPORTED;
}


// B5000/B6000 disasm (for A5xxx, the only difference is ATBZ = MTD)

const u8 b5000_disassembler::b5000_opmap[0x100] =
{
/*  0        1        2        3        4        5        6        7        8        9        A        B        C        D        E        F  */
	em_NOP,  em_TC,   em_TKB,  em_TKBS, em_TDIN, em_TDIN, em_TDIN, em_TDIN, em_TM,   em_TM,   em_TM,   em_TM,   0,       0,       0,       0,       // 0
	em_SM,   em_SM,   em_SM,   em_SM,   em_RSM,  em_RSM,  em_RSM,  em_RSM,  em_RET,  em_RET,  em_RET,  em_RET,  0,       0,       0,       0,       // 1
	em_LB7,  em_LB7,  em_LB7,  em_LB7,  em_LB10, em_LB10, em_LB10, em_LB10, em_LB9,  em_LB9,  em_LB9,  em_LB9,  em_LB8,  em_LB8,  em_LB8,  em_LB8,  // 2
	em_TL,   em_TL,   em_TL,   em_TL,   em_TL,   em_TL,   em_TL,   em_TL,   0,       em_RSC,  0,       em_SC,   em_LB0,  em_LB0,  em_LB0,  em_LB0,  // 3

	em_LAX,  em_LAX,  em_LAX,  em_LAX,  em_LAX,  em_LAX,  em_LAX,  em_LAX,  em_LAX,  em_LAX,  em_LAX,  em_LAX,  em_LAX,  em_LAX,  em_LAX,  em_LAX,  // 4
	em_LDA,  em_LDA,  em_LDA,  em_LDA,  em_EXCP, em_EXCP, em_EXCP, em_EXCP, em_EXC0, em_EXC0, em_EXC0, em_EXC0, em_EXCM, em_EXCM, em_EXCM, em_EXCM, // 5
	em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_READ, // 6
	em_ADD,  em_ADD,  em_ADD,  em_ADD,  em_KSEG, 0,       em_ATBZ, em_ATB,  em_COMP, em_COMP, em_COMP, em_COMP, em_TAM,  em_TAM,  em_TAM,  em_TAM,  // 7

	em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, // 8
	em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, // 9
	em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, // A
	em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, // B

	em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, // C
	em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, // D
	em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, // E
	em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, // F
};

offs_t b5000_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	return common_disasm(b5000_opmap, stream, pc, opcodes, params);
}


// B5500/B6100 disasm (for A5xxx, the only difference is ATBZ = MTD)

const u8 b5500_disassembler::b5500_opmap[0x100] =
{
/*  0        1        2        3        4        5        6        7        8        9        A        B        C        D        E        F  */
	em_NOP,  em_TC,   em_TKB,  em_TKBS, em_TDIN, em_TDIN, em_TDIN, em_TDIN, em_TM,   em_TM,   em_TM,   em_TM,   em_SC,   em_RSC,  0,       0,       // 0
	em_SM,   em_SM,   em_SM,   em_SM,   em_RSM,  em_RSM,  em_RSM,  em_RSM,  em_RET,  em_RET,  em_RET,  em_RET,  em_LB11, em_LB11, em_LB11, em_LB11, // 1
	em_LB7,  em_LB7,  em_LB7,  em_LB7,  em_LB10, em_LB10, em_LB10, em_LB10, em_LB9,  em_LB9,  em_LB9,  em_LB9,  em_LB8,  em_LB8,  em_LB8,  em_LB8,  // 2
	em_TL,   em_TL,   em_TL,   em_TL,   em_TL,   em_TL,   em_TL,   em_TL,   em_TL,   em_TL,   em_TL,   em_TL,   em_LB0,  em_LB0,  em_LB0,  em_LB0,  // 3

	em_LAX,  em_LAX,  em_LAX,  em_LAX,  em_LAX,  em_LAX,  em_LAX,  em_LAX,  em_LAX,  em_LAX,  em_LAX,  em_LAX,  em_LAX,  em_LAX,  em_LAX,  em_LAX,  // 4
	em_LDA,  em_LDA,  em_LDA,  em_LDA,  em_EXCP, em_EXCP, em_EXCP, em_EXCP, em_EXC0, em_EXC0, em_EXC0, em_EXC0, em_EXCM, em_EXCM, em_EXCM, em_EXCM, // 5
	em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_ADX,  em_READ, // 6
	em_ADD,  em_ADD,  em_ADD,  em_ADD,  em_KSEG, 0,       em_ATBZ, em_ATB,  em_COMP, em_COMP, em_COMP, em_COMP, em_TAM,  em_TAM,  em_TAM,  em_TAM,  // 7

	em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, // 8
	em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, // 9
	em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, // A
	em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, em_TRA0, // B

	em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, // C
	em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, // D
	em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, // E
	em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, em_TRA1, // F
};

offs_t b5500_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	return common_disasm(b5500_opmap, stream, pc, opcodes, params);
}
