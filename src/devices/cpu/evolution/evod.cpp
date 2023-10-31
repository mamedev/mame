// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// EVOLUTION disassembler

#include "emu.h"
#include "evod.h"

u32 evolution_disassembler::opcode_alignment() const
{
	return 1;
}

// 3ea2 is table 4x2

const char *const evolution_disassembler::regs[4] = { "a", "b", "c", "d" };

const evolution_disassembler::instruction evolution_disassembler::instructions[] = {
	{ 0x9100, 0xff00, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "bne %06x", (pc + 1 + s8(opcode)) & 0x1fffff); }},
	{ 0x9000, 0xf000, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "bxx %x, %06x", (opcode >> 8) & 0xf, (pc + 1 + s8(opcode)) & 0x1fffff); }},
	{ 0xa000, 0xf900, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%02x = %s", opcode & 0xff, regs[(opcode >> 9) & 3]); }},
	{ 0xb000, 0xf900, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = %02x", regs[(opcode >> 9) & 3], opcode & 0xff); }},
	{ 0xc000, 0xf900, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s.h = #%02x", regs[(opcode >> 9) & 3], opcode & 0xff); }},
	{ 0xc100, 0xf900, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = %s & #%02x", regs[(opcode >> 9) & 3], regs[(opcode >> 9) & 3], opcode & 0xff); }},
	{ 0xd800, 0xf900, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = #%02x", regs[(opcode >> 9) & 3], opcode & 0xff); }},
	{ 0xd900, 0xf900, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = %s ^ #%02x", regs[(opcode >> 9) & 3], regs[(opcode >> 9) & 3], opcode & 0xff); }},
	{ 0xe000, 0xf900, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%02x <> %s", opcode & 0xff, regs[(opcode >> 9) & 3]); }},
	{ 0xe804, 0xf93f, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = %s - 1", regs[(opcode >> 9) & 3], regs[(opcode >> 6) & 3]); }},
	{ 0xfc8d, 0xffff, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "push b"); }},
	{ 0xfccd, 0xffff, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "pull b"); }},
	{ 0xfd40, 0xffe0, 2, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "jsr %06x", ((opcode & 0x1f) << 16) | arg); }},
	{ 0xfe40, 0xffe0, 2, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "jmp %06x", ((opcode & 0x1f) << 16) | arg); }},
	{ 0xff41, 0xffff, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "rti"); }},
	{ 0xff42, 0xffff, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "rts"); }},
	{ 0xffff, 0xffff, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "nop"); }},

	{ 0x0000, 0x0000, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "? %04x (%s %s)", opcode, regs[(opcode >> 9) & 3], regs[(opcode >> 6) & 3]); }}
};

offs_t evolution_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u16 opcode = opcodes.r16(pc);

	int i = 0;
	while((opcode & instructions[i].mask) != instructions[i].value)
		i++;

	instructions[i].fct(stream, opcode, instructions[i].size >= 2 ? opcodes.r16(pc+1) : 0, pc);

	return instructions[i].size;
}
