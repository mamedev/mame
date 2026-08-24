// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
#include "emu.h"
#include "specnext_copper_dasm.h"

u32 specnext_copper_disassembler::opcode_alignment() const
{
	return 2;
}

#define P std::ostream &stream, const data_buffer &opcodes, u16 opcode, u16 pc
const specnext_copper_disassembler::instruction specnext_copper_disassembler::instructions[] {
	// HALT: WAIT 63, 511
	{ 0xffff, 0xffff, [](P) -> u32 { util::stream_format(stream, "HALT"); return 2; } },

	// WAIT hcolumn, vline
	{ 0x8000, 0x8000, [](P) -> u32 { util::stream_format(stream, "WAIT %d, %d", (opcode >> 9) & 0x3f, opcode & 0x1ff); return 2; } },

	// NOOP (address field = 0, hardware ignores value byte)
	{ 0x0000, 0xffff, [](P) -> u32 { util::stream_format(stream, "NOOP"); return 2; } },

	// MOVE register, value
	{ 0x0000, 0x8000, [](P) -> u32 { util::stream_format(stream, "MOVE $%02x, $%02x", (opcode >> 8) & 0x7f, opcode & 0xff); return 2; } },

	// Fallback
	{ 0x0000, 0x0000, [](P) -> u32 { util::stream_format(stream, "?%04x", opcode); return 2; } },
};
#undef P

offs_t specnext_copper_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u16 opcode = opcodes.r16(pc);

	for(u32 i=0;; i++)
		if((opcode & instructions[i].mask) == instructions[i].value)
			return instructions[i].cb(stream, opcodes, opcode, pc) | SUPPORTED;
	return 0;
}
