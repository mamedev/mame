// license:BSD-3-Clause
// copyright-holders:AJR

#include "emu.h"
#include "upd78k4d.h"

upd78k4_disassembler::upd78k4_disassembler(const char *const sfr_names[], const char *const sfrp_names[])
	: upd78k3_disassembler(sfr_names, sfrp_names, s_ix_bases, 0xfd00)
{
}

const char *const upd78k4_disassembler::s_ix_bases[5] =
{
	"TDE",
	"SP",
	"WHL",
	"UUP",
	"VVP"
};

const char *const upd78k4_disassembler::s_imm24_offsets[4] =
{
	"DE",
	"A",
	"HL",
	"B"
};

const char *const upd78k4_disassembler::s_rg_names[4] =
{
	"VVP", // RG4
	"UUP", // RG5
	"TDE", // RG6
	"WHL"  // RG7
};


void upd78k4_disassembler::format_imm24(std::ostream &stream, u32 d)
{
	if (d < 0xa00000)
		util::stream_format(stream, "#%06XH", d);
	else
		util::stream_format(stream, "#0%06XH", d);
}

void upd78k4_disassembler::format_abs20(std::ostream &stream, u32 addr)
{
	if (addr < 0xa0000)
		util::stream_format(stream, "!!%05XH", addr);
	else
		util::stream_format(stream, "!!0%05XH", addr);
}

void upd78k4_disassembler::format_abs24(std::ostream &stream, u32 addr)
{
	if (addr < 0xa00000)
		util::stream_format(stream, "!!%06XH", addr);
	else
		util::stream_format(stream, "!!0%06XH", addr);
}

void upd78k4_disassembler::format_saddr1(std::ostream &stream, u8 addr)
{
	util::stream_format(stream, "0%04XH", 0xfe00 + addr);
}

void upd78k4_disassembler::format_saddrp1(std::ostream &stream, u8 addr)
{
	util::stream_format(stream, "0%04XH", 0xfe00 + addr);
}

void upd78k4_disassembler::format_saddrg1(std::ostream &stream, u8 addr)
{
	util::stream_format(stream, "0%04XH", 0xfe00 + addr);
}

void upd78k4_disassembler::format_saddrg2(std::ostream &stream, u8 addr)
{
	util::stream_format(stream, "0%04XH", (addr < 0x20 ? 0xff00 : 0xfd00) + addr);
}

void upd78k4_disassembler::format_jdisp8(std::ostream &stream, offs_t pc, u8 disp)
{
	u32 addr = (pc + s8(disp)) & 0xfffff;
	if (addr < 0xa0000)
		util::stream_format(stream, "$%05XH", addr);
	else
		util::stream_format(stream, "$0%05XH", addr);
}

void upd78k4_disassembler::format_jdisp16(std::ostream &stream, offs_t pc, u16 disp)
{
	u32 addr = (pc + s16(disp)) & 0xfffff;
	if (addr < 0xa0000)
		util::stream_format(stream, "$!%05XH", addr);
	else
		util::stream_format(stream, "$!0%05XH", addr);
}


offs_t upd78k4_disassembler::dasm_05xx(std::ostream &stream, u8 op2)
{
	if ((op2 & 0xc9) == 0x41)
	{
		util::stream_format(stream, "%-8s", BIT(op2, 4) ? "CALL" : "BR");
		if (BIT(op2, 5))
			util::stream_format(stream, "[%s]", s_rg_names[BIT(op2, 1, 2)]);
		else
			stream << s_rg_names[BIT(op2, 1, 2)];
		return 2 | (BIT(op2, 4) ? STEP_OVER : 0) | SUPPORTED;
	}
	else if ((op2 & 0xf1) == 0xc1)
	{
		util::stream_format(stream, "%-8s", "MOV");
		if (!BIT(op2, 3))
			stream << "A,";
		stream << s_rg_names[BIT(op2, 1, 2)][0];
		if (BIT(op2, 3))
			stream << ",A";
		return 2 | SUPPORTED;
	}
	else if ((op2 & 0xfe) == 0xf8)
	{
		util::stream_format(stream, "%-8sSP", BIT(op2, 0) ? "DECG" : "INCG");
		return 2 | SUPPORTED;
	}
	else if ((op2 & 0xfe) == 0xfa)
	{
		util::stream_format(stream, "%-8s%s", "MOVG", BIT(op2, 0) ? "WHL,SP" : "SP,WHL");
		return 2 | SUPPORTED;
	}
	else if (op2 == 0xfc)
	{
		stream << "SWRS";
		return 2 | SUPPORTED;
	}
	else
		return upd78k3_disassembler::dasm_05xx(stream, op2);
}

offs_t upd78k4_disassembler::dasm_06xx(std::ostream &stream, u8 op2, offs_t pc, const upd78k4_disassembler::data_buffer &opcodes)
{
	if ((op2 & 0x70) < 0x50 && (op2 & 0x0f) == 0x02)
	{
		util::stream_format(stream, "%-8s", "MOVG");
		if (!BIT(op2, 7))
			stream << "WHL,";
		if (BIT(op2, 7))
			stream << ",WHL";
		format_ix_disp8(stream, s_ix_bases[(op2 & 0x70) >> 4], opcodes.r8(pc + 2));
		return 3 | SUPPORTED;
	}
	else
		return upd78k3_disassembler::dasm_06xx(stream, op2, pc, opcodes);
}

offs_t upd78k4_disassembler::dasm_07xx(std::ostream &stream, u8 op2, offs_t pc, const upd78k4_disassembler::data_buffer &opcodes)
{
	if ((op2 & 0x68) == 0x28 || ((op2 & 0x7b) == 0x30 && op2 != 0xb4))
	{
		util::stream_format(stream, "%-8s", s_alu_ops[op2 & 0x07]);
		if (!BIT(op2, 7))
			stream << "A,";
		stream << "[";
		if (BIT(op2, 4))
		{
			stream << "%";
			format_saddrg2(stream, opcodes.r8(pc + 2));
		}
		else
			format_saddrp(stream, opcodes.r8(pc + 2));
		stream << "]";
		if (BIT(op2, 7))
			stream << ",A";
		return 3 | SUPPORTED;
	}
	else if ((op2 & 0x6b) == 0x21 && (op2 & 0x84) != 0x84)
	{
		util::stream_format(stream, "%-8s", BIT(op2, 2) ? "XCHW" : "MOVW");
		if (!BIT(op2, 7))
			stream << "AX,";
		stream << "[";
		if (BIT(op2, 4))
		{
			stream << "%";
			format_saddrg2(stream, opcodes.r8(pc + 2));
		}
		else
			format_saddrp(stream, opcodes.r8(pc + 2));
		stream << "]";
		if (BIT(op2, 7))
			stream << ",AX";
		return 3 | SUPPORTED;
	}
	else if ((op2 & 0x7f) == 0x32)
	{
		util::stream_format(stream, "%-8s", "MOVG");
		if (!BIT(op2, 7))
			stream << "WHL,";
		stream << "[%";
		format_saddrg2(stream, opcodes.r8(pc + 2));
		stream << "]";
		if (BIT(op2, 7))
			stream << ",WHL";
		return 3 | SUPPORTED;
	}
	else if ((op2 & 0xf9) == 0x61)
	{
		util::stream_format(stream, "%-8s%c,", "MOV", s_rg_names[BIT(op2, 1, 2)][0]);
		format_imm8(stream, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;
	}
	else if ((op2 & 0xfe) == 0xda)
	{
		util::stream_format(stream, "%-8s", BIT(op2, 0) ? "PUSH" : "POP");
		format_sfr(stream, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;
	}
	else if (op2 == 0x95)
	{
		util::stream_format(stream, "%-8s", "MACSW");
		format_count(stream, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;
	}
	else
		return upd78k3_disassembler::dasm_07xx(stream, op2, pc, opcodes);
}

offs_t upd78k4_disassembler::dasm_09xx(std::ostream &stream, u8 op2, offs_t pc, const upd78k4_disassembler::data_buffer &opcodes)
{
	if (op2 == 0x20)
	{
		util::stream_format(stream, "%-8sSP,", "MOVG");
		format_imm24(stream, u32(opcodes.r8(pc + 4)) << 16 | opcodes.r16(pc + 2));
		return 5 | SUPPORTED;
	}
	else if ((op2 & 0xfe) == 0x28)
	{
		util::stream_format(stream, "%-8sSP,", BIT(op2, 0) ? "SUBWG" : "ADDWG");
		format_imm16(stream, opcodes.r16(pc + 2));
		return 4 | SUPPORTED;
	}
	else if ((op2 & 0xee) == 0x40)
	{
		util::stream_format(stream, "%-8s", BIT(op2, 0) ? "MOVW" : "MOV");
		if (BIT(op2, 4))
		{
			format_abs24(stream, u32(opcodes.r8(pc + 2)) << 16 | opcodes.r16(pc + 3));
			stream << ",";
			if (BIT(op2, 0))
			{
				format_imm16(stream, opcodes.r16(pc + 5));
				return 7 | SUPPORTED;
			}
			else
			{
				format_imm8(stream, opcodes.r8(pc + 5));
				return 6 | SUPPORTED;
			}
		}
		else
		{
			format_abs16(stream, opcodes.r16(pc + 2));
			stream << ",";
			if (BIT(op2, 0))
			{
				format_imm16(stream, opcodes.r16(pc + 4));
				return 6 | SUPPORTED;
			}
			else
			{
				format_imm8(stream, opcodes.r8(pc + 4));
				return 5 | SUPPORTED;
			}
		}
	}
	else if (op2 == 0x64)
	{
		util::stream_format(stream, "%-8s", "SACW");
		u8 post1 = opcodes.r8(pc + 2);
		u8 post2 = opcodes.r8(pc + 3);
		if (post1 == 0x41 && post2 == 0x46)
			util::stream_format(stream, "[%s+],[%s+]", s_ix_bases[0], s_ix_bases[2]);
		else
			stream << "invalid";
		return 4 | SUPPORTED;
	}
	else if ((op2 & 0xe9) == 0x89)
	{
		util::stream_format(stream, "%-8s%s", BIT(op2, 4) ? "POP" : "PUSH", s_rg_names[BIT(op2, 1, 2)]);
		return 2 | SUPPORTED;
	}
	else if (op2 == 0xb0)
	{
		util::stream_format(stream, "%-8s", "RETCSB");
		format_abs16(stream, opcodes.r16(pc + 2));
		return 4 | SUPPORTED;
	}
	else if (op2 == 0xc1)
	{
		util::stream_format(stream, "%-8s ", "LOCATION");
		u16 w = opcodes.r16(pc + 2);
		if (w == 0x01fe)
			stream << "0H";
		else if (w == 0x00ff)
			stream << "0FH";
		else
			stream << "invalid";
		return 4 | SUPPORTED;
	}
	else if (op2 == 0xd0)
	{
		u8 op3 = opcodes.r8(pc + 2);
		if ((op3 & 0xf0) == 0x10)
		{
			util::stream_format(stream, "%-8s", "MOV1");
			if (BIT(op3, 3))
				format_abs24(stream, u32(opcodes.r8(pc + 3)) << 16 | opcodes.r16(pc + 4));
			else
				format_abs16(stream, opcodes.r16(pc + 3));
			util::stream_format(stream, ".%d,CY", op3 & 0x07);
			return (BIT(op3, 3) ? 6 : 5) | SUPPORTED;
		}
		else if (op3 < 0x70)
		{
			util::stream_format(stream, "%-8sCY,", s_bool_ops[BIT(op3, 5, 2)]);
			if (BIT(op3, 4))
				stream << "/";
			if (BIT(op3, 3))
				format_abs24(stream, u32(opcodes.r8(pc + 3)) << 16 | opcodes.r16(pc + 4));
			else
				format_abs16(stream, opcodes.r16(pc + 3));
			util::stream_format(stream, ".%d", op3 & 0x07);
			return (BIT(op3, 3) ? 6 : 5) | SUPPORTED;
		}
		else if (op3 < 0xa0)
		{
			util::stream_format(stream, "%-8s", op3 < 0x80 ? "NOT1" : BIT(op3, 4) ? "CLR1" : "SET1");
			if (BIT(op3, 3))
				format_abs24(stream, u32(opcodes.r8(pc + 3)) << 16 | opcodes.r16(pc + 4));
			else
				format_abs16(stream, opcodes.r16(pc + 3));
			util::stream_format(stream, ".%d", op3 & 0x07);
			return (BIT(op3, 3) ? 6 : 5) | SUPPORTED;
		}
		else if (op3 < 0xe0)
		{
			if (BIT(op3, 6))
				util::stream_format(stream, "%-8s", BIT(op3, 4) ? "BTCLR" : "BFSET");
			else
				util::stream_format(stream, "%-8s", BIT(op3, 4) ? "BT" : "BF");
			if (BIT(op3, 3))
			{
				format_abs24(stream, u32(opcodes.r8(pc + 3)) << 16 | opcodes.r16(pc + 4));
				util::stream_format(stream, ".%d,", op3 & 0x07);
				format_jdisp8(stream, pc + 7, opcodes.r8(pc + 6));
				return 7 | SUPPORTED;
			}
			else
			{
				format_abs16(stream, opcodes.r16(pc + 3));
				util::stream_format(stream, ".%d,", op3 & 0x07);
				format_jdisp8(stream, pc + 6, opcodes.r8(pc + 5));
				return 6 | SUPPORTED;
			}
		}
		else
			return dasm_illegal3(stream, 0x09, 0xd0, op3);
	}
	else if (op2 >= 0xe0)
	{
		util::stream_format(stream, "%-8s", op2 >= 0xf0 ? "CALL" : "BR");
		format_abs20(stream, u32(op2 & 0x0f) << 16 | opcodes.r16(pc + 2));
		return 4 | (op2 >= 0xf0 ? STEP_OVER : 0) | SUPPORTED;
	}
	else
		return upd78k3_disassembler::dasm_09xx(stream, op2, pc, opcodes);
}

offs_t upd78k4_disassembler::dasm_0axx(std::ostream &stream, u8 op2, offs_t pc, const upd78k4_disassembler::data_buffer &opcodes)
{
	if (!BIT(op2, 6) && (BIT(op2, 3) || (op2 & (BIT(op2, 7) ? 0x06 : 0x02)) == 0x00 || (op2 & 0x0f) == 0x02))
	{
		if (BIT(op2, 3))
			util::stream_format(stream, "%-8s", s_alu_ops[op2 & 0x07]);
		else if ((op2 & 0x0f) == 0x02)
			util::stream_format(stream, "%-8s", "MOVG");
		else if (BIT(op2, 0))
			util::stream_format(stream, "%-8s", BIT(op2, 2) ? "XCHW" : "MOVW");
		else
			util::stream_format(stream, "%-8s", BIT(op2, 2) ? "XCH" : "MOV");
		if ((op2 & 0x8b) == 0x01)
			stream << "AX,";
		else if ((op2 & 0x8b) == 0x02)
			stream << "WHL,";
		else if (!BIT(op2, 7))
			stream << "A,";

		u32 imm24 = u32(opcodes.r8(pc + 4)) << 16 | opcodes.r16(pc + 2);
		if (imm24 >= 0xa00000)
			stream << "0";
		util::stream_format(stream, "%06XH[%s]", imm24, s_imm24_offsets[BIT(op2, 4, 2)]);

		if ((op2 & 0x8b) == 0x81)
			stream << ",AX";
		else if ((op2 & 0x8b) == 0x82)
			stream << ",WHL";
		else if (BIT(op2, 7))
			stream << ",A";
		return 5 | SUPPORTED;
	}
	else if ((op2 & 0x68) == 0x48)
	{
		util::stream_format(stream, "%-8s", s_alu_ops[op2 & 0x07]);
		if (!BIT(op2, 7))
			stream << "A,";
		if (BIT(op2, 4))
			format_abs24(stream, u32(opcodes.r8(pc + 4)) << 16 | opcodes.r16(pc + 2));
		else
			format_abs16(stream, opcodes.r16(pc + 2));
		if (BIT(op2, 7))
			stream << ",A";
		return (BIT(op2, 4) ? 5 : 4) | SUPPORTED;
	}
	else if ((op2 & 0xef) == 0x45)
	{
		util::stream_format(stream, "%-8sAX,", "XCHW");
		if (BIT(op2, 4))
		{
			format_abs24(stream, u32(opcodes.r8(pc + 4)) << 16 | opcodes.r16(pc + 2));
			return 5 | SUPPORTED;
		}
		else
		{
			format_abs16(stream, opcodes.r16(pc + 2));
			return 4 | SUPPORTED;
		}
	}
	else
		return dasm_illegal2(stream, 0x0a, op2);
}

offs_t upd78k4_disassembler::dasm_16xx(std::ostream &stream, u8 op1, u8 op2)
{
	if (BIT(op1, 0) && (op2 & 0x60) == 0x60 && (BIT(op2, 3) || (op2 & (BIT(op2, 7) ? 0x06 : 0x02)) == 0x00))
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
		util::stream_format(stream, "[%s+C]", m_ix_bases[(op2 & 0x10) >> 3]);
		if ((op2 & 0x8b) == 0x81)
			stream << ",AX";
		else if (BIT(op2, 7))
			stream << ",A";
		return 2 | SUPPORTED;
	}
	else if ((op2 & 0x0f) == 0x02 && (BIT(op1, 0) || (op2 & 0xd0) != 0x10))
	{
		util::stream_format(stream, "%-8s", "MOVG");
		if (!BIT(op2, 7))
			stream << "WHL,";
		stream << "[";
		if (BIT(op1, 0) && (op2 & 0x60) == 0x40)
			util::stream_format(stream, "%s+%s", s_ix_bases[4], BIT(op2, 4) ? "HL" : "DE");
		else if ((op2 & 0x60) == 0x60)
			stream << s_ix_bases[BIT(op2, 4) ? 3 : 4];
		else
		{
			stream << s_ix_bases[(op2 & 0x10) >> 3];
			if (BIT(op1, 0))
				util::stream_format(stream, "+%c", BIT(op2, 6) ? 'C' : BIT(op2, 5) ? 'B' : 'A');
			else
				stream << (BIT(op2, 5) ? "-" : "+");
		}
		stream << "]";
		if (BIT(op2, 7))
			stream << ",WHL";
		return 2 | SUPPORTED;
	}
	else
		return upd78k3_disassembler::dasm_16xx(stream, op1, op2);
}

offs_t upd78k4_disassembler::dasm_24xx(std::ostream &stream, u8 op, u8 rr)
{
	if (!BIT(op, 0) && (rr & 0x99) == 0x99)
	{
		util::stream_format(stream, "%-8s%s,%s", "MOVG", s_rg_names[BIT(rr, 5, 2)], s_rg_names[BIT(rr, 1, 2)]);
		return 2 | SUPPORTED;
	}
	else
		return upd78k3_disassembler::dasm_24xx(stream, op, rr);
}

offs_t upd78k4_disassembler::dasm_2a(std::ostream &stream, offs_t pc, const upd78k4_disassembler::data_buffer &opcodes)
{
	u8 op2 = opcodes.r8(pc + 1);
	if ((op2 & 0xc0) == 0 && (BIT(op2, 3) || (op2 & 0x03) == 0))
	{
		if (BIT(op2, 3))
			util::stream_format(stream, "%-8s", s_alu_ops[op2 & 0x07]);
		else
			util::stream_format(stream, "%-8s", BIT(op2, 2) ? "XCH" : "MOV");
		if (BIT(op2, 5))
			format_saddr1(stream, opcodes.r8(pc + 3));
		else
			format_saddr(stream, opcodes.r8(pc + 3));
		stream << ",";
		if (BIT(op2, 4))
			format_saddr1(stream, opcodes.r8(pc + 2));
		else
			format_saddr(stream, opcodes.r8(pc + 2));
		return 4 | SUPPORTED;
	}
	else
		return dasm_illegal2(stream, 0x2a, op2);
}

offs_t upd78k4_disassembler::dasm_38(std::ostream &stream, u8 op, offs_t pc, const upd78k4_disassembler::data_buffer &opcodes)
{
	u8 op2 = opcodes.r8(pc + 1);
	if (!BIT(op2, 3) && (op2 & 0x03) != 0x03 && (!BIT(op, 0) || !BIT(op2, 2)))
	{
		util::stream_format(stream, "%-8s", BIT(op2, 2) ? "XCH" : "MOV");
		if (!BIT(op2, 2))
			util::stream_format(stream, "%s,", s_r_names[BIT(op2, 4, 4)]);
		if (BIT(op2, 1))
			format_sfr(stream, opcodes.r8(pc + 2));
		else if (BIT(op2, 0))
			format_saddr1(stream, opcodes.r8(pc + 2));
		else
			format_saddr(stream, opcodes.r8(pc + 2));
		if (BIT(op2, 2))
			util::stream_format(stream, ",%s", s_r_names[BIT(op2, 4, 4)]);
		return 3 | SUPPORTED;
	}
	else if ((op2 & 0x18) == 0x08 && (op2 & 0x03) != 0x03 && (!BIT(op, 0) || !BIT(op2, 2)))
	{
		util::stream_format(stream, "%-8s", BIT(op2, 2) ? "XCHW" : "MOVW");
		if (!BIT(op2, 2))
			util::stream_format(stream, "%s,", s_rp_names[BIT(op2, 5, 3)]);
		if (BIT(op2, 1))
			format_sfrp(stream, opcodes.r8(pc + 2));
		else if (BIT(op2, 0))
			format_saddrp1(stream, opcodes.r8(pc + 2));
		else
			format_saddrp(stream, opcodes.r8(pc + 2));
		if (BIT(op2, 2))
			util::stream_format(stream, ",%s", s_rp_names[BIT(op2, 5, 3)]);
		return 3 | SUPPORTED;
	}
	else if (!BIT(op, 0) && (op2 & 0x9a) == 0x98)
	{
		util::stream_format(stream, "%-8s", "MOVG");
		if (!BIT(op2, 2))
			util::stream_format(stream, "%s,", s_rg_names[BIT(op2, 5, 2)]);
		if (BIT(op2, 0))
			format_saddrg1(stream, opcodes.r8(pc + 2));
		else
			format_saddrg2(stream, opcodes.r8(pc + 2));
		if (BIT(op2, 2))
			util::stream_format(stream, ",%s", s_rg_names[BIT(op2, 5, 2)]);
		return 3 | SUPPORTED;
	}
	else if (!BIT(op, 0) && (op2 & 0x9f) == 0x9b)
	{
		util::stream_format(stream, "%-8s%s,", "MOVG", s_rg_names[BIT(op2, 5, 2)]);
		format_imm24(stream, u32(opcodes.r8(pc + 4)) << 16 | opcodes.r16(pc + 2));
		return 5 | SUPPORTED;
	}
	else
		return dasm_illegal2(stream, op, op2);
}

offs_t upd78k4_disassembler::dasm_3c(std::ostream &stream, u8 op, offs_t pc, const upd78k4_disassembler::data_buffer &opcodes)
{
	if (op == 0x3f)
	{
		util::stream_format(stream, "%-8s", "CALL");
		format_jdisp16(stream, pc, opcodes.r16(pc + 1));
		return 3 | STEP_OVER | SUPPORTED;
	}
	else
	{
		u8 op2 = opcodes.r8(pc + 1);
		if (op == 0x3e)
		{
			if ((op2 & 0x0d) < 0x05)
			{
				util::stream_format(stream, "%-8s", BIT(op2, 2) ? "XCH" : "MOV");
				if (!BIT(op2, 0))
					util::stream_format(stream, "%s,", s_r_names[BIT(op2, 4, 4)]);
				if (BIT(op2, 1))
					format_abs24(stream, u32(opcodes.r8(pc + 2)) << 16 | opcodes.r16(pc + 3));
				else
					format_abs16(stream, opcodes.r16(pc + 2));
				if (BIT(op2, 0))
					util::stream_format(stream, ",%s", s_r_names[BIT(op2, 4, 4)]);
				return (BIT(op2, 1) ? 5 : 4) | SUPPORTED;
			}
			else if ((op2 & 0x1c) == 0x08)
			{
				util::stream_format(stream, "%-8s", "MOVW");
				if (!BIT(op2, 0))
					util::stream_format(stream, "%s,", s_rp_names[BIT(op2, 5, 3)]);
				if (BIT(op2, 1))
					format_abs24(stream, u32(opcodes.r8(pc + 2)) << 16 | opcodes.r16(pc + 3));
				else
					format_abs16(stream, opcodes.r16(pc + 2));
				if (BIT(op2, 0))
					util::stream_format(stream, ",%s", s_rp_names[BIT(op2, 5, 3)]);
				return (BIT(op2, 1) ? 5 : 4) | SUPPORTED;
			}
			else if ((op2 & 0x9e) == 0x9a)
			{
				util::stream_format(stream, "%-8s", "MOVG");
				if (!BIT(op2, 0))
					util::stream_format(stream, "%s,", s_rg_names[BIT(op2, 5, 2)]);
				format_abs24(stream, u32(opcodes.r8(pc + 2)) << 16 | opcodes.r16(pc + 3));
				if (BIT(op2, 0))
					util::stream_format(stream, ",%s", s_rg_names[BIT(op2, 5, 2)]);
				return 5 | SUPPORTED;
			}
			else if ((op2 & 0x9d) == 0x0d)
			{
				util::stream_format(stream, "%-8s%s", BIT(op2, 1) ? "DECW" : "INCW", s_rp_names[BIT(op2, 5, 2)]);
				return 2 | SUPPORTED;
			}
			else if ((op2 & 0x9d) == 0x9d)
			{
				util::stream_format(stream, "%-8s%s", BIT(op2, 1) ? "DECG" : "INCG", s_rg_names[BIT(op2, 5, 2)]);
				return 2 | SUPPORTED;
			}
			else
				return dasm_illegal2(stream, 0x3e, op2);
		}
		else if (op == 0x3d)
		{
			if ((op2 & 0xf0) == 0x10)
			{
				util::stream_format(stream, "%-8s[%s].%d,CY", "MOV1", s_rg_names[BIT(op2, 3) ? 3 : 2], op2 & 0x07);
				return 2 | SUPPORTED;
			}
			else if (op2 < 0x70)
			{
				util::stream_format(stream, "%-8sCY,", s_bool_ops[BIT(op2, 5, 2)]);
				if (BIT(op2, 4))
					stream << "/";
				util::stream_format(stream, "[%s].%d", s_rg_names[BIT(op2, 3) ? 3 : 2], op2 & 0x07);
				return 2 | SUPPORTED;
			}
			else if (op2 < 0xa0)
			{
				util::stream_format(stream, "%-8s", op2 < 0x80 ? "NOT1" : BIT(op2, 4) ? "CLR1" : "SET1");
				util::stream_format(stream, "[%s].%d", s_rg_names[BIT(op2, 3) ? 3 : 2], op2 & 0x07);
				return 2 | SUPPORTED;
			}
			else if (op2 < 0xe0)
			{
				if (BIT(op2, 6))
					util::stream_format(stream, "%-8s", BIT(op2, 4) ? "BTCLR" : "BFSET");
				else
					util::stream_format(stream, "%-8s", BIT(op2, 4) ? "BT" : "BF");
				util::stream_format(stream, "[%s].%d", s_rg_names[BIT(op2, 3) ? 3 : 2], op2 & 0x07);
				format_jdisp8(stream, pc + 3, opcodes.r8(pc + 2));
				return 3 | SUPPORTED;
			}
			else
				return dasm_illegal2(stream, 0x3d, op2);
		}
		else
		{
			if (op2 == 0x05)
			{
				u8 op3 = opcodes.r8(pc + 2);
				if ((op3 & 0xe8) == 0x08)
				{
					util::stream_format(stream, "%-8s%s", BIT(op3, 4) ? "DIVUW" : "MULU", s_r_names[op3 & 0x0f]);
					return 2 | SUPPORTED;
				}
				else
					return dasm_illegal3(stream, 0x3c, 0x05, op3);
			}
			else if (op2 == 0x07)
			{
				u8 op3 = opcodes.r8(pc + 2);
				if ((op3 & 0x68) == 0x28 || (op3 & 0x7f) == 0x30 || op3 == 0x34)
				{
					util::stream_format(stream, "%-8s", BIT(op3, 3) ? s_alu_ops[op3 & 0x07] : BIT(op3, 2) ? "XCH" : "MOV");
					if (!BIT(op3, 7))
						stream << "A,";
					stream << "[";
					if (BIT(op3, 4))
					{
						stream << "%";
						format_saddrg1(stream, opcodes.r8(pc + 3));
					}
					else
						format_saddrp1(stream, opcodes.r8(pc + 3));
					stream << "]";
					if (BIT(op3, 7))
						stream << ",A";
					return 4 | SUPPORTED;
				}
				else if ((op3 & 0x6b) == 0x21 && (op3 & 0x84) != 0x84)
				{
					util::stream_format(stream, "%-8s", BIT(op3, 2) ? "XCHW" : "MOVW");
					if (!BIT(op3, 7))
						stream << "AX,";
					stream << "[";
					if (BIT(op3, 4))
					{
						stream << "%";
						format_saddrg1(stream, opcodes.r8(pc + 3));
					}
					else
						format_saddrp1(stream, opcodes.r8(pc + 3));
					stream << "]";
					if (BIT(op3, 7))
						stream << ",AX";
					return 4 | SUPPORTED;
				}
				else if ((op3 & 0x7f) == 0x32)
				{
					util::stream_format(stream, "%-8s", "MOVG");
					if (!BIT(op3, 7))
						stream << "WHL,";
					stream << "[%";
					format_saddrg1(stream, opcodes.r8(pc + 3));
					stream << "]";
					if (BIT(op3, 7))
						stream << ",WHL";
					return 4 | SUPPORTED;
				}
				else if ((op3 & 0xfe) == 0xe8)
				{
					util::stream_format(stream, "%-8s", BIT(op3, 0) ? "DECW" : "INCW");
					format_saddrp1(stream, opcodes.r8(pc + 3));
					return 4 | SUPPORTED;
				}
				else
					return dasm_illegal3(stream, 0x3c, 0x07, op3);
			}
			else if (op2 == 0x08)
			{
				u8 op3 = opcodes.r8(pc + 2);
				if ((op3 & 0xf8) == 0x10)
				{
					util::stream_format(stream, "%-8s", "MOV1");
					format_saddr1(stream, opcodes.r8(pc + 3));
					util::stream_format(stream, ".%d,CY", op3 & 0x07);
					return 4 | SUPPORTED;
				}
				else if (op3 < 0x70 && !BIT(op3, 3))
				{
					util::stream_format(stream, "%-8sCY,", s_bool_ops[BIT(op3, 5, 2)]);
					if (BIT(op3, 4))
						stream << "/";
					format_saddr1(stream, opcodes.r8(pc + 3));
					util::stream_format(stream, ".%d", op3 & 0x07);
					return 4 | SUPPORTED;
				}
				else if (op3 < 0xa0 && !BIT(op3, 3))
				{
					util::stream_format(stream, "%-8s", op3 < 0x80 ? "NOT1" : BIT(op3, 4) ? "CLR1" : "SET1");
					format_saddr1(stream, opcodes.r8(pc + 3));
					util::stream_format(stream, ".%d", op3 & 0x07);
					return 4 | SUPPORTED;
				}
				else if (op3 < 0xe0 && !BIT(op3, 3))
				{
					if (BIT(op3, 6))
						util::stream_format(stream, "%-8s", BIT(op3, 4) ? "BTCLR" : "BFSET");
					else
						util::stream_format(stream, "%-8s", BIT(op3, 4) ? "BT" : "BF");
					format_saddr1(stream, opcodes.r8(pc + 3));
					util::stream_format(stream, ".%d,", op3 & 0x07);
					format_jdisp8(stream, pc + 5, opcodes.r8(pc + 4));
					return 5 | SUPPORTED;
				}
				else
					return dasm_illegal3(stream, 0x3c, 0x08, op3);
			}
			else if ((op2 & 0xfc) == 0x0c)
			{
				util::stream_format(stream, "%-8s", s_16bit_ops[op2 & 0x03]);
				format_saddrp1(stream, opcodes.r8(pc + 2));
				stream << ",";
				format_imm16(stream, opcodes.r16(pc + 3));
				return 5 | SUPPORTED;
			}
			else if ((op2 & 0xfe) == 0x18)
			{
				util::stream_format(stream, "%-8s", "MOV");
				if (!BIT(op2, 0))
					stream << "A,";
				stream << "[";
				format_saddrp1(stream, opcodes.r8(pc + 2));
				stream << "]";
				if (BIT(op2, 0))
					stream << ",A";
				return 3 | SUPPORTED;
			}
			else if (op2 == 0x23)
			{
				util::stream_format(stream, "%-8sA,[", "XCH");
				format_saddrp1(stream, opcodes.r8(pc + 2));
				stream << "]";
				return 3 | SUPPORTED;
			}
			else if ((op2 & 0xfe) == 0x24)
			{
				u8 rr = opcodes.r8(pc + 2);
				if (!BIT(rr, 3))
				{
					util::stream_format(stream, "%-8s%s,%s", BIT(op2, 0) ? "XCH" : "MOV", s_r_names[BIT(rr, 4, 4)], s_r_names[(rr & 0x07) | 8]);
					return 3 | SUPPORTED;
				}
				else
					return dasm_illegal3(stream, op, op2, rr);
			}
			else if ((op2 & 0xfe) == 0x30)
			{
				u8 nr = opcodes.r8(pc + 2);
				if (nr < 0xc0)
				{
					util::stream_format(stream, "%-8s%s,%d", s_shift_ops[BIT(op2, 0)][BIT(nr, 6, 2)], s_r_names[(nr & 0x07) | 8], BIT(nr, 3, 3));
					return 3 | SUPPORTED;
				}
				else
					return dasm_illegal3(stream, 0x3c, op2, nr);
			}
			else if (op2 == 0x3a)
			{
				util::stream_format(stream, "%-8s", "MOV");
				format_saddr1(stream, opcodes.r8(pc + 2));
				stream << ",";
				format_imm8(stream, opcodes.r8(pc + 3));
				return 4 | SUPPORTED;
			}
			else if ((op2 & 0xf8) == 0x68)
			{
				util::stream_format(stream, "%-8s", s_alu_ops[op2 & 0x07]);
				format_saddr1(stream, opcodes.r8(pc + 2));
				stream << ",";
				format_imm8(stream, opcodes.r8(pc + 3));
				return 4 | SUPPORTED;
			}
			else if ((op2 & 0xf8) == 0x88)
			{
				u8 rr = opcodes.r8(pc + 2);
				if (!BIT(rr, 3))
				{
					util::stream_format(stream, "%-8s%s,%s", s_alu_ops[op2 & 0x07], s_r_names[BIT(rr, 4, 4)], s_r_names[(rr & 0x07) | 8]);
					return 3 | SUPPORTED;
				}
				else
					return dasm_illegal3(stream, 0x3c, op2, rr);
			}
			else if ((op2 & 0xe8) == 0xa0)
			{
				util::stream_format(stream, "%-8s", BIT(op2, 4) ? "SET1" : "CLR1");
				format_saddr1(stream, opcodes.r8(pc + 2));
				util::stream_format(stream, ".%d", op2 & 0x07);
				return 3 | SUPPORTED;
			}
			else if ((op2 & 0xf8) == 0xb8)
			{
				util::stream_format(stream, "%-8s%s,", "MOV", s_r_names[op2 & 0x0f]);
				format_imm8(stream, opcodes.r8(pc + 2));
				return 3 | SUPPORTED;
			}
			else if ((op2 & 0xf0) == 0xc0)
			{
				util::stream_format(stream, "%-8s%s", BIT(op2, 3) ? "DEC" : "INC", s_r_names[(op2 & 0x07) | 8]);
				return 2 | SUPPORTED;
			}
			else if ((op2 & 0xf0) == 0xd0)
			{
				util::stream_format(stream, "%-8sA,%s", BIT(op2, 3) ? "XCH" : "MOV", s_r_names[(op2 & 0x07) | 8]);
				return 2 | SUPPORTED;
			}
			else
				return dasm_illegal2(stream, 0x3c, op2);
		}
	}
}

offs_t upd78k4_disassembler::dasm_43(std::ostream &stream, offs_t pc, const upd78k4_disassembler::data_buffer &opcodes)
{
	util::stream_format(stream, "%-8s", "BR");
	format_jdisp16(stream, pc, opcodes.r16(pc + 1));
	return 3 | SUPPORTED;
}

offs_t upd78k4_disassembler::dasm_78(std::ostream &stream, u8 op, offs_t pc, const upd78k4_disassembler::data_buffer &opcodes)
{
	u8 op2 = opcodes.r8(pc + 1);
	if ((op2 & 0x0f) < 0x07)
	{
		util::stream_format(stream, "%-8s", s_alu_ops[op & 0x07]);
		if (!BIT(op2, 2))
			util::stream_format(stream, "%s,", s_r_names[BIT(op2, 4, 4)]);
		if (BIT(op2, 1))
		{
			if (BIT(op2, 0))
				format_imm8(stream, opcodes.r16(pc + 2));
			else
				format_sfr(stream, opcodes.r8(pc + 2));
		}
		else
		{
			if (BIT(op2, 0))
				format_saddr1(stream, opcodes.r8(pc + 2));
			else
				format_saddr(stream, opcodes.r8(pc + 2));
		}
		if (BIT(op2, 2))
			util::stream_format(stream, ",%s", s_r_names[BIT(op2, 4, 4)]);
		return 3 | SUPPORTED;
	}
	else if ((op2 & 0x18) == 0x08 && (op2 & 0x0f) != 0x0f && (op == 0x78 || op == 0x7a || op == 0x7f))
	{
		util::stream_format(stream, "%-8s", op == 0x7f ? "CMPW" : op == 0x7a ? "SUBW" : "ADDW");
		if (!BIT(op2, 2))
			util::stream_format(stream, "%s,", s_rp_names[BIT(op2, 5, 3)]);
		if ((op2 & 0x0f) == 0x0b)
		{
			format_imm16(stream, opcodes.r16(pc + 2));
			return 4 | SUPPORTED;
		}
		else
		{
			if (BIT(op2, 1))
				format_sfrp(stream, opcodes.r8(pc + 2));
			else if (BIT(op2, 0))
				format_saddrp1(stream, opcodes.r8(pc + 2));
			else
				format_saddrp(stream, opcodes.r8(pc + 2));
			if (BIT(op2, 2))
				util::stream_format(stream, ",%s", s_rp_names[BIT(op2, 5, 3)]);
			return 3 | SUPPORTED;
		}
	}
	else if ((op & 0x05) == 0 && (op2 & 0xfe) == 0xf8)
	{
		util::stream_format(stream, "%-8sWHL,", BIT(op, 1) ? "SUBG" : "ADDG");
		if (BIT(op2, 0))
			format_saddrg1(stream, opcodes.r8(pc + 2));
		else
			format_saddrg2(stream, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;
	}
	else if ((op & 0x05) == 0 && (op2 & 0x9f) == 0x9b)
	{
		util::stream_format(stream, "%-8s%s,", BIT(op, 1) ? "SUBG" : "ADDG", s_rg_names[BIT(op2, 5, 2)]);
		format_imm24(stream, u32(opcodes.r8(pc + 4)) << 16 | opcodes.r16(pc + 2));
		return 5 | SUPPORTED;
	}
	else
		return dasm_illegal2(stream, op, op2);
}

offs_t upd78k4_disassembler::dasm_88xx(std::ostream &stream, u8 op, u8 rr)
{
	if ((op & 0xfd) == 0x88 && (rr & 0x99) == 0x99)
	{
		util::stream_format(stream, "%-8s%s,%s", BIT(op, 1) ? "SUBG" : "ADDG", s_rg_names[BIT(rr, 5, 2)], s_rg_names[BIT(rr, 1, 2)]);
		return 2 | SUPPORTED;
	}
	else
		return upd78k3_disassembler::dasm_88xx(stream, op, rr);
}


upd784026_disassembler::upd784026_disassembler()
	: upd78k4_disassembler(s_sfr_names, s_sfrp_names)
{
}

const char *const upd784026_disassembler::s_sfr_names[256] =
{
	"P0", "P1", "P2", "P3", "P4", "P5", "P6", "P7",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "P0L", "P0H",
	nullptr, nullptr, nullptr, nullptr, "CR10", nullptr, "CR11", nullptr,
	"CR20", nullptr, "CR21", nullptr, "CR30", nullptr, nullptr, nullptr,
	"PM0", "PM1", nullptr, "PM3", "PM4", "PM5", "PM6", "PM7",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "RTPC", nullptr,
	"CRC0", "TOC", "CRC1", "CRC2", nullptr, nullptr, nullptr, nullptr,
	"CR12", nullptr, "CR22", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, "PMC1", nullptr, "PMC3", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "PUO", nullptr,
	nullptr, nullptr, "TM1", nullptr, "TM2", nullptr, "TM3", nullptr,
	nullptr, nullptr, nullptr, nullptr, "PRM0", "TMC0", "PRM1", "TMC1",
	"DACS0", "DACS1", "DAM", nullptr, nullptr, nullptr, nullptr, nullptr,
	"ADM", nullptr, "ADCR", nullptr, nullptr, nullptr, nullptr, nullptr,
	"PWMC", "PWPR", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, "OSPC", nullptr, nullptr,
	"SBIC", nullptr, "CSIM", nullptr, "CSIM1", "CSIM2", "SIO", nullptr,
	"ASIM", "ASIM2", "ASIS", "ASIS2", "SIO1", "SIO2", nullptr, nullptr,
	"BRGC", "BRGC2", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"INTM0", "INTM1", nullptr, nullptr, "SCS0", nullptr, nullptr, nullptr,
	"ISPR", nullptr, "IMC", nullptr, "MK0L", "MK0H", "MK1L", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"STBC", nullptr, "WDM", nullptr, "MM", "HLDM", "CLOM", "PWC1",
	nullptr, nullptr, nullptr, nullptr, "RFM", "RFA", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external access
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external access
	"PIC0", "PIC1", "PIC2", "PIC3", "CIC00", "CIC01", "CIC10", "CIC11",
	"CIC20", "CIC21", "CIC30", "PIC4", "PIC5", "ADIC", "SERIC", "SRIC",
	"STIC", "CSIIC", "SERIC2", "SRIC2", "STIC2", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, "IMS", nullptr, nullptr, nullptr
};

const char *const upd784026_disassembler::s_sfrp_names[128] =
{
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CR00", "CR01", "CR10W", "CR11W", "CR20W", "CR21W", "CR30W", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, "CR02", "CR12W", "CR22W", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TM0", "TM1W", "TM2W", "TM3W", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, "PWM0", "PWM1", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "MK0", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, "PWC2", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

upd784038_disassembler::upd784038_disassembler()
	: upd78k4_disassembler(s_sfr_names, s_sfrp_names)
{
}

const char *const upd784038_disassembler::s_sfr_names[256] =
{
	"P0", "P1", "P2", "P3", "P4", "P5", "P6", "P7",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "P0L", "P0H",
	nullptr, nullptr, nullptr, nullptr, "CR10", nullptr, "CR11", nullptr,
	"CR20", nullptr, "CR21", nullptr, "CR30", nullptr, nullptr, nullptr,
	"PM0", "PM1", nullptr, "PM3", "PM4", "PM5", "PM6", "PM7",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "RTPC", nullptr,
	"CRC0", "TOC", "CRC1", "CRC2", nullptr, nullptr, nullptr, nullptr,
	"CR12", nullptr, "CR22", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, "PMC1", nullptr, "PMC3", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "PUO", nullptr,
	nullptr, nullptr, "TM1", nullptr, "TM2", nullptr, "TM3", nullptr,
	nullptr, nullptr, nullptr, nullptr, "PRM0", "TMC0", "PRM1", "TMC1",
	"DACS0", "DACS1", "DAM", nullptr, nullptr, nullptr, nullptr, nullptr,
	"ADM", nullptr, "ADCR", nullptr, nullptr, nullptr, nullptr, nullptr,
	"PWMC", "PWPR", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, "OSPC", nullptr, nullptr,
	"IICC", "SPRM", "CSIM", nullptr, "CSIM1", "CSIM2", "SIO", nullptr,
	"ASIM", "ASIM2", "ASIS", "ASIS2", "SIO1", "SIO2", nullptr, nullptr,
	"BRGC", "BRGC2", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"INTM0", "INTM1", nullptr, nullptr, "SCS0", nullptr, nullptr, nullptr,
	"ISPR", nullptr, "IMC", nullptr, "MK0L", "MK0H", "MK1L", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"STBC", nullptr, "WDM", nullptr, "MM", "HLDM", "CLOM", "PWC1",
	nullptr, nullptr, nullptr, nullptr, "RFM", "RFA", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external access
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external access
	"PIC0", "PIC1", "PIC2", "PIC3", "CIC00", "CIC01", "CIC10", "CIC11",
	"CIC20", "CIC21", "CIC30", "PIC4", "PIC5", "ADIC", "SERIC", "SRIC",
	"STIC", "CSIIC", "SERIC2", "SRIC2", "STIC2", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, "IMS", nullptr, nullptr, nullptr
};

const char *const upd784038_disassembler::s_sfrp_names[128] =
{
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"CR00", "CR01", "CR10W", "CR11W", "CR20W", "CR21W", "CR30W", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, "CR02", "CR12W", "CR22W", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TM0", "TM1W", "TM2W", "TM3W", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, "PWM0", "PWM1", nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "MK0", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, "PWC2", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

upd784046_disassembler::upd784046_disassembler()
	: upd78k4_disassembler(s_sfr_names, s_sfrp_names)
{
}

const char *const upd784046_disassembler::s_sfr_names[256] =
{
	"P0", "P1", "P2", "P3", "P4", "P5", "P6", "P7",
	"P8", "P9", nullptr, nullptr, nullptr, nullptr, "P0L", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"PM0", "PM1", "PM2", "PM3", "PM4", "PM5", "PM6", nullptr,
	"PM8", "PM9", nullptr, nullptr, nullptr, nullptr, "RTPC", "PRDC",
	"TUM0", "TMC", "TOC0", "TOC1", "TUM2", "TMC2", "TOC2", "TMC4",
	"PRM", "PRM2", "PRM4", "NPC", "INTM0", "INTM1", "IEF1", "IEF2",
	nullptr, "PMC1", "PMC2", "PMC3", nullptr, nullptr, nullptr, nullptr,
	nullptr, "PMC9", nullptr, nullptr, nullptr, nullptr, "PUOL", "PUOH",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "ADM", nullptr,
	nullptr, "ADCR0H", nullptr, "ADCR1H", nullptr, "ADCR2H", nullptr, "ADCR3H",
	nullptr, "ADCR4H", nullptr, "ADCR5H", nullptr, "ADCR6H", nullptr, "ADCR7H",
	nullptr, nullptr, nullptr, nullptr, "CSIM1", "CSIM2", nullptr, nullptr,
	"ASIM", "ASIM2", "ASIS", "ASIS2", "SIO1", "SIO2", nullptr, nullptr,
	"BRGC", "BRGC2", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"ISPR", nullptr, "IMC", nullptr, "MK0L", "MK0H", "MK1L", "MK1H",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"STBC", nullptr, "WDM", nullptr, "MM", nullptr, nullptr, "PWC1",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "OSTS",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external access
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external access
	"OVIC0", "OVIC1", "OVIC4", "PIC0", "PIC1", "PIC2", "PIC3", "PIC4",
	"PIC5", "PIC6", "CMIC10", "CMIC11", "CMIC20", "CMIC21", "CMIC30", "CMIC31",
	"CMIC40", "CMIC41", "SERIC", "SRIC", "STIC", "SERIC2", "SRIC2", "STIC2",
	"ADIC", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

const char *const upd784046_disassembler::s_sfrp_names[128] =
{
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TM0", "CC00", "CC01", "CC02", "CC03", "TM1", "CM10", "CM11",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TM2", "CM20", "CM21", "TM3", "CM30", "CM31", nullptr, nullptr,
	"TM4", "CM40", "CM41", nullptr, nullptr, nullptr, nullptr, nullptr,
	"ADCR0", "ADCR1", "ADCR2", "ADCR3", "ADCR4", "ADCR5", "ADCR6", "ADCR7",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "MK0", "MK1",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, "PWC2", "BW", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

upd784054_disassembler::upd784054_disassembler()
	: upd78k4_disassembler(s_sfr_names, s_sfrp_names)
{
}

const char *const upd784054_disassembler::s_sfr_names[256] =
{
	"P0", "P1", "P2", "P3", "P4", "P5", "P6", "P7",
	"P8", "P9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"PM0", "PM1", "PM2", "PM3", "PM4", "PM5", "PM6", nullptr,
	nullptr, "PM9", nullptr, nullptr, nullptr, nullptr, nullptr, "PRDC",
	"TUM0", "TMC", "TOC0", "TOC1", nullptr, nullptr, nullptr, "TMC4",
	"PRM", nullptr, "PRM4", "NPC", "INTM0", "INTM1", "IEF1", "IEF2",
	nullptr, nullptr, "PMC2", "PMC3", nullptr, nullptr, nullptr, nullptr,
	nullptr, "PMC9", nullptr, nullptr, nullptr, nullptr, "PUOL", "PUOH",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "ADM", nullptr,
	nullptr, "ADCR0H", nullptr, "ADCR1H", nullptr, "ADCR2H", nullptr, "ADCR3H",
	nullptr, "ADCR4H", nullptr, "ADCR5H", nullptr, "ADCR6H", nullptr, "ADCR7H",
	nullptr, nullptr, nullptr, nullptr, "CSIM1", "CSIM2", nullptr, nullptr,
	"ASIM", "ASIM2", "ASIS", "ASIS2", "SIO1", "SIO2", nullptr, nullptr,
	"BRGC", "BRGC2", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"ISPR", nullptr, "IMC", nullptr, "MK0L", "MK0H", "MK1L", "MK1H",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"STBC", nullptr, "WDM", nullptr, "MM", nullptr, nullptr, "PWC1",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "OSTS",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external access
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external access
	"OVIC0", "OVIC1", "OVIC4", "PIC0", "PIC1", "PIC2", "PIC3", "PIC4",
	"PIC5", "PIC6", "CMIC10", "CMIC11", nullptr, nullptr, nullptr, nullptr,
	"CMIC40", "CMIC41", "SERIC", "SRIC", "STIC", "SERIC2", "SRIC2", "STIC2",
	"ADIC", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

const char *const upd784054_disassembler::s_sfrp_names[128] =
{
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TM0", "CC00", "CC01", "CC02", "CC03", "TM1", "CM10", "CM11",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TM4", "CM40", "CM41", nullptr, nullptr, nullptr, nullptr, nullptr,
	"ADCR0", "ADCR1", "ADCR2", "ADCR3", "ADCR4", "ADCR5", "ADCR6", "ADCR7",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "MK0", "MK1",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, "PWC2", "BW", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

upd784216_disassembler::upd784216_disassembler()
	: upd78k4_disassembler(s_sfr_names, s_sfrp_names)
{
}

const char *const upd784216_disassembler::s_sfr_names[256] =
{
	"P0", "P1", "P2", "P3", "P4", "P5", "P6", "P7",
	"P8", "P9", "P10", nullptr, "P12", "P13", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "CRC0", nullptr,
	"TMC0", nullptr, "TOC0", nullptr, "PRM0", nullptr, nullptr, nullptr,
	"PM0", nullptr, "PM2", "PM3", "PM4", "PM5", "PM6", "PM7",
	"PM8", "PM9", "PM10", nullptr, "PM12", "PM13", nullptr, nullptr,
	"PU0", nullptr, "PU2", "PU3", nullptr, nullptr, nullptr, "PU7",
	"PU8", nullptr, "PU10", nullptr, "PU12", nullptr, nullptr, nullptr,
	"CKS", nullptr, "PF2", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "PUO", nullptr,
	"TM1", "TM2", "CR10", "CR20", "TMC1", "TMC2", "PRM1", "PRM2",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TM5", "TM6", "TM7", "TM8", "CR50", "CR60", "CR70", "CR80",
	"TMC5", "TMC6", "TMC7", "TMC8", "PRM5", "PRM6", "PRM7", "PRM8",
	"ASIM1", "ASIM2", "ASIS1", "ASIS2", "TXS1/RXB1", "TXS2/RXB2", "BRGC1", "BRGC2",
	nullptr, nullptr, "CC", nullptr, nullptr, nullptr, nullptr, nullptr,
	"ADM", "ADIS", nullptr, "ADCR", "DACS0", "DACS1", "DAM0", "DAM1",
	nullptr, nullptr, nullptr, nullptr, "EBTS", nullptr, nullptr, nullptr,
	"CSIM0", "CSIM1", "CSIM2", nullptr, "SIO0", "SIO1", "SIO2", nullptr,
	"RTBL", "RTBH", "RTPM", "RTPC", "WTM", nullptr, nullptr, nullptr,
	"EGP0", nullptr, "EGN0", nullptr, nullptr, nullptr, nullptr, nullptr,
	"ISPR", "SNMI", "IMC", nullptr, "MK0L", "MK0H", "MK1L", "MK1H",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"STBC", nullptr, "WDM", nullptr, "MM", nullptr, nullptr, "PWC1",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "PCS", "OSTS",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external access
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external access
	"WDTIC", "PIC0", "PIC1", "PIC2", "PIC3", "PIC4", "PIC5", "PIC6",
	"CSIIC0", "SERIC1", "SRIC1", "STIC1", "SERIC2", "SRIC2", "STIC2", "TMIC3",
	"TMIC00", "TMIC01", "TMIC1", "TMIC2", "ADIC", "TMIC5", "TMIC6", "TMIC7",
	"TMIC8", "WTIC", "KRIC", nullptr, nullptr, nullptr, nullptr, nullptr
};

const char *const upd784216_disassembler::s_sfrp_names[128] =
{
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TM0", "CR00", "CR01", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TM1W", "CR1W", "TMC1W", "PRM1W", nullptr, nullptr, nullptr, nullptr,
	"TM5W", "TM7W", "CR5W", "CR7W", "TMC5W", "TMC7W", "PRM5W", "PRM7W",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "MK0", "MK1",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

upd784218_disassembler::upd784218_disassembler()
	: upd78k4_disassembler(s_sfr_names, s_sfrp_names)
{
}

const char *const upd784218_disassembler::s_sfr_names[256] =
{
	"P0", "P1", "P2", "P3", "P4", "P5", "P6", "P7",
	"P8", "P9", "P10", nullptr, "P12", "P13", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "CRC0", nullptr,
	"TMC0", nullptr, "TOC0", nullptr, "PRM0", nullptr, nullptr, nullptr,
	"PM0", nullptr, "PM2", "PM3", "PM4", "PM5", "PM6", "PM7",
	"PM8", "PM9", "PM10", nullptr, "PM12", "PM13", nullptr, nullptr,
	"PU0", nullptr, "PU2", "PU3", nullptr, nullptr, nullptr, "PU7",
	"PU8", nullptr, "PU10", nullptr, "PU12", nullptr, nullptr, nullptr,
	"CKS", nullptr, "PF2", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "PUO", nullptr,
	"TM1", "TM2", "CR10", "CR20", "TMC1", "TMC2", "PRM1", "PRM2",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TM5", "TM6", "TM7", "TM8", "CR50", "CR60", "CR70", "CR80",
	"TMC5", "TMC6", "TMC7", "TMC8", "PRM5", "PRM6", "PRM7", "PRM8",
	"ASIM1", "ASIM2", "ASIS1", "ASIS2", "TXS1/RXB1", "TXS2/RXB2", "BRGC1", "BRGC2",
	nullptr, nullptr, "CC", nullptr, nullptr, nullptr, nullptr, nullptr,
	"ADM", "ADIS", nullptr, "ADCR", "DACS0", "DACS1", "DAM0", "DAM1",
	"CORC", "CORAH", nullptr, nullptr, "EBTS", "EXAE", nullptr, nullptr,
	"CSIM0", "CSIM1", "CSIM2", nullptr, "SIO0", "SIO1", "SIO2", nullptr,
	"RTBL", "RTBH", "RTPM", "RTPC", "WTM", nullptr, nullptr, nullptr,
	"EGP0", nullptr, "EGN0", nullptr, nullptr, nullptr, nullptr, nullptr,
	"ISPR", "SNMI", "IMC", nullptr, "MK0L", "MK0H", "MK1L", "MK1H",
	"IICC0", nullptr, "SRPM0", nullptr, "SVA0", nullptr, "IICS0", nullptr,
	"IIC0", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"STBC", nullptr, "WDM", nullptr, "MM", nullptr, nullptr, "PWC1",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "PCS", "OSTS",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external access
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external access
	"WDTIC", "PIC0", "PIC1", "PIC2", "PIC3", "PIC4", "PIC5", "PIC6",
	"CSIIC0", "SERIC1", "SRIC1", "STIC1", "SERIC2", "SRIC2", "STIC2", "TMIC3",
	"TMIC00", "TMIC01", "TMIC1", "TMIC2", "ADIC", "TMIC5", "TMIC6", "TMIC7",
	"TMIC8", "WTIC", "KRIC", nullptr, nullptr, nullptr, nullptr, nullptr
};

const char *const upd784218_disassembler::s_sfrp_names[128] =
{
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TM0", "CR00", "CR01", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TM1W", "CR1W", "TMC1W", "PRM1W", nullptr, nullptr, nullptr, nullptr,
	"TM5W", "TM7W", "CR5W", "CR7W", "TMC5W", "TMC7W", "PRM5W", "PRM7W",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, "CORAL", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "MK0", "MK1",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

upd784225_disassembler::upd784225_disassembler()
	: upd78k4_disassembler(s_sfr_names, s_sfrp_names)
{
}

const char *const upd784225_disassembler::s_sfr_names[256] =
{
	"P0", "P1", "P2", "P3", "P4", "P5", "P6", "P7",
	nullptr, nullptr, nullptr, nullptr, "P12", "P13", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "CRC0", nullptr,
	"TMC0", nullptr, "TOC0", nullptr, "PRM0", nullptr, nullptr, nullptr,
	"PM0", nullptr, "PM2", "PM3", "PM4", "PM5", "PM6", "PM7",
	nullptr, nullptr, nullptr, nullptr, "PM12", "PM13", nullptr, nullptr,
	"PU0", nullptr, "PU2", "PU3", nullptr, nullptr, nullptr, "PU7",
	nullptr, nullptr, nullptr, nullptr, "PU12", nullptr, nullptr, nullptr,
	"CKS", nullptr, "PF2", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "PUO", nullptr,
	"TM1", "TM2", "CR10", "CR20", "TMC1", "TMC2", "PRM1", "PRM2",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TM5", "TM6", nullptr, nullptr, "CR50", "CR60", nullptr, nullptr,
	"TMC5", "TMC6", nullptr, nullptr, "PRM5", "PRM6", nullptr, nullptr,
	"ASIM1", "ASIM2", "ASIS1", "ASIS2", "TXS1/RXB1", "TXS2/RXB2", "BRGC1", "BRGC2",
	nullptr, nullptr, "CC", nullptr, nullptr, nullptr, nullptr, nullptr,
	"ADM", "ADIS", nullptr, "ADCR", "DACS0", "DACS1", "DAM0", "DAM1",
	"CORC", "CORAH", nullptr, nullptr, nullptr, "EXAE", nullptr, nullptr,
	"CSIM0", "CSIM1", "CSIM2", nullptr, "SIO0", "SIO1", "SIO2", nullptr,
	"RTBL", "RTBH", "RTPM", "RTPC", "WTM", nullptr, nullptr, nullptr,
	"EGP0", nullptr, "EGN0", nullptr, nullptr, nullptr, nullptr, nullptr,
	"ISPR", "SNMI", "IMC", nullptr, "MK0L", "MK0H", "MK1L", "MK1H",
	"IICC0", nullptr, "SRPM0", nullptr, "SVA0", nullptr, "IICS0", nullptr,
	"IIC0", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"STBC", nullptr, "WDM", nullptr, "MM", nullptr, nullptr, "PWC1",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "PCS", "OSTS",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external access
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // external access
	"WDTIC", "PIC0", "PIC1", "PIC2", "PIC3", "PIC4", "PIC5", "PIC6",
	"CSIIC0", "SERIC1", "SRIC1", "STIC1", "SERIC2", "SRIC2", "STIC2", "TMIC3",
	"TMIC00", "TMIC01", "TMIC1", "TMIC2", "ADIC", "TMIC5", "TMIC6", nullptr,
	nullptr, "WTIC", nullptr, nullptr, "IMS", nullptr, nullptr, nullptr
};

const char *const upd784225_disassembler::s_sfrp_names[128] =
{
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TM0", "CR00", "CR01", nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TM1W", "CR1W", "TMC1W", "PRM1W", nullptr, nullptr, nullptr, nullptr,
	"TM5W", "TM7W", "CR5W", "CR7W", "TMC5W", nullptr, "PRM5W", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, "CORAL", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "MK0", "MK1",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, "PWC2", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};
