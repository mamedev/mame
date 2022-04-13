// license:BSD-3-Clause
// copyright-holders:AJR

#include "emu.h"
#include "upd78k3d.h"

upd78k3_disassembler::upd78k3_disassembler(const char *const sfr_names[], const char *const sfrp_names[], const char *const psw_bits[], bool has_macw, bool has_macsw)
	: upd78k_family_disassembler(sfr_names, sfrp_names, 0xfe00)
	, m_ix_bases(s_ix_bases)
	, m_psw_bits(psw_bits)
	, m_has_macw(has_macw)
	, m_has_macsw(has_macsw)
{
}

upd78k3_disassembler::upd78k3_disassembler(const char *const sfr_names[], const char *const sfrp_names[], const char *const ix_bases[], u16 saddr_ram_base)
	: upd78k_family_disassembler(sfr_names, sfrp_names, saddr_ram_base)
	, m_ix_bases(ix_bases)
	, m_psw_bits(s_psw_bits)
	, m_has_macw(true)
	, m_has_macsw(true)
{
}

const char *const upd78k3_disassembler::s_r_names[16] =
{
	"R0", // X when RSS = 0
	"R1", // A when RSS = 0
	"R2", // C when RSS = 0
	"R3", // B when RSS = 0
	"R4", // X when RSS = 1
	"R5", // A when RSS = 1
	"R6", // C when RSS = 1
	"R7", // B when RSS = 1
	"VPL",
	"VPH",
	"UPL",
	"UPH",
	"E",
	"D",
	"L",
	"H"
};

const char *const upd78k3_disassembler::s_rp_names[8] =
{
	"RP0", // AX when RSS = 0
	"RP1", // BC when RSS = 0
	"RP2", // AX when RSS = 1
	"RP3", // BC when RSS = 1
	"VP",
	"UP",
	"DE",
	"HL"
};

const char *const upd78k3_disassembler::s_ix_bases[5] =
{
	"DE",
	"SP",
	"HL",
	"UP",
	"VP"
};

const char *const upd78k3_disassembler::s_psw_bits[16] =
{
	"CY",
	"PSWL.1",
	"P/V",
	"IE",
	"AC",
	"RSS",
	"Z",
	"S",
	"PSWH.0",
	"PSWH.1",
	"PSWH.2",
	"PSWH.3",
	"RBS0",
	"RBS1",
	"RBS2",
	"UF"
};

const char *const upd78k3_disassembler::s_alu_ops[8] =
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

const char *const upd78k3_disassembler::s_16bit_ops[4] =
{
	"MOVW",
	"ADDW",
	"SUBW",
	"CMPW"
};

const char *const upd78k3_disassembler::s_bool_ops[4] =
{
	"MOV1",
	"AND1",
	"OR1",
	"XOR1"
};

const char *const upd78k3_disassembler::s_shift_ops[2][4] =
{
	{ "RORC", "ROR", "SHR", "SHRW" },
	{ "ROLC", "ROL", "SHL", "SHLW" }
};

const char *const upd78k3_disassembler::s_bcond[8] =
{
	"BNZ",
	"BZ",
	"BNC",
	"BC",
	"BNV",
	"BV",
	"BN",
	"BP"
};

const char *const upd78k3_disassembler::s_bcond_07f8[6] =
{
	"BLT",
	"BGE",
	"BLE",
	"BGT",
	"BNH",
	"BH"
};

const char *const upd78k3_disassembler::s_cmpscond[4] =
{
	"E",
	"NE",
	"NC",
	"C"
};

offs_t upd78k3_disassembler::dasm_01xx(std::ostream &stream, u8 op2, offs_t pc, const upd78k3_disassembler::data_buffer &opcodes)
{
	if (op2 >= 0x0d && op2 < 0x10)
	{
		util::stream_format(stream, "%-8s", s_16bit_ops[op2 & 0x03]);
		format_sfrp(stream, opcodes.r8(pc + 2));
		stream << ",";
		format_imm16(stream, opcodes.r16(pc + 3));
		return 5 | SUPPORTED;
	}
	else if (op2 == 0x1b)
	{
		util::stream_format(stream, "%-8sAX,", "XCHW");
		format_sfrp(stream, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;
	}
	else if (op2 >= 0x1d && op2 < 0x20)
	{
		util::stream_format(stream, "%-8sAX,", s_16bit_ops[op2 & 0x03]);
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

offs_t upd78k3_disassembler::dasm_02xx(std::ostream &stream, u8 op1, u8 op2, offs_t pc, const upd78k3_disassembler::data_buffer &opcodes)
{
	if ((op2 & 0xf0) == 0x10)
	{
		util::stream_format(stream, "%-8s", "MOV1");
		if (BIT(op1, 0))
			util::stream_format(stream, "%c.%d,CY", BIT(op2, 3) ? 'A' : 'X', op2 & 0x07);
		else
			util::stream_format(stream, "%s,CY", m_psw_bits[op2 & 0x0f]);
		return 2 | SUPPORTED;
	}
	else if (op2 < 0x70)
	{
		util::stream_format(stream, "%-8sCY,", s_bool_ops[(op2 & 0x60) >> 5]);
		if (BIT(op2, 4))
			stream << "/";
		if (BIT(op1, 0))
			util::stream_format(stream, "%c.%d", BIT(op2, 3) ? 'A' : 'X', op2 & 0x07);
		else
			util::stream_format(stream, "%s", m_psw_bits[op2 & 0x0f]);
		return 2 | SUPPORTED;
	}
	else if (op2 < 0xa0)
	{
		util::stream_format(stream, "%-8s", op2 < 0x80 ? "NOT1" : BIT(op2, 4) ? "CLR1" : "SET1");
		if (BIT(op1, 0))
			util::stream_format(stream, "%c.%d", BIT(op2, 3) ? 'A' : 'X', op2 & 0x07);
		else
			util::stream_format(stream, "%s", m_psw_bits[op2 & 0x0f]);
		return 2 | SUPPORTED;
	}
	else if (op2 < 0xe0)
	{
		if (BIT(op2, 6))
			util::stream_format(stream, "%-8s", BIT(op2, 4) ? "BTCLR" : "BFSET");
		else
			util::stream_format(stream, "%-8s", BIT(op2, 4) ? "BT" : "BF");
		if (BIT(op1, 0))
			util::stream_format(stream, "%c.%d,", BIT(op2, 3) ? 'A' : 'X', op2 & 0x07);
		else
			util::stream_format(stream, "%s,", m_psw_bits[op2 & 0x0f]);
		format_jdisp8(stream, pc + 3, opcodes.r8(pc + 2));
		return 3 | STEP_COND | SUPPORTED;
	}
	else
		return dasm_illegal2(stream, op1, op2);
}

offs_t upd78k3_disassembler::dasm_04(std::ostream &stream)
{
	stream << "CVTBW";
	return 1 | SUPPORTED;
}

offs_t upd78k3_disassembler::dasm_05xx(std::ostream &stream, u8 op2)
{
	if ((op2 & 0x88) == 0x08)
	{
		if (BIT(op2, 6))
		{
			util::stream_format(stream, "%-8s", BIT(op2, 4) ? "CALL" : "BR");
			if (BIT(op2, 5))
				util::stream_format(stream, "[%s]", s_rp_names[(op2 & 0x06) >> 1 | (op2 & 0x01) << 2]);
			else
				stream << s_rp_names[(op2 & 0x06) >> 1 | (op2 & 0x01) << 2];
			return 2 | (BIT(op2, 4) ? STEP_OVER : 0) | SUPPORTED;
		}
		else
		{
			if (BIT(op2, 5))
				util::stream_format(stream, "%-8s%s", BIT(op2, 4) ? "MULSW" : "MULUW", s_rp_names[(op2 & 0x06) >> 1 | (op2 & 0x01) << 2]);
			else
				util::stream_format(stream, "%-8s%s", BIT(op2, 4) ? "DIVUW" : "MULU", s_r_names[op2 & 0x07]);
			return 2 | SUPPORTED;
		}
	}
	else if ((op2 & 0xc8) == 0x88)
	{
		if (BIT(op2, 5))
			util::stream_format(stream, "%-8sRB%d%s", "SEL", op2 & 0x07, BIT(op2, 4) ? ",ALT" : "");
		else
			util::stream_format(stream, "%-8s[%s]", BIT(op2, 4) ? "ROL4" : "ROR4", s_rp_names[(op2 & 0x06) >> 1 | (op2 & 0x01) << 2]);
		return 2 | SUPPORTED;
	}
	else if ((op2 & 0xfe) == 0xc8)
	{
		util::stream_format(stream, "%-8sSP", BIT(op2, 4) ? "DECW" : "INCW");
		return 2 | SUPPORTED;
	}
	else if ((op2 & 0xf8) == 0xd8)
	{
		util::stream_format(stream, "%-8sRB%d", "BRKCS", op2 & 0x07);
		return 2 | STEP_OVER | SUPPORTED;
	}
	else if ((op2 & 0xf8) == 0xe8)
	{
		util::stream_format(stream, "%-8s%s", "DIVUX", s_rp_names[(op2 & 0x06) >> 1 | (op2 & 0x01) << 2]);
		return 2 | SUPPORTED;
	}
	else if (op2 >= 0xfe)
	{
		util::stream_format(stream, "ADJB%c", BIT(op2, 0) ? 'S' : 'A');
		return 2 | SUPPORTED;
	}
	else
		return dasm_illegal2(stream, 0x05, op2);
}

offs_t upd78k3_disassembler::dasm_06xx(std::ostream &stream, u8 op2, offs_t pc, const upd78k3_disassembler::data_buffer &opcodes)
{
	if ((op2 & 0x70) < 0x50 && (BIT(op2, 3) || (op2 & (BIT(op2, 7) ? 0x06 : 0x02)) == 0x00))
	{
		if (BIT(op2, 3))
			util::stream_format(stream, "%-8s", s_alu_ops[op2 & 0x07]);
		else if (BIT(op2, 0))
			util::stream_format(stream, "%-8s", BIT(op2, 2) ? "XCHW" : "MOVW");
		else
			util::stream_format(stream, "%-8s", BIT(op2, 2) ? "XCH" : "MOV");
		if ((op2 & 0x8b) == 0x01)
			stream << "AX,";
		else if (!BIT(op2, 7))
			stream << "A,";
		format_ix_disp8(stream, m_ix_bases[(op2 & 0x70) >> 4], opcodes.r8(pc + 2));
		if ((op2 & 0x8b) == 0x81)
			stream << ",AX";
		else if (BIT(op2, 7))
			stream << ",A";
		return 3 | SUPPORTED;
	}
	else
		return dasm_illegal2(stream, 0x06, op2);
}

offs_t upd78k3_disassembler::dasm_07xx(std::ostream &stream, u8 op2, offs_t pc, const upd78k3_disassembler::data_buffer &opcodes)
{
	if (op2 >= 0xf8 && op2 <= 0xfd)
	{
		util::stream_format(stream, "%-8s", s_bcond_07f8[op2 & 0x07]);
		format_jdisp8(stream, pc + 3, opcodes.r8(pc + 2));
		return 3 | STEP_COND | SUPPORTED;
	}
	else if ((op2 & 0xce) == 0xc8)
	{
		if (BIT(op2, 5))
		{
			util::stream_format(stream, "%-8s", BIT(op2, 0) ? "DECW" : "INCW");
			format_saddrp(stream, opcodes.r8(pc + 2));
		}
		else if (BIT(op2, 4))
		{
			util::stream_format(stream, "%-8s", BIT(op2, 0) ? "PUSH" : "POP");
			format_sfrp(stream, opcodes.r8(pc + 2));
		}
		else
		{
			util::stream_format(stream, "%-8s", BIT(op2, 0) ? "CHKLA" : "CHKL");
			format_sfr(stream, opcodes.r8(pc + 2));
		}
		return 3 | SUPPORTED;
	}
	else if (m_has_macw && (op2 | (m_has_macsw ? 0x80 : 0x00)) == 0x85)
	{
		util::stream_format(stream, "%-8s", BIT(op2, 7) ? "MACW" : "MACSW");
		format_count(stream, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;
	}
	else
		return dasm_illegal2(stream, 0x07, op2);
}

offs_t upd78k3_disassembler::dasm_08xx(std::ostream &stream, u8 op2, offs_t pc, const upd78k3_disassembler::data_buffer &opcodes)
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
	else if (op2 < 0xe0)
	{
		if (BIT(op2, 6))
			util::stream_format(stream, "%-8s", BIT(op2, 4) ? "BTCLR" : "BFSET");
		else
			util::stream_format(stream, "%-8s", BIT(op2, 4) ? "BT" : "BF");
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

offs_t upd78k3_disassembler::dasm_09xx_sfrmov(std::ostream &stream, u8 sfr, offs_t pc, const upd78k3_disassembler::data_buffer &opcodes)
{
	util::stream_format(stream, "%-8s", "MOV");
	format_sfr(stream, sfr);
	stream << ",";
	u8 nbyte = opcodes.r8(pc + 2);
	u8 pbyte = opcodes.r8(pc + 3);
	if ((nbyte ^ 0xff) == pbyte)
		format_imm8(stream, pbyte);
	else
		stream << "invalid";
	return 4 | SUPPORTED;
}

offs_t upd78k3_disassembler::dasm_09xx(std::ostream &stream, u8 op2, offs_t pc, const upd78k3_disassembler::data_buffer &opcodes)
{
	if ((op2 & 0xe8) == 0x80)
	{
		util::stream_format(stream, "%-8s", "MOVW");
		if (BIT(op2, 4))
		{
			format_abs16(stream, opcodes.r16(pc + 2));
			util::stream_format(stream, ",%s", (op2 & 0x06) >> 1 | (op2 & 0x01) << 2);
		}
		else
		{
			util::stream_format(stream, "%s,", (op2 & 0x06) >> 1 | (op2 & 0x01) << 2);
			format_abs16(stream, opcodes.r16(pc + 2));
		}
		return 4 | SUPPORTED;
	}
	else if (op2 == 0xa0 && m_has_macw)
	{
		util::stream_format(stream, "%-8s", "MOVTBLW");
		format_abs16(stream, 0xfe00 | opcodes.r8(pc + 2));
		stream << ",";
		format_count(stream, opcodes.r8(pc + 3));
		return 4 | SUPPORTED;
	}
	else if (op2 == 0xb0 && m_has_macsw)
	{
		util::stream_format(stream, "%-8s", "SACW");
		u8 post1 = opcodes.r8(pc + 2);
		u8 post2 = opcodes.r8(pc + 3);
		if (post1 == 0x41 && post2 == 0x46)
			util::stream_format(stream, "[%s+],[%s+]", m_ix_bases[0], m_ix_bases[2]);
		else
			stream << "invalid";
		return 4 | SUPPORTED;
	}
	else if (op2 == 0xc0 || op2 == 0xc2)
		return dasm_09xx_sfrmov(stream, op2, pc, opcodes);
	else if (op2 == 0xe0)
	{
		util::stream_format(stream, "%-8s", "RETCSB");
		format_abs16(stream, opcodes.r16(pc + 2));
		return 4 | SUPPORTED;
	}
	else if ((op2 & 0xfe) == 0xf0)
	{
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

offs_t upd78k3_disassembler::dasm_0axx(std::ostream &stream, u8 op2, offs_t pc, const upd78k3_disassembler::data_buffer &opcodes)
{
	if (!BIT(op2, 6) && (BIT(op2, 3) || (op2 & (BIT(op2, 7) ? 0x06 : 0x02)) == 0x00))
	{
		if (BIT(op2, 3))
			util::stream_format(stream, "%-8s", s_alu_ops[op2 & 0x07]);
		else if (BIT(op2, 0))
			util::stream_format(stream, "%-8s", BIT(op2, 2) ? "XCHW" : "MOVW");
		else
			util::stream_format(stream, "%-8s", BIT(op2, 2) ? "XCH" : "MOV");
		if ((op2 & 0x8b) == 0x01)
			stream << "AX,";
		else if (!BIT(op2, 7))
			stream << "A,";

		if (BIT(op2, 4))
			format_ix_base16(stream, BIT(op2, 5) ? "B" : "A", opcodes.r16(pc + 2));
		else
			format_ix_disp16(stream, m_ix_bases[(op2 & 0x20) >> 4], opcodes.r16(pc + 2));

		if ((op2 & 0x8b) == 0x81)
			stream << ",AX";
		else if (BIT(op2, 7))
			stream << ",A";
		return 4 | SUPPORTED;
	}
	else
		return dasm_illegal2(stream, 0x0a, op2);
}

offs_t upd78k3_disassembler::dasm_15xx(std::ostream &stream, u8 op2)
{
	if ((op2 & 0xc8) == 0x00 && (op2 & 0x06) != 0x02)
	{
		if (BIT(op2, 2))
			util::stream_format(stream, "%-8s", util::string_format("CMP%s%s", BIT(op2, 5) ? "BK" : "M", s_cmpscond[op2 & 0x03]));
		else
			util::stream_format(stream, "%-8s", util::string_format("%s%s", BIT(op2, 0) ? "XCH" : "MOV", BIT(op2, 5) ? "BK" : "M"));
		util::stream_format(stream, "[%s%c],", m_ix_bases[0], BIT(op2, 4) ? '-' : '+');
		if (BIT(op2, 5))
			util::stream_format(stream, "[%s%c]", m_ix_bases[2], BIT(op2, 4) ? '-' : '+');
		else
			stream << "A";
		return 2 | SUPPORTED;
	}
	else
		return dasm_illegal2(stream, 0x15, op2);
}

offs_t upd78k3_disassembler::dasm_16xx(std::ostream &stream, u8 op1, u8 op2)
{
	if ((!BIT(op1, 0) || (op2 & 0x60) != 0x60) && (BIT(op2, 3) || (op2 & (BIT(op2, 7) ? 0x06 : 0x02)) == 0x00))
	{
		if (BIT(op2, 3))
			util::stream_format(stream, "%-8s", s_alu_ops[op2 & 0x07]);
		else if (BIT(op2, 0))
			util::stream_format(stream, "%-8s", BIT(op2, 2) ? "XCHW" : "MOVW");
		else
			util::stream_format(stream, "%-8s", BIT(op2, 2) ? "XCH" : "MOV");
		if ((op2 & 0x8b) == 0x01)
			stream << "AX,";
		else if (!BIT(op2, 7))
			stream << "A,";

		stream << "[";
		if (BIT(op1, 0) && (op2 & 0x60) == 0x40)
			util::stream_format(stream, "%s+%s", m_ix_bases[4], BIT(op2, 4) ? "HL" : "DE");
		else if ((op2 & 0x60) == 0x60)
			stream << m_ix_bases[BIT(op2, 4) ? 3 : 4];
		else
		{
			stream << m_ix_bases[(op2 & 0x10) >> 3];
			if (!BIT(op2, 6))
			{
				if (BIT(op1, 0))
					util::stream_format(stream, "+%c", BIT(op2, 5) ? 'B' : 'A');
				else
					stream << (BIT(op2, 5) ? "-" : "+");
			}
		}
		stream << "]";

		if ((op2 & 0x8b) == 0x81)
			stream << ",AX";
		else if (BIT(op2, 7))
			stream << ",A";
		return 2 | SUPPORTED;
	}
	else
		return dasm_illegal2(stream, 0x16, op2);
}

offs_t upd78k3_disassembler::dasm_24xx(std::ostream &stream, u8 op, u8 rr)
{
	if (!BIT(rr, 3))
	{
		util::stream_format(stream, "%-8s%s,%s", BIT(op, 0) ? "XCH" : "MOV", s_r_names[(rr & 0xf0) >> 4], s_r_names[rr & 0x07]);
		return 2 | SUPPORTED;
	}
	else if (!BIT(rr, 4))
	{
		util::stream_format(stream, "%-8s%s,%s", BIT(op, 0) ? "XCHW" : "MOVW", s_rp_names[(rr & 0xe0) >> 5], s_rp_names[(rr & 0x06) >> 1 | (rr & 0x01) << 2]);
		return 2 | SUPPORTED;
	}
	else
		return dasm_illegal2(stream, op, rr);
}

offs_t upd78k3_disassembler::dasm_2a(std::ostream &stream, offs_t pc, const upd78k3_disassembler::data_buffer &opcodes)
{
	util::stream_format(stream, "%-8s", "XCHW");
	format_saddrp(stream, opcodes.r8(pc + 2));
	stream << ",";
	format_saddrp(stream, opcodes.r8(pc + 1));
	return 3 | SUPPORTED;
}

offs_t upd78k3_disassembler::dasm_38(std::ostream &stream, u8 op, offs_t pc, const upd78k3_disassembler::data_buffer &opcodes)
{
	util::stream_format(stream, "%-8s", BIT(op, 0) ? "XCH" : "MOV");
	format_saddr(stream, opcodes.r8(pc + 2));
	stream << ",";
	format_saddr(stream, opcodes.r8(pc + 1));
	return 3 | SUPPORTED;
}

offs_t upd78k3_disassembler::dasm_3c(std::ostream &stream, u8 op, offs_t pc, const upd78k3_disassembler::data_buffer &opcodes)
{
	util::stream_format(stream, "%-8s", s_16bit_ops[op & 0x03]);
	format_saddrp(stream, opcodes.r8(pc + 2));
	stream << ",";
	format_saddrp(stream, opcodes.r8(pc + 1));
	return 3 | SUPPORTED;
}

offs_t upd78k3_disassembler::dasm_43(std::ostream &stream, offs_t pc, const upd78k3_disassembler::data_buffer &opcodes)
{
	stream << "SWRS";
	return 1 | SUPPORTED;
}

offs_t upd78k3_disassembler::dasm_50(std::ostream &stream, u8 op)
{
	if ((op & 0x06) == 0x06)
	{
		if (op == 0x5e)
		{
			stream << "BRK";
			return 1 | STEP_OVER | SUPPORTED;
		}
		else
		{
			util::stream_format(stream, "RET%s", BIT(op, 3) ? "B" : BIT(op, 0) ? "I" : "");
			return 1 | STEP_OUT | SUPPORTED;
		}
	}
	else
	{
		util::stream_format(stream, "%-8s", "MOV");
		if (BIT(op, 3))
			stream << "A,";
		util::stream_format(stream, "[%s%s]", m_ix_bases[(op & 0x01) << 1], BIT(op, 2) ? "" : BIT(op, 1) ? "-" : "+");
		if (!BIT(op, 3))
			stream << ",A";
		return 1 | SUPPORTED;
	}
}

offs_t upd78k3_disassembler::dasm_78(std::ostream &stream, u8 op, offs_t pc, const upd78k3_disassembler::data_buffer &opcodes)
{
	util::stream_format(stream, "%-8s", s_alu_ops[op & 0x07]);
	format_saddr(stream, opcodes.r8(pc + 2));
	stream << ",";
	format_saddr(stream, opcodes.r8(pc + 1));
	return 3 | SUPPORTED;
}

offs_t upd78k3_disassembler::dasm_88xx(std::ostream &stream, u8 op, u8 rr)
{
	if (!BIT(rr, 3))
	{
		util::stream_format(stream, "%-8s%s,%s", s_alu_ops[op & 0x07], s_r_names[(rr & 0xf0) >> 4], s_r_names[rr & 0x07]);
		return 2 | SUPPORTED;
	}
	else if (!BIT(rr, 4) && (op == 0x88 || op == 0x8a || op == 0x8f))
	{
		util::stream_format(stream, "%-8s%s,%s", std::string(s_alu_ops[op & 0x07]) + "W", s_rp_names[(rr & 0xe0) >> 5], s_rp_names[(rr & 0x06) >> 1 | (rr & 0x01) << 2]);
		return 2 | SUPPORTED;
	}
	else
		return dasm_illegal2(stream, op, rr);
}

offs_t upd78k3_disassembler::disassemble(std::ostream &stream, offs_t pc, const upd78k3_disassembler::data_buffer &opcodes, const upd78k3_disassembler::data_buffer &params)
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

		case 0x04:
			return dasm_04(stream);

		case 0x05:
			return dasm_05xx(stream, opcodes.r8(pc + 1));

		case 0x06:
			return dasm_06xx(stream, opcodes.r8(pc + 1), pc, opcodes);

		case 0x07:
			return dasm_07xx(stream, opcodes.r8(pc + 1), pc, opcodes);

		default: // can't happen here, though compilers believe it could
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

		default:
			if (BIT(op, 2))
			{
				util::stream_format(stream, "%-8s", s_16bit_ops[op & 0x03]);
				format_saddrp(stream, opcodes.r8(pc + 1));
			}
			else
			{
				u8 sfrp = opcodes.r8(pc + 1);
				util::stream_format(stream, "%-8s", "MOVW");
				if (sfrp == 0xfc)
					stream << "SP";
				else
					format_sfrp(stream, sfrp);
			}
			stream << ",";
			format_imm16(stream, opcodes.r16(pc + 2));
			return 4 | SUPPORTED;
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
				if (sfr >= 0xfe)
					util::stream_format(stream, "PSW%c", BIT(sfr, 0) ? 'H' : 'L');
				else
					format_sfr(stream, sfr);
				if (BIT(op, 1))
					stream << ",A";
			}
			return 2 | SUPPORTED;
		}
		else if (BIT(op, 1))
			return dasm_16xx(stream, op, opcodes.r8(pc + 1));
		else if (BIT(op, 0))
			return dasm_15xx(stream, opcodes.r8(pc + 1));
		else
		{
			util::stream_format(stream, "%-8s", "BR");
			format_jdisp8(stream, pc + 2, opcodes.r8(pc + 1));
			return 2 | SUPPORTED;
		}

	case 0x18:
		if (BIT(op, 2))
		{
			util::stream_format(stream, "%-8sAX,", s_16bit_ops[op & 0x03]);
			format_saddrp(stream, opcodes.r8(pc + 1));
		}
		else if (BIT(op, 1))
		{
			if (BIT(op, 0))
				util::stream_format(stream, "%-8sAX,", "XCHW");
			else
				util::stream_format(stream, "%-8s", "MOVW");
			format_saddrp(stream, opcodes.r8(pc + 1));
			if (!BIT(op, 0))
				stream << ",AX";
		}
		else
		{
			util::stream_format(stream, "%-8s", "MOV");
			if (!BIT(op, 0))
				stream << "A,";
			stream << "[";
			format_saddrp(stream, opcodes.r8(pc + 1));
			stream << "]";
			if (BIT(op, 0))
				stream << ",A";
		}
		return 2 | SUPPORTED;

	case 0x20:
		if (BIT(op, 2))
		{
			if (BIT(op, 1))
			{
				util::stream_format(stream, "%-8s", BIT(op, 0) ? "DEC" : "INC");
				format_saddr(stream, opcodes.r8(pc + 1));
				return 2 | SUPPORTED;
			}
			else
				return dasm_24xx(stream, op, opcodes.r8(pc + 1));
		}
		else
		{
			util::stream_format(stream, "%-8s", BIT(op, 0) ? "XCH" : "MOV");
			if (op != 0x22)
				stream << "A,";
			if (op == 0x23)
			{
				stream << "[";
				format_saddrp(stream, opcodes.r8(pc + 1));
				stream << "]";
			}
			else
			{
				format_saddr(stream, opcodes.r8(pc + 1));
				if (op == 0x22)
					stream << ",A";
			}
			return 2 | SUPPORTED;
		}

	case 0x28:
		switch (op)
		{
		case 0x28:
			util::stream_format(stream, "%-8s", "CALL");
			format_abs16(stream, opcodes.r16(pc + 1));
			return 3 | STEP_OVER | SUPPORTED;

		case 0x29:
			util::stream_format(stream, "%-8s", "RETCS");
			format_abs16(stream, opcodes.r16(pc + 1));
			return 3 | STEP_OUT | SUPPORTED;

		case 0x2a:
			return dasm_2a(stream, pc, opcodes);

		case 0x2b:
		{
			util::stream_format(stream, "%-8s", "MOV");
			u8 sfr = opcodes.r8(pc + 1);
			if (sfr >= 0xfe)
				util::stream_format(stream, "PSW%c", BIT(sfr, 0) ? 'H' : 'L');
			else
				format_sfr(stream, sfr);
			stream << ",";
			format_imm8(stream, opcodes.r8(pc + 2));
			return 3 | SUPPORTED;
		}

		case 0x2c:
			util::stream_format(stream, "%-8s", "BR");
			format_abs16(stream, opcodes.r16(pc + 1));
			return 3 | SUPPORTED;

		default:
			util::stream_format(stream, "%-8sAX,", s_16bit_ops[op & 0x03]);
			format_imm16(stream, opcodes.r16(pc + 1));
			return 3 | SUPPORTED;
		}

	case 0x30:
		if (BIT(op, 2))
		{
			u8 post = opcodes.r8(pc + 1);
			if (BIT(op, 0))
			{
				util::stream_format(stream, "%-8s", BIT(op, 1) ? "PUSHU" : "PUSH");
				if (post == 0)
					stream << "0"; // none
				else for (int n = 7; n >= 0; n--)
				{
					if (BIT(post, n))
					{
						if (n == 5 && BIT(op, 1))
							stream << "PSW";
						else
							stream << s_rp_names[n];
						post &= ~(1 << n);
						if (post != 0)
							stream << ",";
					}
				}
			}
			else
			{
				util::stream_format(stream, "%-8s", BIT(op, 1) ? "POPU" : "POP");
				if (post == 0)
					stream << "0"; // none
				else for (int n = 0; n < 8; n++)
				{
					if (BIT(post, n))
					{
						if (n == 5 && BIT(op, 1))
							stream << "PSW";
						else
							stream << s_rp_names[n];
						post &= ~(1 << n);
						if (post != 0)
							stream << ",";
					}
				}
			}
			return 2 | SUPPORTED;
		}
		else if (BIT(op, 1))
		{
			util::stream_format(stream, "%-8s%c,", "DBNZ", BIT(op, 0) ? 'B' : 'C');
			format_jdisp8(stream, pc + 2, opcodes.r8(pc + 1));
			return 2 | STEP_COND | SUPPORTED;
		}
		else
		{
			u8 n = opcodes.r8(pc + 1);
			util::stream_format(stream, "%-8s", s_shift_ops[op & 0x01][(n & 0xc0) >> 6]);
			if (n >= 0xc0)
				stream << s_rp_names[(n & 0x06) >> 1 | (n & 0x01) << 2];
			else
				stream << s_r_names[n & 0x07];
			util::stream_format(stream, ",%d", (n & 0x38) >> 3);
			return 2 | SUPPORTED;
		}

	case 0x38:
		if (BIT(op, 2))
			return dasm_3c(stream, op, pc, opcodes);
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
			util::stream_format(stream, "%-8s%s", "INCW", s_rp_names[op & 0x07]);
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
			util::stream_format(stream, "%-8s%s", "DECW", s_rp_names[op & 0x07]);
		else if (BIT(op, 1))
			util::stream_format(stream, "%cI", BIT(op, 0) ? 'E' : 'D');
		else
			util::stream_format(stream, "%-8sPSW", BIT(op, 0) ? "PUSH" : "POP");
		return 1 | SUPPORTED;

	case 0x50: case 0x58:
		return dasm_50(stream, op);

	case 0x60:
		util::stream_format(stream, "%-8s%s,", "MOVW", s_rp_names[(op & 0x06) >> 1 | (op & 0x01) << 2]);
		format_imm16(stream, opcodes.r16(pc + 1));
		return 3 | SUPPORTED;

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
		util::stream_format(stream, "%-8s", s_bcond[op & 0x07]);
		format_jdisp8(stream, pc + 2, opcodes.r8(pc + 1));
		return 2 | STEP_COND | SUPPORTED;

	case 0x88:
		return dasm_88xx(stream, op, opcodes.r8(pc + 1));

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
		// vectors may be at 00XXH or 80XXH depending on the state of TPF (CCW.1)
		util::stream_format(stream, "%-8s[%02XH]", "CALLT", (op & 0x3f) << 1);
		return 1 | STEP_OVER | SUPPORTED;

	default: // can't happen here, though compilers believe it could
		return dasm_illegal(stream, op);
	}
}

upd78312_disassembler::upd78312_disassembler()
	: upd78k3_disassembler(s_sfr_names, s_sfrp_names, s_psw_bits, false, false)
{
}

const char *const upd78312_disassembler::s_sfr_names[256] =
{
	"P0", "P1", "P2", "P3", "P4", "P5", nullptr, nullptr,
	"CR00L", "CR00H", "CR01L", "CR01H", "CR10L", "CR10H", "CR11L", "CR11H",
	"CPT0L", "CPT0H", "CPT1L", "CPT1H", "PWM0L", "PWM0H", "PWM1L", "PWM1H",
	nullptr, nullptr, nullptr, nullptr, "UDC0L", "UDC0H", "UDC1L", "UDC1H",
	"PM0", "PM1", "PM2", "PM3", nullptr, "PM5", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, "PMC2", "PMC3", nullptr, nullptr, nullptr, nullptr,
	"RTPC", nullptr, "P0L", "P0H", nullptr, nullptr, nullptr, nullptr,
	"MM", "RFM", "WDM", nullptr, "STBC", nullptr, "TBM", nullptr,
	"INTM", nullptr, "ISPR", nullptr, nullptr, nullptr, "CCW", nullptr,
	"SCM", nullptr, "SCC", "BRG", nullptr, nullptr, "RXB", "TXB",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"FRCC", nullptr, nullptr, nullptr, "CPTM", nullptr, "PWMM", nullptr,
	"ADM", nullptr, "ADCR", nullptr, nullptr, nullptr, nullptr, nullptr,
	"CUIM", nullptr, "UDCC0", nullptr, "CRC", nullptr, nullptr, nullptr,
	nullptr, nullptr, "UDCC1", nullptr, nullptr, nullptr, nullptr, nullptr,
	"TMC0", nullptr, "TMC1", nullptr, nullptr, nullptr, nullptr, nullptr,
	"TM0L", "TM0H", "MD0L", "MD0H", "TM1L", "TM1H", "MD1L", "MD1H",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external access
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external access
	"CRIC00", "CRMS00", "CRIC01", nullptr, "CRIC10", "CRMS10", "CRIC11", nullptr,
	"EXIC0", "EXMS0", "EXIC1", "EXMS1", "EXIC2", "EXMS2", "TMIC0", "TMMS0",
	"TMIC1", "TMMS1", "TMIC2", "TMMS2", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, "SEIC", nullptr, "SRIC", "SRMS", "STIC", "STMS",
	"ADIC", "ADMS", "TBIC", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

const char *const upd78312_disassembler::s_sfrp_names[128] =
{
	nullptr, nullptr, nullptr, nullptr, "CR00", "CR01", "CR10", "CR11",
	"CPT0", "CPT1", "PWM0", "PWM1", nullptr, nullptr, "UDC0", "UDC1",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, "TM0", "MD0", "TM1", "MD1",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

const char *const upd78312_disassembler::s_psw_bits[16] =
{
	"CY",
	"SUB",
	"P/V",
	"UF",
	"AC",
	"RSS",
	"Z",
	"S",
	"PSWH.0",
	"IE",
	"PSWH.2",
	"PSWH.3",
	"RBS0",
	"RBS1",
	"RBS2",
	"PSWH.7"
};

offs_t upd78312_disassembler::dasm_04(std::ostream &stream)
{
	stream << "ADJ4";
	return 1 | SUPPORTED;
}

offs_t upd78312_disassembler::dasm_05xx(std::ostream &stream, u8 op2)
{
	// no MULW or ADJBA/ADJBS (which replaced ADJ4)
	if (op2 < 0xfe && (op2 & 0xf0) != 0x30)
		return upd78k3_disassembler::dasm_05xx(stream, op2);
	else
		return dasm_illegal2(stream, 0x05, op2);
}

offs_t upd78312_disassembler::dasm_06xx(std::ostream &stream, u8 op2, offs_t pc, const upd78312_disassembler::data_buffer &opcodes)
{
	// no MOVW/XCHW variants
	if ((op2 & 0x0b) != 0x01)
		return upd78k3_disassembler::dasm_06xx(stream, op2, pc, opcodes);
	else
		return dasm_illegal2(stream, 0x06, op2);
}

offs_t upd78312_disassembler::dasm_07xx(std::ostream &stream, u8 op2, offs_t pc, const upd78312_disassembler::data_buffer &opcodes)
{
	// no CHKL(A) or PUSH/POP sfrp
	if ((op2 & 0xee) != 0xc8)
		return upd78k3_disassembler::dasm_07xx(stream, op2, pc, opcodes);
	else
		return dasm_illegal2(stream, 0x07, op2);
}

offs_t upd78312_disassembler::dasm_09xx(std::ostream &stream, u8 op2, offs_t pc, const upd78312_disassembler::data_buffer &opcodes)
{
	// STBC and WDM are moved; no RETCSB (also no absolute MOVW except on µPD78310A/312A)
	if (op2 == 0x42 || op2 == 0x44)
		return dasm_09xx_sfrmov(stream, op2, pc, opcodes);
	else if ((op2 & 0xd0) != 0xc0)
		return upd78k3_disassembler::dasm_09xx(stream, op2, pc, opcodes);
	else
		return dasm_illegal2(stream, 0x09, op2);
}

offs_t upd78312_disassembler::dasm_0axx(std::ostream &stream, u8 op2, offs_t pc, const upd78312_disassembler::data_buffer &opcodes)
{
	// no MOVW/XCHW variants
	if ((op2 & 0x0b) != 0x01)
		return upd78k3_disassembler::dasm_0axx(stream, op2, pc, opcodes);
	else
		return dasm_illegal2(stream, 0x0a, op2);
}

offs_t upd78312_disassembler::dasm_16xx(std::ostream &stream, u8 op1, u8 op2)
{
	// no MOVW/XCHW variants
	if ((op2 & 0x0b) != 0x01)
		return upd78k3_disassembler::dasm_16xx(stream, op1, op2);
	else
		return dasm_illegal2(stream, op1, op2);
}

offs_t upd78312_disassembler::dasm_50(std::ostream &stream, u8 op)
{
	// no RETB
	if (op != 0x5f)
		return upd78k3_disassembler::dasm_50(stream, op);
	else
		return dasm_illegal(stream, op);
}

upd78322_disassembler::upd78322_disassembler()
	: upd78k3_disassembler(s_sfr_names, s_sfrp_names, s_psw_bits, false, false)
{
}

const char *const upd78322_disassembler::s_sfr_names[256] =
{
	"P0", nullptr, "P2", "P3", "P4", "P5", nullptr, "P7",
	"P8", "P9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"PM0", nullptr, nullptr, "PM3", nullptr, "PM5", nullptr, nullptr,
	"PM8", "PM9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"PMC0", "RTPS", nullptr, "PMC3", nullptr, nullptr, nullptr, nullptr,
	"PMC8", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"RTP", "RTPR", "PRDC", nullptr, nullptr, nullptr, nullptr, nullptr,
	"ADM", nullptr, nullptr, "ADCRH", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CSIM", nullptr, "SBIC", nullptr, nullptr, nullptr, "SIO", nullptr,
	"ASIM", nullptr, "ASIS", nullptr, "RXB", nullptr, "TXS", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TMC", "BRGM", "PRM", nullptr, nullptr, nullptr, nullptr, nullptr,
	"TOC0", "TOC1", nullptr, nullptr, nullptr, nullptr, nullptr, "RPM",
	"STBC", "CCW", "WDM", nullptr, "MM", nullptr, "PWC", nullptr,
	nullptr, "FCC", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external access
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external access
	"IF0L", "IF0H", "IF1L", nullptr, "MK0L", "MK0H", "MK1L", nullptr,
	"PB0L", "PB0H", "PB1L", nullptr, "ISM0L", "ISM0H", "ISM1L", nullptr,
	"CSE0L", "CSE0H", "CSE1L", nullptr, "INTM0", "INTM1", nullptr, nullptr,
	"ISPR", "PRSL", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

const char *const upd78322_disassembler::s_sfrp_names[128] =
{
	nullptr, nullptr, nullptr, nullptr, nullptr, "TM0LW", nullptr, nullptr,
	"CTX0LW", "CT01LW", "CT02LW", "CT03LW", "CCX0LW", "CC01LW", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, "TM0UW", "TM1", nullptr,
	"CTX0UW", "CT01UW", "CT02UW", "CT03UW", "CCX0UW", "CC01UW", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "BRG", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, "ADCR", nullptr, nullptr,
	"CM00", "CM01", "CM02", "CM03", nullptr, nullptr, "CM10", "CM11",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"IF0", "IF1", "MK0", "MK1", "PB0", "PB1", "ISM0", "ISM1",
	"CSE0", "CSE1", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

upd78328_disassembler::upd78328_disassembler()
	: upd78k3_disassembler(s_sfr_names, s_sfrp_names, s_psw_bits, false, false)
{
}

const char *const upd78328_disassembler::s_sfr_names[256] =
{
	"P0", nullptr, "P2", "P3", "P4", "P5", nullptr, "P7",
	"P8", "P9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"PM0", nullptr, nullptr, "PM3", nullptr, "PM5", nullptr, nullptr,
	"PM8", "PM9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, "PMC3", nullptr, nullptr, nullptr, nullptr,
	"PMC8", nullptr, nullptr, nullptr, "BRG", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"P0L", "P0H", "PRDC", "RTPC", "PWMC", nullptr, "PWMB", nullptr,
	"ADM", nullptr, nullptr, "ADCRH", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CSIM", nullptr, "SBIC", nullptr, nullptr, nullptr, "SIO", nullptr,
	"ASIM", nullptr, "ASIS", nullptr, "RXB", nullptr, "TXS", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TMC0", "BRGM", "TMC1", nullptr, "TUM", nullptr, nullptr, nullptr,
	nullptr, nullptr, "TOUT", nullptr, nullptr, nullptr, nullptr, nullptr,
	"STBC", "CCW", "WDM", nullptr, "MM", nullptr, "PWC", nullptr,
	nullptr, "FCC", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external access
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external access
	"IF0L", "IF0H", "IF1L", nullptr, "MK0L", "MK0H", "MK1L", nullptr,
	"PB0L", "PB0H", "PB1L", nullptr, "ISM0L", "ISM0H", "ISM1L", nullptr,
	"CSE0L", "CSE0H", "CSE1L", nullptr, "INTM0", "INTM1", nullptr, nullptr,
	"ISPR", "PRSL", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

const char *const upd78328_disassembler::s_sfrp_names[128] =
{
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "TM2", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "CC10", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, "TM0", "TM1", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, "ADCR", nullptr, nullptr,
	"CM00S", "CM01S", "CM02S", "CM03S", "CM04S", "CM05S", "CM06", "CM20",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CM00R", "CM01R", "CM02R", "CM03R", "CM04R", "CM05R", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"IF0", "IF1", "MK0", "MK1", "PB0", "PB1", "ISM0", "ISM1",
	"CSE0", "CSE1", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

upd78334_disassembler::upd78334_disassembler()
	: upd78k3_disassembler(s_sfr_names, s_sfrp_names, s_psw_bits, false, false)
{
}

const char *const upd78334_disassembler::s_sfr_names[256] =
{
	"P0", "P1", "P2", "P3", "P4", "P5", nullptr, "P7",
	"P8", "P9", "TLA", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"PM0", "PM1", nullptr, "PM3", nullptr, "PM5", nullptr, nullptr,
	nullptr, "PM9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"PMC0", "PMC1", nullptr, "PMC3", nullptr, nullptr, nullptr, nullptr,
	"PMC8", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"RTP", "RTPR", "PRDC", "RTPS", "PWMC", nullptr, "PWM0", nullptr,
	"ADM", nullptr, nullptr, nullptr, nullptr, nullptr, "PWM1", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CSIM", nullptr, "SBIC", nullptr, nullptr, nullptr, "SIO", nullptr,
	"ASIM", nullptr, "ASIS", nullptr, "RXB", nullptr, "TXS", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, "ADCR0H", nullptr, "ADCR1H", nullptr, "ADCR2H", nullptr, "ADCR3H",
	nullptr, "ADCR4H", nullptr, "ADCR5H", nullptr, "ADCR6H", nullptr, "ADCR7H",
	"TMC0", "BRGM", "TMC1", nullptr, "TUM0", "TUM1", nullptr, nullptr,
	"TOC0", "TOC1", nullptr, nullptr, "PPOS", nullptr, nullptr, nullptr,
	"STBC", "CCW", "WDM", nullptr, "MM", nullptr, "PWC", nullptr,
	nullptr, "FCC", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external access
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external access
	"IF0L", "IF0H", "IF1L", nullptr, "MK0L", "MK0H", "MK1L", nullptr,
	"PB0L", "PB0H", "PB1L", nullptr, "ISM0L", "ISM0H", "ISM1L", nullptr,
	"CSE0L", "CSE0H", "CSE1L", nullptr, "INTM0", "INTM1", nullptr, nullptr,
	"ISPR", "PRSL", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

const char *const upd78334_disassembler::s_sfrp_names[128] =
{
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "TM2", nullptr,
	"CT00", "CT01", "CT02", nullptr, nullptr, nullptr, "CT10", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, "TM0", "TM1", "TM3",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "BRG", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CM11", "CM12", "CM20", "CM21", "CM30", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CMX0", "CM01R", "CM02R", "CM03R", "CM04R", "CC00R", "CC01R", nullptr,
	"ADCR0", "ADCR1", "ADCR2", "ADCR3", "ADCR4", "ADCR5", "ADCR6", "ADCR7",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"IF0", "IF1", "MK0", "MK1", "PB0", "PB1", "ISM0", "ISM1",
	"CSE0", "CSE1", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

upd78352_disassembler::upd78352_disassembler()
	: upd78k3_disassembler(s_sfr_names, s_sfrp_names, s_psw_bits, true, false)
{
}

const char *const upd78352_disassembler::s_sfr_names[256] =
{
	"P0", "P1", "P2", "P3", "P4", "P5", nullptr, nullptr,
	nullptr, "P9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"PM0", "PM1", nullptr, "PM3", nullptr, "PM5", nullptr, nullptr,
	nullptr, "PM9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TMC0", "TMC1", nullptr, nullptr, "INTM0", "INTM1", nullptr, nullptr,
	nullptr, nullptr, nullptr, "PMC3", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, "PRDC", nullptr, "PWMC", nullptr, "PWM0", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "PWM1", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"ISPR", nullptr, "IMC", nullptr, "MKL", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"STBC", "CCW", "WDM", nullptr, "MM", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external access
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external access
	"OVIC", "PIC0", "PIC1", "CMIC10", "CMIC20", "PIC2", "PIC3", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

const char *const upd78352_disassembler::s_sfrp_names[128] =
{
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CT00", "CT01", "CM10", nullptr, nullptr, nullptr, nullptr, "CM20",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TM0", "TM1", "TM2", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "MK",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, "PWC", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

upd78356_disassembler::upd78356_disassembler()
	: upd78k3_disassembler(s_sfr_names, s_sfrp_names, s_psw_bits, true, true)
{
}

const char *const upd78356_disassembler::s_sfr_names[256] =
{
	"P0", "P1", "P2", "P3", "P4", "P5", "P6", "P7",
	"P8", "P9", "P10", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"PM0", "PM1", "PM2", "PM3", nullptr, "PM5", nullptr, nullptr,
	"PM8", "PM9", "PM10", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, "INTM0", "INTM1", nullptr, nullptr,
	"PMC0", nullptr, "PMC2", "PMC3", "PUOL", "PUOH", nullptr, nullptr,
	"PMC8", nullptr, "PMC10", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"RTPL", "RTPH", "PRDC", "RTPM", nullptr, nullptr, nullptr, nullptr,
	"ADM0", "ADM1", "DACS0", "DACS1", nullptr, nullptr, nullptr, nullptr,
	"TUM0", "TUM1", "TUM2", "TUM3", "TMC0", "TMC1", "TMC2", "UDCC",
	"TOC0", "TOC1", "TOC2", "TOVS", "NPC", nullptr, nullptr, nullptr,
	"CSIM0", nullptr, "SBIC", nullptr, nullptr, nullptr, "SIO0", nullptr,
	"ASIM", nullptr, "ASIS", nullptr, "RXB", nullptr, "TXS", nullptr,
	"CSIM1", nullptr, nullptr, nullptr, nullptr, nullptr, "SIO1", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"PWMC", nullptr, "PWM0L", nullptr, "PWM1L", nullptr, nullptr, nullptr,
	"ISPR", nullptr, "IMC", nullptr, "MK0L", "MK0H", "MK1L", nullptr,
	nullptr, "ADCR0H", nullptr, "ADCR1H", nullptr, "ADCR2H", nullptr, "ADCR3H",
	nullptr, "ADCR4H", nullptr, "ADCR5H", nullptr, "ADCR6H", nullptr, "ADCR7H",
	"STBC", "CCW", "WDM", nullptr, "MM", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external access
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external access
	"OVIC0", "OVIC3", "PIC0", "PIC1", "PIC2", "PIC3", "PIC4", "CMIC00",
	"CMIC01", "CMIC02", "CMIC03", "CMIC10", "CMIC11", "CMIC20", "CMIC21", "CMIC40",
	"CMICUD0", "CMICUD1", "SERIC", "SRIC", "STIC", "CSIIC0", "CSIIC1", "ADIC",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

const char *const upd78356_disassembler::s_sfrp_names[128] =
{
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CC00", "CC01", "CC02", "CC30", "CC31", "CM00", "CM01", "CM02",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TM0", "TM1", "TM2", "TM3", "TM4", "UDC", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CM03", "CM10", "CM11", "CM20", "CM21", "CM40", "CMUD0", "CMUD1",
	nullptr, nullptr, nullptr, nullptr, nullptr, "DACS", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, "PWM0", "PWM1", nullptr, nullptr, nullptr, "MK0", "MK1",
	"ADCR0", "ADCR1", "ADCR2", "ADCR3", "ADCR4", "ADCR5", "ADCR6", "ADCR7",
	nullptr, nullptr, nullptr, "PWC", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

upd78366a_disassembler::upd78366a_disassembler()
	: upd78k3_disassembler(s_sfr_names, s_sfrp_names, s_psw_bits, true, true)
{
}

const char *const upd78366a_disassembler::s_sfr_names[256] =
{
	"P0", "P1", "P2", "P3", "P4", "P5", nullptr, "P7",
	"P8", "P9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"PM0", "PM1", nullptr, "PM3", nullptr, "PM5", nullptr, nullptr,
	"PM8", "PM9", nullptr, nullptr, nullptr, nullptr, "TUM0", "TUM1",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, "INTM0", "INTM1", nullptr, nullptr,
	"PMC0", nullptr, nullptr, "PMC3", "PUOL", "PUOH", nullptr, nullptr,
	"PMC8", nullptr, nullptr, nullptr, nullptr, nullptr, "SMPC0", "SMPC1",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "TMC4", "TOUT",
	"RTP", "RTPM", "PRDC", nullptr, nullptr, nullptr, nullptr, nullptr,
	"ADM", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"SBUF0", "SBUF1", "SBUF2", "SBUF3", "SBUF4", "SBUF5", "MBUF0", "MBUF1",
	"MBUF2", "MBUF3", "MBUF4", "MBUF5", "TMC0", "TMC1", "TMC2", "TMC3",
	"CSIM", nullptr, "SBIC", nullptr, "BRGC", "BRG", "SIO", nullptr,
	"ASIM", nullptr, "ASIS", nullptr, "RXB", nullptr, "TXS", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"PWMC0", "PWMC1", "PWM0L", nullptr, "PWM1L", nullptr, nullptr, nullptr,
	"ISPR", nullptr, "IMC", nullptr, "MK0L", "MK0H", nullptr, nullptr,
	nullptr, "ADCR0H", nullptr, "ADCR1H", nullptr, "ADCR2H", nullptr, "ADCR3H",
	nullptr, "ADCR4H", nullptr, "ADCR5H", nullptr, "ADCR6H", nullptr, "ADCR7H",
	"STBC", "CCW", "WDM", nullptr, "MM", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external access
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external access
	"OVIC3", "PIC0", "PIC1", "PIC2", "PIC3", "PIC4", "TMIC0", "CMIC03",
	"CMIC10", "CMIC40", "CMIC41", "SERIC", "SRIC", "STIC", "CSIIC", "ADIC",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

const char *const upd78366a_disassembler::s_sfrp_names[128] =
{
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CM00", "CM01", "CM02", "CM03", "BFCM00", "BFCM01", "BFCM02", "TM0",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "DTIME", nullptr,
	"CM10", "TM1", "CC20", "CT20", "TM2", "BFCM03", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CC30", "CT30", "CT31", "TM3", "CM40", "CM41", "TM4", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, "DACS", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, "PWM0", "PWM1", nullptr, nullptr, nullptr, "MK0", nullptr,
	"ADCR0", "ADCR1", "ADCR2", "ADCR3", "ADCR4", "ADCR5", "ADCR6", "ADCR7",
	nullptr, nullptr, nullptr, "PWC", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

upd78372_disassembler::upd78372_disassembler()
	: upd78k3_disassembler(s_sfr_names, s_sfrp_names, s_psw_bits, true, true)
{
}

const char *const upd78372_disassembler::s_sfr_names[256] =
{
	"P0", "P1", "P2", "P3", "P4", "P5", nullptr, "P7",
	"P8", "P9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TLA", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"PM0", "PM1", "PM2", "PM3", nullptr, "PM5", nullptr, nullptr,
	nullptr, "PM9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TUM0", "TUM1", "TUM2", "TOC0", "TOC1", "TOC2", "PRM", "NPC",
	nullptr, nullptr, nullptr, nullptr, "INTM0", "INTM1", nullptr, nullptr,
	"PMC0", nullptr, "PMC2", "PMC3", "PUOL", "PUOH", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, "PRDC", nullptr, nullptr, nullptr, nullptr, nullptr,
	"ADM", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, "ADCR0H", nullptr, "ADCR1H", nullptr, "ADCR2H", nullptr, "ADCR3H",
	nullptr, "ADCR4H", nullptr, "ADCR5H", nullptr, "ADCR6H", nullptr, "ADCR7H",
	"CSIM", nullptr, nullptr, nullptr, nullptr, nullptr, "SIO", nullptr,
	"ASIM", nullptr, "ASIS", nullptr, "RXB", nullptr, "TXS", nullptr,
	"BRGM", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"PWMC", nullptr, "PWM0L", nullptr, "PWM1L", nullptr, nullptr, nullptr,
	"ISPR", nullptr, "IMC", nullptr, "MK0L", "MK0H", "MK1L", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"STBC", "CCW", "WDM", nullptr, "MM", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external access
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external access
	"OVIC0", "OVIC3", "PLIC0", "PHIC0", "PLIC1", "PHIC1", "PLIC2", "PHIC2",
	"PLIC3", "PHIC3", "PIC4", "PIC5", "CMIC10", "CMIC11", "CMIC12", "CMIC13",
	"SERIC", "SRIC", "STIC", "CSIIC", "ADIC", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

const char *const upd78372_disassembler::s_sfrp_names[128] =
{
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, "TM0", "CC00", "CC01", "CC02", "CC03", "CC04", "CC05",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TM1", "CM10", "CM11", "CM12", "CM13", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"ADCR0", "ADCR1", "ADCR2", "ADCR3", "ADCR4", "ADCR5", "ADCR6", "ADCR7",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, "BRG", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "MK0", "MK1",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, "PWC", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};
