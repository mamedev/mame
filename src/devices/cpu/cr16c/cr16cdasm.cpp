// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    National Semiconductor CompactRISC CR16C disassembler

    CR16C is a major extension of the CompactRISC architecture, introducing
    many new addressing modes and a few new instructions. It is source-
    compatible but not object-compatible with CR16B.

***************************************************************************/

#include "util/disasmintf.h"
#include "cr16cdasm.h"

#include "util/strformat.h"

using osd::u32;
using util::BIT;
using offs_t = u32;

cr16c_disassembler::cr16c_disassembler()
	: util::disasm_interface()
{
}

u32 cr16c_disassembler::opcode_alignment() const
{
	return 2;
}

// Condition codes
const char *const cr16c_disassembler::s_cc[14] =
{
	"eq", "ne",
	"cs", "cc",
	"hi", "ls",
	"gt", "le",
	"fs", "fc",
	"lo", "hs",
	"lt", "ge"
};


void cr16c_disassembler::format_reg(std::ostream &stream, u8 value)
{
	switch (value)
	{
	case 12: case 13:
		util::stream_format(stream, "r%d_l", value);
		break;

	case 14:
		stream << "ra_l";
		break;

	case 15:
		stream << "sp_l";
		break;

	default:
		util::stream_format(stream, "r%d", value);
		break;
	}
}

void cr16c_disassembler::format_rpair(std::ostream &stream, u8 value)
{
	switch (value)
	{
	case 11:
		stream << "(r12_l,r11)";
		break;

	case 12: case 13:
		util::stream_format(stream, "(r%d)", value);
		break;

	case 14:
		stream << "(ra)";
		break;

	case 15:
		stream << "(sp)";
		break;

	default:
		util::stream_format(stream, "(r%d,r%d)", value + 1, value);
		break;
	}
}

void cr16c_disassembler::format_rrpair(std::ostream &stream, u8 value)
{
	u8 rpl = (value & 7) * 2 - ((value & 7) >= 6 ? 9 : 0);
	util::stream_format(stream, "(r%d+r%d,r%d)", BIT(value, 3) ? 13 : 12, rpl + 1, rpl);
}

void cr16c_disassembler::format_rproc(std::ostream &stream, u8 value, bool d)
{
	switch (value)
	{
	case 0:
		stream << "dbs";
		break;

	case 1:
		stream << "dsr";
		break;

	case 2: case 3:
		if (d && BIT(value, 0))
			stream << "res";
		else
		{
			stream << "dcr";
			if (!d)
				stream << (BIT(value, 0) ? "h" : "l");
		}
		break;

	case 4: case 5:
	case 6: case 7:
		if (d && BIT(value, 0))
			stream << "res";
		else
		{
			util::stream_format(stream, "car%d", BIT(value, 1));
			if (!d)
				stream << (BIT(value, 0) ? "h" : "l");
		}
		break;

	case 8:
		stream << "cfg";
		break;

	case 9:
		stream << "psr";
		break;

	case 10: case 11:
		if (d && BIT(value, 0))
			stream << "res";
		else
		{
			stream << "intbase";
			if (!d)
				stream << (BIT(value, 0) ? "h" : "l");
		}
		break;

	case 12: case 13:
	case 14: case 15:
		if (d && BIT(value, 0))
			stream << "res";
		else
		{
			util::stream_format(stream, "%csp", BIT(value, 1) ? 'u' : 'i');
			if (!d)
				stream << (BIT(value, 0) ? "h" : "l");
		}
		break;
	}
}

void cr16c_disassembler::format_imm20(std::ostream &stream, u32 imm)
{
	util::stream_format(stream, "$0x%05X", imm);
}

void cr16c_disassembler::format_imm32(std::ostream &stream, u32 imm)
{
	util::stream_format(stream, "$0x%04X%04X", imm & 0xffff, imm >> 16);
}

void cr16c_disassembler::format_abs20(std::ostream &stream, u32 addr)
{
	// 20-bit absolute (last 64K mapped to top of range)
	util::stream_format(stream, "0x%06X", addr < 0xf0000 ? addr : addr | 0xf00000);
}

void cr16c_disassembler::format_abs24(std::ostream &stream, u32 addr)
{
	// 24-bit absolute
	util::stream_format(stream, "0x%06X", addr);
}

void cr16c_disassembler::format_disp4(std::ostream &stream, u8 disp)
{
	// 4-bit unsigned displacement (0-13)
	if (disp == 0)
		stream << "0";
	else
		util::stream_format(stream, "0x%X", disp);
}

void cr16c_disassembler::format_disp4_x2(std::ostream &stream, u8 disp)
{
	// 4-bit unsigned displacement (0-26)
	if (disp == 0)
		stream << "0";
	else
		util::stream_format(stream, "0x%X", disp * 2);
}

void cr16c_disassembler::format_disp14(std::ostream &stream, u16 disp)
{
	// 14-bit unsigned displacement
	util::stream_format(stream, "0x%04X", disp);
}

void cr16c_disassembler::format_disp16(std::ostream &stream, u16 disp)
{
	// 16-bit unsigned displacement
	util::stream_format(stream, "0x%04X", disp);
}

void cr16c_disassembler::format_disp20(std::ostream &stream, u32 disp)
{
	// 20-bit unsigned displacement
	util::stream_format(stream, "0x%05X", disp);
}

void cr16c_disassembler::format_disp20_neg(std::ostream &stream, u32 disp)
{
	// two's complement of 20-bit signed displacement
	util::stream_format(stream, "-0x%05X", 0x100000 - disp);
}

void cr16c_disassembler::format_pc_disp4(std::ostream &stream, offs_t pc, u8 disp)
{
	// 4-bit unsigned displacement (2-32)
	util::stream_format(stream, "0x%06X", (pc + 2 + disp * 2) & 0x1fffffe);
}

void cr16c_disassembler::format_pc_disp8(std::ostream &stream, offs_t pc, u8 disp)
{
	// 8-bit signed displacement
	util::stream_format(stream, "0x%06X", (pc + s8(disp) * 2) & 0x1fffffe);
}

void cr16c_disassembler::format_pc_disp16(std::ostream &stream, offs_t pc, u16 disp)
{
	// 16-bit signed displacement (sign is lowest bit)
	if (BIT(disp, 0))
		util::stream_format(stream, "0x%06X", (pc + (disp & 0xfffe) - 0x10000) & 0x1fffffe);
	else
		util::stream_format(stream, "0x%06X", (pc + disp) & 0x1fffffe);
}

void cr16c_disassembler::format_pc_disp24(std::ostream &stream, offs_t pc, u32 disp)
{
	// 24-bit signed displacement (sign is lowest bit)
	if (BIT(disp, 0))
		util::stream_format(stream, "0x%06X", (pc + (disp & 0xfffffe) - 0x1000000) & 0x1fffffe);
	else
		util::stream_format(stream, "0x%06X", (pc + disp) & 0x1fffffe);
}

void cr16c_disassembler::format_excp_vect(std::ostream &stream, u8 value)
{
	switch (value)
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

	default:
		stream << "und ; reserved";
		break;
	}
}

offs_t cr16c_disassembler::dasm_imm4_16_reg(std::ostream &stream, offs_t pc, u16 opcode, bool i, const cr16c_disassembler::data_buffer &opcodes)
{
	if ((opcode & 0x00f0) == 0x00b0)
	{
		// Escape code for 16-bit immediate value
		if (i)
			util::stream_format(stream, "$0x%04X, ", opcodes.r16(pc + 2));
		else
			util::stream_format(stream, "$0x%02X, ", opcodes.r16(pc + 2) & 0x00ff);
		format_reg(stream, opcode & 0x000f);
		return 4 | SUPPORTED;
	}
	else
	{
		if ((opcode & 0x00f0) == 0x0090)
			stream << "$-1, ";
		else if ((opcode & 0x00f0) == 0x0000)
			stream << "$0, ";
		else
			util::stream_format(stream, "$0x%X, ", (opcode & 0x00f0) >> 4);
		format_reg(stream, opcode & 0x000f);
		return 2 | SUPPORTED;
	}
}

offs_t cr16c_disassembler::dasm_imm4_16_rpair(std::ostream &stream, offs_t pc, u16 opcode, const cr16c_disassembler::data_buffer &opcodes)
{
	if ((opcode & 0x00f0) == 0x00b0)
	{
		// Escape code for 16-bit immediate value
		util::stream_format(stream, "$0x%04X, ", opcodes.r16(pc + 2));
		format_rpair(stream, opcode & 0x000f);
		return 4 | SUPPORTED;
	}
	else
	{
		if ((opcode & 0x00f0) == 0x0090)
			stream << "$-1, ";
		else if ((opcode & 0x00f0) == 0x0000)
			stream << "$0, ";
		else
			util::stream_format(stream, "$0x%X, ", (opcode & 0x00f0) >> 4);
		format_rpair(stream, opcode & 0x000f);
		return 2 | SUPPORTED;
	}
}

offs_t cr16c_disassembler::disassemble(std::ostream &stream, offs_t pc, const cr16c_disassembler::data_buffer &opcodes, const cr16c_disassembler::data_buffer &params)
{
	u16 opcode = opcodes.r16(pc);

	switch (opcode >> 8)
	{
	case 0x00:
		switch (opcode & 0x00ff)
		{
		case 0x03:
			stream << "retx";
			return 2 | STEP_OUT | SUPPORTED;

		case 0x04:
			stream << "di";
			return 2 | SUPPORTED;

		case 0x05:
			stream << "ei";
			return 2 | SUPPORTED;

		case 0x06:
			stream << "wait";
			return 2 | SUPPORTED;

		case 0x07:
			stream << "eiwait";
			return 2 | SUPPORTED;

		case 0x0a: case 0x0b:
			util::stream_format(stream, "cinv    [i%s]", BIT(opcode, 0) ? ",u" : "");
			return 2 | SUPPORTED;

		case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			util::stream_format(stream, "cinv    [d%s]", BIT(opcode, 1) ? ",i" : "", BIT(opcode, 0) ? ",u" : "");
			return 2 | SUPPORTED;

		case 0x10: case 0x11:
		{
			u16 op2 = opcodes.r16(pc + 2);
			u16 op3 = opcodes.r16(pc + 4);

			if ((op2 & 0xc000) != 0)
			{
				util::stream_format(stream, "%cbit%c   $%d, ",
					"cst"[((op2 & 0xc000) >> 14) - 1],
					BIT(opcode, 0) ? 'w' : 'b',
					(op2 & (BIT(opcode, 0) ? 0x00f0 : 0x0070)) >> 4);

				switch ((opcode & 0x3000) >> 12)
				{
				case 0:
					format_disp20(stream, u32(op2 & 0x0f00) << 8 | op3);
					stream << "(";
					format_reg(stream, op2 & 0x000f);
					stream << ")";
					break;

				case 1:
					format_disp20(stream, u32(op2 & 0x0f00) << 8 | op3);
					stream << "(";
					format_rpair(stream, op2 & 0x000f);
					stream << ")";
					break;

				case 2:
					format_disp20(stream, u32(op2 & 0x0f00) << 8 | op3);
					stream << "(";
					format_rrpair(stream, op2 & 0x000f);
					stream << ")";
					break;

				case 3:
					format_abs24(stream, u32(op2 & 0x000f) << 20 | u32(op2 & 0x0f00) << 8 | op3);
					break;
				}
			}
			else if (!BIT(opcode, 0) && (op2 & 0xd000) == 0x0000)
			{
				if (BIT(op2, 13))
				{
					stream << "bal     ";
					format_rpair(stream, (op2 & 0x00f0) >> 4);
					stream << ", ";
				}
				else if ((op2 & 0x00e0) == 0x00e0)
					stream << "br      ";
				else
					util::stream_format(stream, "b%s     ", s_cc[(opcode & 0x00f0) >> 4]);
				format_pc_disp24(stream, pc, u32(op2 & 0x000f) << 20 | u32(op2 & 0x0f00) << 8 | op3);
			}
			else
				stream << "res/nop";

			return 6 | SUPPORTED;
		}

		case 0x12: case 0x13:
		{
			u16 op2 = opcodes.r16(pc + 2);
			u16 op3 = opcodes.r16(pc + 4);

			if ((op2 & 0xc000) == 0)
				util::stream_format(stream, "stor%c   $0x%X, ", BIT(opcode, 0) ? 'w' : 'b', (op2 & 0x00f0) >> 4);
			else if (BIT(opcode, 0))
			{
				util::stream_format(stream, "stor%c   ", "bdw"[((op2 & 0xc000) >> 14) - 1]);
				if ((op2 & 0xc000) == 0x8000)
					format_rpair(stream, (op2 & 0x00f0) >> 4);
				else
					format_reg(stream, (op2 & 0x00f0) >> 4);
				stream << ", ";
			}
			else
				util::stream_format(stream, "load%c   ", "bdw"[((op2 & 0xc000) >> 14) - 1]);

			// Source for load, destination for store
			switch ((op2 & 0x3000) >> 12)
			{
			case 0:
				format_disp20(stream, u32(op2 & 0x0f00) << 8 | op3);
				format_reg(stream, op2 & 0x000f);
				break;

			case 1:
				format_disp20(stream, u32(op2 & 0x0f00) << 8 | op3);
				format_rpair(stream, op2 & 0x000f);
				break;

			case 2:
				format_disp20(stream, u32(op2 & 0x0f00) << 8 | op3);
				format_rrpair(stream, op2 & 0x000f);
				break;

			case 3:
				format_abs24(stream, u32(op2 & 0x000f) << 16 | u32(op2 & 0x0f00) << 8 | op3);
				break;
			}

			if ((op2 & 0xc000) != 0 && !BIT(opcode, 0))
			{
				stream << ", ";
				if ((op2 & 0xc000) == 0x8000)
					format_rpair(stream, (op2 & 0x00f0) >> 4);
				else
					format_reg(stream, (op2 & 0x00f0) >> 4);
			}

			return 6 | SUPPORTED;
		}

		case 0x14:
		{
			u16 op2 = opcodes.r16(pc + 2);
			switch ((op2 & 0xf000) >> 12)
			{
			case 0x0:
				stream << "lpr     ";
				format_reg(stream, op2 & 0x000f);
				stream << ", ";
				format_rproc(stream, (op2 & 0x00f0) >> 4, false);
				break;

			case 0x1:
				stream << "lprd    ";
				format_rpair(stream, op2 & 0x000f);
				stream << ", ";
				format_rproc(stream, (op2 & 0x00f0) >> 4, true);
				break;

			case 0x2:
				stream << "spr     ";
				format_rproc(stream, (op2 & 0x00f0) >> 4, false);
				stream << ", ";
				format_reg(stream, op2 & 0x000f);
				break;

			case 0x3:
				stream << "sprd    ";
				format_rproc(stream, (op2 & 0x00f0) >> 4, true);
				stream << ", ";
				format_rpair(stream, op2 & 0x000f);
				break;

			case 0x8:
				stream << "jal     ";
				format_rpair(stream, op2 & 0x000f);
				stream << ", ";
				format_rpair(stream, (op2 & 0x00f0) >> 4);
				break;

			case 0x9:
				stream << "andd    ";
				format_rpair(stream, (op2 & 0x00f0) >> 4);
				stream << ", ";
				format_rpair(stream, op2 & 0x000f);
				break;

			case 0xa:
				stream << "ord     ";
				format_rpair(stream, (op2 & 0x00f0) >> 4);
				stream << ", ";
				format_rpair(stream, op2 & 0x000f);
				break;

			case 0xb:
				stream << "xord    ";
				format_rpair(stream, (op2 & 0x00f0) >> 4);
				stream << ", ";
				format_rpair(stream, op2 & 0x000f);
				break;

			case 0xc:
				stream << "subd    ";
				format_rpair(stream, (op2 & 0x00f0) >> 4);
				stream << ", ";
				format_rpair(stream, op2 & 0x000f);
				break;

			case 0xd: case 0xe: case 0xf:
				// Multiply and accumulate Q15 signed fraction, unsigned or signed integer
				util::stream_format(stream, "mac%cw   ", "qus"[((op2 & 0x3000) >> 12) - 1]);
				format_reg(stream, (op2 & 0x00f0) >> 4);
				stream << ", ";
				format_reg(stream, op2 & 0x000f);
				stream << ", ";
				format_rpair(stream, (op2 & 0x0f00) >> 8);
				break;

			default:
				stream << "res/nop";
				break;
			}
			if ((op2 & 0xf000) < 0x9000 && (op2 & 0x0f00) != 0)
				util::stream_format(stream, " ; res $%X", (op2 & 0x0f00) >> 8);
			return 4 | SUPPORTED;
		}

		case 0x15:
		{
			stream << "res/und";
			return 4 | SUPPORTED;
		}

		case 0x16:
		{
			stream << "res/und ; movmcr";
			return 4 | SUPPORTED;
		}

		case 0x17:
		{
			stream << "res/und ; movmrc";
			return 4 | SUPPORTED;
		}

		case 0x18:
		{
			u16 op2 = opcodes.r16(pc + 2);
			if ((op2 & 0xc000) != 0 && !BIT(op2, 13))
			{
				util::stream_format(stream, "load%c   ", "bdw"[((op2 & 0xc000) >> 14) - 1]);
				format_disp20_neg(stream, u32(op2 & 0x0f00) << 8 | opcodes.r16(pc + 4));
				if (BIT(op2, 12))
					format_rpair(stream, op2 & 0x000f);
				else
				{
					stream << "(";
					format_reg(stream, op2 & 0x000f);
					stream << ")";
				}
				stream << ", ";
				if ((op2 & 0xc000) == 0x8000)
					format_rpair(stream, (op2 & 0x00f0) >> 4);
				else
					format_reg(stream, (op2 & 0x00f0) >> 4);
			}
			else
				stream << "res/und";
			return 6 | SUPPORTED;
		}

		case 0x19:
		{
			u16 op2 = opcodes.r16(pc + 2);
			if ((op2 & 0xc000) != 0 && !BIT(op2, 13))
			{
				util::stream_format(stream, "stor%c   ", "bdw"[((op2 & 0xc000) >> 14) - 1]);
				if ((op2 & 0xc000) == 0x8000)
					format_rpair(stream, (op2 & 0x00f0) >> 4);
				else
					format_reg(stream, (op2 & 0x00f0) >> 4);
				stream << ", ";
				format_disp20_neg(stream, u32(op2 & 0x0f00) << 8 | opcodes.r16(pc + 4));
				if (BIT(op2, 12))
					format_rpair(stream, op2 & 0x000f);
				else
				{
					stream << "(";
					format_reg(stream, op2 & 0x000f);
					stream << ")";
				}

			}
			else
				stream << "res/und";
			return 6 | SUPPORTED;
		}

		case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
		{
			stream << "res/und";
			return 6 | SUPPORTED;
		}

		case 0x20: case 0x21: case 0x22: case 0x23:
		case 0x24: case 0x25: case 0x26: case 0x27:
		case 0x28: case 0x29: case 0x2a: case 0x2b:
		case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			stream << "addd    ";
			format_imm32(stream, opcodes.r32(pc + 2));
			stream << ", ";
			format_rpair(stream, opcode & 0x000f);
			return 6 | SUPPORTED;

		case 0x30: case 0x31: case 0x32: case 0x33:
		case 0x34: case 0x35: case 0x36: case 0x37:
		case 0x38: case 0x39: case 0x3a: case 0x3b:
		case 0x3c: case 0x3d: case 0x3e: case 0x3f:
			stream << "subd    ";
			format_imm32(stream, opcodes.r32(pc + 2));
			stream << ", ";
			format_rpair(stream, opcode & 0x000f);
			return 6 | SUPPORTED;

		case 0x40: case 0x41: case 0x42: case 0x43:
		case 0x44: case 0x45: case 0x46: case 0x47:
		case 0x48: case 0x49: case 0x4a: case 0x4b:
		case 0x4c: case 0x4d: case 0x4e: case 0x4f:
			stream << "andd    ";
			format_imm32(stream, opcodes.r32(pc + 2));
			stream << ", ";
			format_rpair(stream, opcode & 0x000f);
			return 6 | SUPPORTED;

		case 0x50: case 0x51: case 0x52: case 0x53:
		case 0x54: case 0x55: case 0x56: case 0x57:
		case 0x58: case 0x59: case 0x5a: case 0x5b:
		case 0x5c: case 0x5d: case 0x5e: case 0x5f:
			stream << "ord     ";
			format_imm32(stream, opcodes.r32(pc + 2));
			stream << ", ";
			format_rpair(stream, opcode & 0x000f);
			return 6 | SUPPORTED;

		case 0x60: case 0x61: case 0x62: case 0x63:
		case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x68: case 0x69: case 0x6a: case 0x6b:
		case 0x6c: case 0x6d: case 0x6e: case 0x6f:
			stream << "xord    ";
			format_imm32(stream, opcodes.r32(pc + 2));
			stream << ", ";
			format_rpair(stream, opcode & 0x000f);
			return 6 | SUPPORTED;

		case 0x70: case 0x71: case 0x72: case 0x73:
		case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7a: case 0x7b:
		case 0x7c: case 0x7d: case 0x7e: case 0x7f:
			stream << "movd    ";
			format_imm32(stream, opcodes.r32(pc + 2));
			stream << ", ";
			format_rpair(stream, opcode & 0x000f);
			return 6 | SUPPORTED;

		case 0x80: case 0x81: case 0x82: case 0x83:
		case 0x84: case 0x85: case 0x86: case 0x87:
		case 0x88: case 0x89: case 0x8a: case 0x8b:
		case 0x8c: case 0x8d: case 0x8e: case 0x8f:
			util::stream_format(stream, "cinst   $%d, ", (opcode & 0x000f));
			format_imm32(stream, opcodes.r32(pc + 2));
			return 6 | SUPPORTED;

		case 0xa0: case 0xa1: case 0xa2: case 0xa3:
		case 0xa4: case 0xa5: case 0xa6: case 0xa7:
			util::stream_format(stream, "loadm   $%d", (opcode & 0x0007) + 1);
			return 2 | SUPPORTED;

		case 0xa8: case 0xa9: case 0xaa: case 0xab:
		case 0xac: case 0xad: case 0xae: case 0xaf:
			util::stream_format(stream, "loadmp  $%d", (opcode & 0x0007) + 1);
			return 2 | SUPPORTED;

		case 0xb0: case 0xb1: case 0xb2: case 0xb3:
		case 0xb4: case 0xb5: case 0xb6: case 0xb7:
			util::stream_format(stream, "storm   $%d", (opcode & 0x0007) + 1);
			return 2 | SUPPORTED;

		case 0xb8: case 0xb9: case 0xba: case 0xbb:
		case 0xbc: case 0xbd: case 0xbe: case 0xbf:
			util::stream_format(stream, "stormp  $%d", (opcode & 0x0007) + 1);
			return 2 | SUPPORTED;

		case 0xc0: case 0xc1: case 0xc2: case 0xc3:
		case 0xc4: case 0xc5: case 0xc6: case 0xc7:
		case 0xc8: case 0xc9: case 0xca: case 0xcb:
		case 0xcc: case 0xcd: case 0xce: case 0xcf:
			stream << "excp    ";
			format_excp_vect(stream, opcode & 0x000f);
			return 2 | STEP_OVER | SUPPORTED;

		case 0xd0: case 0xd1: case 0xd2: case 0xd3:
		case 0xd4: case 0xd5: case 0xd6: case 0xd7:
		case 0xd8: case 0xd9: case 0xda: case 0xdb:
		case 0xdc: case 0xdd: case 0xde: case 0xdf:
			stream << "jal     ";
			format_rpair(stream, opcode & 0x000f);
			return 2 | ((opcode & 0x000f) == 0x000e ? STEP_OVER : 0) | SUPPORTED;

		default:
			stream << "res/und";
			return 2 | SUPPORTED;
		}

	case 0x01:
		util::stream_format(stream, "push    $%d, ", ((opcode & 0x0070) >> 4) + 1);
		format_reg(stream, opcode & 0x000f);
		if (BIT(opcode, 7))
			stream << ", ra";
		return 2 | SUPPORTED;

	case 0x02:
		util::stream_format(stream, "pop     $%d, ", ((opcode & 0x0070) >> 4) + 1);
		format_reg(stream, opcode & 0x000f);
		if (BIT(opcode, 7))
			stream << ", ra";
		return 2 | SUPPORTED;

	case 0x03:
		util::stream_format(stream, "popret  $%d, ", ((opcode & 0x0070) >> 4) + 1);
		format_reg(stream, opcode & 0x000f);
		if (BIT(opcode, 7))
			stream << ", ra";
		return 2 | STEP_OUT | SUPPORTED;

	case 0x04:
		stream << "addd    ";
		format_rpair(stream, (opcode & 0x00f0) >> 4);
		stream << ", ";
		format_imm20(stream, u32(opcode & 0x000f) << 16 | opcodes.r16(pc + 2));
		return 4 | SUPPORTED;

	case 0x05:
		stream << "movd    ";
		format_rpair(stream, (opcode & 0x00f0) >> 4);
		stream << ", ";
		format_imm20(stream, u32(opcode & 0x000f) << 16 | opcodes.r16(pc + 2));
		return 4 | SUPPORTED;

	case 0x06:
		util::stream_format(stream, "tbit    $%d, ", (opcode & 0x00f0) >> 4);
		format_reg(stream, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x07:
		stream << "tbit    ";
		format_reg(stream, (opcode & 0x00f0) >> 4);
		stream << ", ";
		format_reg(stream, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x08:
		// Scond
		if ((opcode & 0x00e0) == 0x00e0)
			stream << "suc     "; // set unconditional? (not documented, but not illegal either)
		else
			util::stream_format(stream, "s%s     ", s_cc[(opcode & 0x00f0) >> 4]);
		format_reg(stream, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x09:
		if (BIT(opcode, 7))
		{
			util::stream_format(stream, "lshb    -$%d, ", 8 - ((opcode & 0x0070) >> 4));
			format_reg(stream, opcode & 0x000f);
			return 2 | SUPPORTED;
		}
		else
		{
			stream << "res/und ";
			return 4 | SUPPORTED;
		}

	case 0x0a:
		// Jcond/JUMP/JUSR
		if ((opcode & 0x00e0) == 0x00e0)
			util::stream_format(stream, "ju%s    ", BIT(opcode, 4) ? "sr" : "mp");
		else
			util::stream_format(stream, "j%s     ", s_cc[(opcode & 0x00f0) >> 4]);
		format_rpair(stream, opcode & 0x000f);
		return 2 | ((opcode & 0x00ef) == 0x00ee ? STEP_OUT : 0) | SUPPORTED;

	case 0x0b:
		stream << "mulsb   ";
		format_reg(stream, (opcode & 0x00f0) >> 4);
		stream << ", ";
		format_reg(stream, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x0c: case 0x0d: case 0x0e: case 0x0f:
		// BEQ0i/BNE0i
		util::stream_format(stream, "b%s0%c   ", BIT(opcode, 10) ? "eq" : "ne", BIT(opcode, 9) ? 'w' : 'b');
		format_reg(stream, opcode & 0x000f);
		stream << ", ";
		format_pc_disp4(stream, pc, (opcode & 0x00f0) >> 4);
		return 2 | SUPPORTED;

	case 0x10: case 0x11: case 0x12: case 0x13:
	case 0x14: case 0x15: case 0x16: case 0x17:
	case 0x18: case 0x19: case 0x1a: case 0x1b:
	case 0x1c: case 0x1d: case 0x1e: case 0x1f:
		// Bcond/BR
		if ((opcode & 0x00e0) == 0x00e0)
			stream << "br      ";
		else
			util::stream_format(stream, "b%s     ", s_cc[(opcode & 0x00f0) >> 4]);
		if ((opcode & 0x0f0f) == 0x0800)
		{
			format_pc_disp16(stream, pc, opcodes.r16(pc + 2));
			return 4 | SUPPORTED;
		}
		else
		{
			format_pc_disp8(stream, pc, (opcode & 0x0f00) >> 4 | (opcode & 0x000f));
			return 2 | SUPPORTED;
		}

	case 0x20: case 0x21: case 0x22: case 0x23:
		util::stream_format(stream, "and%c    ", BIT(opcode, 9) ? 'w' : 'b');
		if (BIT(opcode, 8))
		{
			format_reg(stream, (opcode & 0x00f0) >> 4);
			stream << ", ";
			format_reg(stream, opcode & 0x000f);
			return 2 | SUPPORTED;
		}
		else
			return dasm_imm4_16_reg(stream, pc, opcode, BIT(opcode, 9), opcodes);

	case 0x24: case 0x25: case 0x26: case 0x27:
		util::stream_format(stream, "or%c     ", BIT(opcode, 9) ? 'w' : 'b');
		if (BIT(opcode, 8))
		{
			format_reg(stream, (opcode & 0x00f0) >> 4);
			stream << ", ";
			format_reg(stream, opcode & 0x000f);
			return 2 | SUPPORTED;
		}
		else
			return dasm_imm4_16_reg(stream, pc, opcode, BIT(opcode, 9), opcodes);

	case 0x28: case 0x29: case 0x2a: case 0x2b:
		util::stream_format(stream, "xor%c    ", BIT(opcode, 9) ? 'w' : 'b');
		if (BIT(opcode, 8))
		{
			format_reg(stream, (opcode & 0x00f0) >> 4);
			stream << ", ";
			format_reg(stream, opcode & 0x000f);
			return 2 | SUPPORTED;
		}
		else
			return dasm_imm4_16_reg(stream, pc, opcode, BIT(opcode, 9), opcodes);

	case 0x2c: case 0x2d: case 0x2e: case 0x2f:
		// NOP = ADDUB $0x0, r0
		if (opcode == 0x2c00)
		{
			stream << "nop";
			return 2 | SUPPORTED;
		}
		else
		{
			util::stream_format(stream, "addu%c   ", BIT(opcode, 9) ? 'w' : 'b');
			if (BIT(opcode, 8))
			{
				format_reg(stream, (opcode & 0x00f0) >> 4);
				stream << ", ";
				format_reg(stream, opcode & 0x000f);
				return 2 | SUPPORTED;
			}
			else
				return dasm_imm4_16_reg(stream, pc, opcode, BIT(opcode, 9), opcodes);
		}

	case 0x30: case 0x31: case 0x32: case 0x33:
		util::stream_format(stream, "add%c    ", BIT(opcode, 9) ? 'w' : 'b');
		if (BIT(opcode, 8))
		{
			format_reg(stream, (opcode & 0x00f0) >> 4);
			stream << ", ";
			format_reg(stream, opcode & 0x000f);
			return 2 | SUPPORTED;
		}
		else
			return dasm_imm4_16_reg(stream, pc, opcode, BIT(opcode, 9), opcodes);

	case 0x34: case 0x35: case 0x36: case 0x37:
		util::stream_format(stream, "addc%c   ", BIT(opcode, 9) ? 'w' : 'b');
		if (BIT(opcode, 8))
		{
			format_reg(stream, (opcode & 0x00f0) >> 4);
			stream << ", ";
			format_reg(stream, opcode & 0x000f);
			return 2 | SUPPORTED;
		}
		else
			return dasm_imm4_16_reg(stream, pc, opcode, BIT(opcode, 9), opcodes);

	case 0x38: case 0x39: case 0x3a: case 0x3b:
		util::stream_format(stream, "sub%c    ", BIT(opcode, 9) ? 'w' : 'b');
		if (BIT(opcode, 8))
		{
			format_reg(stream, (opcode & 0x00f0) >> 4);
			stream << ", ";
			format_reg(stream, opcode & 0x000f);
			return 2 | SUPPORTED;
		}
		else
			return dasm_imm4_16_reg(stream, pc, opcode, BIT(opcode, 9), opcodes);

	case 0x3c: case 0x3d: case 0x3e: case 0x3f:
		util::stream_format(stream, "subc%c   ", BIT(opcode, 9) ? 'w' : 'b');
		if (BIT(opcode, 8))
		{
			format_reg(stream, (opcode & 0x00f0) >> 4);
			stream << ", ";
			format_reg(stream, opcode & 0x000f);
			return 2 | SUPPORTED;
		}
		else
			return dasm_imm4_16_reg(stream, pc, opcode, BIT(opcode, 9), opcodes);

	case 0x40:
		stream << "ashub   ";
		if (BIT(opcode, 7))
			util::stream_format(stream, "-$%d, ", 8 - ((opcode & 0x0070) >> 4));
		else
			util::stream_format(stream, "+$%d, ", (opcode & 0x0070) >> 4);
		format_reg(stream, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x41: case 0x45:
		util::stream_format(stream, "ashu%c   ", BIT(opcode, 10) ? 'w' : 'b');
		format_reg(stream, (opcode & 0x00f0) >> 4);
		stream << ", ";
		format_reg(stream, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x42: case 0x43:
		stream << "ashuw   ";
		if (BIT(opcode, 8))
			util::stream_format(stream, "-$%d, ", 16 - ((opcode & 0x00f0) >> 4));
		else
			util::stream_format(stream, "+$%d, ", (opcode & 0x00f0) >> 4);
		format_reg(stream, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x44: case 0x46:
		util::stream_format(stream, "lsh%c    ", BIT(opcode, 9) ? 'w' : 'b');
		format_reg(stream, (opcode & 0x00f0) >> 4);
		stream << ", ";
		format_reg(stream, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x47:
		stream << "lshd    ";
		format_reg(stream, (opcode & 0x00f0) >> 4);
		stream << ", ";
		format_rpair(stream, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x48:
		stream << "ashud   ";
		format_reg(stream, (opcode & 0x00f0) >> 4);
		stream << ", ";
		format_rpair(stream, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x49:
		util::stream_format(stream, "lshw    -$%d, ", 16 - ((opcode & 0x00f0) >> 4));
		format_rpair(stream, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x4a: case 0x4b:
		util::stream_format(stream, "lshd    -$%d, ", 32 - ((opcode & 0x01f0) >> 4));
		format_rpair(stream, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x4c: case 0x4d: case 0x4e: case 0x4f:
		stream << "ashud   ";
		if (BIT(opcode, 9))
			util::stream_format(stream, "-$%d, ", 32 - ((opcode & 0x01f0) >> 4));
		else
			util::stream_format(stream, "+$%d, ", (opcode & 0x01f0) >> 4);
		format_rpair(stream, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x50: case 0x51: case 0x52: case 0x53:
		util::stream_format(stream, "cmp%c    ", BIT(opcode, 9) ? 'w' : 'b');
		if (BIT(opcode, 8))
		{
			format_reg(stream, (opcode & 0x00f0) >> 4);
			stream << ", ";
			format_reg(stream, opcode & 0x000f);
			return 2 | SUPPORTED;
		}
		else
			return dasm_imm4_16_reg(stream, pc, opcode, BIT(opcode, 9), opcodes);

	case 0x54:
		stream << "movd    ";
		return dasm_imm4_16_rpair(stream, pc, opcode, opcodes);

	case 0x55:
		stream << "movd    ";
		format_rpair(stream, (opcode & 0x00f0) >> 4);
		stream << ", ";
		format_rpair(stream, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x56:
		stream << "cmpd    ";
		return dasm_imm4_16_rpair(stream, pc, opcode, opcodes);

	case 0x57:
		stream << "cmpd    ";
		format_rpair(stream, (opcode & 0x00f0) >> 4);
		stream << ", ";
		format_rpair(stream, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x58: case 0x59: case 0x5a: case 0x5b:
		util::stream_format(stream, "mov%c    ", BIT(opcode, 9) ? 'w' : 'b');
		if (BIT(opcode, 8))
		{
			format_reg(stream, (opcode & 0x00f0) >> 4);
			stream << ", ";
			format_reg(stream, opcode & 0x000f);
			return 2 | SUPPORTED;
		}
		else
			return dasm_imm4_16_reg(stream, pc, opcode, BIT(opcode, 9), opcodes);

	case 0x5c: case 0x5d:
		// Sign-extend or zero-extend byte to word
		util::stream_format(stream, "mov%cb   ", BIT(opcode, 8) ? 'z' : 'x');
		format_reg(stream, (opcode & 0x00f0) >> 4);
		stream << ", ";
		format_reg(stream, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x5e: case 0x5f:
		// Sign-extend or zero-extend word to double word
		util::stream_format(stream, "mov%cw   ", BIT(opcode, 8) ? 'z' : 'x');
		format_reg(stream, (opcode & 0x00f0) >> 4);
		stream << ", ";
		format_rpair(stream, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x60:
		stream << "addd    ";
		return dasm_imm4_16_rpair(stream, pc, opcode, opcodes);

	case 0x61:
		stream << "addd    ";
		format_rpair(stream, (opcode & 0x00f0) >> 4);
		stream << ", ";
		format_rpair(stream, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x62: case 0x63:
		// MULSW/MULUW
		util::stream_format(stream, "mul%cw   ", BIT(opcode, 8) ? 'u' : 's');
		format_reg(stream, (opcode & 0x00f0) >> 4);
		stream << ", ";
		format_rpair(stream, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x64: case 0x66:
		util::stream_format(stream, "mul%c    ", BIT(opcode, 9) ? 'w' : 'b');
		return dasm_imm4_16_rpair(stream, pc, opcode, opcodes);

	case 0x65: case 0x67:
		util::stream_format(stream, "mul%c    ", BIT(opcode, 9) ? 'w' : 'b');
		format_reg(stream, (opcode & 0x00f0) >> 4);
		stream << ", ";
		format_reg(stream, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x68: case 0x70: case 0x78:
		util::stream_format(stream, "%cbitb   $%d, ", "cst"[((opcode & 0x1800) >> 11) - 1], (opcode & 0x0070) >> 4);
		format_abs20(stream, u32(opcode & 0x000f) << 16 | opcodes.r16(pc + 2));
		format_rpair(stream, BIT(opcode, 7) ? 13 : 12);
		return 4 | SUPPORTED;

	case 0x69: case 0x71: case 0x79:
		util::stream_format(stream, "%cbitw   $%d, ", "cst"[((opcode & 0x1800) >> 11) - 1], (opcode & 0x00f0) >> 4);
		format_disp16(stream, opcodes.r16(pc + 2));
		format_rpair(stream, opcode & 0x000f);
		return 4 | SUPPORTED;

	case 0x6a: case 0x72: case 0x7a:
		if (BIT(opcode, 7))
		{
			u16 op2 = opcodes.r16(pc + 2);
			util::stream_format(stream, "%cbit%c   $%d, ",
				"cst"[((opcode & 0x1800) >> 11) - 1],
				BIT(opcode, 6) ? 'w' : 'b',
				(op2 & (BIT(opcode, 6) ? 0x00f0 : 0x0070)) >> 4);
			format_disp14(stream, (op2 & 0x3f0f) | (opcode & 0x0030) | (op2 & 0xc000) >> 8);
			format_rrpair(stream, opcode & 0x000f);
			return 4 | SUPPORTED;
		}
		else
		{
			util::stream_format(stream, "%cbitb   $%d, 0", "cst"[((opcode & 0x1800) >> 11) - 1], (opcode & 0x0070) >> 4);
			format_rpair(stream, opcode & 0x000f);
			return 2 | SUPPORTED;
		}

	case 0x6b: case 0x73: case 0x7b:
		util::stream_format(stream, "%cbitb   $%d, ", "cst"[((opcode & 0x1800) >> 11) - 1], (opcode & 0x0070) >> 4);
		if (BIT(opcode, 7))
			format_abs20(stream, u32(opcode & 0x000f) << 16 | opcodes.r16(pc + 2));
		else
		{
			format_disp16(stream, opcodes.r16(pc + 2));
			format_rpair(stream, opcode & 0x000f);
		}
		return 4 | SUPPORTED;

	case 0x6c: case 0x6d: case 0x74: case 0x75: case 0x7c: case 0x7d:
		util::stream_format(stream, "%cbitw   $%d, ", "cst"[((opcode & 0x1800) >> 11) - 1], (opcode & 0x00f0) >> 4);
		format_abs20(stream, u32(opcode & 0x000f) << 16 | opcodes.r16(pc + 2));
		format_rpair(stream, BIT(opcode, 8) ? 13 : 12);
		return 4 | SUPPORTED;

	case 0x6e: case 0x76: case 0x7e:
		util::stream_format(stream, "%cbitw   $%d, 0", "cst"[((opcode & 0x1800) >> 11) - 1], (opcode & 0x00f0) >> 4);
		format_rpair(stream, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x6f: case 0x77: case 0x7f:
		util::stream_format(stream, "%cbitw   $%d, ", "cst"[((opcode & 0x1800) >> 11) - 1], (opcode & 0x00f0) >> 4);
		format_abs20(stream, u32(opcode & 0x000f) << 16 | opcodes.r16(pc + 2));
		return 4 | SUPPORTED;

	case 0x81: case 0xc1:
		util::stream_format(stream, "stor%c   $0x%X, ", BIT(opcode, 13) ? 'w' : 'b', (opcode & 0x00f0) >> 4);
		format_abs20(stream, u32(opcode & 0x000f) << 16 | opcodes.r16(pc + 2));
		return 4 | SUPPORTED;

	case 0x82: case 0xc2:
		util::stream_format(stream, "stor%c   $0x%X, 0", BIT(opcode, 13) ? 'w' : 'b', (opcode & 0x00f0) >> 4);
		format_rpair(stream, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0x83: case 0xc3:
		util::stream_format(stream, "stor%c   $0x%X, ", BIT(opcode, 13) ? 'w' : 'b', (opcode & 0x00f0) >> 4);
		format_disp16(stream, opcodes.r16(pc + 2));
		format_rpair(stream, opcode & 0x000f);
		return 4 | SUPPORTED;

	case 0x84: case 0x85: case 0xc4: case 0xc5:
		util::stream_format(stream, "stor%c   $0x%X, ", BIT(opcode, 13) ? 'w' : 'b', (opcode & 0x00f0) >> 4);
		format_abs20(stream, u32(opcode & 0x000f) << 16 | opcodes.r16(pc + 2));
		format_rpair(stream, BIT(opcode, 8) ? 13 : 12);
		return 4 | SUPPORTED;

	case 0x86:
	{
		u16 op2 = opcodes.r16(pc + 2);

		if (BIT(opcode, 6))
			util::stream_format(stream, "load%c   ", BIT(opcode, 7) ? 'w' : 'b');
		else if (BIT(opcode, 7))
			stream << "loadd   ";
		else
			util::stream_format(stream, "storb   $0x%X, ", (op2 & 0x00f0) >> 4);

		// Source for load, destination for store
		format_disp14(stream, (op2 & 0x3f0f) | (opcode & 0x0030) | (op2 & 0xc000) >> 8);
		format_rrpair(stream, opcode & 0x000f);

		if (BIT(opcode, 6))
		{
			stream << ", ";
			format_reg(stream, (op2 & 0x00f0) >> 4);
		}
		else if (BIT(opcode, 7))
		{
			stream << ", ";
			format_rpair(stream, (op2 & 0x00f0) >> 4);
		}

		return 4 | SUPPORTED;
	}

	case 0x87:
		stream << "loadd   ";
		format_abs20(stream, u32(opcode & 0x000f) << 16 | opcodes.r16(pc + 2));
		stream << ", ";
		format_rpair(stream, (opcode & 0x00f0) >> 4);
		return 4 | SUPPORTED;

	case 0x88: case 0x89:
		util::stream_format(stream, "load%c   ", BIT(opcode, 8) ? 'w' : 'b');
		format_abs20(stream, u32(opcode & 0x000f) << 16 | opcodes.r16(pc + 2));
		stream << ", ";
		format_reg(stream, (opcode & 0x00f0) >> 4);
		return 4 | SUPPORTED;

	case 0x8a: case 0x8b: case 0x8e: case 0x8f:
		util::stream_format(stream, "load%c   ", BIT(opcode, 10) ? 'w' : 'b');
		format_reg(stream, (opcode & 0x00f0) >> 4);
		stream << ", ";
		format_abs20(stream, u32(opcode & 0x000f) << 16 | opcodes.r16(pc + 2));
		format_rpair(stream, BIT(opcode, 8) ? 13 : 12);
		return 4 | SUPPORTED;

	case 0x8c: case 0x8d:
		stream << "loadd   ";
		format_rpair(stream, (opcode & 0x00f0) >> 4);
		stream << ", ";
		format_abs20(stream, u32(opcode & 0x000f) << 16 | opcodes.r16(pc + 2));
		format_rpair(stream, BIT(opcode, 8) ? 13 : 12);
		return 4 | SUPPORTED;

	case 0x90: case 0x91: case 0x92: case 0x93:
	case 0x94: case 0x95: case 0x96: case 0x97:
	case 0x98: case 0x99: case 0x9a: case 0x9b:
	case 0x9c: case 0x9d:
		stream << "loadw   ";
		format_disp4_x2(stream, (opcode & 0x0f00) >> 8);
		format_rpair(stream, opcode & 0x000f);
		stream << ", ";
		format_reg(stream, (opcode & 0x00f0) >> 4);
		return 2 | SUPPORTED;

	case 0x9e: case 0xbe:
		util::stream_format(stream, "load%c   0", BIT(opcode, 13) ? 'b' : 'w');
		format_rrpair(stream, opcode & 0x000f);
		stream << ", ";
		format_reg(stream, (opcode & 0x00f0) >> 4);
		return 2 | SUPPORTED;

	case 0x9f: case 0xbf:
		util::stream_format(stream, "load%c   ", BIT(opcode, 13) ? 'b' : 'w');
		format_disp16(stream, opcodes.r16(pc + 2));
		format_rpair(stream, opcode & 0x000f);
		stream << ", ";
		format_reg(stream, (opcode & 0x00f0) >> 4);
		return 4 | SUPPORTED;

	case 0xa0: case 0xa1: case 0xa2: case 0xa3:
	case 0xa4: case 0xa5: case 0xa6: case 0xa7:
	case 0xa8: case 0xa9: case 0xaa: case 0xab:
	case 0xac: case 0xad:
		stream << "loadd   ";
		format_disp4_x2(stream, (opcode & 0x0f00) >> 8);
		format_rpair(stream, opcode & 0x000f);
		stream << ", ";
		format_rpair(stream, (opcode & 0x00f0) >> 4);
		return 2 | SUPPORTED;

	case 0xae:
		stream << "loadd   0";
		format_rrpair(stream, opcode & 0x000f);
		stream << ", ";
		format_rpair(stream, (opcode & 0x00f0) >> 4);
		return 2 | SUPPORTED;

	case 0xaf:
		stream << "loadd   ";
		format_disp16(stream, opcodes.r16(pc + 2));
		format_rpair(stream, opcode & 0x000f);
		stream << ", ";
		format_rpair(stream, (opcode & 0x00f0) >> 4);
		return 4 | SUPPORTED;

	case 0xb0: case 0xb1: case 0xb2: case 0xb3:
	case 0xb4: case 0xb5: case 0xb6: case 0xb7:
	case 0xb8: case 0xb9: case 0xba: case 0xbb:
	case 0xbc: case 0xbd:
		stream << "loadb   ";
		format_disp4(stream, (opcode & 0x0f00) >> 8);
		format_rpair(stream, opcode & 0x000f);
		stream << ", ";
		format_reg(stream, (opcode & 0x00f0) >> 4);
		return 2 | SUPPORTED;

	case 0xc0:
		stream << "bal     (ra), ";
		format_pc_disp24(stream, pc, u32(opcode & 0x00ff) << 16 | opcodes.r16(pc + 2));
		return 4 | STEP_OVER | SUPPORTED;

	case 0xc6:
	{
		u16 op2 = opcodes.r16(pc + 2);
		if (BIT(opcode, 6))
		{
			util::stream_format(stream, "stor%c   ", BIT(opcode, 7) ? 'w' : 'b');
			format_reg(stream, (op2 & 0x00f0) >> 4);
		}
		else if (BIT(opcode, 7))
		{
			stream << "stord   ";
			format_rpair(stream, (op2 & 0x00f0) >> 4);
		}
		else
			util::stream_format(stream, "storw   $0x%X, ", (op2 & 0x00f0) >> 4);
		format_disp14(stream, (op2 & 0x3f0f) | (opcode & 0x0030) | (op2 & 0xc000) >> 8);
		format_rrpair(stream, opcode & 0x000f);
		return 4 | SUPPORTED;
	}

	case 0xc7:
		stream << "stord   ";
		format_rpair(stream, (opcode & 0x00f0) >> 4);
		stream << ", ";
		format_abs20(stream, u32(opcode & 0x000f) << 16 | opcodes.r16(pc + 2));
		return 4 | SUPPORTED;

	case 0xc8: case 0xc9:
		util::stream_format(stream, "stor%c   ", BIT(opcode, 8) ? 'w' : 'b');
		format_reg(stream, (opcode & 0x00f0) >> 4);
		stream << ", ";
		format_abs20(stream, u32(opcode & 0x000f) << 16 | opcodes.r16(pc + 2));
		return 4 | SUPPORTED;

	case 0xca: case 0xcb: case 0xce: case 0xcf:
		util::stream_format(stream, "stor%c   ", BIT(opcode, 10) ? 'w' : 'b');
		format_reg(stream, (opcode & 0x00f0) >> 4);
		stream << ", ";
		format_abs20(stream, u32(opcode & 0x000f) << 16 | opcodes.r16(pc + 2));
		format_rpair(stream, BIT(opcode, 8) ? 13 : 12);
		return 4 | SUPPORTED;

	case 0xcc: case 0xcd:
		stream << "stord   ";
		format_rpair(stream, (opcode & 0x00f0) >> 4);
		stream << ", ";
		format_abs20(stream, u32(opcode & 0x000f) << 16 | opcodes.r16(pc + 2));
		format_rpair(stream, BIT(opcode, 8) ? 13 : 12);
		return 4 | SUPPORTED;

	case 0xd0: case 0xd1: case 0xd2: case 0xd3:
	case 0xd4: case 0xd5: case 0xd6: case 0xd7:
	case 0xd8: case 0xd9: case 0xda: case 0xdb:
	case 0xdc: case 0xdd:
		stream << "storw   ";
		format_reg(stream, (opcode & 0x00f0) >> 4);
		stream << ", ";
		format_disp4_x2(stream, (opcode & 0x0f00) >> 8);
		format_rpair(stream, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0xde: case 0xfe:
		util::stream_format(stream, "stor%c   ", BIT(opcode, 13) ? 'b' : 'w');
		format_reg(stream, (opcode & 0x00f0) >> 4);
		stream << ", 0";
		format_rrpair(stream, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0xdf: case 0xff:
		if (opcode == 0xffff)
		{
			stream << "res/und";
			return 2 | SUPPORTED;
		}
		else
		{
			util::stream_format(stream, "stor%c   ", BIT(opcode, 13) ? 'b' : 'w');
			format_reg(stream, (opcode & 0x00f0) >> 4);
			stream << ", ";
			format_disp16(stream, opcodes.r16(pc + 2));
			format_rpair(stream, opcode & 0x000f);
			return 4 | SUPPORTED;
		}

	case 0xe0: case 0xe1: case 0xe2: case 0xe3:
	case 0xe4: case 0xe5: case 0xe6: case 0xe7:
	case 0xe8: case 0xe9: case 0xea: case 0xeb:
	case 0xec: case 0xed:
		stream << "stord   ";
		format_rpair(stream, (opcode & 0x00f0) >> 4);
		stream << ", ";
		format_disp4_x2(stream, (opcode & 0x0f00) >> 8);
		format_rpair(stream, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0xee:
		stream << "stord   ";
		format_rpair(stream, (opcode & 0x00f0) >> 4);
		stream << ", 0";
		format_rrpair(stream, opcode & 0x000f);
		return 2 | SUPPORTED;

	case 0xef:
		stream << "stord   ";
		format_rpair(stream, (opcode & 0x00f0) >> 4);
		stream << ", ";
		format_disp16(stream, opcodes.r16(pc + 2));
		format_rpair(stream, opcode & 0x000f);
		return 4 | SUPPORTED;

	case 0xf0: case 0xf1: case 0xf2: case 0xf3:
	case 0xf4: case 0xf5: case 0xf6: case 0xf7:
	case 0xf8: case 0xf9: case 0xfa: case 0xfb:
	case 0xfc: case 0xfd:
		stream << "storb   ";
		format_reg(stream, (opcode & 0x00f0) >> 4);
		stream << ", ";
		format_disp4(stream, (opcode & 0x0f00) >> 8);
		format_rpair(stream, opcode & 0x000f);
		return 2 | SUPPORTED;

	default:
		stream << "res/und";
		return 2 | SUPPORTED;
	}
}
