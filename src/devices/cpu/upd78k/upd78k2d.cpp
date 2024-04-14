// license:BSD-3-Clause
// copyright-holders:AJR

#include "emu.h"
#include "upd78k2d.h"

upd78k2_disassembler::upd78k2_disassembler(const char *const sfr_names[], const char *const sfrp_names[])
	: upd78k1_disassembler(sfr_names, sfrp_names)
{
}

offs_t upd78k2_disassembler::dasm_01xx(std::ostream &stream, u8 op2, offs_t pc, const upd78k2_disassembler::data_buffer &opcodes)
{
	if (op2 == 0x05)
	{
		u8 op3 = opcodes.r8(pc + 2);
		if ((op3 & 0xed) == 0x8c)
		{
			util::stream_format(stream, "%-8s&[%s]", BIT(op3, 4) ? "ROL4" : "ROR4", BIT(op3, 1) ? "HL" : "DE");
			return 3 | SUPPORTED;
		}
		else if ((op3 & 0xfa) == 0xe2)
		{
			util::stream_format(stream, "%-8s", "MOVW");
			if (BIT(op3, 2))
				util::stream_format(stream, "&[%s],AX", BIT(op3, 0) ? "HL" : "DE");
			else
				util::stream_format(stream, "AX,&[%s]", BIT(op3, 0) ? "HL" : "DE");
			return 3 | SUPPORTED;
		}
		else
			return dasm_illegal3(stream, 0x01, op2, op3);
	}
	else if (op2 == 0x06)
	{
		u8 op3 = opcodes.r8(pc + 2);
		if ((op3 & 0x70) < 0x30 && (BIT(op3, 7) ? (op3 & 0x0f) == 0 : BIT(op3, 3) || (op3 & 0x03) == 0))
		{
			util::stream_format(stream, "%-8s", BIT(op3, 3) ? s_alu_ops[op3 & 0x07] : BIT(op3, 2) ? "XCH" : "MOV");
			if (!BIT(op3, 7))
				stream << "A,";
			stream << "&";
			format_ix_disp8(stream, BIT(op3, 5) ? "HL" : BIT(op3, 4) ? "SP" : "DE", opcodes.r8(pc + 3));
			if (BIT(op3, 7))
				stream << ",A";
			return 4 | SUPPORTED;
		}
		else
			return dasm_illegal3(stream, 0x01, op2, op3);
	}
	else if (op2 == 0x09)
	{
		u8 op3 = opcodes.r8(pc + 2);
		if ((op3 & 0xfe) == 0xf0)
		{
			util::stream_format(stream, "%-8s", "MOV");
			if (!BIT(op3, 0))
				stream << "A,";
			stream << "&";
			format_abs16(stream, opcodes.r16(pc + 3));
			if (BIT(op3, 0))
				stream << ",A";
			return 5 | SUPPORTED;
		}
		else
			return dasm_illegal3(stream, 0x01, op2, op3);
	}
	else if (op2 == 0x0a)
	{
		u8 op3 = opcodes.r8(pc + 2);
		if (!BIT(op3, 6) && (BIT(op3, 7) ? (op3 & 0x0f) == 0 : BIT(op3, 3) || (op3 & 0x03) == 0))
		{
			util::stream_format(stream, "%-8s", BIT(op3, 3) ? s_alu_ops[op3 & 0x07] : BIT(op3, 2) ? "XCH" : "MOV");
			if (!BIT(op3, 7))
				stream << "A,";
			stream << "&";
			if (BIT(op3, 4))
				format_ix_base16(stream, BIT(op3, 5) ? "B" : "A", opcodes.r16(pc + 3));
			else
				format_ix_disp16(stream, BIT(op3, 5) ? "HL" : "DE", opcodes.r16(pc + 3));
			if (BIT(op3, 7))
				stream << ",A";
			return 5 | SUPPORTED;
		}
		else
			return dasm_illegal3(stream, 0x01, op2, op3);
	}
	else if (op2 == 0x16)
	{
		u8 op3 = opcodes.r8(pc + 2);
		if ((op3 & 0x60) != 0x60 && (BIT(op3, 7) ? (op3 & 0x0f) == 0x00 : BIT(op3, 3) || (op3 & 0x03) == 0x00))
		{
			util::stream_format(stream, "%-8s", BIT(op3, 3) ? s_alu_ops[op3 & 0x07] : BIT(op3, 2) ? "XCH" : "MOV");
			if (!BIT(op3, 7))
				stream << "A,";
			util::stream_format(stream, "&[%s%s]", BIT(op3, 4) ? "HL" : "DE", BIT(op3, 6) ? "" : BIT(op3, 5) ? "-" : "+");
			if (BIT(op3, 7))
				stream << ",A";
			return 3 | SUPPORTED;
		}
		else
			return dasm_illegal3(stream, 0x01, op2, op3);
	}
	else if ((op2 & 0xf0) == 0x50 && (op2 & 0x06) != 0x06)
	{
		util::stream_format(stream, "%-8s", "MOV");
		if (BIT(op2, 3))
			stream << "A,";
		util::stream_format(stream, "&[%s%s]", BIT(op2, 0) ? "HL" : "DE", BIT(op2, 2) ? "" : BIT(op2, 1) ? "-" : "+");
		if (!BIT(op2, 3))
			stream << ",A";
		return 2 | SUPPORTED;
	}
	else
		return upd78k1_disassembler::dasm_01xx(stream, op2, pc, opcodes);
}

offs_t upd78k2_disassembler::dasm_05xx(std::ostream &stream, u8 op2, offs_t pc, const upd78k2_disassembler::data_buffer &opcodes)
{
	if ((op2 & 0xe8) == 0x08)
	{
		util::stream_format(stream, "%-8s%s", BIT(op2, 4) ? "DIVUW" : "MULU", s_r_names[op2 & 0x07]);
		return 2 | SUPPORTED;
	}
	else if ((op2 & 0xe9) == 0x48)
	{
		util::stream_format(stream, "%-8s%s", BIT(op2, 4) ? "CALL" : "BR", s_rp_names[(op2 & 0x06) >> 1]);
		return 2 | (BIT(op2, 4) ? STEP_OVER : 0) | SUPPORTED;
	}
	else if ((op2 & 0xed) == 0x8c)
	{
		util::stream_format(stream, "%-8s[%s]", BIT(op2, 4) ? "ROL4" : "ROR4", BIT(op2, 1) ? "HL" : "DE");
		return 2 | SUPPORTED;
	}
	else if ((op2 & 0xfc) == 0xa8)
	{
		util::stream_format(stream, "%-8sRB%d", "SEL", op2 & 0x03);
		return 2 | SUPPORTED;
	}
	else if ((op2 & 0xfe) == 0xc8)
	{
		util::stream_format(stream, "%-8sSP", BIT(op2, 0) ? "DECW" : "INCW");
		return 2 | SUPPORTED;
	}
	else if ((op2 & 0xfa) == 0xe2)
	{
		util::stream_format(stream, "%-8s", "MOVW");
		if (BIT(op2, 2))
			util::stream_format(stream, "[%s],AX", BIT(op2, 0) ? "HL" : "DE");
		else
			util::stream_format(stream, "AX,[%s]", BIT(op2, 0) ? "HL" : "DE");
		return 2 | SUPPORTED;
	}
	else
		return dasm_illegal2(stream, 0x05, op2);
}

offs_t upd78k2_disassembler::dasm_06(std::ostream &stream, offs_t pc, const upd78k2_disassembler::data_buffer &opcodes)
{
	u8 op2 = opcodes.r8(pc + 1);
	if ((op2 & 0x70) < 0x30 && (BIT(op2, 7) ? (op2 & 0x0f) == 0 : BIT(op2, 3) || (op2 & 0x03) == 0))
	{
		util::stream_format(stream, "%-8s", BIT(op2, 3) ? s_alu_ops[op2 & 0x07] : BIT(op2, 2) ? "XCH" : "MOV");
		if (!BIT(op2, 7))
			stream << "A,";
		format_ix_disp8(stream, BIT(op2, 5) ? "HL" : BIT(op2, 4) ? "SP" : "DE", opcodes.r8(pc + 2));
		if (BIT(op2, 7))
			stream << ",A";
		return 3 | SUPPORTED;
	}
	else
		return dasm_illegal2(stream, 0x06, op2);
}

offs_t upd78k2_disassembler::dasm_0axx(std::ostream &stream, u8 op2, offs_t pc, const upd78k2_disassembler::data_buffer &opcodes)
{
	if (!BIT(op2, 6) && (BIT(op2, 7) ? (op2 & 0x0f) == 0 : BIT(op2, 3) || (op2 & 0x03) == 0))
	{
		util::stream_format(stream, "%-8s", BIT(op2, 3) ? s_alu_ops[op2 & 0x07] : BIT(op2, 2) ? "XCH" : "MOV");
		if (!BIT(op2, 7))
			stream << "A,";
		if (BIT(op2, 4))
			format_ix_base16(stream, BIT(op2, 5) ? "B" : "A", opcodes.r16(pc + 2));
		else
			format_ix_disp16(stream, BIT(op2, 5) ? "HL" : "DE", opcodes.r16(pc + 2));
		if (BIT(op2, 7))
			stream << ",A";
		return 4 | SUPPORTED;
	}
	else
		return dasm_illegal2(stream, 0x0a, op2);
}

offs_t upd78k2_disassembler::dasm_16xx(std::ostream &stream, u8 op2, const upd78k2_disassembler::data_buffer &opcodes)
{
	if ((op2 & 0x60) != 0x60 && (BIT(op2, 7) ? (op2 & 0x0f) == 0x00 : BIT(op2, 3) || (op2 & 0x03) == 0x00))
	{
		util::stream_format(stream, "%-8s", BIT(op2, 3) ? s_alu_ops[op2 & 0x07] : BIT(op2, 2) ? "XCH" : "MOV");
		if (!BIT(op2, 7))
			stream << "A,";
		util::stream_format(stream, "[%s%s]", BIT(op2, 4) ? "HL" : "DE", BIT(op2, 6) ? "" : BIT(op2, 5) ? "-" : "+");
		if (BIT(op2, 7))
			stream << ",A";
		return 2 | SUPPORTED;
	}
	else
		return dasm_illegal2(stream, 0x16, op2);
}

offs_t upd78k2_disassembler::dasm_25(std::ostream &stream, offs_t pc, const upd78k2_disassembler::data_buffer &opcodes)
{
	u8 rr = opcodes.r8(pc + 1);
	if ((rr & 0x88) == 0x00)
	{
		util::stream_format(stream, "%-8s%s,%s", "XCH", s_r_names[(rr & 0x70) >> 4], s_r_names[rr & 0x07]);
		return 2 | SUPPORTED;
	}
	else
		return dasm_illegal2(stream, 0x25, rr);
}

offs_t upd78k2_disassembler::dasm_29(std::ostream &stream, offs_t pc, const upd78k2_disassembler::data_buffer &opcodes)
{
	util::stream_format(stream, "%-8s", "PUSH");
	format_sfr(stream, opcodes.r8(pc + 1));
	return 2 | SUPPORTED;
}

offs_t upd78k2_disassembler::dasm_38(std::ostream &stream, u8 op, offs_t pc, const upd78k2_disassembler::data_buffer &opcodes)
{
	util::stream_format(stream, "%-8s", BIT(op, 0) ? "XCH" : "MOV");
	format_saddr(stream, opcodes.r8(pc + 2));
	stream << ",";
	format_saddr(stream, opcodes.r8(pc + 1));
	return 3 | SUPPORTED;
}

offs_t upd78k2_disassembler::dasm_43(std::ostream &stream, offs_t pc, const upd78k2_disassembler::data_buffer &opcodes)
{
	util::stream_format(stream, "%-8s", "POP");
	format_sfr(stream, opcodes.r8(pc + 1));
	return 2 | SUPPORTED;
}

offs_t upd78k2_disassembler::dasm_50(std::ostream &stream, u8 op)
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
		util::stream_format(stream, "[%s%s]", BIT(op, 0) ? "HL" : "DE", BIT(op, 2) ? "" : BIT(op, 1) ? "-" : "+");
		if (!BIT(op, 3))
			stream << ",A";
		return 1 | SUPPORTED;
	}
}

offs_t upd78k2_disassembler::dasm_78(std::ostream &stream, u8 op, offs_t pc, const upd78k2_disassembler::data_buffer &opcodes)
{
	util::stream_format(stream, "%-8s", s_alu_ops[op & 0x07]);
	format_saddr(stream, opcodes.r8(pc + 2));
	stream << ",";
	format_saddr(stream, opcodes.r8(pc + 1));
	return 3 | SUPPORTED;
}

upd78214_disassembler::upd78214_disassembler()
	: upd78k2_disassembler(s_sfr_names, s_sfrp_names)
{
}

const char *const upd78214_disassembler::s_sfr_names[256] =
{
	"P0", nullptr, "P2", "P3", "P4", "P5", "P6", "P7",
	nullptr, nullptr, "P0L", "P0H", "RTPC", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, "CR10", "CR20", "CR21", "CR30",
	nullptr, nullptr, "CR22", nullptr, "CR11", nullptr, nullptr, nullptr,
	"PM0", nullptr, nullptr, "PM3", nullptr, "PM5", "PM6", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CRC0", "TOC", "CRC1", nullptr, "CRC2", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"PUO", nullptr, nullptr, "PMC3", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, "TM1", nullptr, "TM2", nullptr, "TM3", nullptr,
	nullptr, nullptr, nullptr, nullptr, "PRM0", "TMC0", "PRM1", "TMC1",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"ADM", nullptr, "ADCR", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CSIM", nullptr, "SBIC", nullptr, nullptr, nullptr, "SIO", nullptr,
	"ASIM", nullptr, "ASIS", nullptr, "RXB", nullptr, "TXS", nullptr,
	"BRGC", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"STBC", nullptr, nullptr, nullptr, "MM", "PW", "RFM", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external SFRs
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external SFRs
	"IF0L", "IF0H", nullptr, nullptr, "MK0L", "MK0H", nullptr, nullptr,
	"PR0L", "PR0H", nullptr, nullptr, "ISM0L", "ISM0H", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, "INTM0", "INTM1", nullptr, nullptr,
	"IST", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

const char *const upd78214_disassembler::s_sfrp_names[128] =
{
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CR00", "CR01", nullptr, nullptr, "CR02", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TM0", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"IF0", nullptr, "MK0", nullptr, "PR0", nullptr, "ISM0", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

upd78218a_disassembler::upd78218a_disassembler()
	: upd78k2_disassembler(s_sfr_names, s_sfrp_names)
{
}

const char *const upd78218a_disassembler::s_sfr_names[256] =
{
	"P0", nullptr, "P2", "P3", "P4", "P5", "P6", "P7",
	nullptr, nullptr, "P0L", "P0H", "RTPC", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, "CR10", "CR20", "CR21", "CR30",
	nullptr, nullptr, "CR22", nullptr, "CR11", nullptr, nullptr, nullptr,
	"PM0", nullptr, nullptr, "PM3", nullptr, "PM5", "PM6", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CRC0", "TOC", "CRC1", nullptr, "CRC2", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"PUO", nullptr, nullptr, "PMC3", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, "TM1", nullptr, "TM2", nullptr, "TM3", nullptr,
	nullptr, nullptr, nullptr, nullptr, "PRM0", "TMC0", "PRM1", "TMC1",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"ADM", nullptr, "ADCR", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, "OSPC", nullptr, nullptr,
	"CSIM", nullptr, "SBIC", nullptr, nullptr, nullptr, "SIO", nullptr,
	"ASIM", nullptr, "ASIS", nullptr, "RXB", nullptr, "TXS", nullptr,
	"BRGC", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"STBC", nullptr, nullptr, nullptr, "MM", "PW", "RFM", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"IF0L", "IF0H", nullptr, nullptr, "MK0L", "MK0H", nullptr, nullptr,
	"PR0L", "PR0H", nullptr, nullptr, "ISM0L", "ISM0H", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, "INTM0", "INTM1", nullptr, nullptr,
	"IST", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

const char *const upd78218a_disassembler::s_sfrp_names[128] =
{
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CR00", "CR01", nullptr, nullptr, "CR02", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TM0", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"IF0", nullptr, "MK0", nullptr, "PR0", nullptr, "ISM0", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

upd78224_disassembler::upd78224_disassembler()
	: upd78k2_disassembler(s_sfr_names, s_sfrp_names)
{
}

const char *const upd78224_disassembler::s_sfr_names[256] =
{
	"P0", "P1", "P2", "P3", "P4", "P5", "P6", "P7",
	nullptr, nullptr, "P0L", "P0H", "RTPC", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, "CR10", "CR20", "CR21", "CR30",
	nullptr, nullptr, "CR22", nullptr, "CR11", nullptr, nullptr, nullptr,
	"PM0", "PM1", nullptr, "PM3", nullptr, "PM5", "PM6", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CRC0", "TOC", "CRC1", nullptr, "CRC2", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, "PMC3", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, "TM1", nullptr, "TM2", nullptr, "TM3", nullptr,
	nullptr, nullptr, nullptr, nullptr, "PRM0", "TMC0", "PRM1", "TMC1",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "PMT", "PT",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CSIM", nullptr, "SBIC", nullptr, nullptr, nullptr, "SIO", nullptr,
	"ASIM", nullptr, "ASIS", nullptr, "RXB", nullptr, "TXS", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"STBC", nullptr, nullptr, nullptr, "MM", "PW", "RFM", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"IF0L", "IF0H", nullptr, nullptr, "MK0L", "MK0H", nullptr, nullptr,
	"PR0L", "PR0H", nullptr, nullptr, "ISM0L", "ISM0H", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, "INTM0", "INTM1", nullptr, nullptr,
	"IST", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

const char *const upd78224_disassembler::s_sfrp_names[128] =
{
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CR00", "CR01", nullptr, nullptr, "CR02", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TM0", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"IF0", nullptr, "MK0", nullptr, "PR0", nullptr, "ISM0", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

upd78234_disassembler::upd78234_disassembler()
	: upd78k2_disassembler(s_sfr_names, s_sfrp_names)
{
}

const char *const upd78234_disassembler::s_sfr_names[256] =
{
	"P0", "P1", "P2", "P3", "P4", "P5", "P6", "P7",
	nullptr, nullptr, "P0L", "P0H", "RTPC", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, "CR10", "CR20", "CR21", "CR30",
	nullptr, nullptr, "CR22", nullptr, "CR11", nullptr, nullptr, nullptr,
	"PM0", "PM1", nullptr, "PM3", nullptr, "PM5", "PM6", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CRC0", "TOC", "CRC1", nullptr, "CRC2", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"PUO", nullptr, nullptr, "PMC3", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, "TM1", nullptr, "TM2", nullptr, "TM3", nullptr,
	nullptr, nullptr, nullptr, nullptr, "PRM0", "TMC0", "PRM1", "TMC1",
	"DACS0", "DACS1", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"ADM", nullptr, "ADCR", nullptr, nullptr, nullptr, nullptr, nullptr,
	"PWMC", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, "OSPC", nullptr, nullptr,
	"CSIM", nullptr, "SBIC", nullptr, nullptr, nullptr, "SIO", nullptr,
	"ASIM", nullptr, "ASIS", nullptr, "RXB", nullptr, "TXS", nullptr,
	"BRGC", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"STBC", nullptr, nullptr, nullptr, "MM", "PW", "RFM", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "IMS",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external SFRs
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external SFRs
	"IF0L", "IF0H", nullptr, nullptr, "MK0L", "MK0H", nullptr, nullptr,
	"PR0L", "PR0H", nullptr, nullptr, "ISM0L", "ISM0H", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, "INTM0", "INTM1", nullptr, nullptr,
	"IST", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

const char *const upd78234_disassembler::s_sfrp_names[128] =
{
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CR00", "CR01", nullptr, nullptr, "CR02", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TM0", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
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

upd78244_disassembler::upd78244_disassembler()
	: upd78k2_disassembler(s_sfr_names, s_sfrp_names)
{
}

const char *const upd78244_disassembler::s_sfr_names[256] =
{
	"P0", nullptr, "P2", "P3", "P4", "P5", "P6", "P7",
	nullptr, nullptr, "P0L", "P0H", "RTPC", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, "CR10", "CR20", "CR21", "CR30",
	nullptr, nullptr, "CR22", nullptr, "CR11", nullptr, nullptr, nullptr,
	"PM0", nullptr, nullptr, "PM3", nullptr, "PM5", "PM6", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CRC0", "TOC", "CRC1", nullptr, "CRC2", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"PUO", nullptr, nullptr, "PMC3", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, "TM1", nullptr, "TM2", nullptr, "TM3", nullptr,
	nullptr, nullptr, nullptr, nullptr, "PRM0", "TMC0", "PRM1", "TMC1",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"ADM", nullptr, "ADCR", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"EWC", nullptr, nullptr, nullptr, nullptr, "OSPC", nullptr, nullptr,
	"CSIM", nullptr, "SBIC", nullptr, nullptr, nullptr, "SIO", nullptr,
	"ASIM", nullptr, "ASIS", nullptr, "RXB", nullptr, "TXS", nullptr,
	"BRGC", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"STBC", nullptr, nullptr, nullptr, "MM", "PW", "RFM", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external SFRs
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external SFRs
	"IF0L", "IF0H", nullptr, nullptr, "MK0L", "MK0H", nullptr, nullptr,
	"PR0L", "PR0H", nullptr, nullptr, "ISM0L", "ISM0H", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, "INTM0", "INTM1", nullptr, nullptr,
	"IST", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

const char *const upd78244_disassembler::s_sfrp_names[128] =
{
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CR00", "CR01", nullptr, nullptr, "CR02", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TM0", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"IF0", nullptr, "MK0", nullptr, "PR0", nullptr, "ISM0", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};
