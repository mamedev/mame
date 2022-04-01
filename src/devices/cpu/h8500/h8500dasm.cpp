// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Hitachi H8/500 disassembler

***************************************************************************/

#include "emu.h"
#include "h8500dasm.h"

h8500_disassembler::h8500_disassembler(bool expanded)
	: util::disasm_interface()
	, m_expanded(expanded)
{
}

const char *const h8500_disassembler::s_general_ops[0x20] =
{
	nullptr, nullptr,
	nullptr, nullptr,
	"ADD",   "ADDS",
	"SUB",   "SUBS",
	"OR",    nullptr,
	"AND",   nullptr,
	"XOR",   nullptr,
	"CMP",   nullptr,
	"MOV",   "LDC",
	"MOV",   "STC",
	"ADDX",  "MULXU",
	"SUBX",  "DIVXU",
	nullptr, nullptr,
	nullptr, nullptr,
	nullptr, nullptr,
	nullptr, nullptr
};

const char *const h8500_disassembler::s_bit_ops[4] =
{
	"BSET",
	"BCLR",
	"BNOT",
	"BTST"
};

const char *const h8500_disassembler::s_unary_ops[0x10] =
{
	"SWAP", "EXTS", "EXTU",
	"CLR", "NEG", "NOT", "TST", "TAS",
	"SHAL", "SHAR", "SHLL", "SHLR",
	"ROTL", "ROTR", "ROTXL", "ROTXR"
};

const char *const h8500_disassembler::s_branches[0x10] =
{
	"BRA", "BRN", // alias BT & BF
	"BHI", "BLS",
	"BCC", "BCS", // alias BHS & BLO
	"BNE", "BEQ",
	"BVC", "BVS",
	"BPL", "BMI",
	"BGE", "BLT",
	"BGT", "BLE"
};

offs_t h8500_disassembler::opcode_alignment() const
{
	return 1;
}

u32 h8500_disassembler::interface_flags() const
{
	return m_expanded ? PAGED : 0;
}

u32 h8500_disassembler::page_address_bits() const
{
	return 16;
}

void h8500_disassembler::format_reg(std::ostream &stream, u8 n, bool w)
{
	(void)w;
	switch (n)
	{
	case 6:
		stream << "FP";
		break;

	case 7:
		stream << "SP";
		break;

	default:
		util::stream_format(stream, "R%d", n);
		break;
	}
}

void h8500_disassembler::format_creg(std::ostream &stream, u8 n, bool w)
{
	if (n == 0 && w)
	{
		// Status register (word access only)
		stream << "SR";
	}
	else switch (n)
	{
	case 1:
		// Condition code register
		stream << "CCR";
		break;

	case 3:
		// Base register
		stream << "BR";
		break;

	case 4:
		// Extended page register
		stream << "EP";
		break;

	case 5:
		// Data page register
		stream << "DP";
		break;

	case 7:
		// Stack page register
		stream << "TP";
		break;

	default:
		util::stream_format(stream, "CR%d", n);
		break;
	}
}

void h8500_disassembler::format_reglist(std::ostream &stream, u8 x)
{
	stream << "(";
	bool first = true;
	int start = -1;
	for (int n = 0; n <= 8; n++)
	{
		if (start == -1 && BIT(x, n))
			start = n;
		else if (start != -1 && !BIT(x, n))
		{
			if (first)
				first = false;
			else
				stream << ",";
			format_reg(stream, start, true);
			if (n - 1 != start)
			{
				stream << "-";
				format_reg(stream, n - 1, true);
			}
			start = -1;
		}
	}
	stream << ")";
}

void h8500_disassembler::format_imm8(std::ostream &stream, u8 x)
{
	util::stream_format(stream, "#H'%02X", x);
}

void h8500_disassembler::format_imm16(std::ostream &stream, u16 x)
{
	util::stream_format(stream, "#H'%04X", x);
}

void h8500_disassembler::format_bdisp(std::ostream &stream, s16 disp, offs_t pc)
{
	util::stream_format(stream, "H'%04X", (pc + disp) & 0xffff);
}

void h8500_disassembler::format_ea(std::ostream &stream, u8 ea, u16 disp)
{
	if (ea >= 0xe0)
	{
		if (ea >= 0xf0)
			util::stream_format(stream, "@(H'%04X,", disp);
		else if (s16(disp) >= -9 && s16(disp) <= 9)
			util::stream_format(stream, "@(%d,", s16(disp));
		else if (s16(disp) < 0)
			util::stream_format(stream, "@(-H'%02X,", u16(-disp));
		else
			util::stream_format(stream, "@(H'%02X,", disp);
		format_reg(stream, ea & 0x07, true);
		stream << ")";
	}
	else if (ea >= 0xb0)
	{
		stream << "@";
		if ((ea & 0xf0) == 0xb0)
			stream << "-";
		format_reg(stream, ea & 0x07, true);
		if ((ea & 0xf0) == 0xc0)
			stream << "+";
	}
	else if (ea >= 0xa0)
		format_reg(stream, ea & 0x07, BIT(ea, 3));
	else
	{
		assert((ea & 0xe7) == 0x05);
		if (BIT(ea, 4))
			util::stream_format(stream, "@H'%04X:16", disp);
		else
			util::stream_format(stream, "@H'%02X:8", disp & 0xff);
	}
}

offs_t h8500_disassembler::dasm_illegal(std::ostream &stream, std::initializer_list<u8> ops)
{
	util::stream_format(stream, "%-9s", ".DATA.B");

	bool first = true;
	for (u8 op : ops)
	{
		if (first)
			first = false;
		else
			stream << ",";
		util::stream_format(stream, "H'%02X", op);
	}

	return (ops.end() - ops.begin()) | SUPPORTED;
}

offs_t h8500_disassembler::dasm_general(std::ostream &stream, offs_t pc, u8 ea, const h8500_disassembler::data_buffer &opcodes)
{
	u8 eabytes = 1;
	s16 disp = 0;
	if (ea >= 0xe0 || (ea & 0xe7) == 0x05)
	{
		if (BIT(ea, 4))
		{
			eabytes = 3;
			disp = opcodes.r16(pc + 1);
		}
		else
		{
			eabytes = 2;
			disp = s16(s8(opcodes.r8(pc + 1)));
		}
	}

	bool w = BIT(ea, 3);
	u8 op = opcodes.r8(pc + eabytes);
	if ((ea & 0xf8) == 0xa8 && (op & 0xf8) == 0x90)
	{
		util::stream_format(stream, "%-9s", "XCH");
		format_reg(stream, ea & 0x07, w);
		stream << ",";
		format_reg(stream, op & 0x07, w);
		return (eabytes + 1) | SUPPORTED;
	}
	else if (s_general_ops[op >> 3] != nullptr)
	{
		util::stream_format(stream, "%-9s", util::string_format("%s.%c", s_general_ops[op >> 3], w ? 'W' : 'B'));
		if ((op & 0xf0) == 0x90)
		{
			if (BIT(op, 3))
				format_creg(stream, op & 0x07, w);
			else
				format_reg(stream, op & 0x07, w);
			stream << ",";
		}
		format_ea(stream, ea, disp);
		if ((op & 0xf8) == 0x88)
		{
			stream << ",";
			format_creg(stream, op & 0x07, w);
		}
		else if ((op & 0xf0) != 0x90)
		{
			stream << ",";
			format_reg(stream, op & 0x07, w);
		}
		return (eabytes + 1) | SUPPORTED;
	}
	else if (BIT(op, 6))
	{
		util::stream_format(stream, "%-9s", util::string_format("%s.%c", s_bit_ops[BIT(op, 4, 2)], w ? 'W' : 'B'));
		if (BIT(op, 7))
			util::stream_format(stream, "#%d", op & 0x0f);
		else
			format_reg(stream, op & 0x07, w);
		stream << ",";
		format_ea(stream, ea, disp);
		return (eabytes + 1) | SUPPORTED;
	}
	else if ((op & 0xf0) == 0x10 && ((ea & 0xf8) == 0xa0 || op >= 0x13))
	{
		util::stream_format(stream, "%-9s", util::string_format("%s.%c", s_unary_ops[op & 0x0f], w ? 'W' : 'B'));
		format_ea(stream, ea, disp);
		return (eabytes + 1) | SUPPORTED;
	}
	else if ((op & 0xfc) == 0x04 && (ea & 0xf0) != 0xa0)
	{
		util::stream_format(stream, "%-9s", util::string_format("%s.%c", BIT(op, 1) ? "MOV" : "CMP", w ? 'W' : 'B'));
		if (BIT(op, 0))
			format_imm16(stream, opcodes.r16(pc + eabytes + 1));
		else
			format_imm8(stream, opcodes.r8(pc + eabytes + 1));
		stream << ",";
		format_ea(stream, ea, disp);
		return (eabytes + (BIT(op, 0) ? 3 : 2)) | SUPPORTED;
	}
	else if ((op & 0xfa) == 0x08)
	{
		// ADD:Q
		util::stream_format(stream, "%-9s#%s%d,", util::string_format("ADD.%c", w ? 'W' : 'B'), BIT(op, 2) ? "-" : "", BIT(op, 0) ? 2 : 1);
		format_ea(stream, ea, disp);
		return (eabytes + 1) | SUPPORTED;
	}
	else if (op == 0x00)
	{
		u8 op2 = opcodes.r8(pc + eabytes + 1);
		if ((op2 & 0xa8) == 0x80 && (ea & 0xf0) != 0xa0)
		{
			util::stream_format(stream, "%-9s", util::string_format("MOV%cPE.%c", BIT(op2, 4) ? 'T' : 'F', w ? 'W' : 'B'));
			if (BIT(op2, 4))
			{
				format_reg(stream, op2 & 0x07, w);
				stream << ",";
				format_ea(stream, ea, disp);
			}
			else
			{
				format_ea(stream, ea, disp);
				stream << ",";
				format_reg(stream, op2 & 0x07, w);
			}
			return (eabytes + 2) | SUPPORTED;
		}
		else if ((op2 & 0xe8) == 0xa0 && (ea & 0xf8) == 0xa0)
		{
			util::stream_format(stream, "%-9s", "DADD.B");
			format_reg(stream, ea & 0x07, w);
			stream << ",";
			format_reg(stream, op2 & 0x07, w);
			return (eabytes + 2) | SUPPORTED;
		}
		else
		{
			if (eabytes == 3)
				return dasm_illegal(stream, {ea, u8(disp >> 8), u8(disp), op, op2});
			else if (eabytes == 2)
				return dasm_illegal(stream, {ea, u8(disp), op, op2});
			else
				return dasm_illegal(stream, {ea, op, op2});
		}
	}
	else
	{
		if (eabytes == 3)
			return dasm_illegal(stream, {ea, u8(disp >> 8), u8(disp), op});
		else if (eabytes == 2)
			return dasm_illegal(stream, {ea, u8(disp), op});
		else
			return dasm_illegal(stream, {ea, op});
	}
}

offs_t h8500_disassembler::dasm_misc(std::ostream &stream, offs_t pc, u8 ea, const h8500_disassembler::data_buffer &opcodes)
{
	u8 op = opcodes.r8(pc + 1);
	if (ea == 0x11 && op >= 0xd0)
	{
		u8 eabytes = 1;
		s16 disp = 0;
		if (op >= 0xe0)
		{
			if (BIT(op, 4))
			{
				eabytes = 3;
				disp = opcodes.r16(pc + 2);
			}
			else
			{
				eabytes = 2;
				disp = s16(s8(opcodes.r8(pc + 2)));
			}
		}
		util::stream_format(stream, "%-9s", BIT(op, 3) ? "JSR" : "JMP");
		format_ea(stream, op, disp);
		return (eabytes + 1) | (BIT(op, 3) ? STEP_OVER : 0) | SUPPORTED;
	}
	else if (ea == 0x11 && op >= 0xc0 && m_expanded)
	{
		util::stream_format(stream, "%-9s@", BIT(op, 3) ? "PJSR" : "PJMP");
		format_reg(stream, op & 0x07, true);
		return 2 | (BIT(op, 3) ? STEP_OVER : 0) | SUPPORTED;
	}
	else if ((op & 0xf8) == 0xb8)
	{
		util::stream_format(stream, "%-9s", util::string_format("SCB/%s", ea == 0x01 ? "F" : ea == 0x06 ? "NE" : ea == 0x07 ? "EQ" : "?"));
		format_reg(stream, op & 0x07, true);
		stream << ",";
		format_bdisp(stream, s16(s8(opcodes.r8(pc + 2))), pc + 3);
		return 3 | STEP_COND | SUPPORTED;
	}
	else if (ea == 0x11 && (op & 0xf7) == 0x14 && m_expanded)
	{
		util::stream_format(stream, "%-9s", "PRTD");
		if (BIT(op, 3))
		{
			format_imm16(stream, opcodes.r16(pc + 2));
			return 4 | STEP_OUT | SUPPORTED;
		}
		else
		{
			format_imm8(stream, opcodes.r8(pc + 2));
			return 3 | STEP_OUT | SUPPORTED;
		}
	}
	else if (ea == 0x11 && op == 0x19 && m_expanded)
	{
		stream << "PRTS";
		return 2 | STEP_OUT | SUPPORTED;
	}
	else
		return dasm_illegal(stream, {ea, op});
}

offs_t h8500_disassembler::dasm_immop(std::ostream &stream, offs_t pc, bool w, const h8500_disassembler::data_buffer &opcodes)
{
	u16 imm = w ? opcodes.r16(pc + 1) : opcodes.r8(pc + 1);
	u8 op = opcodes.r8(pc + (w ? 3 : 2));
	if (s_general_ops[op >> 3] != nullptr ? (op & 0xf0) != 0x90 : (op & 0xc8) == 0x48 && op < 0x70)
	{
		if ((op & 0xc8) == 0x48)
			util::stream_format(stream, "%-9s", util::string_format("%sC.%c", s_general_ops[(op & 0xf0) >> 3], w ? 'W' : 'B'));
		else
			util::stream_format(stream, "%-9s", util::string_format("%s.%c", s_general_ops[op >> 3], w ? 'W' : 'B'));
		if (w)
			format_imm16(stream, imm);
		else
			format_imm8(stream, imm);
		stream << ",";
		if (BIT(op, 3) && op >= 0x48 && op < 0x90)
			format_creg(stream, op & 0x07, w);
		else
			format_reg(stream, op & 0x07, w);
		return (w ? 4 : 3) | SUPPORTED;
	}
	else
	{
		if (w)
			return dasm_illegal(stream, {0x04, u8(imm), op});
		else
			return dasm_illegal(stream, {0x0c, u8(imm >> 8), u8(imm), op});
	}
}

offs_t h8500_disassembler::disassemble(std::ostream &stream, offs_t pc, const h8500_disassembler::data_buffer &opcodes, const h8500_disassembler::data_buffer &params)
{
	u8 op = opcodes.r8(pc);
	if (op >= 0xa0 || (op & 0xe7) == 0x05)
		return dasm_general(stream, pc, op, opcodes);
	else if (op >= 0x60)
	{
		// MOV:L, MOV:S, MOV:F
		bool w = BIT(op, 3);
		s16 disp = s16(s8(opcodes.r8(pc + 1)));
		util::stream_format(stream, "%-9s", util::string_format("MOV.%c", w ? 'W' : 'B'));
		if (BIT(op, 4))
		{
			format_reg(stream, op & 0x07, w);
			stream << ",";
		}
		format_ea(stream, (op >= 0x80 ? 0xe6 : 0x05) | (op & 0x08), disp);
		if (!BIT(op, 4))
		{
			stream << ",";
			format_reg(stream, op & 0x07, w);
		}
		return 2 | SUPPORTED;
	}
	else if (op >= 0x40)
	{
		// CMP:E, CMP:I, MOV:E, MOV:I
		bool w = BIT(op, 3);
		util::stream_format(stream, "%-9s", util::string_format("%s.%c", BIT(op, 4) ? "MOV" : "CMP", w ? 'W' : 'B'));
		if (w)
			format_imm16(stream, opcodes.r16(pc + 1));
		else
			format_imm8(stream, opcodes.r8(pc + 1));
		stream << ",";
		format_reg(stream, op & 0x07, w);
		return (w ? 3 : 2) | SUPPORTED;
	}
	else if (op >= 0x20)
	{
		util::stream_format(stream, "%-9s", s_branches[op & 0x0f]);
		if (BIT(op, 4))
			format_bdisp(stream, s16(opcodes.r16(pc + 1)), pc + 3);
		else
			format_bdisp(stream, s16(s8(opcodes.r8(pc + 1))), pc + 2);
		return (BIT(op, 4) ? 3 : 2) | ((op & 0x0e) != 0 ? STEP_COND : 0) | SUPPORTED;
	}
	else switch (op)
	{
	case 0x00:
		stream << "NOP";
		return 1 | SUPPORTED;

	case 0x03: case 0x13:
		if (m_expanded)
		{
			util::stream_format(stream, "%-9s@H'%02X%04X", BIT(op, 4) ? "PJSR" : "PJMP", opcodes.r8(pc + 1), opcodes.r16(pc + 2));
			return 4 | (BIT(op, 4) ? STEP_OVER : 0) | SUPPORTED;
		}
		else
			return dasm_illegal(stream, {op});

	case 0x01: case 0x06: case 0x07: case 0x11:
		return dasm_misc(stream, pc, op, opcodes);

	case 0x02:
		util::stream_format(stream, "%-9s", "LDM.W");
		format_ea(stream, 0xcf, 0);
		stream << ",";
		format_reglist(stream, opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0x04: case 0x0c:
		return dasm_immop(stream, pc, BIT(op, 3), opcodes);

	case 0x08:
	{
		util::stream_format(stream, "%-9s", "TRAPA");
		u8 v = opcodes.r8(pc + 1);
		if ((v & 0xf0) == 0x10)
			format_imm8(stream, v);
		else
			stream << "illegal";
		return 2 | STEP_OVER | SUPPORTED;
	}

	case 0x09:
		stream << "TRAP/VS";
		return 1 | SUPPORTED;

	case 0x0a:
		stream << "RTE";
		return 1 | STEP_OUT | SUPPORTED;

	case 0x0e: case 0x1e:
		util::stream_format(stream, "%-9s", "BSR");
		if (BIT(op, 4))
			format_bdisp(stream, s16(opcodes.r16(pc + 1)), pc + 3);
		else
			format_bdisp(stream, s16(s8(opcodes.r8(pc + 1))), pc + 2);
		return (BIT(op, 4) ? 3 : 2) | STEP_OVER | SUPPORTED;

	case 0x0f:
		util::stream_format(stream, "%-9s", "UNLK");
		format_reg(stream, 6, true);
		return 1 | SUPPORTED;

	case 0x10: case 0x18:
		util::stream_format(stream, "%-9s@H'%04X", BIT(op, 3) ? "JSR" : "JMP", opcodes.r16(pc + 1));
		return 3 | (BIT(op, 3) ? STEP_OVER : 0) | SUPPORTED;

	case 0x12:
		util::stream_format(stream, "%-9s", "STM.W");
		format_reglist(stream, opcodes.r8(pc + 1));
		stream << ",";
		format_ea(stream, 0xbf, 0);
		return 2 | SUPPORTED;

	case 0x14: case 0x1c:
		util::stream_format(stream, "%-9s", "RTD");
		if (BIT(op, 3))
			format_imm16(stream, opcodes.r16(pc + 1));
		else
			format_imm8(stream, opcodes.r8(pc + 1));
		return (BIT(op, 3) ? 3 : 2) | STEP_OVER | SUPPORTED;

	case 0x17: case 0x1f:
		util::stream_format(stream, "%-9s", "LINK");
		format_reg(stream, 6, true);
		stream << ",";
		if (BIT(op, 3))
			format_imm16(stream, opcodes.r16(pc + 1));
		else
			format_imm8(stream, opcodes.r8(pc + 1));
		return (BIT(op, 3) ? 3 : 2) | SUPPORTED;

	case 0x19:
		stream << "RTS";
		return 1 | STEP_OUT | SUPPORTED;

	case 0x1a:
		stream << "SLEEP";
		return 1 | SUPPORTED;

	default:
		return dasm_illegal(stream, {op});
	}
}
