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

// 36b0 = Penguin Rescue
// 3593 = table of pairs (adr, 0040)

// 403e1f: indexes the table at 403ea2 somehow

/*
  start:
400051: fc80       ? fc80 (c c a)
400052: f842       push2 c
400053: f862       push2 d
400054: f802       push2 a
400055: f882       ? f882 (a c a)
400056: f8c2       ? f8c2 (a d c)
400057: f8e2       ? f8e2 (a d d)
400058: fc82       ? fc82 (c c a)
400059: fc9c       ? fc9c (c c a)
40005a: fc83       ? fc83 (c c a)
40005b: fc9d       ? fc9d (c c a)

  end:
4000a5: fcdd       ? fcdd (c d c)
4000a6: fcc3       ? fcc3 (c d c)
4000a7: fcdc       ? fcdc (c d c)
4000a8: fcc2       ? fcc2 (c d c)
4000a9: f8e3       ? f8e3 (a d d)
4000aa: f8c3       ? f8c3 (a d c)
4000ab: f883       ? f883 (a c a)
4000ac: f803       pull2 a
4000ad: f863       pull2 d
4000ae: f843       pull2 c
4000af: fcc0       ? fcc0 (c d c)

40019e: 8d..?  3e.3d = 5b8119, 40.3f = 400b5b, jst 403a4f

40309b: simple access?
-> 4015c6

*/

// 4005e9: 3e.3d = 5b1981 = address of a table of addresses

const char *const evolution_disassembler::regs[] = { "a", "b", "c", "d", "e", "f", "g", "h" };

const evolution_disassembler::instruction evolution_disassembler::instructions[] = {
	{ 0x8000, 0xf000, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "bra %06x", (pc + 1 + util::sext(opcode, 12)) & 0xffffff); }},
	{ 0x9000, 0xff00, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "beq %06x", (pc + 1 + util::sext(opcode, 8)) & 0xffffff); }},
	{ 0x9100, 0xff00, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "bne %06x", (pc + 1 + util::sext(opcode, 8)) & 0xffffff); }},
	{ 0x9000, 0xf000, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "bxx %x, %06x", (opcode >> 8) & 0xf, (pc + 1 + s8(opcode)) & 0xffffff); }},
	{ 0xa000, 0xf900, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%02x = %s", opcode & 0xff, regs[(opcode >> 9) & 3]); }},
	{ 0xa100, 0xf900, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "(%02x) = %s", opcode & 0xff, regs[(opcode >> 9) & 3]); }},
	{ 0xb000, 0xf900, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = %02x", regs[(opcode >> 9) & 3], opcode & 0xff); }},
	{ 0xb100, 0xf900, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = (%02x)", regs[(opcode >> 9) & 3], opcode & 0xff); }},
	{ 0xc000, 0xf900, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = %s + #%04x", regs[(opcode >> 9) & 3], regs[(opcode >> 9) & 3], (opcode & 0xff) << 8); }},
	{ 0xc100, 0xf900, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = %s - #%04x", regs[(opcode >> 9) & 3], regs[(opcode >> 9) & 3], (opcode & 0xff) << 8); }},
	{ 0xd000, 0xf900, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = %s + #%02x", regs[(opcode >> 9) & 3], regs[(opcode >> 9) & 3], opcode & 0xff); }},
	{ 0xd800, 0xf900, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = #%02x", regs[(opcode >> 9) & 3], opcode & 0xff); }},
	{ 0xd900, 0xf900, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = %s - #%02x", regs[(opcode >> 9) & 3], regs[(opcode >> 9) & 3], opcode & 0xff); }},
	{ 0xe000, 0xf900, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%02x <> %s", opcode & 0xff, regs[(opcode >> 9) & 3]); }},
	{ 0xe100, 0xf900, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%02x ?? %s", opcode & 0xff, regs[(opcode >> 9) & 3]); }},
	{ 0xe800, 0xf93c, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = %s + #%x", regs[(opcode >> 9) & 3], regs[(opcode >> 6) & 3], 1 << (opcode & 3)); }},
	{ 0xe804, 0xf93c, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = %s - #%x", regs[(opcode >> 9) & 3], regs[(opcode >> 6) & 3], 1 << (opcode & 3)); }},
	{ 0xf802, 0xff3f, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "push2 %s", regs[(opcode >> 6) & 3]); }},
	{ 0xf803, 0xff3f, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "pull2 %s", regs[(opcode >> 6) & 3]); }},
	{ 0xfc8d, 0xffff, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "push b"); }},
	{ 0xfccd, 0xffff, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "pull b"); }},
	{ 0xfd00, 0xff00, 2, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "jsr %06x", ((opcode & 0xff) << 16) | arg); }},
	{ 0xfe00, 0xff00, 2, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "jmp %06x", ((opcode & 0xff) << 16) | arg); }},
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
