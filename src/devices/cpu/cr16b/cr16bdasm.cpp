// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    National Semiconductor CompactRISC CR16B disassembler

    Core architecture versions supported by this disassembler:
    * CR16A: 17-bit PC, 18-bit address space
    * CR16B: 21-bit PC with large memory model, enhanced instruction set

    CR16C has a 24-bit PC and expands the instruction set still further.
    Its instruction encoding is entirely different and is therefore not
    supported here.

    It should be noted that the PC is guaranteed to be always aligned to
    even addresses. To this end, whenever the PC is stored in or loaded
    from a register or register pair or a stack, the bits are shifted
    right or left by one.

***************************************************************************/

#include "util/disasmintf.h"
#include "cr16bdasm.h"

#include "util/strformat.h"

using osd::u32;
using util::BIT;
using offs_t = u32;

cr16b_disassembler::cr16b_disassembler(cr16_arch arch)
	: util::disasm_interface()
	, m_arch(arch)
{
}

cr16a_disassembler::cr16a_disassembler()
	: cr16b_disassembler(cr16_arch::CR16A)
{
}

cr16b_disassembler::cr16b_disassembler()
	: cr16b_disassembler(cr16_arch::CR16B)
{
}

u32 cr16b_disassembler::opcode_alignment() const
{
	return 2;
}

// Condition codes
const char *const cr16b_disassembler::s_cc[14] =
{
	"eq", "ne",
	"cs", "cc",
	"hi", "ls",
	"gt", "le",
	"fs", "fc",
	"lo", "hs",
	"lt", "ge"
};


void cr16b_disassembler::format_reg(std::ostream &stream, u8 reg)
{
	if (reg == 15)
		stream << "sp";
	else if (reg == 14)
		stream << "ra";
	else if (reg == 13 && m_arch != cr16_arch::CR16A)
		stream << "era"; // R13 in SMM
	else
		util::stream_format(stream, "r%d", reg);
}

void cr16b_disassembler::format_rpair(std::ostream &stream, u8 reg)
{
	if (reg == 13 && m_arch != cr16_arch::CR16A)
		stream << "(ra,era)";
	else
		util::stream_format(stream, "(r%d,r%d)", reg + 1, reg);
}

void cr16a_disassembler::format_rproc(std::ostream &stream, u8 reg)
{
	switch (reg)
	{
	case 1:
		stream << "psr";
		break;

	case 3:
		stream << "intbase";
		break;

	case 11:
		stream << "isp";
		break;

	default:
		stream << "res";
		break;
	}
}

void cr16b_disassembler::format_rproc(std::ostream &stream, u8 reg)
{
	switch (reg)
	{
	case 1:
		stream << "psr";
		break;

	case 3:
		stream << "intbasel";
		break;

	case 4:
		stream << "intbaseh";
		break;

	case 5:
		stream << "cfg";
		break;

	case 7:
		stream << "dsr";
		break;

	case 9:
		stream << "dcr";
		break;

	case 11:
		stream << "isp";
		break;

	case 13:
		stream << "carl";
		break;

	case 14:
		stream << "carh";
		break;

	default:
		stream << "res";
		break;
	}
}

void cr16b_disassembler::format_short_imm(std::ostream &stream, u8 imm)
{
	// 5-bit short immediate value (0 to 15, -16, -14 to -1)
	if (imm == 0)
		stream << "$0";
	else if (imm >= 0x10)
		util::stream_format(stream, "$-0x%X", 0x10 - (imm & 0x0f));
	else
		util::stream_format(stream, "$0x%X", imm);
}

void cr16b_disassembler::format_short_imm_unsigned(std::ostream &stream, u8 imm, bool i)
{
	if (imm == 0)
		stream << "$0";
	else if (i)
		util::stream_format(stream, "$0x%04X", (imm >= 0x10) ? 0xfff0 | (imm & 0x0f) : imm);
	else
		util::stream_format(stream, "$0x%02X", (imm >= 0x10) ? 0xf0 | (imm & 0x0f) : imm);
}

void cr16b_disassembler::format_short_imm_decimal(std::ostream &stream, u8 imm)
{
	if (imm >= 0x10)
		util::stream_format(stream, "$-%d", 0x10 - (imm & 0x0f));
	else
		util::stream_format(stream, "$%d", imm);
}

void cr16b_disassembler::format_medium_imm(std::ostream &stream, u16 imm)
{
	if (imm >= 0x8000)
		util::stream_format(stream, "$-0x%X", 0x10000 - imm);
	else
		util::stream_format(stream, "$0x%X", imm);
}

void cr16b_disassembler::format_medium_imm_unsigned(std::ostream &stream, u16 imm, bool i)
{
	if (i)
		util::stream_format(stream, "$0x%04X", imm);
	else
		util::stream_format(stream, "$0x%02X", imm & 0xff);
}

void cr16b_disassembler::format_medium_imm_decimal(std::ostream &stream, u16 imm)
{
	util::stream_format(stream, "$%d", s16(imm));
}

void cr16b_disassembler::format_imm21(std::ostream &stream, u32 imm)
{
	util::stream_format(stream, "$0x%06X", imm);
}

void cr16b_disassembler::format_disp5(std::ostream &stream, u8 disp)
{
	// 5-bit short displacement (0 to 15, 16 to 30 even)
	if (disp == 0)
		stream << "0";
	else
		util::stream_format(stream, "0x%X", disp);
}

void cr16b_disassembler::format_disp16(std::ostream &stream, u16 disp)
{
	// 16-bit displacement (0 to 64K-1)
	util::stream_format(stream, "0x%X", disp);
}

void cr16b_disassembler::format_disp18(std::ostream &stream, u32 disp)
{
	// 18-bit medium displacement (-128K to 128K-1 or 0 to 256K-1, result truncated to 18 bits)
	if (disp == 0)
		stream << "0";
	else if (disp >= 0x20000)
		util::stream_format(stream, "-0x%X", 0x40000 - disp);
	else
		util::stream_format(stream, "0x%X", disp);
}

void cr16b_disassembler::format_abs18(std::ostream &stream, u32 addr)
{
	// 18-bit absolute (any memory location within first 256K)
	util::stream_format(stream, "0x%05X", addr);
}

void cr16b_disassembler::format_pc_disp5(std::ostream &stream, offs_t pc, u8 disp)
{
	if (m_arch == cr16_arch::CR16A)
		util::stream_format(stream, "0x%05X", (disp >= 0x10 ? pc + 0x20 - disp : pc + disp) & 0x1ffff); // SMM
	else
		util::stream_format(stream, "0x%06X", (disp >= 0x10 ? pc + 0x20 - disp : pc + disp) & 0x1fffff); // LMM
}

void cr16b_disassembler::format_pc_disp9(std::ostream &stream, offs_t pc, u16 disp)
{
	if (m_arch == cr16_arch::CR16A)
		util::stream_format(stream, "0x%05X", (disp >= 0x100 ? pc + 0x200 - disp : pc + disp) & 0x1ffff); // SMM
	else
		util::stream_format(stream, "0x%06X", (disp >= 0x100 ? pc + 0x200 - disp : pc + disp) & 0x1fffff); // LMM
}

void cr16b_disassembler::format_pc_disp17(std::ostream &stream, offs_t pc, u32 disp)
{
	util::stream_format(stream, "0x%05X", (disp >= 0x10000 ? pc + 0x20000 - disp : pc + disp) & 0x1ffff);
}

void cr16b_disassembler::format_pc_disp21(std::ostream &stream, offs_t pc, u32 disp)
{
	util::stream_format(stream, "0x%06X", (pc + ((disp & 0x0ffffe) | (disp & 0x000001) << 20)) & 0x1fffff);
}

void cr16b_disassembler::format_excp_vector(std::ostream &stream, u8 vec)
{
	switch (vec)
	{
	case 0x05: // Supervisor call
		stream << "svc";
		break;

	case 0x06: // Division by zero
		stream << "dvz";
		break;

	case 0x07: // Flag
		stream << "flg";
		break;

	case 0x08: // Breakpoint
		stream << "bpt";
		break;

	case 0x0a: // Undefined instruction
		stream << "und";
		break;

	case 0x0e: // Debug
		stream << (m_arch != cr16_arch::CR16A ? "dbg" : "und ; reserved");
		break;

	default:
		stream << "und ; reserved";
		break;
	}
}

offs_t cr16b_disassembler::disassemble(std::ostream &stream, offs_t pc, const cr16b_disassembler::data_buffer &opcodes, const cr16b_disassembler::data_buffer &params)
{
	u16 opcode = opcodes.r16(pc);

	if (BIT(opcode, 15))
	{
		// Load and store group (excluding LOADM, STORM)
		if (BIT(opcode, 14))
		{
			util::stream_format(stream, "stor%c   ", BIT(opcode, 13) ? 'w' : 'b');
			format_reg(stream, (opcode & 0x01e0) >> 5);
			stream << ", ";
		}
		else
			util::stream_format(stream, "load%c   ", BIT(opcode, 13) ? 'w' : 'b');

		switch (opcode & 0x1801)
		{
		case 0x1001:
			format_disp18(stream, u32(opcode & 0x0600) << 7 | opcodes.r16(pc + 2));
			stream << "(";
			format_reg(stream, (opcode & 0x001e) >> 1);
			stream << ")";
			if (!BIT(opcode, 14))
			{
				stream << ", ";
				format_reg(stream, (opcode & 0x01e0) >> 5);
			}
			return 4 | SUPPORTED;

		case 0x1801:
			if ((opcode & 0x001e) == 0x001e)
				format_abs18(stream, u32(opcode & 0x0600) << 7 | opcodes.r16(pc + 2));
			else
			{
				format_disp18(stream, u32(opcode & 0x0600) << 7 | opcodes.r16(pc + 2));
				format_rpair(stream, (opcode & 0x001e) >> 1);
			}
			if (!BIT(opcode, 14))
			{
				stream << ", ";
				format_reg(stream, (opcode & 0x01e0) >> 5);
			}
			return 4 | SUPPORTED;

		default:
			format_disp5(stream, (opcode & 0x1e00) >> 8 | (opcode & 0x0001));
			stream << "(";
			format_reg(stream, (opcode & 0x001e) >> 1);
			stream << ")";
			if (!BIT(opcode, 14))
			{
				stream << ", ";
				format_reg(stream, (opcode & 0x01e0) >> 5);
			}
			return 2 | SUPPORTED;
		}
	}
	else switch (opcode & 0x7e01)
	{
	case 0x0000: case 0x0001: case 0x2000: case 0x2001:
		util::stream_format(stream, "add%c    ", BIT(opcode, 13) ? 'w' : 'b');
		if ((opcode & 0x001f) == 0x0011)
		{
			format_medium_imm(stream, opcodes.r16(pc + 2));
			stream << ", ";
			format_reg(stream, (opcode & 0x01e0) >> 5);
			return 4 | SUPPORTED;
		}
		else
		{
			format_short_imm(stream, opcode & 0x001f);
			stream << ", ";
			format_reg(stream, (opcode & 0x01e0) >> 5);
			return 2 | SUPPORTED;
		}

	case 0x0200: case 0x0201: case 0x2200: case 0x2201:
		if (opcode == 0x0200) // NOP = ADDU $0, R0
			stream << "nop";
		else
		{
			util::stream_format(stream, "addu%c   ", BIT(opcode, 13) ? 'w' : 'b');
			if ((opcode & 0x001f) == 0x0011)
			{
				format_medium_imm(stream, opcodes.r16(pc + 2));
				stream << ", ";
				format_reg(stream, (opcode & 0x01e0) >> 5);
				return 4 | SUPPORTED;
			}
			else
			{
				format_short_imm(stream, opcode & 0x001f);
				stream << ", ";
				format_reg(stream, (opcode & 0x01e0) >> 5);
				return 2 | SUPPORTED;
			}
		}
		return 2 | SUPPORTED;

	case 0x0400: case 0x0401: case 0x2400: case 0x2401:
	case 0x4401: case 0x6401:
		// Bit manipulation and store immediate (memory operand)
		if (m_arch == cr16_arch::CR16A)
		{
			stream << "res";
			return 2 | SUPPORTED;
		}
		switch (opcode & 0x00c0)
		{
		case 0x0000:
			util::stream_format(stream, "cbit%c   $%d", BIT(opcode, 13) ? 'w' : 'b', (opcode & 0x001e) >> 1);
			break;

		case 0x0040:
			util::stream_format(stream, "sbit%c   $%d", BIT(opcode, 13) ? 'w' : 'b', (opcode & 0x001e) >> 1);
			break;

		case 0x0080:
			util::stream_format(stream, "tbit%c   $%d", BIT(opcode, 13) ? 'w' : 'b', (opcode & 0x001e) >> 1);
			break;

		case 0x00c0:
			util::stream_format(stream, "stor%c   ", BIT(opcode, 13) ? 'w' : 'b');
			format_short_imm(stream, (opcode & 0x001e) >> 1); // unsigned 4-bit value
			break;
		}
		if (BIT(opcode, 14))
		{
			stream << ", 0(";
			format_reg(stream, (opcode & 0x0120) >> 5);
			stream << ")";
			return 2 | SUPPORTED;
		}
		else
		{
			stream << ", ";
			if (BIT(opcode, 0))
			{
				format_disp16(stream, opcodes.r16(pc + 2));
				stream << "(";
				format_reg(stream, (opcode & 0x0120) >> 5);
				stream << ")";
			}
			else
				format_abs18(stream, u32(opcode & 0x0100) << 9 | u32(opcode & 0x0020) << 11 | opcodes.r16(pc + 2));
			return 4 | SUPPORTED;
		}

	case 0x0600: case 0x0601: case 0x2600: case 0x2601:
		util::stream_format(stream, "mul%c    ", BIT(opcode, 13) ? 'w' : 'b');
		if ((opcode & 0x001f) == 0x0011)
		{
			format_medium_imm(stream, opcodes.r16(pc + 2));
			stream << ", ";
			format_reg(stream, (opcode & 0x01e0) >> 5);
			return 4 | SUPPORTED;
		}
		else
		{
			format_short_imm(stream, opcode & 0x001f);
			stream << ", ";
			format_reg(stream, (opcode & 0x01e0) >> 5);
			return 2 | SUPPORTED;
		}

	case 0x0800: case 0x0801: case 0x2800: case 0x2801:
		util::stream_format(stream, "ashu%c   ", BIT(opcode, 13) ? 'w' : 'b');
		if ((opcode & 0x001f) == 0x0011)
		{
			format_medium_imm_decimal(stream, opcodes.r16(pc + 2));
			stream << ", ";
			format_reg(stream, (opcode & 0x01e0) >> 5);
			return 4 | SUPPORTED;
		}
		else
		{
			format_short_imm_decimal(stream, opcode & 0x001f);
			stream << ", ";
			format_reg(stream, (opcode & 0x01e0) >> 5);
			return 2 | SUPPORTED;
		}

	case 0x0a00: case 0x0a01: case 0x2a00: case 0x2a01:
		util::stream_format(stream, "lsh%c    ", BIT(opcode, 13) ? 'w' : 'b');
		if ((opcode & 0x001f) == 0x0011)
		{
			format_medium_imm_decimal(stream, opcodes.r16(pc + 2));
			stream << ", ";
			format_reg(stream, (opcode & 0x01e0) >> 5);
			return 4 | SUPPORTED;
		}
		else
		{
			format_short_imm_decimal(stream, opcode & 0x001f);
			stream << ", ";
			format_reg(stream, (opcode & 0x01e0) >> 5);
			return 2 | SUPPORTED;
		}

	case 0x0c00: case 0x0c01: case 0x2c00: case 0x2c01:
		util::stream_format(stream, "xor%c    ", BIT(opcode, 13) ? 'w' : 'b');
		if ((opcode & 0x001f) == 0x0011)
		{
			format_medium_imm_unsigned(stream, opcodes.r16(pc + 2), BIT(opcode, 13));
			stream << ", ";
			format_reg(stream, (opcode & 0x01e0) >> 5);
			return 4 | SUPPORTED;
		}
		else
		{
			format_short_imm_unsigned(stream, opcode & 0x001f, BIT(opcode, 13));
			stream << ", ";
			format_reg(stream, (opcode & 0x01e0) >> 5);
			return 2 | SUPPORTED;
		}

	case 0x0e00: case 0x0e01: case 0x2e00: case 0x2e01:
		util::stream_format(stream, "cmp%c    ", BIT(opcode, 13) ? 'w' : 'b');
		if ((opcode & 0x001f) == 0x0011)
		{
			format_medium_imm(stream, opcodes.r16(pc + 2));
			stream << ", ";
			format_reg(stream, (opcode & 0x01e0) >> 5);
			return 4 | SUPPORTED;
		}
		else
		{
			format_short_imm(stream, opcode & 0x001f);
			stream << ", ";
			format_reg(stream, (opcode & 0x01e0) >> 5);
			return 2 | SUPPORTED;
		}

	case 0x1000: case 0x1001: case 0x3000: case 0x3001:
		util::stream_format(stream, "and%c    ", BIT(opcode, 13) ? 'w' : 'b');
		if ((opcode & 0x001f) == 0x0011)
		{
			format_medium_imm_unsigned(stream, opcodes.r16(pc + 2), BIT(opcode, 13));
			stream << ", ";
			format_reg(stream, (opcode & 0x01e0) >> 5);
			return 4 | SUPPORTED;
		}
		else
		{
			format_short_imm_unsigned(stream, opcode & 0x001f, BIT(opcode, 13));
			stream << ", ";
			format_reg(stream, (opcode & 0x01e0) >> 5);
			return 2 | SUPPORTED;
		}

	case 0x1200: case 0x1201: case 0x3200: case 0x3201:
		util::stream_format(stream, "addc%c   ", BIT(opcode, 13) ? 'w' : 'b');
		if ((opcode & 0x001f) == 0x0011)
		{
			format_medium_imm(stream, opcodes.r16(pc + 2));
			stream << ", ";
			format_reg(stream, (opcode & 0x01e0) >> 5);
			return 4 | SUPPORTED;
		}
		else
		{
			format_short_imm(stream, opcode & 0x001f);
			stream << ", ";
			format_reg(stream, (opcode & 0x01e0) >> 5);
			return 2 | SUPPORTED;
		}

	case 0x1400:
		// Conditional or unconditional branch to small address
		if ((opcode & 0x000e) == 0x000e || (opcode & 0x01e0) != 0x01e0)
		{
			if ((opcode & 0x01e0) == 0x01c0)
				stream << "br      ";
			else
				util::stream_format(stream, "b%s     ", s_cc[(opcode & 0x01e0) >> 5]);
			format_pc_disp17(stream, pc, u32(opcode & 0x0010) << 12 | opcodes.r16(pc + 2));
			return 4 | SUPPORTED;
		}
		else
		{
			stream << "res";
			return 2 | SUPPORTED;
		}

	case 0x1401: case 0x3401:
		// Compare and branch group
		if (m_arch == cr16_arch::CR16A)
			stream << "res";
		else
		{
			util::stream_format(stream, "b%s%c%c   ", BIT(opcode, 7) ? "ne" : "eq", BIT(opcode, 6) ? '1' : '0', BIT(opcode, 13) ? 'w' : 'b');
			format_reg(stream, (opcode & 0x0120) >> 5);
			stream << ", ";
			format_pc_disp5(stream, pc, opcode & 0x001e);
		}
		return 2 | SUPPORTED;

	case 0x1600:
		// Jump and link to large address
		if (m_arch == cr16_arch::CR16A)
			stream << "res";
		else
		{
			stream << "jal     ";
			format_rpair(stream, (opcode & 0x01e0) >> 5);
			stream << ", ";
			format_rpair(stream, (opcode & 0x001e) >> 1);
		}
		return 2 | SUPPORTED;

	case 0x1601:
		if ((opcode & 0x01e0) != 0x01e0 && m_arch != cr16_arch::CR16A)
		{
			if ((opcode & 0x01e0) == 0x01c0)
				stream << "jump    ";
			else
				util::stream_format(stream, "j%s     ", s_cc[(opcode & 0x01e0) >> 5]);
			format_rpair(stream, (opcode & 0x001e) >> 1);
			return 2 | (opcode == 0x17db ? STEP_OUT : 0) | SUPPORTED;
		}
		else
		{
			stream << "res";
			return 2 | SUPPORTED;
		}

	case 0x1800: case 0x1801: case 0x3800: case 0x3801:
		util::stream_format(stream, "mov%c    ", BIT(opcode, 13) ? 'w' : 'b');
		if ((opcode & 0x001f) == 0x0011)
		{
			format_medium_imm_unsigned(stream, opcodes.r16(pc + 2), BIT(opcode, 13));
			stream << ", ";
			format_reg(stream, (opcode & 0x01e0) >> 5);
			return 4 | SUPPORTED;
		}
		else
		{
			format_short_imm_unsigned(stream, opcode & 0x001f, BIT(opcode, 13));
			stream << ", ";
			format_reg(stream, (opcode & 0x01e0) >> 5);
			return 2 | SUPPORTED;
		}

	case 0x1a00: case 0x1a01: case 0x3a00: case 0x3a01:
		util::stream_format(stream, "subc%c   ", BIT(opcode, 13) ? 'w' : 'b');
		if ((opcode & 0x001f) == 0x0011)
		{
			format_medium_imm(stream, opcodes.r16(pc + 2));
			stream << ", ";
			format_reg(stream, (opcode & 0x01e0) >> 5);
			return 4 | SUPPORTED;
		}
		else
		{
			format_short_imm(stream, opcode & 0x001f);
			stream << ", ";
			format_reg(stream, (opcode & 0x01e0) >> 5);
			return 2 | SUPPORTED;
		}

	case 0x1c00: case 0x1c01: case 0x3c00: case 0x3c01:
		util::stream_format(stream, "or%c     ", BIT(opcode, 13) ? 'w' : 'b');
		if ((opcode & 0x001f) == 0x0011)
		{
			format_medium_imm_unsigned(stream, opcodes.r16(pc + 2), BIT(opcode, 13));
			stream << ", ";
			format_reg(stream, (opcode & 0x01e0) >> 5);
			return 4 | SUPPORTED;
		}
		else
		{
			format_short_imm_unsigned(stream, opcode & 0x001f, BIT(opcode, 13));
			stream << ", ";
			format_reg(stream, (opcode & 0x01e0) >> 5);
			return 2 | SUPPORTED;
		}

	case 0x1e00: case 0x1e01: case 0x3e00: case 0x3e01:
		util::stream_format(stream, "sub%c    ", BIT(opcode, 13) ? 'w' : 'b');
		if ((opcode & 0x001f) == 0x0011)
		{
			format_medium_imm(stream, opcodes.r16(pc + 2));
			stream << ", ";
			format_reg(stream, (opcode & 0x01e0) >> 5);
			return 4 | SUPPORTED;
		}
		else
		{
			format_short_imm(stream, opcode & 0x001f);
			stream << ", ";
			format_reg(stream, (opcode & 0x01e0) >> 5);
			return 2 | SUPPORTED;
		}

	case 0x3400:
		// Branch and link to small address
		if ((opcode & 0x000e) == 0x000e)
		{
			stream << "bal     ";
			format_reg(stream, (opcode & 0x01e0) >> 5);
			stream << ", ";
			format_pc_disp17(stream, pc, u32(opcode & 0x0010) << 12 | opcodes.r16(pc + 2));
			return 4 | STEP_OVER | SUPPORTED;
		}
		else
		{
			stream << "res";
			return 2 | SUPPORTED;
		}

	case 0x3600: case 0x3601:
		// TBIT imm, reg only exists as a word operation
		stream << "tbit    ";
		if ((opcode & 0x001f) == 0x0011)
		{
			format_medium_imm_decimal(stream, opcodes.r16(pc + 2));
			stream << ", ";
			format_reg(stream, (opcode & 0x01e0) >> 5);
			return 4 | SUPPORTED;
		}
		else
		{
			format_short_imm_decimal(stream, opcode & 0x001f);
			stream << ", ";
			format_reg(stream, (opcode & 0x01e0) >> 5);
			return 2 | SUPPORTED;
		}

	case 0x4000: case 0x4200: case 0x4400: case 0x4600:
	case 0x4800: case 0x4a00: case 0x4c00: case 0x4e00:
	case 0x5000: case 0x5200: case 0x5400: case 0x5600:
	case 0x5800: case 0x5a00: case 0x5c00: case 0x5e00:
		// Conditional or unconditional branch with 9-bit displacement
		if ((opcode & 0x01e0) != 0x01e0)
		{
			if ((opcode & 0x01e0) == 0x01c0)
				stream << "br      ";
			else
				util::stream_format(stream, "b%s     ", s_cc[(opcode & 0x01e0) >> 5]);
			format_pc_disp9(stream, pc, (opcode & 0x1e00) >> 4 | (opcode & 0x001e));
		}
		else
			stream << "res";
		return 2 | SUPPORTED;

	case 0x4001: case 0x6001:
		util::stream_format(stream, "add%c    ", BIT(opcode, 13) ? 'w' : 'b');
		format_reg(stream, (opcode & 0x001e) >> 1);
		stream << ", ";
		format_reg(stream, (opcode & 0x01e0) >> 5);
		return 2 | SUPPORTED;

	case 0x4201: case 0x6201:
		util::stream_format(stream, "addu%c   ", BIT(opcode, 13) ? 'w' : 'b');
		format_reg(stream, (opcode & 0x001e) >> 1);
		stream << ", ";
		format_reg(stream, (opcode & 0x01e0) >> 5);
		return 2 | SUPPORTED;

	case 0x4601: case 0x6601:
		util::stream_format(stream, "mul%c    ", BIT(opcode, 13) ? 'w' : 'b');
		format_reg(stream, (opcode & 0x001e) >> 1);
		stream << ", ";
		format_reg(stream, (opcode & 0x01e0) >> 5);
		return 2 | SUPPORTED;

	case 0x4801: case 0x6801:
		util::stream_format(stream, "ashu%c   ", BIT(opcode, 13) ? 'w' : 'b');
		format_reg(stream, (opcode & 0x001e) >> 1);
		stream << ", ";
		format_reg(stream, (opcode & 0x01e0) >> 5);
		return 2 | SUPPORTED;

	case 0x4a01: case 0x6a01:
		util::stream_format(stream, "lsh%c    ", BIT(opcode, 13) ? 'w' : 'b');
		format_reg(stream, (opcode & 0x001e) >> 1);
		stream << ", ";
		format_reg(stream, (opcode & 0x01e0) >> 5);
		return 2 | SUPPORTED;

	case 0x4c01: case 0x6c01:
		util::stream_format(stream, "xor%c    ", BIT(opcode, 13) ? 'w' : 'b');
		format_reg(stream, (opcode & 0x001e) >> 1);
		stream << ", ";
		format_reg(stream, (opcode & 0x01e0) >> 5);
		return 2 | SUPPORTED;

	case 0x4e01: case 0x6e01:
		util::stream_format(stream, "cmp%c    ", BIT(opcode, 13) ? 'w' : 'b');
		format_reg(stream, (opcode & 0x001e) >> 1);
		stream << ", ";
		format_reg(stream, (opcode & 0x01e0) >> 5);
		return 2 | SUPPORTED;

	case 0x5001: case 0x7001:
		util::stream_format(stream, "and%c    ", BIT(opcode, 13) ? 'w' : 'b');
		format_reg(stream, (opcode & 0x001e) >> 1);
		stream << ", ";
		format_reg(stream, (opcode & 0x01e0) >> 5);
		return 2 | SUPPORTED;

	case 0x5201: case 0x7201:
		util::stream_format(stream, "addc%c   ", BIT(opcode, 13) ? 'w' : 'b');
		format_reg(stream, (opcode & 0x001e) >> 1);
		stream << ", ";
		format_reg(stream, (opcode & 0x01e0) >> 5);
		return 2 | SUPPORTED;

	case 0x5401:
		if ((opcode & 0x01e0) != 0x01e0)
		{
			if ((opcode & 0x01e0) == 0x01c0)
				stream << "jump    ";
			else
				util::stream_format(stream, "j%s     ", s_cc[(opcode & 0x01e0) >> 5]);
			format_reg(stream, (opcode & 0x001e) >> 1);
			return 2 | (opcode == 0x55dd ? STEP_OUT : 0) | SUPPORTED;
		}
		else
		{
			stream << "res";
			return 2 | SUPPORTED;
		}

	case 0x5801: case 0x7801:
		util::stream_format(stream, "mov%c    ", BIT(opcode, 13) ? 'w' : 'b');
		format_reg(stream, (opcode & 0x001e) >> 1);
		stream << ", ";
		format_reg(stream, (opcode & 0x01e0) >> 5);
		return 2 | SUPPORTED;

	case 0x5a01: case 0x7a01:
		util::stream_format(stream, "subc%c   ", BIT(opcode, 13) ? 'w' : 'b');
		format_reg(stream, (opcode & 0x001e) >> 1);
		stream << ", ";
		format_reg(stream, (opcode & 0x01e0) >> 5);
		return 2 | SUPPORTED;

	case 0x5c01: case 0x7c01:
		util::stream_format(stream, "or%c     ", BIT(opcode, 13) ? 'w' : 'b');
		format_reg(stream, (opcode & 0x001e) >> 1);
		stream << ", ";
		format_reg(stream, (opcode & 0x01e0) >> 5);
		return 2 | SUPPORTED;

	case 0x5e01: case 0x7e01:
		util::stream_format(stream, "sub%c    ", BIT(opcode, 13) ? 'w' : 'b');
		format_reg(stream, (opcode & 0x001e) >> 1);
		stream << ", ";
		format_reg(stream, (opcode & 0x01e0) >> 5);
		return 2 | SUPPORTED;

	case 0x6000:
		if (m_arch == cr16_arch::CR16A)
			stream << "res";
		else
		{
			stream << "mulsb   ";
			format_reg(stream, (opcode & 0x001e) >> 1);
			stream << ", ";
			format_reg(stream, (opcode & 0x01e0) >> 5);
		}
		return 2 | SUPPORTED;

	case 0x6200:
		if (m_arch == cr16_arch::CR16A)
			stream << "res";
		else
		{
			stream << "mulsw   ";
			format_reg(stream, (opcode & 0x001e) >> 1);
			stream << ", ";
			format_rpair(stream, (opcode & 0x01e0) >> 5);
		}
		return 2 | SUPPORTED;

	case 0x6400: case 0x6600:
		if (m_arch == cr16_arch::CR16A)
		{
			stream << "res";
			return 2 | SUPPORTED;
		}
		else
		{
			stream << "movd    ";
			format_imm21(stream, u32(opcode & 0x0200) << 11 | u32(opcode & 0x000e) << 16 | u32(opcode & 0x0010) << 12 | opcodes.r16(pc + 2));
			stream << ", ";
			format_rpair(stream, (opcode & 0x01e0) >> 5);
			return 4 | SUPPORTED;
		}

	case 0x6800: case 0x6a00:
		util::stream_format(stream, "mov%cb   ", BIT(opcode, 9) ? 'z' : 'x');
		format_reg(stream, (opcode & 0x001e) >> 1);
		stream << ", ";
		format_reg(stream, (opcode & 0x01e0) >> 5);
		return 2 | SUPPORTED;

	case 0x6c00:
		// Push and pop group
		if (m_arch == cr16_arch::CR16A)
			stream << "res";
		else
		{
			if (BIT(opcode, 8))
				stream << "popret  ";
			else if (BIT(opcode, 7))
				stream << "pop     ";
			else
				stream << "push    ";
			util::stream_format(stream, "$%d, ", ((opcode & 0x0060) >> 5) + 1);
			format_reg(stream, (opcode & 0x001e) >> 1);
			if (BIT(opcode, 8))
			{
				util::stream_format(stream, " ; %cMM", BIT(opcode, 7) ? 'L' : 'S');
				return 2 | STEP_OUT | SUPPORTED;
			}
		}
		return 2 | SUPPORTED;

	case 0x6e00:
		if ((opcode & 0x01c0) != 0x01c0)
		{
			util::stream_format(stream, "s%s     ", s_cc[(opcode & 0x01e0) >> 5]);
			format_reg(stream, (opcode & 0x001e) >> 1);
		}
		else
			stream << "res";
		return 2 | SUPPORTED;

	case 0x7000:
		stream << "lpr     ";
		format_reg(stream, (opcode & 0x001e) >> 1);
		stream << ", ";
		format_rproc(stream, (opcode & 0x01e0) >> 5);
		return 2 | SUPPORTED;

	case 0x7200:
		stream << "spr     ";
		format_rproc(stream, (opcode & 0x01e0) >> 5);
		stream << ", ";
		format_reg(stream, (opcode & 0x001e) >> 1);
		return 2 | SUPPORTED;

	case 0x7400:
		// Conditional or unconditional branch to large address
		if ((opcode & 0x01e0) != 0x01e0 && m_arch != cr16_arch::CR16A)
		{
			if ((opcode & 0x01e0) == 0x01c0)
				stream << "br      ";
			else
				util::stream_format(stream, "b%s     ", s_cc[(opcode & 0x01e0) >> 5]);
			format_pc_disp21(stream, pc, u32(opcode & 0x000e) << 16 | u32(opcode & 0x0010) << 12 | opcodes.r16(pc + 2));
			return 4 | SUPPORTED;
		}
		else
		{
			stream << "res";
			return 2 | SUPPORTED;
		}

	case 0x7401:
		// Jump and link to small address
		stream << "jal     ";
		format_reg(stream, (opcode & 0x01e0) >> 5);
		stream << ", ";
		format_reg(stream, (opcode & 0x001e) >> 1);
		return 2 | SUPPORTED;

	case 0x7600:
		// Branch and link to large address
		if (m_arch == cr16_arch::CR16A)
		{
			stream << "res";
			return 2 | SUPPORTED;
		}
		else
		{
			stream << "bal     ";
			format_rpair(stream, (opcode & 0x01e0) >> 5);
			stream << ", ";
			format_pc_disp21(stream, pc, u32(opcode & 0x000e) << 16 | u32(opcode & 0x0010) << 12 | opcodes.r16(pc + 2));
			return 4 | STEP_OVER | SUPPORTED;
		}

	case 0x7601:
		// TBIT reg, reg only exists as a word operation
		stream << "tbit    ";
		format_reg(stream, (opcode & 0x001e) >> 1);
		stream << ", ";
		format_reg(stream, (opcode & 0x01e0) >> 5);
		return 2 | SUPPORTED;

	case 0x7800:
		if (opcode == 0x79fe)
		{
			stream << "retx";
			return 2 | STEP_OUT | SUPPORTED;
		}
		else
		{
			stream << "res";
			return 2 | SUPPORTED;
		}

	case 0x7a00:
		if ((opcode & 0x01e0) == 0x01e0)
		{
			stream << "excp    ";
			format_excp_vector(stream, (opcode & 0x001e) >> 1);
			return 2 | STEP_OVER | SUPPORTED;
		}
		else
		{
			stream << "res";
			return 2 | SUPPORTED;
		}

	case 0x7c00:
		if (opcode == 0x7dde)
			stream << "di";
		else if (opcode == 0x7dfe)
			stream << "ei";
		else
			stream << "res";
		return 2 | SUPPORTED;

	case 0x7e00:
		// Various special operations
		if (opcode == 0x7ffe)
			stream << "wait";
		else if (m_arch == cr16_arch::CR16A)
			stream << "res";
		else if ((opcode & 0x000c) == 0x0000)
		{
			stream << "muluw   ";
			format_reg(stream, (opcode & 0x0012) >> 1);
			stream << ", ";
			format_rpair(stream, (opcode & 0x01e0) >> 5);
		}
		else if ((opcode & 0x011e) == 0x0004)
		{
			if (BIT(opcode, 7))
				stream << "storm   ";
			else
				stream << "loadm   ";
			util::stream_format(stream, "$%d", ((opcode & 0x0060) >> 5) + 1);
		}
		else if (opcode == 0x7fe6)
			stream << "eiwait";
		else
			stream << "res";
		return 2 | SUPPORTED;

	default:
		stream << "res";
		return 2 | SUPPORTED;
	}
}
