// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "s1c33dasm.h"

s1c33_disassembler::s1c33_disassembler()
	: util::disasm_interface()
{
}

u32 s1c33_disassembler::opcode_alignment() const
{
	return 2;
}

offs_t s1c33_disassembler::handle_0000_000x_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op)
{
	switch (op & 0b0000'0000'1100'0000)
	{
	case 0b0000'0000'0000'0000: stream << "NOP"; return 2 | SUPPORTED;
	case 0b0000'0000'0100'0000: stream << "SLP"; return 2 | SUPPORTED;
	case 0b0000'0000'1000'0000: stream << "HALT"; return 2 | SUPPORTED;
	case 0b0000'0000'1100'0000: stream << "<unhandled 0000_000x_11xx_xxxx>"; return 2 | SUPPORTED;
	}
	return 2;
}

offs_t s1c33_disassembler::handle_0000_001x_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op)
{
	switch (op & 0b0000'0000'1100'0000)
	{
	case 0b0000'0000'0000'0000: { int imm2 = op & 0x000f; util::stream_format(stream, "PUSHN RS(%01x)", imm2); return 2 | SUPPORTED; }
	case 0b0000'0000'0100'0000: { int imm2 = op & 0x000f; util::stream_format(stream, "POPN RD(%01x)", imm2); return 2 | SUPPORTED; }
	case 0b0000'0000'1000'0000: stream << "<unhandled 0000_001x_10xx_xxxx>"; return 2 | SUPPORTED;
	case 0b0000'0000'1100'0000: stream << "<unhandled 0000_001x_11xx_xxxx>"; return 2 | SUPPORTED;
	}
	return 2;
}

offs_t s1c33_disassembler::handle_0000_010x_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op)
{
	switch (op & 0b0000'0000'1100'0000)
	{
	case 0b0000'0000'0000'0000: stream << "BRK"; return 2 | SUPPORTED;
	case 0b0000'0000'0100'0000: stream << "RETD"; return 2 | SUPPORTED;
	case 0b0000'0000'1000'0000: { int imm2 = op & 0x000f; util::stream_format(stream, "INT $%01x", imm2); return 2 | SUPPORTED; }
	case 0b0000'0000'1100'0000: stream << "RETI"; return 2 | SUPPORTED;
	}
	return 2;
}

offs_t s1c33_disassembler::handle_0000_011x_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op)
{
	switch (op & 0b0000'0000'1100'0000)
	{
	case 0b0000'0000'0000'0000: { int imm2 = op & 0x000f; int delay = op & 0x0100; util::stream_format(stream, "CALL%s R(%01x)", delay ? ".d" : "", imm2); return 2 | SUPPORTED; }
	case 0b0000'0000'0100'0000: { int delay = op & 0x0100; util::stream_format(stream, "RET%s", delay ? ".d" : ""); return 2 | SUPPORTED; }
	case 0b0000'0000'1000'0000: { int imm2 = op & 0x000f; int delay = op & 0x0100; util::stream_format(stream, "JP%s R(%01x)", delay ? ".d" : "", imm2); return 2 | SUPPORTED; }
	case 0b0000'0000'1100'0000: stream << "<unhandled 0000_011x_11xx_xxxx>"; return 2 | SUPPORTED;
	}
	return 2;
}

offs_t s1c33_disassembler::handle_000x_xxxx_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op)
{
	switch (op & 0b0001'1110'0000'0000)
	{
	case 0b0000'0000'0000'0000: return handle_0000_000x_xxxx_xxxx(stream, pc, opcodes, params, op);
	case 0b0000'0010'0000'0000: return handle_0000_001x_xxxx_xxxx(stream, pc, opcodes, params, op);
	case 0b0000'0100'0000'0000: return handle_0000_010x_xxxx_xxxx(stream, pc, opcodes, params, op);
	case 0b0000'0110'0000'0000: return handle_0000_011x_xxxx_xxxx(stream, pc, opcodes, params, op);
	case 0b0000'1000'0000'0000: { int sign8 = op & 0x00ff; int delay = op & 0x0100; util::stream_format(stream, "JR_GT%s SIGN8(%02x)",  delay ? ".d" : "", sign8); return 2 | SUPPORTED; }
	case 0b0000'1010'0000'0000: { int sign8 = op & 0x00ff; int delay = op & 0x0100; util::stream_format(stream, "JR_GE%s SIGN8(%02x)",  delay ? ".d" : "", sign8); return 2 | SUPPORTED; }
	case 0b0000'1100'0000'0000: { int sign8 = op & 0x00ff; int delay = op & 0x0100; util::stream_format(stream, "JR_LT%s SIGN8(%02x)",  delay ? ".d" : "", sign8); return 2 | SUPPORTED; }
	case 0b0000'1110'0000'0000: { int sign8 = op & 0x00ff; int delay = op & 0x0100; util::stream_format(stream, "JR_LE%s SIGN8(%02x)",  delay ? ".d" : "", sign8); return 2 | SUPPORTED; }
	case 0b0001'0000'0000'0000: { int sign8 = op & 0x00ff; int delay = op & 0x0100; util::stream_format(stream, "JR_UGT%s SIGN8(%02x)", delay ? ".d" : "", sign8); return 2 | SUPPORTED; }
	case 0b0001'0010'0000'0000: { int sign8 = op & 0x00ff; int delay = op & 0x0100; util::stream_format(stream, "JR_UGE%s SIGN8(%02x)", delay ? ".d" : "", sign8); return 2 | SUPPORTED; }
	case 0b0001'0100'0000'0000: { int sign8 = op & 0x00ff; int delay = op & 0x0100; util::stream_format(stream, "JR_ULT%s SIGN8(%02x)", delay ? ".d" : "", sign8); return 2 | SUPPORTED; }
	case 0b0001'0110'0000'0000: { int sign8 = op & 0x00ff; int delay = op & 0x0100; util::stream_format(stream, "JR_ULE%s SIGN8(%02x)", delay ? ".d" : "", sign8); return 2 | SUPPORTED; }
	case 0b0001'1000'0000'0000: { int sign8 = op & 0x00ff; int delay = op & 0x0100; util::stream_format(stream, "JR_EQ%s SIGN8(%02x)",  delay ? ".d" : "", sign8); return 2 | SUPPORTED; }
	case 0b0001'1010'0000'0000: { int sign8 = op & 0x00ff; int delay = op & 0x0100; util::stream_format(stream, "JR_NE%s SIGN8(%02x)",  delay ? ".d" : "", sign8); return 2 | SUPPORTED; }
	case 0b0001'1100'0000'0000: { int sign8 = op & 0x00ff; int delay = op & 0x0100; util::stream_format(stream, "CALL%s SIGN8(%02x)",   delay ? ".d" : "", sign8); return 2 | SUPPORTED; }
	case 0b0001'1110'0000'0000: { int sign8 = op & 0x00ff; int delay = op & 0x0100; util::stream_format(stream, "JP%s SIGN8(%02x)",     delay ? ".d" : "", sign8); return 2 | SUPPORTED; }
	}
	return 2;
}

offs_t s1c33_disassembler::handle_001x_xx00_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op)
{
	switch (op & 0b0001'1100'0000'0000)
	{
	case 0b0000'0000'0000'0000: { int rd = op & 0x000f; int rb = (op & 0x00f0) >> 4; util::stream_format(stream, "LD.B R(%01x), [R(%01x)]", rd, rb); return 2 | SUPPORTED; }
	case 0b0000'0100'0000'0000: { int rd = op & 0x000f; int rb = (op & 0x00f0) >> 4; util::stream_format(stream, "LD.UB R(%01x), [R(%01x)]", rd, rb); return 2 | SUPPORTED; }
	case 0b0000'1000'0000'0000: { int rd = op & 0x000f; int rb = (op & 0x00f0) >> 4; util::stream_format(stream, "LD.H R(%01x), [R(%01x)]", rd, rb); return 2 | SUPPORTED; }
	case 0b0000'1100'0000'0000: { int rd = op & 0x000f; int rb = (op & 0x00f0) >> 4; util::stream_format(stream, "LD.UH R(%01x), [R(%01x)]", rd, rb); return 2 | SUPPORTED; }
	case 0b0001'0000'0000'0000: { int rd = op & 0x000f; int rb = (op & 0x00f0) >> 4; util::stream_format(stream, "LD.W R(%01x), [R(%01x)]", rd, rb); return 2 | SUPPORTED; }

	case 0b0001'0100'0000'0000: { int rs = op & 0x000f; int rb = (op & 0x00f0) >> 4; util::stream_format(stream, "LD.B [R(%01x)], R(%01x)", rb, rs); return 2 | SUPPORTED; }
	case 0b0001'1000'0000'0000: { int rs = op & 0x000f; int rb = (op & 0x00f0) >> 4; util::stream_format(stream, "LD.H [R(%01x)], R(%01x)", rb, rs); return 2 | SUPPORTED; }
	case 0b0001'1100'0000'0000: { int rs = op & 0x000f; int rb = (op & 0x00f0) >> 4; util::stream_format(stream, "LD.W [R(%01x)], R(%01x)", rb, rs); return 2 | SUPPORTED; }
	}
	return 2;
}

offs_t s1c33_disassembler::handle_001x_xx01_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op)
{
	switch (op & 0b0001'1100'0000'0000)
	{
	case 0b0000'0000'0000'0000: { int rd = op & 0x000f; int rb = (op & 0x00f0) >> 4; util::stream_format(stream, "LD.B R(%01x), [R(%01x)]+", rd, rb); return 2 | SUPPORTED; }
	case 0b0000'0100'0000'0000: { int rd = op & 0x000f; int rb = (op & 0x00f0) >> 4; util::stream_format(stream, "LD.UB R(%01x), [R(%01x)]+", rd, rb); return 2 | SUPPORTED; }
	case 0b0000'1000'0000'0000: { int rd = op & 0x000f; int rb = (op & 0x00f0) >> 4; util::stream_format(stream, "LD.H R(%01x), [R(%01x)]+", rd, rb); return 2 | SUPPORTED; }
	case 0b0000'1100'0000'0000: { int rd = op & 0x000f; int rb = (op & 0x00f0) >> 4; util::stream_format(stream, "LD.UH R(%01x), [R(%01x)]+", rd, rb); return 2 | SUPPORTED; }
	case 0b0001'0000'0000'0000: { int rd = op & 0x000f; int rb = (op & 0x00f0) >> 4; util::stream_format(stream, "LD.W R(%01x), [R(%01x)]+", rd, rb); return 2 | SUPPORTED; }

	case 0b0001'0100'0000'0000: { int rs = op & 0x000f; int rb = (op & 0x00f0) >> 4; util::stream_format(stream, "LD.B [R(%01x)]+, R(%01x)", rb, rs); return 2 | SUPPORTED; }
	case 0b0001'1000'0000'0000: { int rs = op & 0x000f; int rb = (op & 0x00f0) >> 4; util::stream_format(stream, "LD.H [R(%01x)]+, R(%01x)", rb, rs); return 2 | SUPPORTED; }
	case 0b0001'1100'0000'0000: { int rs = op & 0x000f; int rb = (op & 0x00f0) >> 4; util::stream_format(stream, "LD.W [R(%01x)]+, R(%01x)", rb, rs); return 2 | SUPPORTED; }

	}
	return 2;
}

offs_t s1c33_disassembler::handle_001x_xx10_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op)
{
	switch (op & 0b0001'1100'0000'0000)
	{
	case 0b0000'0000'0000'0000: { int rd = op & 0x000f; int rs = (op & 0x00f0) >> 4; util::stream_format(stream, "ADD R(%01x), R(%01x)", rd, rs); return 2 | SUPPORTED; }
	case 0b0000'0100'0000'0000: { int rd = op & 0x000f; int rs = (op & 0x00f0) >> 4; util::stream_format(stream, "SUB R(%01x), R(%01x)", rd, rs); return 2 | SUPPORTED; }
	case 0b0000'1000'0000'0000: { int rd = op & 0x000f; int rs = (op & 0x00f0) >> 4; util::stream_format(stream, "CMP R(%01x), R(%01x)", rd, rs); return 2 | SUPPORTED; }
	case 0b0000'1100'0000'0000: { int rd = op & 0x000f; int rs = (op & 0x00f0) >> 4; util::stream_format(stream, "LD.W R(%01x), R(%01x)", rd, rs); return 2 | SUPPORTED; }
	case 0b0001'0000'0000'0000: { int rd = op & 0x000f; int rs = (op & 0x00f0) >> 4; util::stream_format(stream, "AND R(%01x), R(%01x)", rd, rs); return 2 | SUPPORTED; }
	case 0b0001'0100'0000'0000: { int rd = op & 0x000f; int rs = (op & 0x00f0) >> 4; util::stream_format(stream, "OR R(%01x), R(%01x)", rd, rs); return 2 | SUPPORTED; }
	case 0b0001'1000'0000'0000: { int rd = op & 0x000f; int rs = (op & 0x00f0) >> 4; util::stream_format(stream, "XOR R(%01x), R(%01x)", rd, rs); return 2 | SUPPORTED; }
	case 0b0001'1100'0000'0000: { int rd = op & 0x000f; int rs = (op & 0x00f0) >> 4; util::stream_format(stream, "NOT R(%01x), R(%01x)", rd, rs); return 2 | SUPPORTED; }
	}
	return 2;
}

offs_t s1c33_disassembler::handle_001x_xx11_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op)
{
	stream << "<unhandled>";
	return 2 | SUPPORTED;
}

offs_t s1c33_disassembler::handle_001x_xxxx_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op)
{
	switch (op & 0b0000'0011'0000'0000)
	{
	case 0b0000'0000'0000'0000: return handle_001x_xx00_xxxx_xxxx(stream, pc, opcodes, params, op);
	case 0b0000'0001'0000'0000: return handle_001x_xx01_xxxx_xxxx(stream, pc, opcodes, params, op);
	case 0b0000'0010'0000'0000: return handle_001x_xx10_xxxx_xxxx(stream, pc, opcodes, params, op);
	case 0b0000'0011'0000'0000: return handle_001x_xx11_xxxx_xxxx(stream, pc, opcodes, params, op);

	}
	return 2;
}

offs_t s1c33_disassembler::handle_010x_xxxx_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op)
{
	switch (op & 0b0001'1100'0000'0000)
	{
	case 0b0000'0000'0000'0000: { int rd = op & 0x000f; int imm6 = (op & 0x03f0) >> 4; util::stream_format(stream, "LD.B R(%01x), [SP+IMM6(%02x)]", rd, imm6); return 2 | SUPPORTED; }
	case 0b0000'0100'0000'0000: { int rd = op & 0x000f; int imm6 = (op & 0x03f0) >> 4; util::stream_format(stream, "LD.UB R(%01x), [SP+IMM6(%02x)]", rd, imm6); return 2 | SUPPORTED; }
	case 0b0000'1000'0000'0000: { int rd = op & 0x000f; int imm6 = (op & 0x03f0) >> 4; util::stream_format(stream, "LD.H R(%01x), [SP+IMM6(%02x)]", rd, imm6); return 2 | SUPPORTED; }
	case 0b0000'1100'0000'0000: { int rd = op & 0x000f; int imm6 = (op & 0x03f0) >> 4; util::stream_format(stream, "LD.UH R(%01x), [SP+IMM6(%02x)]", rd, imm6); return 2 | SUPPORTED; }
	case 0b0001'0000'0000'0000: { int rd = op & 0x000f; int imm6 = (op & 0x03f0) >> 4; util::stream_format(stream, "LD.W R(%01x), [SP+IMM6(%02x)]", rd, imm6); return 2 | SUPPORTED; }

	case 0b0001'0100'0000'0000: { int rs = op & 0x000f; int imm6 = (op & 0x03f0) >> 4; util::stream_format(stream, "LD.B [SP+IMM6(%02x)], R(%01x)", imm6, rs); return 2 | SUPPORTED; }
	case 0b0001'1000'0000'0000: { int rs = op & 0x000f; int imm6 = (op & 0x03f0) >> 4; util::stream_format(stream, "LD.H [SP+IMM6(%02x)], R(%01x)", imm6, rs); return 2 | SUPPORTED; }
	case 0b0001'1100'0000'0000: { int rs = op & 0x000f; int imm6 = (op & 0x03f0) >> 4; util::stream_format(stream, "LD.W [SP+IMM6(%02x)], R(%01x)", imm6, rs); return 2 | SUPPORTED; }
	}
	return 2;
}

offs_t s1c33_disassembler::handle_011x_xxxx_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op)
{
	switch (op & 0b0001'1100'0000'0000)
	{
	case 0b0000'0000'0000'0000: { int rd = op & 0x000f; int imm6 = (op & 0x03f0) >> 4; util::stream_format(stream, "ADD R(%01x), IMM6(%02x)", rd, imm6); return 2 | SUPPORTED; }
	case 0b0000'0100'0000'0000: { int rd = op & 0x000f; int imm6 = (op & 0x03f0) >> 4; util::stream_format(stream, "SUB R(%01x), IMM6(%02x)", rd, imm6); return 2 | SUPPORTED; }

	case 0b0000'1000'0000'0000: { int rd = op & 0x000f; int sign6 = (op & 0x03f0) >> 4; util::stream_format(stream, "CMP R(%01x), SIGN6(%02x)", rd, sign6); return 2 | SUPPORTED; }
	case 0b0000'1100'0000'0000: { int rd = op & 0x000f; int sign6 = (op & 0x03f0) >> 4; util::stream_format(stream, "LD.W R(%01x), SIGN6(%02x)", rd, sign6); return 2 | SUPPORTED; }
	case 0b0001'0000'0000'0000: { int rd = op & 0x000f; int sign6 = (op & 0x03f0) >> 4; util::stream_format(stream, "AND R(%01x), SIGN6(%02x)", rd, sign6); return 2 | SUPPORTED; }
	case 0b0001'0100'0000'0000: { int rd = op & 0x000f; int sign6 = (op & 0x03f0) >> 4; util::stream_format(stream, "OR R(%01x), SIGN6(%02x)", rd, sign6); return 2 | SUPPORTED; }
	case 0b0001'1000'0000'0000: { int rd = op & 0x000f; int sign6 = (op & 0x03f0) >> 4; util::stream_format(stream, "XOR R(%01x), SIGN6(%02x)", rd, sign6); return 2 | SUPPORTED; }
	case 0b0001'1100'0000'0000: { int rd = op & 0x000f; int sign6 = (op & 0x03f0) >> 4; util::stream_format(stream, "NOT R(%01x), SIGN6(%02x)", rd, sign6); return 2 | SUPPORTED; }
	}
	return 2;
}

offs_t s1c33_disassembler::handle_100y_00xx_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op)
{
	switch (op & 0b0001'1100'0000'0000)
	{
	case 0b0000'0000'0000'0000: stream << "<unreachable>"; return 2 | SUPPORTED; // handled by handle_1000_00xx_xxxx_xxxx
	case 0b0000'0100'0000'0000: stream << "<unreachable>"; return 2 | SUPPORTED; // handled by handle_1000_01xx_xxxx_xxxx
	case 0b0000'1000'0000'0000: { int rd = op & 0x000f; int imm4 = (op & 0x00f0) >> 4; util::stream_format(stream, "SRL R(%01x), IMM4(%01x)", rd, imm4); return 2 | SUPPORTED; }
	case 0b0000'1100'0000'0000: { int rd = op & 0x000f; int imm4 = (op & 0x00f0) >> 4; util::stream_format(stream, "SLL R(%01x), IMM4(%01x)", rd, imm4); return 2 | SUPPORTED; }
	case 0b0001'0000'0000'0000: { int rd = op & 0x000f; int imm4 = (op & 0x00f0) >> 4; util::stream_format(stream, "SRA R(%01x), IMM4(%01x)", rd, imm4); return 2 | SUPPORTED; }
	case 0b0001'0100'0000'0000: { int rd = op & 0x000f; int imm4 = (op & 0x00f0) >> 4; util::stream_format(stream, "SLA R(%01x), IMM4(%01x)", rd, imm4); return 2 | SUPPORTED; }
	case 0b0001'1000'0000'0000: { int rd = op & 0x000f; int imm4 = (op & 0x00f0) >> 4; util::stream_format(stream, "RR R(%01x), IMM4(%01x)", rd, imm4); return 2 | SUPPORTED; }
	case 0b0001'1100'0000'0000: { int rd = op & 0x000f; int imm4 = (op & 0x00f0) >> 4; util::stream_format(stream, "RL R(%01x), IMM4(%01x)", rd, imm4); return 2 | SUPPORTED; }
	}
	return 2;
}

offs_t s1c33_disassembler::handle_100y_01xx_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op)
{
	switch (op & 0b0001'1100'0000'0000)
	{
	case 0b0000'0000'0000'0000: stream << "<unreachable>"; return 2 | SUPPORTED; // handled by handle_1000_00xx_xxxx_xxxx
	case 0b0000'0100'0000'0000: stream << "<unreachable>"; return 2 | SUPPORTED; // handled by handle_1000_01xx_xxxx_xxxx
	case 0b0000'1000'0000'0000: { int rd = op & 0x000f; int rs = (op & 0x00f0) >> 4; util::stream_format(stream, "SRL R(%01x), R(%01x)", rd, rs); return 2 | SUPPORTED; }
	case 0b0000'1100'0000'0000: { int rd = op & 0x000f; int rs = (op & 0x00f0) >> 4; util::stream_format(stream, "SLL R(%01x), R(%01x)", rd, rs); return 2 | SUPPORTED; }
	case 0b0001'0000'0000'0000: { int rd = op & 0x000f; int rs = (op & 0x00f0) >> 4; util::stream_format(stream, "SRA R(%01x), R(%01x)", rd, rs); return 2 | SUPPORTED; }
	case 0b0001'0100'0000'0000: { int rd = op & 0x000f; int rs = (op & 0x00f0) >> 4; util::stream_format(stream, "SLA R(%01x), R(%01x)", rd, rs); return 2 | SUPPORTED; }
	case 0b0001'1000'0000'0000: { int rd = op & 0x000f; int rs = (op & 0x00f0) >> 4; util::stream_format(stream, "RR R(%01x), R(%01x)", rd, rs); return 2 | SUPPORTED; }
	case 0b0001'1100'0000'0000: { int rd = op & 0x000f; int rs = (op & 0x00f0) >> 4; util::stream_format(stream, "RL R(%01x), R(%01x)", rd, rs); return 2 | SUPPORTED; }
	}
	return 2;
}

offs_t s1c33_disassembler::handle_100y_10xx_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op)
{
	switch (op & 0b0001'1100'0000'0000)
	{
	case 0b0000'0000'0000'0000: stream << "<unreachable>"; return 2 | SUPPORTED; // handled by handle_1000_00xx_xxxx_xxxx
	case 0b0000'0100'0000'0000: stream << "<unreachable>"; return 2 | SUPPORTED; // handled by handle_1000_01xx_xxxx_xxxx
	case 0b0000'1000'0000'0000: { int rd = op & 0x000f; int rs = (op & 0x00f0) >> 4; util::stream_format(stream, "SCAN0 R(%01x), R(%01x)", rd, rs); return 2 | SUPPORTED; }
	case 0b0000'1100'0000'0000: { int rd = op & 0x000f; int rs = (op & 0x00f0) >> 4; util::stream_format(stream, "SCAN1 R(%01x), R(%01x)", rd, rs); return 2 | SUPPORTED; }
	case 0b0001'0000'0000'0000: { int rd = op & 0x000f; int rs = (op & 0x00f0) >> 4; util::stream_format(stream, "SWAP R(%01x), R(%01x)", rd, rs); return 2 | SUPPORTED; }
	case 0b0001'0100'0000'0000: { int rd = op & 0x000f; int rs = (op & 0x00f0) >> 4; util::stream_format(stream, "MIRROR R(%01x), R(%01x)", rd, rs); return 2 | SUPPORTED; }
	case 0b0001'1000'0000'0000: stream << "<unhandled>"; return 2 | SUPPORTED;
	case 0b0001'1100'0000'0000: stream << "<unhandled>"; return 2 | SUPPORTED;
	}
	return 2;
}

offs_t s1c33_disassembler::handle_100y_11xx_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op)
{
	switch (op & 0b0001'1100'0000'0000)
	{
	case 0b0000'0000'0000'0000: stream << "<unreachable>"; return 2 | SUPPORTED; // handled by handle_1000_00xx_xxxx_xxxx
	case 0b0000'0100'0000'0000: stream << "<unreachable>"; return 2 | SUPPORTED; // handled by handle_1000_01xx_xxxx_xxxx
	case 0b0000'1000'0000'0000: { int rs = (op & 0x00f0) >> 4; util::stream_format(stream, "DIV0S R(%01x)", rs); return 2 | SUPPORTED; }
	case 0b0000'1100'0000'0000: { int rs = (op & 0x00f0) >> 4; util::stream_format(stream, "DIV0U R(%01x)", rs); return 2 | SUPPORTED; }
	case 0b0001'0000'0000'0000: { int rs = (op & 0x00f0) >> 4; util::stream_format(stream, "DIV1 R(%01x)", rs); return 2 | SUPPORTED; }
	case 0b0001'0100'0000'0000: { int rs = (op & 0x00f0) >> 4; util::stream_format(stream, "DIV2S R(%01x)", rs); return 2 | SUPPORTED; }
	case 0b0001'1000'0000'0000: { util::stream_format(stream, "DIV3"); return 2 | SUPPORTED; }
	case 0b0001'1100'0000'0000: stream << "<unhandled>"; return 2 | SUPPORTED;
	}
	return 2;
}

offs_t s1c33_disassembler::handle_100y_yyxx_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op)
{
	switch (op & 0b0000'0011'0000'0000)
	{
	case 0b0000'0000'0000'0000: return handle_100y_00xx_xxxx_xxxx(stream, pc, opcodes, params, op);
	case 0b0000'0001'0000'0000: return handle_100y_01xx_xxxx_xxxx(stream, pc, opcodes, params, op);
	case 0b0000'0010'0000'0000: return handle_100y_10xx_xxxx_xxxx(stream, pc, opcodes, params, op);
	case 0b0000'0011'0000'0000: return handle_100y_11xx_xxxx_xxxx(stream, pc, opcodes, params, op);
	}
	return 2;
}

offs_t s1c33_disassembler::handle_1000_00xx_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op)
{
	int imm10 = op & 0x03ff; util::stream_format(stream, "ADD SP, IMM10(%03x)", imm10); return 2 | SUPPORTED;
}

offs_t s1c33_disassembler::handle_1000_01xx_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op)
{
	int imm10 = op & 0x03ff; util::stream_format(stream, "SUB SP, IMM10(%03x)", imm10); return 2 | SUPPORTED;
}

offs_t s1c33_disassembler::handle_100x_xxxx_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op)
{
	switch (op & 0b0001'1100'0000'0000)
	{
	case 0b0000'0000'0000'0000: return handle_1000_00xx_xxxx_xxxx(stream, pc, opcodes, params, op);
	case 0b0000'0100'0000'0000: return handle_1000_01xx_xxxx_xxxx(stream, pc, opcodes, params, op);
	case 0b0000'1000'0000'0000: return handle_100y_yyxx_xxxx_xxxx(stream, pc, opcodes, params, op);
	case 0b0000'1100'0000'0000: return handle_100y_yyxx_xxxx_xxxx(stream, pc, opcodes, params, op);
	case 0b0001'0000'0000'0000: return handle_100y_yyxx_xxxx_xxxx(stream, pc, opcodes, params, op);
	case 0b0001'0100'0000'0000: return handle_100y_yyxx_xxxx_xxxx(stream, pc, opcodes, params, op);
	case 0b0001'1000'0000'0000: return handle_100y_yyxx_xxxx_xxxx(stream, pc, opcodes, params, op);
	case 0b0001'1100'0000'0000: return handle_100y_yyxx_xxxx_xxxx(stream, pc, opcodes, params, op);
	}
	return 2;
}

offs_t s1c33_disassembler::handle_101x_xx00_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op)
{
	switch (op & 0b0001'1100'0000'0000)
	{
	case 0b0000'0000'0000'0000: { int sd_rd = op & 0x000f; int rs_ss = (op & 0x00f0) >> 4; util::stream_format(stream, "LD.W S:R(%01x), R(%01x)", sd_rd, rs_ss); return 2 | SUPPORTED; }
	case 0b0000'0100'0000'0000: { int sd_rd = op & 0x000f; int rs_ss = (op & 0x00f0) >> 4; util::stream_format(stream, "LD.W R(%01x), S:R(%01x)", sd_rd, rs_ss); return 2 | SUPPORTED; }
	case 0b0000'1000'0000'0000: { int imm3 = op & 0x0007; int rb = (op & 0x00f0) >> 4; util::stream_format(stream, "BTST R(%01x), IMM3(%01x)", rb, imm3); return 2 | SUPPORTED; }
	case 0b0000'1100'0000'0000: { int imm3 = op & 0x0007; int rb = (op & 0x00f0) >> 4; util::stream_format(stream, "BCLR R(%01x), IMM3(%01x)", rb, imm3); return 2 | SUPPORTED; }
	case 0b0001'0000'0000'0000: { int imm3 = op & 0x0007; int rb = (op & 0x00f0) >> 4; util::stream_format(stream, "BSET R(%01x), IMM3(%01x)", rb, imm3); return 2 | SUPPORTED; }
	case 0b0001'0100'0000'0000: { int imm3 = op & 0x0007; int rb = (op & 0x00f0) >> 4; util::stream_format(stream, "BNOT R(%01x), IMM3(%01x)", rb, imm3); return 2 | SUPPORTED; }
	case 0b0001'1000'0000'0000: { int rd = op & 0x000f; int rs = (op & 0x00f0) >> 4; util::stream_format(stream, "ADC R(%01x), R(%01x)", rd, rs); return 2 | SUPPORTED; }
	case 0b0001'1100'0000'0000: { int rd = op & 0x000f; int rs = (op & 0x00f0) >> 4; util::stream_format(stream, "ABC R(%01x), R(%01x)", rd, rs); return 2 | SUPPORTED; }
	}
	return 2;
}

offs_t s1c33_disassembler::handle_101x_xx01_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op)
{
	switch (op & 0b0001'1100'0000'0000)
	{
	case 0b0000'0000'0000'0000: { int rd = op & 0x000f; int rs = (op & 0x00f0) >> 4; util::stream_format(stream, "LD.B R(%01x), R(%01x)", rd, rs); return 2 | SUPPORTED; }
	case 0b0000'0100'0000'0000: { int rd = op & 0x000f; int rs = (op & 0x00f0) >> 4; util::stream_format(stream, "LD.UB R(%01x), R(%01x)", rd, rs); return 2 | SUPPORTED; }
	case 0b0000'1000'0000'0000: { int rd = op & 0x000f; int rs = (op & 0x00f0) >> 4; util::stream_format(stream, "LD.H R(%01x), R(%01x)", rd, rs); return 2 | SUPPORTED; }
	case 0b0000'1100'0000'0000: { int rd = op & 0x000f; int rs = (op & 0x00f0) >> 4; util::stream_format(stream, "LD.UH R(%01x), R(%01x)", rd, rs); return 2 | SUPPORTED; }
	case 0b0001'0000'0000'0000: stream << "<unhandled 1011_0001_xxxx_xxxx>"; return 2 | SUPPORTED;
	case 0b0001'0100'0000'0000: stream << "<unhandled 1011_0101_xxxx_xxxx>"; return 2 | SUPPORTED;
	case 0b0001'1000'0000'0000: stream << "<unhandled 1011_1001_xxxx_xxxx>"; return 2 | SUPPORTED;
	case 0b0001'1100'0000'0000: stream << "<unhandled 1011_1101_xxxx_xxxx>"; return 2 | SUPPORTED;
	}
	return 2;
}

offs_t s1c33_disassembler::handle_101x_xx10_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op)
{
	switch (op & 0b0001'1100'0000'0000)
	{
	case 0b0000'0000'0000'0000: { int rd = op & 0x000f; int rs = (op & 0x00f0) >> 4; util::stream_format(stream, "MLT.H R(%01x), R(%01x)", rd, rs); return 2 | SUPPORTED; }
	case 0b0000'0100'0000'0000: { int rd = op & 0x000f; int rs = (op & 0x00f0) >> 4; util::stream_format(stream, "MLTU.H R(%01x), R(%01x)", rd, rs); return 2 | SUPPORTED; }
	case 0b0000'1000'0000'0000: { int rd = op & 0x000f; int rs = (op & 0x00f0) >> 4; util::stream_format(stream, "MLT.W R(%01x), R(%01x)", rd, rs); return 2 | SUPPORTED; }
	case 0b0000'1100'0000'0000: { int rd = op & 0x000f; int rs = (op & 0x00f0) >> 4; util::stream_format(stream, "MLTU.W R(%01x), R(%01x)", rd, rs); return 2 | SUPPORTED; }
	case 0b0001'0000'0000'0000: { int rs = (op & 0x00f0) >> 4; util::stream_format(stream, "MAC R(%01x)", rs); return 2 | SUPPORTED; }
	case 0b0001'0100'0000'0000: stream << "<unhandled 1011_0110_xxxx_xxxx>"; return 2 | SUPPORTED;
	case 0b0001'1000'0000'0000: stream << "<unhandled 1011_1010_xxxx_xxxx>"; return 2 | SUPPORTED;
	case 0b0001'1100'0000'0000: stream << "<unhandled 1011_1110_xxxx_xxxx>"; return 2 | SUPPORTED;
	}
	return 2;
}

offs_t s1c33_disassembler::handle_101x_xx11_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op)
{
	stream << "<unhandled 101x_xx11_xxxx_xxxx>";
	return 2 | SUPPORTED;
}

offs_t s1c33_disassembler::handle_101x_xxxx_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op)
{
	switch (op & 0b0000'0011'0000'0000)
	{
	case 0b0000'0000'0000'0000: return handle_101x_xx00_xxxx_xxxx(stream, pc, opcodes, params, op);
	case 0b0000'0001'0000'0000: return handle_101x_xx01_xxxx_xxxx(stream, pc, opcodes, params, op);
	case 0b0000'0010'0000'0000: return handle_101x_xx10_xxxx_xxxx(stream, pc, opcodes, params, op);
	case 0b0000'0011'0000'0000: return handle_101x_xx11_xxxx_xxxx(stream, pc, opcodes, params, op);
	}
	return 2;
}

offs_t s1c33_disassembler::handle_110x_xxxx_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op)
{
	int imm13 = op & 0x1fff; util::stream_format(stream, "EXT IMM13(%04x)", imm13); return 2 | SUPPORTED;
}

offs_t s1c33_disassembler::handle_111x_xxxx_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op)
{
	stream << "<unhandled 111x_xxxx_xxxx_xxxx>";
	return 2 | SUPPORTED;
}

offs_t s1c33_disassembler::disassemble(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params)
{
	u16 op = opcodes.r16(pc);
	switch (op & 0b1110'0000'0000'0000)
	{
	case 0b0000'0000'0000'0000: return handle_000x_xxxx_xxxx_xxxx(stream, pc, opcodes, params, op);
	case 0b0010'0000'0000'0000: return handle_001x_xxxx_xxxx_xxxx(stream, pc, opcodes, params, op);
	case 0b0100'0000'0000'0000: return handle_010x_xxxx_xxxx_xxxx(stream, pc, opcodes, params, op);
	case 0b0110'0000'0000'0000: return handle_011x_xxxx_xxxx_xxxx(stream, pc, opcodes, params, op);
	case 0b1000'0000'0000'0000: return handle_100x_xxxx_xxxx_xxxx(stream, pc, opcodes, params, op);
	case 0b1010'0000'0000'0000: return handle_101x_xxxx_xxxx_xxxx(stream, pc, opcodes, params, op);
	case 0b1100'0000'0000'0000: return handle_110x_xxxx_xxxx_xxxx(stream, pc, opcodes, params, op);
	case 0b1110'0000'0000'0000: return handle_111x_xxxx_xxxx_xxxx(stream, pc, opcodes, params, op);
	}
	return 2;
}
