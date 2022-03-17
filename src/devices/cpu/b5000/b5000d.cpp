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
	"?", "NOP"
};

// bitmask for opcode parameter
const u8 b5000_common_disassembler::s_bits[] =
{
	0, 0
};

const u32 b5000_common_disassembler::s_flags[] =
{
	0, 0
};


// common disasm

offs_t b5000_common_disassembler::common_disasm(const u8 *lut_opmap, std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	// get raw opcode
	u8 op = opcodes.r8(pc);
	u8 instr = lut_opmap[op];

	// get parameter
	u8 mask = s_bits[instr];

	// disassemble it
	util::stream_format(stream, "%-8s ", s_name[instr]);

	if (mask > 0)
	{
		;
	}

	return 1 | s_flags[instr] | SUPPORTED;
}


// B5000 disasm

const u8 b5000_disassembler::b5000_opmap[0x100] =
{
/*  0        1        2        3        4        5        6        7        8        9        A        B        C        D        E        F  */
	em_NOP,  0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       // 0
	0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       // 1
	0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       // 2
	0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       // 3

	0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       // 4
	0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       // 5
	0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       // 6
	0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       // 7

	0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       // 8
	0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       // 9
	0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       // A
	0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       // B

	0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       // C
	0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       // D
	0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       // E
	0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       // F
};

offs_t b5000_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	return common_disasm(b5000_opmap, stream, pc, opcodes, params);
}
