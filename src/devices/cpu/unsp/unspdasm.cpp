// license:GPL-2.0+
// copyright-holders:Segher Boessenkool, David Haywood
/*****************************************************************************

    SunPlus µ'nSP disassembler

    Copyright 2008-2017  Segher Boessenkool  <segher@kernel.crashing.org>
    Licensed under the terms of the GNU GPL, version 2
    http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

*****************************************************************************/

#include "emu.h"
#include "unspdasm.h"

char const* const unsp_disassembler::regs[] =
{
	"sp", "r1", "r2", "r3", "r4", "bp", "sr", "pc"
};

char const* const unsp_disassembler::bitops[] =
{
	"tstb", "setb", "clrb", "invb"
};

char const* const unsp_disassembler::signmodes[] =
{
	"uu", "us", "su?", "ss" // su? might be invalid
};

char const* const unsp_disassembler::lsft[] =
{
	"asr", "asror", "lsl", "lslor", "lsr", "lsror", "rol", "ror"
};

char const* const unsp_disassembler::extregs[] =
{
	"r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
};


// TODO: use alu_op_start-like formatting
char const* const unsp_disassembler::aluops[] =
{
	"add","adc","sub","sbc","cmp","(invalid)","neg","--","xor","load","or","and","test","store","(invalid)","(invalid)"
};

char const* const unsp_disassembler::forms[] =
{
	"[%s]", "[%s--]", "[%s++]", "[++%s]"
};

u32 unsp_disassembler::opcode_alignment() const
{
	return 1;
}

void unsp_disassembler::print_alu_op_start(std::ostream &stream, uint8_t op0, uint8_t opA)
{
	static const char* const alu_op_start[] =
	{
		"%s += ", "%s += ", "%s -= ", "%s -= ",
		"cmp %s, ", "<BAD>", "%s =- ", "<BAD>",
		"%s ^= ", "%s = ", "%s |= ", "%s &= ",
		"test %s, "
	};

	util::stream_format(stream, alu_op_start[op0], regs[opA]);
}

void unsp_disassembler::print_alu_op3(std::ostream &stream, uint8_t op0, uint8_t opB)
{
	static const char* const alu_op3[] =
	{
		"%s + ", "%s + ", "%s - ", "%s - ",
		"cmp %s, ", "<BAD>", "-", "<BAD>",
		"%s ^ ", "", "%s | ", "%s & ",
		"test %s, "
	};

	util::stream_format(stream, alu_op3[op0], regs[opB]);
}

void unsp_disassembler::print_mul(std::ostream& stream, uint16_t op)
{
	int rs =   (op & 0x0007) >> 0;
	//         (op & 0x0078) >> 3; fixed 0001
	//         (op & 0x0080) >> 7; fixed 0
	int srd =  (op & 0x0100) >> 8;
	int rd =   (op & 0x0e00) >> 9;
	int srs =  (op & 0x1000) >> 12;
	//         (op & 0xe000) >> 13; fixed 111

	int sign = (srs << 1) | srd;

	util::stream_format(stream, "MR = %s*%s, %s", regs[rd], regs[rs], signmodes[sign] );
}

void unsp_disassembler::print_muls(std::ostream& stream, uint16_t op)
{
	int rs =   (op & 0x0007) >> 0;
	int size = (op & 0x0078) >> 3;
	//         (op & 0x0080) >> 7; fixed 1
	int srd =  (op & 0x0100) >> 8;
	int rd =   (op & 0x0e00) >> 9;
	int srs =  (op & 0x1000) >> 12;
	//         (op & 0xe000) >> 13; fixed 111

	int sign = (srs << 1) | srd;
	if (size == 0) size = 16;

	util::stream_format(stream, "MR = [%s]*[%s], %s, %d", regs[rd], regs[rs], signmodes[sign], size );
}


void unsp_disassembler::print_alu_op_end(std::ostream &stream, uint8_t op0)
{
	if (op0 == 1 || op0 == 3)
		util::stream_format(stream, ", carry");
}

void unsp_disassembler::print_indirect_op(std::ostream &stream, uint8_t opN, uint8_t opB)
{
	if (opN & 4)
		util::stream_format(stream, "ds:");
	util::stream_format(stream, forms[opN & 3], regs[opB]);
}



offs_t unsp_disassembler::disassemble_code(std::ostream &stream, offs_t pc, uint16_t op, uint16_t ximm, const data_buffer &opcodes)
{
	uint32_t len = 1;

	// all-zero and all-one are invalid insns:
	if (op == 0 || op == 0xffff)
	{
		util::stream_format(stream, "--");
		return UNSP_DASM_OK;
	}

	uint8_t op0 = (op >> 12);

	if (op0 == 0xf)
		return disassemble_fxxx_group(stream, pc, op, ximm, opcodes);

	uint8_t opA = (op >> 9) & 7;
	uint8_t op1 = (op >> 6) & 7;

	if (op0 < 15 && opA == 7 && op1 < 2)
		return disassemble_jumps(stream, pc, op);

	if (op0 == 0xe)
		return disassemble_exxx_group(stream, pc, op, ximm);

	return disassemble_remaining(stream, pc, op, ximm, opcodes);
}



offs_t unsp_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	uint16_t op = opcodes.r16(pc);
	uint16_t imm16 = opcodes.r16(pc+1);

	return disassemble_code(stream, pc, op, imm16, opcodes);
}
