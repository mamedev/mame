// license:BSD-3-Clause
// copyright-holders:AJR

#include "emu.h"
#include "upd78k1d.h"

upd78k1_disassembler::upd78k1_disassembler(const char *const sfr_names[], const char *const sfrp_names[])
	: upd78k_8reg_disassembler(sfr_names, sfrp_names)
{
}

const char *const upd78k1_disassembler::s_alu_ops[8] =
{
	"ADD",
	"ADDC",
	"SUB",
	"SUBC",
	"AND",
	"XOR",
	"OR",
	"CMP"
};

const char *const upd78k1_disassembler::s_alu_ops16[3] =
{
	"ADDW",
	"SUBW",
	"CMPW"
};

const char *const upd78k1_disassembler::s_bool_ops[4] =
{
	"MOV1",
	"AND1",
	"OR1",
	"XOR1"
};

const char *const upd78k1_disassembler::s_shift_ops[2][4] =
{
	{ "RORC", "ROR", "SHR", "SHRW" },
	{ "ROLC", "ROL", "SHL", "SHLW" }
};

const char *const upd78k1_disassembler::s_bcond[4] =
{
	"BNZ",
	"BZ",
	"BNC",
	"BC"
};

offs_t upd78k1_disassembler::dasm_01xx(std::ostream &stream, u8 op2, offs_t pc, const upd78k1_disassembler::data_buffer &opcodes)
{
	if (op2 >= 0x1d && op2 < 0x20)
	{
		util::stream_format(stream, "%-8sAX,", s_alu_ops16[op2 - 0x1d]);
		format_sfrp(stream, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;
	}
	else if (op2 == 0x21)
	{
		util::stream_format(stream, "%-8sA,", "XCH");
		format_sfr(stream, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;
	}
	else if ((op2 & 0xf8) == 0x68)
	{
		util::stream_format(stream, "%-8s", s_alu_ops[op2 & 0x07]);
		format_sfr(stream, opcodes.r8(pc + 2));
		stream << ",";
		format_imm8(stream, opcodes.r8(pc + 3));
		return 4 | SUPPORTED;
	}
	else if ((op2 & 0xf8) == 0x98)
	{
		util::stream_format(stream, "%-8sA,", s_alu_ops[op2 & 0x07]);
		format_sfr(stream, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;
	}
	else
		return dasm_illegal2(stream, 0x01, op2);
}

offs_t upd78k1_disassembler::dasm_02xx(std::ostream &stream, u8 op1, u8 op2, offs_t pc, const upd78k1_disassembler::data_buffer &opcodes)
{
	if (!BIT(op1, 0) && BIT(op2, 3))
		return dasm_illegal2(stream, op1, op2);
	else if ((op2 & 0xf0) == 0x10)
	{
		util::stream_format(stream, "%-8s", "MOV1");
		if (BIT(op1, 0))
			util::stream_format(stream, "%s.%d,CY", s_r_names[(op2 & 0x08) >> 3], op2 & 0x07);
		else
			util::stream_format(stream, "%s,CY", s_psw_bits[op2 & 0x07]);
		return 2 | SUPPORTED;
	}
	else if (op2 < 0x70)
	{
		util::stream_format(stream, "%-8sCY,", s_bool_ops[(op2 & 0x60) >> 5]);
		if (BIT(op2, 4))
			stream << "/";
		if (BIT(op1, 0))
			util::stream_format(stream, "%s.%d", s_r_names[(op2 & 0x08) >> 3], op2 & 0x07);
		else
			util::stream_format(stream, "%s", s_psw_bits[op2 & 0x07]);
		return 2 | SUPPORTED;
	}
	else if (op2 < 0xa0)
	{
		util::stream_format(stream, "%-8s", op2 < 0x80 ? "NOT1" : BIT(op2, 4) ? "CLR1" : "SET1");
		if (BIT(op1, 0))
			util::stream_format(stream, "%s.%d", s_r_names[(op2 & 0x08) >> 3], op2 & 0x07);
		else
			util::stream_format(stream, "%s", s_psw_bits[op2 & 0x07]);
		return 2 | SUPPORTED;
	}
	else if ((op2 & 0xe0) == 0xa0 || (op2 & 0xf0) == 0xd0)
	{
		util::stream_format(stream, "%-8s", BIT(op2, 6) ? "BTCLR" : BIT(op2, 4) ? "BT" : "BF");
		if (BIT(op1, 0))
			util::stream_format(stream, "%s.%d,", s_r_names[(op2 & 0x08) >> 3], op2 & 0x07);
		else
			util::stream_format(stream, "%s,", s_psw_bits[op2 & 0x07]);
		format_jdisp8(stream, pc + 3, opcodes.r8(pc + 2));
		return 3 | STEP_COND | SUPPORTED;
	}
	else
		return dasm_illegal2(stream, op1, op2);
}

offs_t upd78k1_disassembler::dasm_05xx(std::ostream &stream, u8 op2, offs_t pc, const upd78k1_disassembler::data_buffer &opcodes)
{
	if ((op2 & 0xe8) == 0x08)
	{
		// not valid with A or X as operand
		util::stream_format(stream, "%-8s%s", BIT(op2, 4) ? "DIVUW" : "MULUW", s_r_names[op2 & 0x07]);
		return 2 | SUPPORTED;
	}
	else if ((op2 & 0xf8) == 0x38)
	{
		// not valid with A or X as operand; not available on µPD78134
		util::stream_format(stream, "%-8s%s", "MULSW", s_r_names[op2 & 0x07]);
		return 2 | SUPPORTED;
	}
	else if ((op2 & 0xe9) == 0x48)
	{
		// CALL rp not available on µPD78134
		util::stream_format(stream, "%-8s%s", BIT(op2, 4) ? "CALL" : "BR", s_rp_names[(op2 & 0x06) >> 1]);
		return 2 | (BIT(op2, 4) ? STEP_OVER : 0) | SUPPORTED;
	}
	else if ((op2 & 0xed) == 0x89)
	{
		util::stream_format(stream, "%-8s[%c]", BIT(op2, 4) ? "ROL4" : "ROR4", BIT(op2, 1) ? 'D' : 'E');
		return 2 | SUPPORTED;
	}
	else if ((op2 & 0xfc) == 0xa8)
	{
		util::stream_format(stream, "%-8sRB%d", "SEL", op2 & 0x03);
		return 2 | SUPPORTED;
	}
	else
		return dasm_illegal2(stream, 0x05, op2);
}

offs_t upd78k1_disassembler::dasm_06(std::ostream &stream, offs_t pc, const upd78k1_disassembler::data_buffer &opcodes)
{
	return dasm_illegal(stream, 0x06);
}

offs_t upd78k1_disassembler::dasm_08xx(std::ostream &stream, u8 op2, offs_t pc, const upd78k1_disassembler::data_buffer &opcodes)
{
	if ((op2 & 0xf0) == 0x10)
	{
		util::stream_format(stream, "%-8s", "MOV1");
		if (BIT(op2, 3))
			format_sfr(stream, opcodes.r8(pc + 2));
		else
			format_saddr(stream, opcodes.r8(pc + 2));
		util::stream_format(stream, ".%d,CY", op2 & 0x07);
		return 3 | SUPPORTED;
	}
	else if (op2 < 0x70)
	{
		util::stream_format(stream, "%-8sCY,", s_bool_ops[(op2 & 0x60) >> 5]);
		if (BIT(op2, 4))
			stream << "/";
		if (BIT(op2, 3))
			format_sfr(stream, opcodes.r8(pc + 2));
		else
			format_saddr(stream, opcodes.r8(pc + 2));
		util::stream_format(stream, ".%d", op2 & 0x07);
		return 3 | SUPPORTED;
	}
	else if (op2 < (BIT(op2, 3) ? 0xa0 : 0x80))
	{
		util::stream_format(stream, "%-8s", op2 < 0x80 ? "NOT1" : BIT(op2, 4) ? "CLR1" : "SET1");
		if (BIT(op2, 3))
			format_sfr(stream, opcodes.r8(pc + 2));
		else
			format_saddr(stream, opcodes.r8(pc + 2));
		util::stream_format(stream, ".%d", op2 & 0x07);
		return 3 | SUPPORTED;
	}
	else if ((op2 & 0xe0) == 0xa0 || (op2 & 0xf0) == 0xd0)
	{
		util::stream_format(stream, "%-8s", BIT(op2, 6) ? "BTCLR" : BIT(op2, 4) ? "BT" : "BF");
		if (BIT(op2, 3))
			format_sfr(stream, opcodes.r8(pc + 2));
		else
			format_saddr(stream, opcodes.r8(pc + 2));
		util::stream_format(stream, ".%d,", op2 & 0x07);
		format_jdisp8(stream, pc + 4, opcodes.r8(pc + 3));
		return 4 | STEP_COND | SUPPORTED;
	}
	else
		return dasm_illegal2(stream, 0x08, op2);
}

offs_t upd78k1_disassembler::dasm_09xx(std::ostream &stream, u8 op2, offs_t pc, const upd78k1_disassembler::data_buffer &opcodes)
{
	if (op2 == 0xc0)
	{
		util::stream_format(stream, "%-8sSTBC,", "MOV");
		u8 nbyte = opcodes.r8(pc + 2);
		u8 pbyte = opcodes.r8(pc + 3);
		if ((nbyte ^ 0xff) == pbyte)
			format_imm8(stream, pbyte);
		else
			stream << "invalid";
		return 4 | SUPPORTED;
	}
	else if ((op2 & 0xfe) == 0xf0)
	{
		// not available on µPD78134
		util::stream_format(stream, "%-8s", "MOV");
		if (!BIT(op2, 0))
			stream << "A,";
		format_abs16(stream, opcodes.r16(pc + 2));
		if (BIT(op2, 0))
			stream << ",A";
		return 4 | SUPPORTED;
	}
	else
		return dasm_illegal2(stream, 0x09, op2);
}

offs_t upd78k1_disassembler::dasm_0axx(std::ostream &stream, u8 op2, offs_t pc, const upd78k1_disassembler::data_buffer &opcodes)
{
	if ((op2 & 0x50) == 0x10 && (BIT(op2, 7) ? (op2 & 0x0f) == 0x00 : BIT(op2, 3) || (op2 & 0x03) == 0))
	{
		// word[A], word[B] modes only available for MOV on µPD78134
		util::stream_format(stream, "%-8s", BIT(op2, 3) ? s_alu_ops[op2 & 0x07] : BIT(op2, 2) ? "XCH" : "MOV");
		if (!BIT(op2, 7))
			stream << "A,";
		format_ix_base16(stream, BIT(op2, 5) ? "B" : "A", opcodes.r16(pc + 2));
		if (BIT(op2, 7))
			stream << ",A";
		return 4 | SUPPORTED;
	}
	else
		return dasm_illegal2(stream, 0x0a, op2);
}

offs_t upd78k1_disassembler::dasm_16xx(std::ostream &stream, u8 op2, const upd78k1_disassembler::data_buffer &opcodes)
{
	if ((op2 & 0xc0) == 0x40 && (BIT(op2, 3) || (!BIT(op2, 5) && (op2 & 0x03) == 0)))
	{
		// [DE] modes and XCH A, [HL] not available on µPD78134
		util::stream_format(stream, "%-8sA,", BIT(op2, 3) ? s_alu_ops[op2 & 0x07] : BIT(op2, 2) ? "XCH" : "MOV");
		if (BIT(op2, 5))
			util::stream_format(stream, "[%c]", BIT(op2, 4) ? 'D' : 'E');
		else
			util::stream_format(stream, "[%s]", BIT(op2, 4) ? "HL" : "DE");
		return 2 | SUPPORTED;
	}
	else
		return dasm_illegal2(stream, 0x16, op2);
}

offs_t upd78k1_disassembler::dasm_25(std::ostream &stream, offs_t pc, const upd78k1_disassembler::data_buffer &opcodes)
{
	return dasm_illegal(stream, 0x25);
}

offs_t upd78k1_disassembler::dasm_29(std::ostream &stream, offs_t pc, const upd78k1_disassembler::data_buffer &opcodes)
{
	return dasm_illegal(stream, 0x29);
}

offs_t upd78k1_disassembler::dasm_38(std::ostream &stream, u8 op, offs_t pc, const upd78k1_disassembler::data_buffer &opcodes)
{
	return dasm_illegal(stream, op);
}

offs_t upd78k1_disassembler::dasm_43(std::ostream &stream, offs_t pc, const upd78k1_disassembler::data_buffer &opcodes)
{
	return dasm_illegal(stream, 0x43);
}

offs_t upd78k1_disassembler::dasm_50(std::ostream &stream, u8 op)
{
	if ((op & 0x0e) == 0x06)
	{
		util::stream_format(stream, "RET%s", BIT(op, 0) ? "I" : "");
		return 1 | STEP_OUT | SUPPORTED;
	}
	else if (!BIT(op, 1))
	{
		// [DE+], [HL+], [DE] modes not available on µPD78134
		util::stream_format(stream, "%-8s", "MOV");
		if (BIT(op, 3))
			stream << "A,";
		util::stream_format(stream, "[%s%s]", BIT(op, 0) ? "HL" : "DE", BIT(op, 2) ? "" : "+");
		if (!BIT(op, 3))
			stream << ",A";
		return 1 | SUPPORTED;
	}
	else
		return dasm_illegal(stream, op);
}

offs_t upd78k1_disassembler::dasm_78(std::ostream &stream, u8 op, offs_t pc, const upd78k1_disassembler::data_buffer &opcodes)
{
	if ((op & 0x03) == 0x03)
		util::stream_format(stream, "%-8sA,[%c]", "XCH", BIT(op, 2) ? 'D' : 'E');
	else
	{
		util::stream_format(stream, "%-8s", "MOV");
		if (BIT(op, 2))
			stream << "A,";
		util::stream_format(stream, "[%c%s]", BIT(op, 1) ? 'D' : 'E', BIT(op, 0) ? "+" : "");
		if (!BIT(op, 2))
			stream << ",A";
	}
	return 1 | SUPPORTED;
}

offs_t upd78k1_disassembler::disassemble(std::ostream &stream, offs_t pc, const upd78k1_disassembler::data_buffer &opcodes, const upd78k1_disassembler::data_buffer &params)
{
	u8 op = opcodes.r8(pc);
	switch (op & 0xf8)
	{
	case 0x00:
		switch (op)
		{
		case 0x00:
			stream << "NOP";
			return 1 | SUPPORTED;

		case 0x01:
			return dasm_01xx(stream, opcodes.r8(pc + 1), pc, opcodes);

		case 0x02: case 0x03:
			return dasm_02xx(stream, op, opcodes.r8(pc + 1), pc, opcodes);

		case 0x05:
			return dasm_05xx(stream, opcodes.r8(pc + 1), pc, opcodes);

		case 0x06:
			return dasm_06(stream, pc, opcodes);

		default:
			return dasm_illegal(stream, op);
		}

	case 0x08:
		switch (op)
		{
		case 0x08:
			return dasm_08xx(stream, opcodes.r8(pc + 1), pc, opcodes);

		case 0x09:
			return dasm_09xx(stream, opcodes.r8(pc + 1), pc, opcodes);

		case 0x0a:
			return dasm_0axx(stream, opcodes.r8(pc + 1), pc, opcodes);

		case 0x0b:
		{
			util::stream_format(stream, "%-8s", "MOVW");
			u8 sfrp = opcodes.r8(pc + 1);
			if (sfrp == 0xfc)
				stream << "SP";
			else
				format_sfrp(stream, sfrp);
			stream << ",";
			format_imm16(stream, opcodes.r16(pc + 2));
			return 4 | SUPPORTED;
		}

		case 0x0c:
			util::stream_format(stream, "%-8s", "MOVW");
			format_saddrp(stream, opcodes.r8(pc + 1));
			stream << ",";
			format_imm16(stream, opcodes.r16(pc + 2));
			return 4 | SUPPORTED;

		case 0x0e: case 0x0f:
			util::stream_format(stream, "ADJB%c", BIT(op, 0) ? 'S' : 'A');
			return 1 | SUPPORTED;

		default:
			return dasm_illegal(stream, op);
		}

	case 0x10:
		if (!BIT(op, 2))
		{
			if (BIT(op, 0))
			{
				util::stream_format(stream, "%-8s", "MOVW");
				if (!BIT(op, 1))
					stream << "AX,";
				u8 sfrp = opcodes.r8(pc + 1);
				if (sfrp == 0xfc)
					stream << "SP";
				else
					format_sfrp(stream, sfrp);
				if (BIT(op, 1))
					stream << ",AX";
			}
			else
			{
				util::stream_format(stream, "%-8s", "MOV");
				if (!BIT(op, 1))
					stream << "A,";
				u8 sfr = opcodes.r8(pc + 1);
				if (sfr == 0xfe)
					stream << "PSW";
				else
					format_sfr(stream, sfr);
				if (BIT(op, 1))
					stream << ",A";
			}
			return 2 | SUPPORTED;
		}
		else if (op == 0x14)
		{
			util::stream_format(stream, "%-8s", "BR");
			format_jdisp8(stream, pc + 2, opcodes.r8(pc + 1));
			return 2 | SUPPORTED;
		}
		else if (op == 0x16)
			return dasm_16xx(stream, opcodes.r8(pc + 1), opcodes);
		else
			return dasm_illegal(stream, op);

	case 0x18:
		if (BIT(op, 2))
		{
			util::stream_format(stream, "%-8sAX,", op == 0x1c ? "MOVW" : s_alu_ops16[op - 0x1d]);
			format_saddrp(stream, opcodes.r8(pc + 1));
			return 2 | SUPPORTED;
		}
		else if (op == 0x1a)
		{
			util::stream_format(stream, "%-8s", "MOVW");
			format_saddrp(stream, opcodes.r8(pc + 1));
			stream << ",AX";
			return 2 | SUPPORTED;
		}
		else
			return dasm_illegal(stream, op);

	case 0x20:
		switch (op)
		{
		case 0x20: case 0x21:
			util::stream_format(stream, "%-8sA,", BIT(op, 0) ? "XCH" : "MOV");
			format_saddr(stream, opcodes.r8(pc + 1));
			return 2 | SUPPORTED;

		case 0x22:
			util::stream_format(stream, "%-8s", "MOV");
			format_saddr(stream, opcodes.r8(pc + 1));
			stream << ",A";
			return 2 | SUPPORTED;

		case 0x24:
		{
			u8 rr = opcodes.r8(pc + 1);
			if ((rr & 0x88) == 0x00)
			{
				util::stream_format(stream, "%-8s%s,%s", "MOV", s_r_names[(rr & 0x70) >> 4], s_r_names[rr & 0x07]);
				return 2 | SUPPORTED;
			}
			else if ((rr & 0x99) == 0x08)
			{
				util::stream_format(stream, "%-8s%s,%s", "MOVW", s_rp_names[(rr & 0x60) >> 5], s_rp_names[(rr & 0x06) >> 1]);
				return 2 | SUPPORTED;
			}
			else
				return dasm_illegal2(stream, op, rr);
		}

		case 0x25:
			return dasm_25(stream, pc, opcodes);

		case 0x26: case 0x27:
			util::stream_format(stream, "%-8s", BIT(op, 0) ? "DEC" : "INC");
			format_saddr(stream, opcodes.r8(pc + 1));
			return 2 | SUPPORTED;

		default:
			return dasm_illegal(stream, op);
		}

	case 0x28:
		if ((op & 0x03) == 0x00)
		{
			util::stream_format(stream, "%-8s", BIT(op, 2) ? "BR" : "CALL");
			format_abs16(stream, opcodes.r16(pc + 1));
			return 3 | (BIT(op, 2) ? 0 : STEP_OVER) | SUPPORTED;
		}
		else if (BIT(op, 2))
		{
			util::stream_format(stream, "%-8sAX,", s_alu_ops16[op - 0x2d]);
			format_imm16(stream, opcodes.r16(pc + 1));
			return 3 | SUPPORTED;
		}
		else if (op == 0x2b)
		{
			util::stream_format(stream, "%-8s", "MOV");
			u8 sfr = opcodes.r8(pc + 1);
			if (sfr == 0xfe)
				stream << "PSW";
			else
				format_sfr(stream, sfr);
			stream << ",";
			format_imm8(stream, opcodes.r8(pc + 2));
			return 3 | SUPPORTED;
		}
		else if (op == 0x29)
			return dasm_29(stream, pc, opcodes);
		else
			return dasm_illegal(stream, op);

	case 0x30:
		if (BIT(op, 2))
		{
			util::stream_format(stream, "%-8s%s", "POP", s_rp_names[op & 0x03]);
			return 1 | SUPPORTED;
		}
		else if (BIT(op, 1))
		{
			util::stream_format(stream, "%-8s%s,", "DBNZ", s_r_names[op & 0x07]);
			format_jdisp8(stream, pc + 2, opcodes.r8(pc + 1));
			return 2 | STEP_COND | SUPPORTED;
		}
		else
		{
			u8 n = opcodes.r8(pc + 1);
			if ((n & 0xc1) != 0xc1)
			{
				util::stream_format(stream, "%-8s", s_shift_ops[op & 0x01][(n & 0xc0) >> 6]);
				if (n >= 0xc0)
					stream << s_rp_names[(n & 0x06) >> 1];
				else
					stream << s_r_names[n & 0x07];
				util::stream_format(stream, ",%d", (n & 0x38) >> 3);
				return 2 | SUPPORTED;
			}
			else
				return dasm_illegal2(stream, op, n);
		}

	case 0x38:
		if (BIT(op, 2))
		{
			util::stream_format(stream, "%-8s%s", "PUSH", s_rp_names[op & 0x03]);
			return 1 | SUPPORTED;
		}
		else if (BIT(op, 1))
		{
			util::stream_format(stream, "%-8s", BIT(op, 0) ? "DBNZ" : "MOV");
			format_saddr(stream, opcodes.r8(pc + 1));
			stream << ",";
			if (BIT(op, 0))
			{
				format_jdisp8(stream, pc + 3, opcodes.r8(pc + 2));
				return 3 | STEP_COND | SUPPORTED;
			}
			else
			{
				format_imm8(stream, opcodes.r8(pc + 2));
				return 3 | SUPPORTED;
			}
		}
		else
			return dasm_38(stream, op, pc, opcodes);

	case 0x40:
		if (BIT(op, 2))
		{
			util::stream_format(stream, "%-8s%s", "INCW", s_rp_names[op & 0x03]);
			return 1 | SUPPORTED;
		}
		else if (BIT(op, 1))
		{
			if (BIT(op, 0))
				return dasm_43(stream, pc, opcodes);
			else
			{
				util::stream_format(stream, "%-8sCY", "NOT1");
				return 1 | SUPPORTED;
			}
		}
		else
		{
			util::stream_format(stream, "%-8sCY", BIT(op, 0) ? "SET1" : "CLR1");
			return 1 | SUPPORTED;
		}

	case 0x48:
		if (BIT(op, 2))
			util::stream_format(stream, "%-8s%s", "DECW", s_rp_names[op & 0x03]);
		else if (BIT(op, 1))
			util::stream_format(stream, "%cI", BIT(op, 0) ? 'E' : 'D');
		else
			util::stream_format(stream, "%-8sPSW", BIT(op, 0) ? "PUSH" : "POP");
		return 1 | SUPPORTED;

	case 0x50: case 0x58:
		return dasm_50(stream, op);

	case 0x60:
		if (BIT(op, 0))
			return dasm_illegal(stream, op);
		else
		{
			util::stream_format(stream, "%-8s%s,", "MOVW", s_rp_names[(op & 0x06) >> 1]);
			format_imm16(stream, opcodes.r16(pc + 1));
			return 3 | SUPPORTED;
		}

	case 0x68:
		util::stream_format(stream, "%-8s", s_alu_ops[op & 0x07]);
		format_saddr(stream, opcodes.r8(pc + 1));
		stream << ",";
		format_imm8(stream, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0x70:
		util::stream_format(stream, "%-8s", "BT");
		format_saddr(stream, opcodes.r8(pc + 1));
		util::stream_format(stream, ".%d,", op & 0x07);
		format_jdisp8(stream, pc + 3, opcodes.r8(pc + 2));
		return 3 | STEP_COND | SUPPORTED;

	case 0x78:
		return dasm_78(stream, op, pc, opcodes);

	case 0x80:
		if (BIT(op, 2))
			return dasm_illegal(stream, op);
		else
		{
			util::stream_format(stream, "%-8s", s_bcond[op & 0x03]);
			format_jdisp8(stream, pc + 2, opcodes.r8(pc + 1));
			return 2 | STEP_COND | SUPPORTED;
		}

	case 0x88:
	{
		u8 rr = opcodes.r8(pc + 1);
		if ((rr & 0x88) == 0x00)
		{
			util::stream_format(stream, "%-8s%s,%s", s_alu_ops[op & 0x07], s_r_names[(rr & 0x70) >> 4], s_r_names[rr & 0x07]);
			return 2 | SUPPORTED;
		}
		else if ((rr & 0xf9) == 0x08 && (op == 0x88 || op == 0x8a || op == 0x8f))
		{
			util::stream_format(stream, "%-8sAX,%s", std::string(s_alu_ops[op & 0x07]) + "W", s_rp_names[(rr & 0x06) >> 1]);
			return 2 | SUPPORTED;
		}
		else
			return dasm_illegal2(stream, op, rr);
	}

	case 0x90:
		util::stream_format(stream, "%-8s", "CALLF");
		format_abs16(stream, 0x0800 | u16(op & 0x07) << 8 | opcodes.r8(pc + 1));
		return 2 | STEP_OVER | SUPPORTED;

	case 0x98:
		util::stream_format(stream, "%-8sA,", s_alu_ops[op & 0x07]);
		format_saddr(stream, opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0xa0: case 0xb0:
		util::stream_format(stream, "%-8s", BIT(op, 4) ? "SET1" : "CLR1");
		format_saddr(stream, opcodes.r8(pc + 1));
		util::stream_format(stream, ".%d", op & 0x07);
		return 2 | SUPPORTED;

	case 0xa8:
		util::stream_format(stream, "%-8sA,", s_alu_ops[op & 0x07]);
		format_imm8(stream, opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0xb8:
		util::stream_format(stream, "%-8s%s,", "MOV", s_r_names[op & 0x07]);
		format_imm8(stream, opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0xc0: case 0xc8:
		util::stream_format(stream, "%-8s%s", BIT(op, 3) ? "DEC" : "INC", s_r_names[op & 0x07]);
		return 1 | SUPPORTED;

	case 0xd0: case 0xd8:
		util::stream_format(stream, "%-8sA,%s", BIT(op, 3) ? "XCH" : "MOV", s_r_names[op & 0x07]);
		return 1 | SUPPORTED;

	case 0xe0: case 0xe8: case 0xf0: case 0xf8:
		util::stream_format(stream, "%-8s[%04XH]", "CALLT", u16(op & 0x3f) << 1);
		return 1 | STEP_OVER | SUPPORTED;

	default: // can't happen here, though compilers believe it could
		return dasm_illegal(stream, op);
	}
}

upd78138_disassembler::upd78138_disassembler()
	: upd78k1_disassembler(s_sfr_names, s_sfrp_names)
{
}

const char *const upd78138_disassembler::s_sfr_names[256] =
{
	"P0", "P1", "P2", "P3", "P4", "P5", "P6", "P7",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, "CPT2L", "PRM3", nullptr, nullptr,
	"PM0", "PM1", nullptr, "PM3", nullptr, "PM5", "PM6", "PM7",
	"PM8", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TMC0", "TMC1", "CPTM", nullptr, nullptr, "TM3", "CR30", "CPT30",
	"PUO", nullptr, nullptr, "PMC3", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, "P0L", "P0H", "RTPC", nullptr, nullptr, nullptr,
	"ICR", nullptr, nullptr, "EDVC", "ECC1", "ECC0", "EC", nullptr,
	"TOM0", "TOC0", "TOM1", "TOC1", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"ADM", nullptr, "ADCR", nullptr, nullptr, nullptr, nullptr, nullptr,
	"PWMC", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "CLOM",
	"CSIM", nullptr, "SBIC", nullptr, nullptr, nullptr, "SIO", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"STBC", nullptr, nullptr, nullptr, "MM", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "IMS",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"IF0L", "IF0H", nullptr, nullptr, "MK0L", "MK0H", nullptr, nullptr,
	"PR0L", "PR0H", nullptr, nullptr, "ISM0L", "ISM0H", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, "INTM0", "INTM1", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

const char *const upd78138_disassembler::s_sfrp_names[128] =
{
	nullptr, nullptr, nullptr, nullptr, "CR00", "CR01", "CR02", "CR10",
	"CR11", "CR12", "CPT0", "CPT1", "CPT2H", "CPT3", nullptr, "CR20",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TM0", "TM1", "FRC", "TM2", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, "PWM0", "PWM1", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"IF0", nullptr, "MK0", nullptr, "PR0", nullptr, "ISM0", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

upd78148_disassembler::upd78148_disassembler()
	: upd78k1_disassembler(s_sfr_names, s_sfrp_names)
{
}

const char *const upd78148_disassembler::s_sfr_names[256] =
{
	"P0", "P1", "P2", "P3", "P4", "P5", "P6", "P7",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, "CPT2L", "PRM3", nullptr, nullptr,
	"PM0", "PM1", nullptr, "PM3", nullptr, "PM5", "PM6", nullptr,
	"PM8", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TMC0", "TMC1", "CPTM", "TM6", "TM7", "TM3", "CR30", "CPT30",
	"PUO", "PMC1", "CPT3L", "PMC3", nullptr, nullptr, "P1L", "P1H",
	"PMC8", "PMC9", "P0L", "P0H", "RTPC", "TRGS", "P8L", "WM",
	"ICR", "UDC", "CRC", "EDVC", "ECC1", "ECC0", "EC", "PRM4",
	"TOM0", "TOC0", "TOM1", "TOC1", "TM4", "CR40", "TM5", "CR50",
	"P8", "P9", "AMPM", "UDCC", "ADM0", "ADM1", "ADCR0", "ADCR1",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"PWMC0", "PWMC1", nullptr, nullptr, nullptr, nullptr, "PWM2", "PWM3",
	nullptr, nullptr, "PWM4", "CR41", nullptr, nullptr, "TMC2", "CLOM",
	"CSIM0", "CSIM1", "SBIC", nullptr, "ADTC1", "ADTP1", "SIO0", "SIO1",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"STBC", nullptr, nullptr, nullptr, "MM", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "IMS",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"IF0L", "IF0H", "IF1L", nullptr, "MK0L", "MK0H", "MK1L", nullptr,
	"PR0L", "PR0H", "PR1L", nullptr, "ISM0L", "ISM0H", "ISM1L", nullptr,
	nullptr, nullptr, nullptr, nullptr, "INTM0", "INTM1", nullptr, nullptr,
	nullptr, nullptr, nullptr, "CC", nullptr, nullptr, nullptr, nullptr
};

const char *const upd78148_disassembler::s_sfrp_names[128] =
{
	nullptr, nullptr, nullptr, nullptr, "CR00", "CR01", "CR02", "CR10",
	"CR11", "CR12", "CPT0", "CPT1", "CPT2H", "CPT3H", nullptr, "CR20",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "MULL", "MULH",
	"TM0", "TM1", "FRC", "TM2", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, "CR13", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, "PWM0", "PWM1", nullptr, nullptr, nullptr, "PWM5", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"IF0", nullptr, "MK0", nullptr, "PR0", nullptr, "ISM0", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};
