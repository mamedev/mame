// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, AJR

// Sonix 16-bit DSP (SNC7xx, SNL3xx, etc.) disassembler
// Thanks to https://github.com/marian-m12l/s9ke-toolchain for helping figure out the instruction decoding

#include "emu.h"
#include "sonix16d.h"

u32 sonix16_disassembler::opcode_alignment() const
{
	return 1;
}

const char *const sonix16_disassembler::regs[] = { "x0", "x1", "r0", "r1", "y0", "y1", "mr0", "mr1" };
const char *const sonix16_disassembler::immregs[] = { "x0", "x1", "r0", "r1", "y0", "y1", "ix0", "ix1" };
const char *const sonix16_disassembler::indregs[] = { "ix0", "ix1", "iy0", "iy1" };
const char *const sonix16_disassembler::yregs[] = { "y0", "y1", "r0", "r1" };
const char *const sonix16_disassembler::macregs[] = { "x0", "x1", "y0", "y1" };

const char *const sonix16_disassembler::conds[] = { "eq", "ne", "gt", "ge", "lt", "le", "av", "nav", "ac", "nac", "mr0s", "nmr0s", "mv", "nmv", "ixv", "irr" };
const char *const sonix16_disassembler::mods[] = { "", ", im", ", 1", ", -1" };

const sonix16_disassembler::instruction sonix16_disassembler::instructions[] = {
	{ 0x0000, 0x8000, 1 | STEP_OVER, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "call 0x%06x", ((pc + 1) & 0xff8000) + (opcode & 0x7fff)); }},
	{ 0x8000, 0xf000, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "jmp 0x%06x", ((pc + 1) + util::sext(opcode, 12)) & 0xffffff); }},
	{ 0x9000, 0xf000, 1 | STEP_COND, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "j%s 0x%06x", conds[(opcode >> 8) & 0xf], ((pc + 1) + util::sext(opcode, 8)) & 0xffffff); }},
	{ 0xa000, 0xf000, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "RAM(0x%03x) = %s", (opcode & 0x800) >> 3 | (opcode & 0xff), regs[(opcode >> 8) & 7]); }},
	{ 0xb000, 0xf000, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = RAM(0x%03x)", regs[(opcode >> 8) & 7], (opcode & 0x800) >> 3 | (opcode & 0xff)); }},
	{ 0xc000, 0xf800, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s.h = 0x%02x", immregs[(opcode >> 8) & 7], opcode & 0xff); }},
	{ 0xc800, 0xf81f, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "RAM(%s%s) = %s + 1", indregs[(opcode >> 8) & 1], mods[(opcode >> 9) & 3], regs[(opcode >> 5) & 7]); }},
	{ 0xc804, 0xf81f, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "RAM(%s%s) = %s - 1", indregs[(opcode >> 8) & 1], mods[(opcode >> 9) & 3], regs[(opcode >> 5) & 7]); }},
	{ 0xc808, 0xf89c, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "RAM(%s%s) = %s + %s", indregs[(opcode >> 8) & 1], mods[(opcode >> 9) & 3], regs[(opcode >> 5) & 3], yregs[opcode & 3]); }},
	{ 0xc80c, 0xf89c, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "RAM(%s%s) = %s + %s + C", indregs[(opcode >> 8) & 1], mods[(opcode >> 9) & 3], regs[(opcode >> 5) & 3], yregs[opcode & 3]); }},
	{ 0xc810, 0xf89c, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "RAM(%s%s) = %s - %s", indregs[(opcode >> 8) & 1], mods[(opcode >> 9) & 3], regs[(opcode >> 5) & 3], yregs[opcode & 3]); }},
	{ 0xc814, 0xf89c, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "RAM(%s%s) = %s - %s + C - 1", indregs[(opcode >> 8) & 1], mods[(opcode >> 9) & 3], regs[(opcode >> 5) & 3], yregs[opcode & 3]); }},
	{ 0xc818, 0xf89c, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "RAM(%s%s) = -%s + %s", indregs[(opcode >> 8) & 1], mods[(opcode >> 9) & 3], regs[(opcode >> 5) & 3], yregs[opcode & 3]); }},
	{ 0xc81c, 0xf89c, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "RAM(%s%s) = -%s + %s + C - 1", indregs[(opcode >> 8) & 1], mods[(opcode >> 9) & 3], regs[(opcode >> 5) & 3], yregs[opcode & 3]); }},
	{ 0xc880, 0xf89c, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = %s AND %s", regs[(opcode >> 8) & 7], regs[(opcode >> 5) & 3], yregs[opcode & 3]); }},
	{ 0xc888, 0xf89c, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = %s OR %s", regs[(opcode >> 8) & 7], regs[(opcode >> 5) & 3], yregs[opcode & 3]); }},
	{ 0xc890, 0xf89c, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = %s XOR %s", regs[(opcode >> 8) & 7], regs[(opcode >> 5) & 3], yregs[opcode & 3]); }},
	{ 0xc898, 0xf89f, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = NOT %s", regs[(opcode >> 8) & 7], regs[(opcode >> 5) & 3]); }},
	{ 0xc884, 0xf89c, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "r%d = BSET.%d %s", (opcode >> 8) & 1, ((opcode >> 7) & 0xc) | ((opcode >> 5) & 3), yregs[opcode & 3]); }},
	{ 0xc88c, 0xf89c, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "r%d = BCLR.%d %s", (opcode >> 8) & 1, ((opcode >> 7) & 0xc) | ((opcode >> 5) & 3), yregs[opcode & 3]); }},
	{ 0xc894, 0xf89c, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "r%d = BTOG.%d %s", (opcode >> 8) & 1, ((opcode >> 7) & 0xc) | ((opcode >> 5) & 3), yregs[opcode & 3]); }},
	{ 0xc89c, 0xf89c, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "r%d = BTST.%d %s", (opcode >> 8) & 1, ((opcode >> 7) & 0xc) | ((opcode >> 5) & 3), yregs[opcode & 3]); }},
	{ 0xd000, 0xf800, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s.l = 0x%02x", immregs[(opcode >> 8) & 7], opcode & 0xff); }},
	{ 0xd800, 0xf800, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = 0x%04x", immregs[(opcode >> 8) & 7], opcode & 0xff); }},
	{ 0xe000, 0xf8c3, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "RAM(%s%s) = %s", indregs[(opcode >> 2) & 3], mods[(opcode >> 4) & 3], regs[(opcode >> 8) & 7]); }},
	{ 0xe040, 0xf8c3, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = RAM(%s%s)", regs[(opcode >> 8) & 7], indregs[(opcode >> 2) & 3], mods[(opcode >> 4) & 3]); }},
	{ 0xe041, 0xf8c3, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = ROM(%s%s)", regs[(opcode >> 8) & 7], indregs[(opcode >> 2) & 3], mods[(opcode >> 4) & 3]); }},
	{ 0xe04a, 0xf8df, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "r%d = SL.idx %s", (opcode >> 5) & 1, regs[(opcode >> 8) & 7]); }},
	{ 0xe052, 0xf8df, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "r%d = SRA.idx %s", (opcode >> 5) & 1, regs[(opcode >> 8) & 7]); }},
	{ 0xe05a, 0xf8df, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "r%d = SRL.idx %s", (opcode >> 5) & 1, regs[(opcode >> 8) & 7]); }},
	{ 0xe080, 0xfc80, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = %s", ioreg(opcode & 0x7f), regs[(opcode >> 8) & 3]); }},
	{ 0xe480, 0xfc80, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = %s", regs[(opcode >> 8) & 3], ioreg(opcode & 0x7f)); }},
	{ 0xe800, 0xf81c, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = %s + %d", regs[(opcode >> 8) & 7], regs[(opcode >> 5) & 7], 1 << (opcode & 3)); }},
	{ 0xe804, 0xf81c, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = %s - %d", regs[(opcode >> 8) & 7], regs[(opcode >> 5) & 7], 1 << (opcode & 3)); }},
	{ 0xe808, 0xf81c, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = %s + %s", regs[(opcode >> 8) & 7], regs[(opcode >> 5) & 7], yregs[opcode & 3]); }},
	{ 0xe80c, 0xf81c, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = %s + %s + C", regs[(opcode >> 8) & 7], regs[(opcode >> 5) & 7], yregs[opcode & 3]); }},
	{ 0xe810, 0xf81c, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = %s - %s", regs[(opcode >> 8) & 7], regs[(opcode >> 5) & 7], yregs[opcode & 3]); }},
	{ 0xe814, 0xf81c, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = %s - %s + C - 1", regs[(opcode >> 8) & 7], regs[(opcode >> 5) & 7], yregs[opcode & 3]); }},
	{ 0xe818, 0xf81c, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = -%s + %s", regs[(opcode >> 8) & 7], regs[(opcode >> 5) & 7], yregs[opcode & 3]); }},
	{ 0xe81c, 0xf81c, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = -%s + %s + C - 1", regs[(opcode >> 8) & 7], regs[(opcode >> 5) & 7], yregs[opcode & 3]); }},
	{ 0xf000, 0xfbfc, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "mr = %s * %s (%s)", regs[(opcode >> 1) & 1], yregs[opcode & 1], (opcode & 0x400) ? "FS" : "IS"); }},
	{ 0xf100, 0xfbfc, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "mr = mr + %s * %s (%s)", regs[(opcode >> 1) & 1], yregs[opcode & 1], (opcode & 0x400) ? "FS" : "IS"); }},
	{ 0xf200, 0xfbfc, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "mr = mr - %s * %s (%s)", regs[(opcode >> 1) & 1], yregs[opcode & 1], (opcode & 0x400) ? "FS" : "IS"); }},
	{ 0xf080, 0xfb80, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "mr = %s * %s (%s), %s = RAM(%s%s)", regs[(opcode >> 1) & 1], yregs[opcode & 1], (opcode & 0x400) ? "FS" : "IS", macregs[(opcode >> 2) & 3], indregs[(opcode >> 6) & 1], mods[(opcode >> 4) & 3]); }},
	{ 0xf180, 0xfb80, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "mr = mr + %s * %s (%s), %s = RAM(%s%s)", regs[(opcode >> 1) & 1], yregs[opcode & 1], (opcode & 0x400) ? "FS" : "IS", macregs[(opcode >> 2) & 3], indregs[(opcode >> 6) & 1], mods[(opcode >> 4) & 3]); }},
	{ 0xf280, 0xfb80, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "mr = mr - %s * %s (%s), %s = RAM(%s%s)", regs[(opcode >> 1) & 1], yregs[opcode & 1], (opcode & 0x400) ? "FS" : "IS", macregs[(opcode >> 2) & 3], indregs[(opcode >> 6) & 1], mods[(opcode >> 4) & 3]); }},
	{ 0xf800, 0xff03, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = %s", regs[(opcode >> 2) & 7], regs[(opcode >> 5) & 7]); }},
	{ 0xf802, 0xff1f, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "push %s", regs[(opcode >> 5) & 7]); }},
	{ 0xf803, 0xff1f, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "pop %s", regs[(opcode >> 5) & 7]); }},
	{ 0xfa08, 0xfe18, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = SL.%d %s", regs[(opcode >> 8) & 3], (opcode & 7) + 1, regs[(opcode >> 5) & 7]); }},
	{ 0xfa10, 0xfe18, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = SRA.%d %s", regs[(opcode >> 8) & 3], (opcode & 7) + 1, regs[(opcode >> 5) & 7]); }},
	{ 0xfa18, 0xfe18, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = SRL.%d %s", regs[(opcode >> 8) & 3], (opcode & 7) + 1, regs[(opcode >> 5) & 7]); }},
	{ 0xfc00, 0xff80, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "do%d 0x%02X", (opcode >> 6) & 1, opcode & 0x3f); }},
	{ 0xfc80, 0xffc0, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "push %s", ioreg(opcode & 0x3f)); }},
	{ 0xfcc0, 0xffc0, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "pop %s", ioreg(opcode & 0x3f)); }},
	{ 0xfd00, 0xff00, 2 | STEP_OVER, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "callff 0x%06x", ((opcode & 0xff) << 16) | arg); }},
	{ 0xfe00, 0xff00, 2, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "jmpff 0x%06x", ((opcode & 0xff) << 16) | arg); }},
	{ 0xff40, 0xffff, 1 | STEP_OUT, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "ret"); }},
	{ 0xff41, 0xffff, 1 | STEP_OUT, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "reti"); }},
	{ 0xff42, 0xffff, 1 | STEP_OUT, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "retff"); }},
	{ 0xfffc, 0xfffd, 1 | STEP_COND, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "loop%d", (opcode >> 1) & 1); }},
	{ 0xffff, 0xffff, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "nop"); }},

	{ 0x0000, 0x0000, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "? %04x", opcode); }}
};

std::string sonix16_disassembler::ioreg(u8 reg)
{
	switch (reg)
	{
	case 0x00:
		return "ssf";

	case 0x02: case 0x03:
		return indregs[reg - 0x02];

	case 0x0d:
		return "rambk";

	case 0x0e: case 0x0f:
		return util::string_format("ix%dbk", reg & 1);

	case 0x13: case 0x14:
		return indregs[reg - 0x11];

	case 0x15:
		return "pch";

	case 0x16:
		return "pcl";

	case 0x18:
		return "sp";

	case 0x1a: case 0x1b:
		return util::string_format("iy%dbk", reg & 1);

	case 0x1c: case 0x1d:
		return util::string_format("ix%dbkram", reg & 1);

	case 0x20:
		return "inten";

	case 0x21:
		return "intrq";

	case 0x22:
		return "intpr";

	case 0x23:
		return "intcr";

	case 0x3e:
		return "shidx";

	default:
		return util::string_format("IO(0x%02x)", reg);
	}
}

offs_t sonix16_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u16 opcode = opcodes.r16(pc);

	if((opcode & 0xf800) == 0xd800) {
		u16 next = opcodes.r16(pc + 1);
		if(((next ^ opcode) & 0xff00) == 0x1800) {
			util::stream_format(stream, "%s = LO 0x%02x%02x", immregs[(opcode >> 8) & 7], next & 0xff, opcode & 0xff);
			return 1;
		}
	}

	int i = 0;
	while((opcode & instructions[i].mask) != instructions[i].value)
		i++;

	instructions[i].fct(stream, opcode, ((instructions[i].size & LENGTHMASK) >= 2) ? opcodes.r16(pc+1) : 0, pc);

	return instructions[i].size;
}
