// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Panasonic MN1880 family disassembler

    Due to the dearth of available documentation for the MN1880 series,
    its instruction list has been reconstructed from MN1870 and MN1890
    manuals, a (decidedly incomplete) list of MN1880 instructions not
    provided in the MN1870 series, and a few educated guesses based on
    reverse engineering (which include CALL XP actually being provided by
    the MN1880 despite the MN1870 manual claiming it as exclusive).

    Almost nothing about MN1860 is known except that it seems to be
    closely related to the MN1880. The MN1880 and MN1860 opcode maps are
    currently believed to be complete; it remains possible that a few
    rarely-used instructions are missing, or that some mnemonics were
    renamed.

    The direct flag (bit 5 of the flag status register) is the key to
    data memory addressing. When DF is cleared, the upper bytes of direct
    addresses are zeroed; when DF is set, the upper address bytes are
    taken from YPh for source operands and XPh for destination operands.
    MN1860 appears to have a somewhat more complex bankswitching model.

    Note that the REP opcode causes a temporary mode switch, decoding
    up to 15 extra sets of operands for the following instruction.
    REPEAT_COMBINED attempts to process this correctly by treating the
    full sequence as a single instruction, but this option has been
    disabled by default because it results in some repeated instructions
    being dozens of bytes long and the formatted output being difficult
    to read anyway since MAME requires it all be on a single line.

***************************************************************************/

#include "emu.h"
#include "mn1880d.h"

#define REPEAT_COMBINED 0

mn1880_disassembler::mn1880_disassembler(const char *const *inst_names)
	: util::disasm_interface()
	, m_inst_names(inst_names)
{
}

mn1880_disassembler::mn1880_disassembler()
	: mn1880_disassembler(s_inst_names)
{
}

// MOV, RDTBL, MOV1, MOV1N, ADDC, SUBC and NOT may also be known as MV, MVROM, MV1, MV1CPL, ADD, SUB and CPL
const char *const mn1880_disassembler::s_inst_names[256] =
{
	"nop", "rep", "rep", "rep", "rep", "rep", "rep", "rep",
	"rep", "rep", "rep", "rep", "rep", "rep", "rep", "rep",
	"clr", "clr", "clr", "clr", "clr", "clr", "clr", "clr",
	"set", "set", "set", "set", "set", "set", "set", "set",
	"t1bnz", "t1bnz", "t1bnz", "t1bnz", "t1bnz", "t1bnz", "t1bnz", "t1bnz",
	"t1bz", "t1bz", "t1bz", "t1bz", "t1bz", "t1bz", "t1bz", "t1bz",
	nullptr, "movl", nullptr, "movl", "movl", "movl", "mov", "mov",
	"movl", "movl", "movl", "movl", "asl", "asl", "asr", "asr",
	"dec", "dec", "not", "not", "cmpm", "cmpm", "xch4", "xch4",
	"inc", "inc", "clr", "clr", "rol", "rol", "ror", "ror",
	"cmpm", "div", "cmpm", "movda", "mov", "mov", "mov", "mov",
	"xch", "mul", "xch", nullptr, "movl", "movl", "movl", "movl",
	"cmp", "cmp", "cmp", "cmp", "and", "and", "and", "and",
	"xor", "xor", "xor", "xor", "or", "or", "or", "or",
	"subc", "subc", "subc", "subc", "subd", "subd", "subd", "subd",
	"addc", "addc", "addc", "addc", "addd", "addd", "addd", "addd",
	"skip", "bc", "bz", "ble", "cmpl", "clr", "cmpl", "clr",
	"br", "bnc", "bnz", "bgt", nullptr, "set", nullptr, "set",
	"call", "call", "call", "call", "call", "call", "call", "call",
	"call", "call", "call", "call", "call", "call", "call", "call",
	"br", "br", "br", "br", "br", "br", "br", "br",
	"br", "br", "br", "br", "br", "br", "br", "br",
	nullptr, "pop", nullptr, "pop", "pop", "pop", "pop", "pop",
	nullptr, "push", nullptr, "push", "push", "push", "push", "push",
	"subcl", "div", "subcl", nullptr, "xch", "xch", "xch", "xch",
	"addcl", "mul", "addcl", nullptr, "mov", "mov", "mov", "mov",
	"cmp", "cmp", "cmp", "cmp", "xch", "xch", nullptr, "xch",
	"mov", "mov", "mov", "mov", "mov", "mov", "mov", "mov",
	"loop", "rloop", "loop", "rloop", "dec", "inc", "dec", "inc",
	"addr", "addr", "addr", "addr", "addr", "addr", "addr", "addr",
	"cmpbne", "cmpbne", "mov1", "wait", "ret", "reti", "br", "call",
	"cmpbe", "cmpbe", "mov1n", "push", "br", "call", "rdtbl", "pi"
};

mn1870_disassembler::mn1870_disassembler()
	: mn1880_disassembler(s_inst_names)
{
}

// STOP and HALT appear to be macros each implemented as a SET followed by a T1BNZ
// (their machine language codes are given as 19 16 21 16 FD and 18 16 20 16 FD)
const char *const mn1870_disassembler::s_inst_names[256] =
{
	"nop", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"clr", "clr", "clr", "clr", "clr", "clr", "clr", "clr",
	"set", "set", "set", "set", "set", "set", "set", "set",
	"t1bnz", "t1bnz", "t1bnz", "t1bnz", "t1bnz", "t1bnz", "t1bnz", "t1bnz",
	"t1bz", "t1bz", "t1bz", "t1bz", "t1bz", "t1bz", "t1bz", "t1bz",
	nullptr, "movl", nullptr, "movl", nullptr, nullptr, "mov", "mov",
	"movl", "movl", "movl", "movl", nullptr, nullptr, nullptr, nullptr,
	"dec", "dec", "not", "not", "cmpm", "cmpm", "xch4", "xch4",
	"inc", "inc", "clr", "clr", "rol", "rol", "ror", "ror",
	"cmpm", "div", "cmpm", nullptr, "mov", "mov", "mov", "mov",
	"xch", "mul", "xch", nullptr, "movl", "movl", "movl", "movl",
	"cmp", "cmp", "cmp", "cmp", "and", "and", "and", "and",
	"xor", "xor", "xor", "xor", "or", "or", "or", "or",
	"subc", "subc", "subc", "subc", "subd", "subd", "subd", "subd",
	"addc", "addc", "addc", "addc", "addd", "addd", "addd", "addd",
	"skip", "bc", "bz", "ble", "cmpl", "clr", nullptr, "clr",
	"br", "bnc", "bnz", "bgt", nullptr, "set", nullptr, "set",
	"call", "call", "call", "call", "call", "call", "call", "call",
	"call", "call", "call", "call", "call", "call", "call", "call",
	"br", "br", "br", "br", "br", "br", "br", "br",
	"br", "br", "br", "br", "br", "br", "br", "br",
	"clr", "pop", "t1bnz", "pop", "pop", "pop", "pop", "pop",
	"set", "push", "t1bz", "push", "push", "push", "push", "push",
	nullptr, "div", nullptr, "decl", nullptr, nullptr, nullptr, nullptr,
	nullptr, "mul", nullptr, "incl", "mov", "mov", "mov", "mov",
	nullptr, "cmp", nullptr, "cmp", nullptr, "xch", nullptr, "xch",
	"mov", "mov", "mov", "mov", nullptr, nullptr, nullptr, nullptr,
	"loop", "rloop", "loop", "rloop", "dec", "inc", "dec", "inc",
	nullptr, "addr", nullptr, "addr", nullptr, "addr", nullptr, "addr",
	"cmpbne", "cmpbne", "mov1", nullptr, "ret", "reti", "br", "call",
	"cmpbe", "cmpbe", "mov1n", "push", "br", "call", "rdtbl", nullptr
};

mn1860_disassembler::mn1860_disassembler()
	: mn1880_disassembler(s_inst_names)
{
}

// MN1860 probably includes most if not all MN1880 opcodes
const char *const mn1860_disassembler::s_inst_names[256] =
{
	"nop", "rep", "rep", "rep", "rep", "rep", "rep", "rep",
	"rep", "rep", "rep", "rep", "rep", "rep", "rep", "rep",
	"clr", "clr", "clr", "clr", "clr", "clr", "clr", "clr",
	"set", "set", "set", "set", "set", "set", "set", "set",
	"t1bnz", "t1bnz", "t1bnz", "t1bnz", "t1bnz", "t1bnz", "t1bnz", "t1bnz",
	"t1bz", "t1bz", "t1bz", "t1bz", "t1bz", "t1bz", "t1bz", "t1bz",
	"addrl", "movl", "addrl", "movl", "movl", "movl", "mov", "mov",
	"movl", "movl", "movl", "movl", "asl", "asl", "asr", "asr",
	"dec", "dec", "not", "not", "cmpm", "cmpm", "xch4", "xch4",
	"inc", "inc", "clr", "clr", "rol", "rol", "ror", "ror",
	"cmpm", "div", "cmpm", "movda", "mov", "mov", "mov", "mov",
	"xch", "mul", "xch", "movda", "movl", "movl", "movl", "movl",
	"cmp", "cmp", "cmp", "cmp", "and", "and", "and", "and",
	"xor", "xor", "xor", "xor", "or", "or", "or", "or",
	"subc", "subc", "subc", "subc", "subd", "subd", "subd", "subd",
	"addc", "addc", "addc", "addc", "addd", "addd", "addd", "addd",
	"skip", "bc", "bz", "ble", "cmpl", "clr", "cmpl", "clr",
	"br", "bnc", "bnz", "bgt", "cmpl", "set", "cmpl", "set",
	"call", "call", "call", "call", "call", "call", "call", "call",
	"call", "call", "call", "call", "call", "call", "call", "call",
	"br", "br", "br", "br", "br", "br", "br", "br",
	"br", "br", "br", "br", "br", "br", "br", "br",
	"roll", "asll", "divl", "pop", "pop", "pop", "pop", "pop",
	"rorl", "asrl", "mull", "push", "push", "push", "push", "push",
	"subcl", "div", "subcl", "subcl", "xch", "xch", "xch", "xch",
	"addcl", "mul", "addcl", "addcl", "mov", "mov", "mov", "mov",
	"cmp", "cmp", "cmp", "cmp", "xch", "xch", nullptr, "xch",
	"mov", "mov", "mov", "mov", "mov", "mov", "mov", "mov",
	"loop", "rloop", "loop", "rloop", "dec", "inc", "dec", "inc",
	"addr", "addr", "addr", "addr", "addr", "addr", "addr", "addr",
	"cmpbne", "cmpbne", "mov1", "wait", "ret", "reti", "br", "call",
	"cmpbe", "cmpbe", "mov1n", "push", "br", "call", "rdtbl", "pi"
};

u32 mn1880_disassembler::opcode_alignment() const
{
	return 1;
}

void mn1880_disassembler::format_direct(std::ostream &stream, u8 da) const
{
	util::stream_format(stream, "(x'%02X')", da);
}

void mn1880_disassembler::format_direct16(std::ostream &stream, u16 da) const
{
	util::stream_format(stream, "(x'%04X')", da);
}

void mn1880_disassembler::format_direct_bp(std::ostream &stream, u8 da, u8 bp) const
{
	util::stream_format(stream, "(x'%02X')%d", da, bp);
}

void mn1880_disassembler::format_indirect_bp(std::ostream &stream, const char *ptr, u8 bp) const
{
	util::stream_format(stream, "(%s)%d", ptr, bp);
}

void mn1880_disassembler::format_direct_masked(std::ostream &stream, u8 da, u8 mask) const
{
	util::stream_format(stream, "(x'%02X')x'%02X'", da, mask);
}

void mn1880_disassembler::format_indirect_masked(std::ostream &stream, const char *ptr, u8 mask) const
{
	util::stream_format(stream, "(%s)x'%02X'", ptr, mask);
}

void mn1880_disassembler::format_imm(std::ostream &stream, u8 imm) const
{
	util::stream_format(stream, "x'%02X'", imm);
}

void mn1880_disassembler::format_imm4(std::ostream &stream, u8 imm) const
{
	util::stream_format(stream, "%d", imm & 0x0f);
}

void mn1880_disassembler::format_imm16(std::ostream &stream, u16 imm) const
{
	util::stream_format(stream, "x'%04X'", imm);
}

void mn1880_disassembler::format_rel(std::ostream &stream, u16 base, s8 disp) const
{
	util::stream_format(stream, "$x'%04X'", (base + disp) & 0xffff);
}

void mn1880_disassembler::format_abs4k(std::ostream &stream, u16 label) const
{
	util::stream_format(stream, "x'%04X'", label);
}

void mn1880_disassembler::format_abs64k(std::ostream &stream, u16 label) const
{
	util::stream_format(stream, "/x'%04X'", label);
}

void mn1880_disassembler::dasm_operands(std::ostream &stream, u8 opcode, offs_t &pc, offs_t &flags, const mn1880_disassembler::data_buffer &opcodes)
{
	if (opcode < 0x10)
	{
		// REP
		format_imm4(stream, opcode);
	}
	else if (opcode < 0x30)
	{
		// CLR, SET, T1BNZ, T1BZ direct
		format_direct_bp(stream, opcodes.r8(pc++), opcode & 0x07);
		if (opcode >= 0x20)
		{
			stream << ", ";
			format_rel(stream, pc + 1, opcodes.r8(pc));
			++pc;
			flags |= STEP_COND;
		}
	}
	else if (opcode < 0x34)
	{
		util::stream_format(stream, "%s, ", BIT(opcode, 1) ? "yp" : "xp");
		if (BIT(opcode, 0))
		{
			// MOVL immediate to pointer register
			format_imm16(stream, swapendian_int16(opcodes.r16(pc)));
			pc += 2;
		}
		else
		{
			// ADDRL direct + immediate? (MN1860 only)
			format_direct(stream, opcodes.r8(pc + 1));
			stream << ", ";
			format_imm16(stream, u16(opcodes.r8(pc + 2)) << 8 | opcodes.r8(pc));
			pc += 3;
		}
	}
	else if (opcode < 0x3c)
	{
		// MOV/MOVL direct
		if (BIT(opcode, 0))
		{
			format_direct(stream, opcodes.r8(pc++));
			stream << ", ";
		}
		if (opcode < 0x38)
			util::stream_format(stream, "(%s)", BIT(opcode, 0) ? "yp" : "xp");
		else if (BIT(opcode, 1))
			stream << "yp";
		else
			stream << "xp";
		if (!BIT(opcode, 0))
		{
			stream << ", ";
			format_direct(stream, opcodes.r8(pc++));
		}
	}
	else if ((opcode & 0xfe) == 0x44)
	{
		// CMPM immediate
		if (BIT(opcode, 0))
		{
			format_direct_masked(stream, opcodes.r8(pc), opcodes.r8(pc + 1));
			pc += 2;
		}
		else
			format_indirect_masked(stream, "xp", opcodes.r8(pc++));
		stream << ", ";
		format_imm(stream, opcodes.r8(pc++));
	}
	else if (opcode < 0x50 || (opcode & 0xf7) == 0xb5 || ((opcode & 0xf6) == 0xb0 && m_inst_names[opcode][3] == 'l') || ((opcode & 0xf7) == 0xc3 && m_inst_names[opcode][4] == '\0'))
	{
		// Unary operations
		if (BIT(opcode, 0) || opcode >= 0xb0)
			format_direct(stream, opcodes.r8(pc++));
		else
			stream << "(xp)";
	}
	else if ((opcode & 0xfd) == 0x50)
	{
		// CMPM with two memory operands
		if (BIT(opcode, 1))
		{
			u8 da2 = opcodes.r8(pc++);
			u8 mask = opcodes.r8(pc++);
			format_direct_masked(stream, opcodes.r8(pc++), mask);
			stream << ", ";
			format_direct_masked(stream, da2, mask);
		}
		else
		{
			u8 mask = opcodes.r8(pc++);
			format_indirect_masked(stream, "xp", mask);
			stream << ", ";
			format_indirect_masked(stream, "yp", mask);
		}
	}
	else if ((opcode & 0xf7) == 0x51 || (opcode & 0xf7) == 0xb7)
	{
		// MUL, DIV, POP, PUSH indirect
		stream << "(xp)";
	}
	else if (opcode == 0x53)
	{
		// MOVDA (MN1880, MN1860 only)
		format_direct16(stream, opcodes.r16(pc + 2));
		stream << ", ";
		format_direct16(stream, opcodes.r16(pc));
		pc += 4;
	}
	else if (opcode == 0x5b)
	{
		// MOVDA with immediate source? (MN1860 only)
		format_direct16(stream, opcodes.r16(pc + 1));
		stream << ", ";
		format_imm(stream, opcodes.r8(pc));
		pc += 3;
	}
	else if (opcode < 0x80 || (opcode & 0xfd) == 0x84 || (opcode & 0xfd) == 0x8c || (opcode & 0xf4) == 0xc0)
	{
		// Binary operations
		if (BIT(opcode, 1))
		{
			// Direct modes
			u8 op2 = opcodes.r8(pc++);
			format_direct(stream, opcodes.r8(pc++));
			stream << ", ";
			if (opcode == 0x5f || opcode == 0x8e || (opcode & 0xf7) == 0xc3)
				format_imm16(stream, op2 | u16(opcodes.r8(pc++)) << 8);
			else if (BIT(opcode, 0))
			{
				if ((opcode & 0xf4) == 0x74)
					format_imm4(stream, op2);
				else
					format_imm(stream, op2);
			}
			else
				format_direct(stream, op2);
		}
		else
		{
			// Indirect modes
			stream << "(xp), ";
			if (opcode == 0x5d || opcode == 0x8c)
			{
				format_imm16(stream, swapendian_int16(opcodes.r16(pc)));
				pc += 2;
			}
			else if (BIT(opcode, 0))
			{
				if ((opcode & 0xf4) == 0x74)
					format_imm4(stream, opcodes.r8(pc++));
				else
					format_imm(stream, opcodes.r8(pc++));
			}
			else
				stream << "(yp)";
		}
	}
	else if (opcode < 0x90 || (opcode & 0xfc) == 0xe0)
	{
		if (BIT(opcode, 2))
		{
			if (BIT(opcode, 0))
			{
				// CLR, SET special flag
				if (BIT(opcode, 1))
					stream << "cf";
				else
					stream << "df";
			}
			else
				stream << "(unknown)";
		}
		else
		{
			// Relative branches and loops
			if ((opcode & 0xfd) == 0xe1)
				util::stream_format(stream, "%s, ", BIT(opcode, 1) ? "yp" : "xp");
			format_rel(stream, pc + 1, opcodes.r8(pc));
			++pc;
			if ((opcode & 0xf7) != 0x80)
				flags |= STEP_COND;
		}
	}
	else if (opcode < 0xb0)
	{
		// CALL, BR within 4K page
		format_abs4k(stream, ((pc + 1) & 0xf000) | u16(opcode & 0x0f) << 8 | opcodes.r8(pc));
		++pc;
		if (opcode < 0xa0)
			flags |= STEP_OVER;
	}
	else if ((opcode & 0xf4) == 0xb0)
	{
		if (BIT(opcode, 0))
			stream << "fs";
		else if (m_inst_names[opcode][0] == 't' || m_inst_names[opcode][3] == '\0')
		{
			// CLR, SET, T1BNZ, T1BZ indirect (MN1870 only)
			format_indirect_bp(stream, "xp", opcodes.r8(pc++) & 0x07);
			if (BIT(opcode, 1))
			{
				stream << ", ";
				format_rel(stream, pc + 1, opcodes.r8(pc));
				++pc;
				flags |= STEP_COND;
			}
		}
		else
		{
			// 3 direct operands? (MN1860 only)
			format_direct(stream, opcodes.r8(pc + 2));
			stream << ", ";
			format_direct(stream, opcodes.r8(pc));
			stream << ", ";
			format_direct(stream, opcodes.r8(pc + 1));
			pc += 3;
		}
	}
	else if (opcode < 0xc0 || (opcode & 0xfc) == 0xe4 || (opcode & 0xfe) == 0xfc)
	{
		// PUSH, POP, DEC, INC, BR, CALL pointer
		if (BIT(opcode, 1))
			stream << "yp";
		else
			stream << "xp";
		if (opcode == 0xfd)
			flags |= STEP_OVER;
	}
	else if (opcode < 0xd0)
	{
		// XCH, MOV lower half of register
		if ((opcode & 0xf5) == 0xc5)
		{
			util::stream_format(stream, "%sl, ", BIT(opcode, 1) ? "yp" : "xp");
			format_direct(stream, opcodes.r8(pc++));
		}
		else if (BIT(opcode, 3))
		{
			format_direct(stream, opcodes.r8(pc++));
			util::stream_format(stream, ", %sl", BIT(opcode, 1) ? "yp" : "xp");
		}
		else
			util::stream_format(stream, "%sl, %sh", BIT(opcode, 1) ? "yp" : "xp", BIT(opcode, 1) ? "yp" : "xp");
	}
	else if (opcode < 0xe0)
	{
		if (BIT(opcode, 2))
		{
			// XCH, MOV between pointer registers
			if (opcode == 0xd4)
				stream << "xp, lp";
			else
			{
				if (BIT(opcode, 0))
					stream << "xp, ";
				if (BIT(opcode, 1))
					stream << "yp";
				else
					stream << "sp";
				if (!BIT(opcode, 0))
					stream << ", xp";
			}
		}
		else
		{
			// CMP, MOV 8-bit
			util::stream_format(stream, "%s%c, ", BIT(opcode, 1) ? "yp" : "xp", (opcode & 0xfd) == 0xd9 ? 'h' : 'l');
			if ((opcode & 0xfd) == 0xd0)
				format_direct(stream, opcodes.r8(pc++));
			else
				format_imm(stream, opcodes.r8(pc++));
		}
	}
	else if (opcode < 0xf0)
	{
		// ADDR
		util::stream_format(stream, "%sl, ", BIT(opcode, 1) ? "yp" : "xp");
		if (BIT(opcode, 2))
		{
			util::stream_format(stream, "%sl, ", BIT(opcode, 1) ? "yp" : "xp");
			if (BIT(opcode, 0))
				format_imm(stream, opcodes.r8(pc++));
			else
				util::stream_format(stream, "%sh", BIT(opcode, 1) ? "yp" : "xp");
		}
		else
		{
			u8 op2 = opcodes.r8(pc++);
			format_direct(stream, opcodes.r8(pc++));
			stream << ", ";
			if (BIT(opcode, 0))
				format_imm(stream, op2);
			else
				format_direct(stream, op2);
		}
	}
	else if ((opcode & 0xf6) == 0xf0)
	{
		// CMPBNE, CMPBE
		u8 imm = opcodes.r8(pc++);
		if (BIT(opcode, 0))
			format_direct(stream, opcodes.r8(pc++));
		else
			stream << "(xp)";
		stream << ", ";
		format_imm(stream, imm);
		stream << ", ";
		format_rel(stream, pc + 1, opcodes.r8(pc));
		++pc;
		flags |= STEP_COND;
	}
	else if ((opcode & 0xf7) == 0xf2)
	{
		// MOV1, MOV1N
		u8 da2 = opcodes.r8(pc++);
		u8 bp = opcodes.r8(pc++);
		format_direct_bp(stream, opcodes.r8(pc++), (bp & 0x70) >> 4);
		stream << ", ";
		format_direct_bp(stream, da2, bp & 0x07);
	}
	else if (opcode == 0xf3)
	{
		// WAIT (MN1880 only)
		format_imm16(stream, opcodes.r16(pc));
		pc += 2;
	}
	else if ((opcode & 0xfe) == 0xf6)
	{
		// BR, CALL absolute
		format_abs64k(stream, opcodes.r16(pc));
		pc += 2;
		if (BIT(opcode, 0))
			flags |= STEP_OVER;
	}
	else if (opcode == 0xfb)
	{
		// PUSH immediate
		format_imm(stream, opcodes.r8(pc++));
	}
	else if (opcode == 0xfe)
	{
		// RDTBL
		stream << "(xp), (yp)";
	}
	else
		stream << "(unknown)";
}

offs_t mn1880_disassembler::disassemble(std::ostream &stream, offs_t pc, const mn1880_disassembler::data_buffer &opcodes, const mn1880_disassembler::data_buffer &params)
{
	const offs_t pc0 = pc;
	offs_t flags = SUPPORTED;

	u8 opcode = opcodes.r8(pc++);
	const char *mnemonic = m_inst_names[opcode];

	u8 rc = 0;
	if (REPEAT_COMBINED && mnemonic != nullptr && opcode > 0x00 && opcode < 0x10)
	{
		rc = opcode;
		opcode = opcodes.r8(pc++);
		mnemonic = m_inst_names[opcode];
	}

	if (mnemonic == nullptr)
	{
		util::stream_format(stream, "%-8s", "db");
		format_imm(stream, opcode);
	}
	else if (opcode == 0x00 || opcode == 0x80)
	{
		// NOP, SKIP (latter actually executes in immediate mode)
		stream << mnemonic;
	}
	else if (opcode == 0xff)
	{
		// PI (software interrupt)
		stream << mnemonic;
		flags |= STEP_OVER;
	}
	else if ((opcode & 0xfe) == 0xf4)
	{
		// RET, RETI
		stream << mnemonic;
		flags |= STEP_OUT;
	}
	else
	{
		util::stream_format(stream, "%-8s", mnemonic);
		offs_t pc1 = pc;
		dasm_operands(stream, opcode, pc, flags, opcodes);
		if (REPEAT_COMBINED && rc != 0 && pc1 != pc)
		{
			do {
				stream << ", ";
				dasm_operands(stream, opcode, pc, flags, opcodes);
			}
			while (--rc != 0);
		}
	}

	if (REPEAT_COMBINED && rc != 0)
		util::stream_format(stream, ", rep %d", rc);

	return (pc - pc0) | flags;
}
