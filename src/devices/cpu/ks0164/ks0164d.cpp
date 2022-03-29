// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, David Carne

// KS0164 disassembler

#include "emu.h"
#include "ks0164d.h"

u32 ks0164_disassembler::opcode_alignment() const
{
	return 2;
}

const char *const ks0164_disassembler::regs[8] = {
	"r0", "r1", "r2", "r3", "sp", "psw", "pc", "zero"
};

s32 ks0164_disassembler::off10(u32 opcode)
{
	return opcode & 0x200 ? opcode | 0xfffffc00 : opcode & 0x3ff;
}

std::string ks0164_disassembler::imm8(s8 dt)
{
	return dt < 0 ? util::string_format("-%x", -dt) : util::string_format("%x", dt);
}

std::string ks0164_disassembler::off16(s16 dt)
{
	return dt < 0 ? util::string_format(" - %x", -dt) : dt ? util::string_format(" + %x", dt) : "";
}

#define P std::ostream &stream, u32 opcode, const data_buffer &opcodes, u32 pc
const ks0164_disassembler::instruction ks0164_disassembler::instructions[] {
	{ 0x0000, 0xfc00, [](P) -> u32 { util::stream_format(stream, "bne %04x",  (pc + 2 + off10(opcode)) & 0xffff); return 2 | STEP_COND; } },
	{ 0x0400, 0xfc00, [](P) -> u32 { util::stream_format(stream, "beq %04x",  (pc + 2 + off10(opcode)) & 0xffff); return 2 | STEP_COND; } },
	{ 0x0800, 0xfc00, [](P) -> u32 { util::stream_format(stream, "bgeu %04x",  (pc + 2 + off10(opcode)) & 0xffff); return 2 | STEP_COND; } },
	{ 0x0c00, 0xfc00, [](P) -> u32 { util::stream_format(stream, "bltu %04x",  (pc + 2 + off10(opcode)) & 0xffff); return 2 | STEP_COND; } },
	{ 0x1000, 0xfc00, [](P) -> u32 { util::stream_format(stream, "bpl %04x",  (pc + 2 + off10(opcode)) & 0xffff); return 2 | STEP_COND; } },
	{ 0x1400, 0xfc00, [](P) -> u32 { util::stream_format(stream, "bmi %04x",  (pc + 2 + off10(opcode)) & 0xffff); return 2 | STEP_COND; } },
	{ 0x1800, 0xfc00, [](P) -> u32 { util::stream_format(stream, "bvc %04x",  (pc + 2 + off10(opcode)) & 0xffff); return 2 | STEP_COND; } },
	{ 0x1c00, 0xfc00, [](P) -> u32 { util::stream_format(stream, "bvs %04x",  (pc + 2 + off10(opcode)) & 0xffff); return 2 | STEP_COND; } },
	{ 0x2000, 0xfc00, [](P) -> u32 { util::stream_format(stream, "bnv %04x",  (pc + 2 + off10(opcode)) & 0xffff); return 2 | STEP_COND; } },
	{ 0x2400, 0xfc00, [](P) -> u32 { util::stream_format(stream, "bgtu %04x",  (pc + 2 + off10(opcode)) & 0xffff); return 2 | STEP_COND; } },
	{ 0x2800, 0xfc00, [](P) -> u32 { util::stream_format(stream, "bleu %04x", (pc + 2 + off10(opcode)) & 0xffff); return 2 | STEP_COND; } },
	{ 0x2c00, 0xfc00, [](P) -> u32 { util::stream_format(stream, "bgts %04x",  (pc + 2 + off10(opcode)) & 0xffff); return 2 | STEP_COND; } },
	{ 0x3000, 0xfc00, [](P) -> u32 { util::stream_format(stream, "bles %04x",  (pc + 2 + off10(opcode)) & 0xffff); return 2 | STEP_COND; } },
	{ 0x3400, 0xfc00, [](P) -> u32 { util::stream_format(stream, "bges %04x",  (pc + 2 + off10(opcode)) & 0xffff); return 2 | STEP_COND; } },
	{ 0x3800, 0xfc00, [](P) -> u32 { util::stream_format(stream, "blts %04x",  (pc + 2 + off10(opcode)) & 0xffff); return 2 | STEP_COND; } },
	{ 0x3c00, 0xfc00, [](P) -> u32 { util::stream_format(stream, "bra %04x",  (pc + 2 + off10(opcode)) & 0xffff); return 2; } },

	{ 0x4000, 0xf800, [](P) -> u32 { util::stream_format(stream, "%s += %s",   regs[(opcode >> 8) & 7], imm8(opcode)); return 2; } },
	{ 0x4800, 0xf800, [](P) -> u32 { util::stream_format(stream, "%s -= %s",   regs[(opcode >> 8) & 7], imm8(opcode)); return 2; } },
	{ 0x5000, 0xf800, [](P) -> u32 { util::stream_format(stream, "cmp %s, %s", regs[(opcode >> 8) & 7], imm8(opcode)); return 2; } },
	{ 0x5800, 0xf800, [](P) -> u32 { util::stream_format(stream, "%s &= %s",   regs[(opcode >> 8) & 7], imm8(opcode)); return 2; } },
	{ 0x6000, 0xf800, [](P) -> u32 { util::stream_format(stream, "%s |= %s",   regs[(opcode >> 8) & 7], imm8(opcode)); return 2; } },
	{ 0x6800, 0xf800, [](P) -> u32 { util::stream_format(stream, "%s ^= %s",   regs[(opcode >> 8) & 7], imm8(opcode)); return 2; } },
	{ 0x7000, 0xf800, [](P) -> u32 { util::stream_format(stream, "%s = %s",    regs[(opcode >> 8) & 7], imm8(opcode)); return 2; } },
	{ 0x7800, 0xf800, [](P) -> u32 { util::stream_format(stream, "%s *= %s",   regs[(opcode >> 8) & 7], imm8(opcode)); return 2; } },

	{ 0x8004, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s += %s.bu",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0x8084, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s += %s.bs",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0x800c, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s += %s",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0x8804, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s -= %s.bu",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0x8884, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s -= %s.bs",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0x880c, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s -= %s",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0x9004, 0xf88f, [](P) -> u32 { util::stream_format(stream, "cmp %s, %s.bu", regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0x9084, 0xf88f, [](P) -> u32 { util::stream_format(stream, "cmp %s, %s.bs", regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0x900c, 0xf88f, [](P) -> u32 { util::stream_format(stream, "cmp %s, %s", regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0x9804, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s &= %s.bu",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0x9884, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s &= %s.bs",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0x980c, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s &= %s",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0xa004, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s |= %s.bu",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0xa084, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s |= %s.bs",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0xa00c, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s |= %s",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0xa804, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s ^= %s.bu",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0xa884, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s ^= %s.bs",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0xa80c, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s ^= %s",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0xb004, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s = %s.bu",    regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0xb084, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s = %s.bs",    regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0xb00c, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s = %s",    regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0xb804, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s *= %s.bu",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0xb884, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s *= %s.bs",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0xb80c, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s *= %s",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },

	{ 0x8075, 0xf8ff, [](P) -> u32 { u16 imm = opcodes.r16(pc+2); util::stream_format(stream, "%s += %02x",   regs[(opcode >> 8) & 7], imm); return 4; } },
	{ 0x807d, 0xf8ff, [](P) -> u32 { u16 imm = opcodes.r16(pc+2); util::stream_format(stream, "%s += %04x",   regs[(opcode >> 8) & 7], imm); return 4; } },
	{ 0x8875, 0xf8ff, [](P) -> u32 { u16 imm = opcodes.r16(pc+2); util::stream_format(stream, "%s -= %02x",   regs[(opcode >> 8) & 7], imm); return 4; } },
	{ 0x887d, 0xf8ff, [](P) -> u32 { u16 imm = opcodes.r16(pc+2); util::stream_format(stream, "%s -= %04x",   regs[(opcode >> 8) & 7], imm); return 4; } },
	{ 0x9075, 0xf8ff, [](P) -> u32 { u16 imm = opcodes.r16(pc+2); util::stream_format(stream, "cmp %s, %02x", regs[(opcode >> 8) & 7], imm); return 4; } },
	{ 0x907d, 0xf8ff, [](P) -> u32 { u16 imm = opcodes.r16(pc+2); util::stream_format(stream, "cmp %s, %04x", regs[(opcode >> 8) & 7], imm); return 4; } },
	{ 0x9875, 0xf8ff, [](P) -> u32 { u16 imm = opcodes.r16(pc+2); util::stream_format(stream, "%s &= %02x",   regs[(opcode >> 8) & 7], imm); return 4; } },
	{ 0x987d, 0xf8ff, [](P) -> u32 { u16 imm = opcodes.r16(pc+2); util::stream_format(stream, "%s &= %04x",   regs[(opcode >> 8) & 7], imm); return 4; } },
	{ 0xa075, 0xf8ff, [](P) -> u32 { u16 imm = opcodes.r16(pc+2); util::stream_format(stream, "%s |= %02x",   regs[(opcode >> 8) & 7], imm); return 4; } },
	{ 0xa07d, 0xf8ff, [](P) -> u32 { u16 imm = opcodes.r16(pc+2); util::stream_format(stream, "%s |= %04x",   regs[(opcode >> 8) & 7], imm); return 4; } },
	{ 0xa875, 0xf8ff, [](P) -> u32 { u16 imm = opcodes.r16(pc+2); util::stream_format(stream, "%s ^= %02x",   regs[(opcode >> 8) & 7], imm); return 4; } },
	{ 0xa87d, 0xf8ff, [](P) -> u32 { u16 imm = opcodes.r16(pc+2); util::stream_format(stream, "%s ^= %04x",   regs[(opcode >> 8) & 7], imm); return 4; } },
	{ 0xb075, 0xf8ff, [](P) -> u32 { u16 imm = opcodes.r16(pc+2); util::stream_format(stream, "%s = %02x",    regs[(opcode >> 8) & 7], imm); return 4; } },
	{ 0xb07d, 0xf8ff, [](P) -> u32 { u16 imm = opcodes.r16(pc+2); util::stream_format(stream, "%s = %04x",    regs[(opcode >> 8) & 7], imm); return 4; } },
	{ 0xb875, 0xf8ff, [](P) -> u32 { u16 imm = opcodes.r16(pc+2); util::stream_format(stream, "%s *u= %02x",   regs[(opcode >> 8) & 7], imm); return 4; } },
	{ 0xb8f5, 0xf8ff, [](P) -> u32 { u16 imm = opcodes.r16(pc+2); util::stream_format(stream, "%s *s= %02x",   regs[(opcode >> 8) & 7], imm); return 4; } },
	{ 0xb87d, 0xf8ff, [](P) -> u32 { u16 imm = opcodes.r16(pc+2); util::stream_format(stream, "%s *= %04x",   regs[(opcode >> 8) & 7], imm); return 4; } },

	{ 0x8006, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s += (%s).bu",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0x8086, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s += (%s).bs",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0x800e, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s += (%s).w",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0x8806, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s -= (%s).bu",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0x8886, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s -= (%s).bs",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0x880e, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s -= (%s).w",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0x9006, 0xf88f, [](P) -> u32 { util::stream_format(stream, "cmp %s, (%s).bu", regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0x9086, 0xf88f, [](P) -> u32 { util::stream_format(stream, "cmp %s, (%s).bs", regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0x900e, 0xf88f, [](P) -> u32 { util::stream_format(stream, "cmp %s, (%s).w", regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0x9806, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s &= (%s).bu",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0x9886, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s &= (%s).bs",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0x980e, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s &= (%s).w",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0xa006, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s |= (%s).bu",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0xa086, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s |= (%s).bs",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0xa00e, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s |= (%s).w",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0xa806, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s ^= (%s).bu",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0xa886, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s ^= (%s).bs",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0xa80e, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s ^= (%s).w",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0xb006, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s = (%s).bu",    regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0xb086, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s = (%s).bs",    regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0xb00e, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s = (%s).w",    regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0xb806, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s *= (%s).bu",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0xb886, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s *= (%s).bs",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0xb80e, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s *= (%s).w",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },

	{ 0x8077, 0xf8ff, [](P) -> u32 { util::stream_format(stream, "%s += (%04x).bu",   regs[(opcode >> 8) & 7], opcodes.r16(pc+2)); return 4; } },
	{ 0x80f7, 0xf8ff, [](P) -> u32 { util::stream_format(stream, "%s += (%04x).bs",   regs[(opcode >> 8) & 7], opcodes.r16(pc+2)); return 4; } },
	{ 0x807f, 0xf8ff, [](P) -> u32 { util::stream_format(stream, "%s += (%04x).w",   regs[(opcode >> 8) & 7], opcodes.r16(pc+2)); return 4; } },
	{ 0x8877, 0xf8ff, [](P) -> u32 { util::stream_format(stream, "%s -= (%04x).bu",   regs[(opcode >> 8) & 7], opcodes.r16(pc+2)); return 4; } },
	{ 0x88f7, 0xf8ff, [](P) -> u32 { util::stream_format(stream, "%s -= (%04x).bs",   regs[(opcode >> 8) & 7], opcodes.r16(pc+2)); return 4; } },
	{ 0x887f, 0xf8ff, [](P) -> u32 { util::stream_format(stream, "%s -= (%04x).w",   regs[(opcode >> 8) & 7], opcodes.r16(pc+2)); return 4; } },
	{ 0x9077, 0xf8ff, [](P) -> u32 { util::stream_format(stream, "cmp %s, (%04x).bu", regs[(opcode >> 8) & 7], opcodes.r16(pc+2)); return 4; } },
	{ 0x90f7, 0xf8ff, [](P) -> u32 { util::stream_format(stream, "cmp %s, (%04x).bs", regs[(opcode >> 8) & 7], opcodes.r16(pc+2)); return 4; } },
	{ 0x907f, 0xf8ff, [](P) -> u32 { util::stream_format(stream, "cmp %s, (%04x).w", regs[(opcode >> 8) & 7], opcodes.r16(pc+2)); return 4; } },
	{ 0x9877, 0xf8ff, [](P) -> u32 { util::stream_format(stream, "%s &= (%04x).bu",   regs[(opcode >> 8) & 7], opcodes.r16(pc+2)); return 4; } },
	{ 0x98f7, 0xf8ff, [](P) -> u32 { util::stream_format(stream, "%s &= (%04x).bs",   regs[(opcode >> 8) & 7], opcodes.r16(pc+2)); return 4; } },
	{ 0x987f, 0xf8ff, [](P) -> u32 { util::stream_format(stream, "%s &= (%04x).w",   regs[(opcode >> 8) & 7], opcodes.r16(pc+2)); return 4; } },
	{ 0xa077, 0xf8ff, [](P) -> u32 { util::stream_format(stream, "%s |= (%04x).bu",   regs[(opcode >> 8) & 7], opcodes.r16(pc+2)); return 4; } },
	{ 0xa0f7, 0xf8ff, [](P) -> u32 { util::stream_format(stream, "%s |= (%04x).bs",   regs[(opcode >> 8) & 7], opcodes.r16(pc+2)); return 4; } },
	{ 0xa07f, 0xf8ff, [](P) -> u32 { util::stream_format(stream, "%s |= (%04x).w",   regs[(opcode >> 8) & 7], opcodes.r16(pc+2)); return 4; } },
	{ 0xa877, 0xf8ff, [](P) -> u32 { util::stream_format(stream, "%s ^= (%04x).bu",   regs[(opcode >> 8) & 7], opcodes.r16(pc+2)); return 4; } },
	{ 0xa8f7, 0xf8ff, [](P) -> u32 { util::stream_format(stream, "%s ^= (%04x).bs",   regs[(opcode >> 8) & 7], opcodes.r16(pc+2)); return 4; } },
	{ 0xa87f, 0xf8ff, [](P) -> u32 { util::stream_format(stream, "%s ^= (%04x).w",   regs[(opcode >> 8) & 7], opcodes.r16(pc+2)); return 4; } },
	{ 0xb077, 0xf8ff, [](P) -> u32 { util::stream_format(stream, "%s = (%04x).bu",    regs[(opcode >> 8) & 7], opcodes.r16(pc+2)); return 4; } },
	{ 0xb0f7, 0xf8ff, [](P) -> u32 { util::stream_format(stream, "%s = (%04x).bs",    regs[(opcode >> 8) & 7], opcodes.r16(pc+2)); return 4; } },
	{ 0xb07f, 0xf8ff, [](P) -> u32 { util::stream_format(stream, "%s = (%04x).w",    regs[(opcode >> 8) & 7], opcodes.r16(pc+2)); return 4; } },
	{ 0xb877, 0xf8ff, [](P) -> u32 { util::stream_format(stream, "%s *= (%04x).bu",   regs[(opcode >> 8) & 7], opcodes.r16(pc+2)); return 4; } },
	{ 0xb8f7, 0xf8ff, [](P) -> u32 { util::stream_format(stream, "%s *= (%04x).bs",   regs[(opcode >> 8) & 7], opcodes.r16(pc+2)); return 4; } },
	{ 0xb87f, 0xf8ff, [](P) -> u32 { util::stream_format(stream, "%s *= (%04x).w",   regs[(opcode >> 8) & 7], opcodes.r16(pc+2)); return 4; } },

	{ 0x8007, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s += (%s%s).bu",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7], off16(opcodes.r16(pc+2))); return 4; } },
	{ 0x8087, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s += (%s%s).bs",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7], off16(opcodes.r16(pc+2))); return 4; } },
	{ 0x800f, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s += (%s%s).w",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7], off16(opcodes.r16(pc+2))); return 4; } },
	{ 0x8807, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s -= (%s%s).bu",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7], off16(opcodes.r16(pc+2))); return 4; } },
	{ 0x8887, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s -= (%s%s).bs",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7], off16(opcodes.r16(pc+2))); return 4; } },
	{ 0x880f, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s -= (%s%s).w",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7], off16(opcodes.r16(pc+2))); return 4; } },
	{ 0x9007, 0xf88f, [](P) -> u32 { util::stream_format(stream, "cmp %s, (%s%s).bu", regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7], off16(opcodes.r16(pc+2))); return 4; } },
	{ 0x9087, 0xf88f, [](P) -> u32 { util::stream_format(stream, "cmp %s, (%s%s).bs", regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7], off16(opcodes.r16(pc+2))); return 4; } },
	{ 0x900f, 0xf88f, [](P) -> u32 { util::stream_format(stream, "cmp %s, (%s%s).w", regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7], off16(opcodes.r16(pc+2))); return 4; } },
	{ 0x9807, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s &= (%s%s).bu",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7], off16(opcodes.r16(pc+2))); return 4; } },
	{ 0x9887, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s &= (%s%s).bs",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7], off16(opcodes.r16(pc+2))); return 4; } },
	{ 0x980f, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s &= (%s%s).w",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7], off16(opcodes.r16(pc+2))); return 4; } },
	{ 0xa007, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s |= (%s%s).bu",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7], off16(opcodes.r16(pc+2))); return 4; } },
	{ 0xa087, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s |= (%s%s).bs",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7], off16(opcodes.r16(pc+2))); return 4; } },
	{ 0xa00f, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s |= (%s%s).w",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7], off16(opcodes.r16(pc+2))); return 4; } },
	{ 0xa807, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s ^= (%s%s).bu",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7], off16(opcodes.r16(pc+2))); return 4; } },
	{ 0xa887, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s ^= (%s%s).bs",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7], off16(opcodes.r16(pc+2))); return 4; } },
	{ 0xa80f, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s ^= (%s%s).w",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7], off16(opcodes.r16(pc+2))); return 4; } },
	{ 0xb007, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s = (%s%s).bu",    regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7], off16(opcodes.r16(pc+2))); return 4; } },
	{ 0xb087, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s = (%s%s).bs",    regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7], off16(opcodes.r16(pc+2))); return 4; } },
	{ 0xb00f, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s = (%s%s).w",    regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7], off16(opcodes.r16(pc+2))); return 4; } },
	{ 0xb807, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s *= (%s%s).bu",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7], off16(opcodes.r16(pc+2))); return 4; } },
	{ 0xb887, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s *= (%s%s).bs",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7], off16(opcodes.r16(pc+2))); return 4; } },
	{ 0xb80f, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s *= (%s%s).w",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7], off16(opcodes.r16(pc+2))); return 4; } },

	{ 0xc800, 0xf88f, [](P) -> u32 { util::stream_format(stream, "-(%s).w = %s",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0xc001, 0xffff, [](P) -> u32 { util::stream_format(stream, "push r0-r3"); return 2; } },
	{ 0xc003, 0xffff, [](P) -> u32 { util::stream_format(stream, "pop r0-r3"); return 2; } },
	{ 0xc008, 0xf88f, [](P) -> u32 { util::stream_format(stream, "(%s)+.w = %s",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0xc00a, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s = (%s)+.w",   regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },

	{ 0xc00c, 0xf80f, [](P) -> u32 { util::stream_format(stream, "%s <<= %x",   regs[(opcode >> 8) & 7], (opcode >> 4) & 0xf); return 2; } },
	{ 0xc80c, 0xf80f, [](P) -> u32 { util::stream_format(stream, "%s >>= %x",   regs[(opcode >> 8) & 7], (opcode >> 4) & 0xf); return 2; } },
	{ 0xc80d, 0xf80f, [](P) -> u32 { util::stream_format(stream, "%s >>s= %x",   regs[(opcode >> 8) & 7], (opcode >> 4) & 0xf); return 2; } },
	{ 0xc00f, 0xf80f, [](P) -> u32 { util::stream_format(stream, "%s <<c= %x",   regs[(opcode >> 8) & 7], (opcode >> 4) & 0xf); return 2; } },
	{ 0xc80f, 0xf80f, [](P) -> u32 { util::stream_format(stream, "%s >>c= %x",   regs[(opcode >> 8) & 7], (opcode >> 4) & 0xf); return 2; } },

	{ 0xd008, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s = maxu(%s, %04x)", regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7], opcodes.r16(pc+2)); return 4; } },
	{ 0xd808, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s = minu(%s, %04x)", regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7], opcodes.r16(pc+2)); return 4; } },
	{ 0xd088, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s = maxs(%s, %04x)", regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7], opcodes.r16(pc+2)); return 4; } },
	{ 0xd888, 0xf88f, [](P) -> u32 { util::stream_format(stream, "%s = mins(%s, %04x)", regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7], opcodes.r16(pc+2)); return 4; } },
	{ 0xd001, 0xffff, [](P) -> u32 { util::stream_format(stream, "jmp %04x", opcodes.r16(pc+2)); return 4; } },
	{ 0xd002, 0xffff, [](P) -> u32 { util::stream_format(stream, "jsr %04x", opcodes.r16(pc+2)); return 4 | STEP_OVER; } },
	{ 0xd003, 0xffff, [](P) -> u32 { util::stream_format(stream, "rts"); return 2 | STEP_OUT; } },
	{ 0xd004, 0xffff, [](P) -> u32 { util::stream_format(stream, "rti"); return 2 | STEP_OUT; } },
	{ 0xd005, 0xf8ff, [](P) -> u32 { util::stream_format(stream, "%s = ~%s",   regs[(opcode >> 8) & 7], regs[(opcode >> 8) & 7]); return 2; } },
	{ 0xd085, 0xf8ff, [](P) -> u32 { util::stream_format(stream, "%s = -%s",   regs[(opcode >> 8) & 7], regs[(opcode >> 8) & 7]); return 2; } },

	{ 0xe008, 0xf80f, [](P) -> u32 { util::stream_format(stream, "btst %s, %x",     regs[(opcode >> 8) & 7], (opcode >> 4) & 0xf); return 2; } },
	{ 0xe001, 0xf80f, [](P) -> u32 { util::stream_format(stream, "bset %s, %x",     regs[(opcode >> 8) & 7], (opcode >> 4) & 0xf); return 2; } },
	{ 0xe002, 0xf80f, [](P) -> u32 { util::stream_format(stream, "btst (%s).b, %x",   regs[(opcode >> 8) & 7], (opcode >> 4) & 0xf); return 2; } },
	{ 0xe00a, 0xf80f, [](P) -> u32 { util::stream_format(stream, "btst (%s).w, %x",   regs[(opcode >> 8) & 7], (opcode >> 4) & 0xf); return 2; } },
	{ 0xe703, 0xff0f, [](P) -> u32 { util::stream_format(stream, "btst (%04x).b, %x", opcodes.r16(pc+2), (opcode >> 4) & 0xf); return 4; } },
	{ 0xe70b, 0xff0f, [](P) -> u32 { util::stream_format(stream, "btst (%04x).w, %x", opcodes.r16(pc+2), (opcode >> 4) & 0xf); return 4; } },
	{ 0xe003, 0xf80f, [](P) -> u32 { util::stream_format(stream, "btst (%s%s).b, %x", regs[(opcode >> 8) & 7], off16(opcodes.r16(pc+2)), (opcode >> 4) & 0xf); return 4; } },
	{ 0xe00b, 0xf80f, [](P) -> u32 { util::stream_format(stream, "btst (%s%s).w, %x", regs[(opcode >> 8) & 7], off16(opcodes.r16(pc+2)), (opcode >> 4) & 0xf); return 4; } },
	{ 0xe004, 0xf80f, [](P) -> u32 { util::stream_format(stream, "bclr %s, %x",     regs[(opcode >> 8) & 7], (opcode >> 4) & 0xf); return 2; } },
	{ 0xe006, 0xf88f, [](P) -> u32 { util::stream_format(stream, "(%s).b = %s",     regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0xe00e, 0xf88f, [](P) -> u32 { util::stream_format(stream, "(%s).w = %s",     regs[(opcode >> 8) & 7], regs[(opcode >> 4) & 7]); return 2; } },
	{ 0xe707, 0xff8f, [](P) -> u32 { util::stream_format(stream, "(%04x).b = %s",   opcodes.r16(pc+2), regs[(opcode >> 4) & 7]); return 4; } },
	{ 0xe70f, 0xff8f, [](P) -> u32 { util::stream_format(stream, "(%04x).w = %s",   opcodes.r16(pc+2), regs[(opcode >> 4) & 7]); return 4; } },
	{ 0xe007, 0xf88f, [](P) -> u32 { util::stream_format(stream, "(%s%s).b = %s",   regs[(opcode >> 8) & 7], off16(opcodes.r16(pc+2)), regs[(opcode >> 4) & 7]); return 4; } },
	{ 0xe00f, 0xf88f, [](P) -> u32 { util::stream_format(stream, "(%s%s).w = %s",   regs[(opcode >> 8) & 7], off16(opcodes.r16(pc+2)), regs[(opcode >> 4) & 7]); return 4; } },

	{ 0xf004, 0xf8ff, [](P) -> u32 { util::stream_format(stream, "dbra %s, %04x", regs[(opcode >> 8) & 7], opcodes.r16(pc+2)); return 4 | STEP_COND; } },
	{ 0xf00d, 0xf8ff, [](P) -> u32 { util::stream_format(stream, "cmpbeq %s, %04x, %04x", regs[(opcode >> 8) & 7], opcodes.r16(pc+2), opcodes.r16(pc+4)); return 6 | STEP_COND; } },


	{ 0x0000, 0x0000, [](P) -> u32 { util::stream_format(stream, "?%04x",   opcode); return 2; } },
};

#undef P

offs_t ks0164_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u32 opcode = opcodes.r16(pc);

	for(u32 i=0;; i++)
		if((opcode & instructions[i].mask) == instructions[i].value)
			return instructions[i].cb(stream, opcode, opcodes, pc) | SUPPORTED;
	return 0;
}
