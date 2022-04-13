// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Mitsubishi/Renesas M32C disassembler

***************************************************************************/

#include "emu.h"
#include "m32cdasm.h"

m32c_disassembler::m32c_disassembler()
	: util::disasm_interface()
{
}

const char *const m32c_disassembler::s_aregs[4] =
{
	"A0", "A1", "SB", "FB"
};

const char *const m32c_disassembler::s_sregs_16bit[8] =
{
	"DCT0", "DCT1", "FLG", "SVF", "DRC0", "DRC1", "DMD0", "DMD1"
};

const char *const m32c_disassembler::s_sregs_24bit[8] =
{
	"INTB", "SP", "SB", "FB", "SVP", "VCT", "<invalid>", "ISP"
};

const char *const m32c_disassembler::s_sregs_dma[8] =
{
	"<invalid>", "<invalid>", "DMA0", "DMA1", "DRA0", "DRA1", "DSA0", "DSA1"
};

const char *const m32c_disassembler::s_cnds[16] =
{
	"nc", "leu", "ne", "pz", "no", "gt", "ge", "?z",
	"c", "gtu", "eq", "n", "o", "le", "lt", "?o"
};

const char *const m32c_disassembler::s_dadd_ops[6] =
{
	"dadd", "dsub", "adc", "sbb", "dadc", "dsbb"
};

const char *const m32c_disassembler::s_bit_ops[8] =
{
	"bntst", "band", "bt?", "bnand", "bor", "bxor", "bnor", "bnxor"
};

const char *const m32c_disassembler::s_long_ops[4] =
{
	"add", "sub", "cmp", "mov"
};

const char *const m32c_disassembler::s_long_shift_ops[5] =
{
	"shlnc", "shl", "sha", "sh?", "shanc"
};

const char *const m32c_disassembler::s_imm1111_ops[8] =
{
	"mulu", "mul", "or", "and", "stz", "stnz", "mov", "stzx"
};

const char *const m32c_disassembler::s_str_ops[4] =
{
	"smovf", "sin", "sout", "smovb"
};

const char *const m32c_disassembler::s_sstr_ops[4] =
{
	"sstr", "rmpa", "smovu", "scmpu"
};

const char *const m32c_disassembler::s_btst_ops[8] =
{
	"btst", "bit?", "bmcnd", "bnot", "btstc", "btsts", "bclr", "bset"
};

const u8 m32c_disassembler::s_short_code[4] =
{
	0b10010, 0b01111, 0b00110, 0b00111
};

u32 m32c_disassembler::opcode_alignment() const
{
	return 1;
}

void m32c_disassembler::format_imm8(std::ostream &stream, u8 imm) const
{
	stream << '#';
	if (imm >= 0xa0)
		stream << '0';
	util::stream_format(stream, "%02Xh", imm);
}

void m32c_disassembler::format_imm16(std::ostream &stream, u16 imm) const
{
	stream << '#';
	if (imm >= 0xa000)
		stream << '0';
	util::stream_format(stream, "%04Xh", imm);
}

void m32c_disassembler::format_imm24(std::ostream &stream, u32 imm) const
{
	stream << '#';
	if (imm >= 0xa00000)
		stream << '0';
	util::stream_format(stream, "%06Xh", imm);
}

void m32c_disassembler::format_imm32(std::ostream &stream, u32 imm) const
{
	stream << '#';
	if (imm >= 0xa0000000)
		stream << '0';
	util::stream_format(stream, "%08Xh", imm);
}

void m32c_disassembler::format_label(std::ostream &stream, u32 label) const
{
	label &= 0xffffff;
	if (label >= 0xa00000)
		stream << '0';
	util::stream_format(stream, "%06Xh", label);
}

void m32c_disassembler::format_imm_signed(std::ostream &stream, s32 imm) const
{
	stream << '#';
	if (imm < 0)
	{
		stream << '-';
		imm = -imm;
	}
	if (imm > 9)
	{
		int places = 1;
		for (u32 temp = imm; temp > 9; temp >>= 4)
			++places;
		util::stream_format(stream, "%0*Xh", places, u32(imm));
	}
	else
		util::stream_format(stream, "%d", imm);
}

void m32c_disassembler::format_relative(std::ostream &stream, const char *reg, s32 disp) const
{
	if (disp < 0)
	{
		stream << '-';
		disp = -disp;
	}
	if (disp > 9)
	{
		int places = 1;
		for (u32 temp = disp; temp > 9; temp >>= 4)
			++places;
		util::stream_format(stream, "%0*Xh[%s]", places, u32(disp), reg);
	}
	else
		util::stream_format(stream, "%d[%s]", disp, reg);
}

void m32c_disassembler::dasm_abs16(std::ostream &stream, offs_t &pc, const m32c_disassembler::data_buffer &opcodes) const
{
	u16 abs = opcodes.r16(pc);
	pc += 2;
	if (abs >= 0xa000)
		stream << "0";
	util::stream_format(stream, "%04Xh", abs);
}

void m32c_disassembler::dasm_abs24(std::ostream &stream, offs_t &pc, const m32c_disassembler::data_buffer &opcodes) const
{
	u32 abs = opcodes.r16(pc);
	abs |= u32(opcodes.r8(pc + 2)) << 16;
	pc += 3;
	if (abs >= 0xa00000)
		stream << "0";
	util::stream_format(stream, "%06Xh", abs);
}

void m32c_disassembler::dasm_operand(std::ostream &stream, offs_t &pc, const m32c_disassembler::data_buffer &opcodes, u8 code, u8 size, bool indirect) const
{
	if (indirect)
		stream << '[';
	if (code < 0b00010)
		util::stream_format(stream, "[%s]", s_aregs[code]);
	else if (code < 0b00100 && !indirect)
		stream << s_aregs[code & 1];
	else if (code >= 0b00100 && code < 0b01110)
	{
		if (code < 0b01000)
			format_relative(stream, s_aregs[code & 3], s8(opcodes.r8(pc++)));
		else
		{
			s16 disp = s16(opcodes.r16(pc));
			pc += 2;
			if (code >= 0b01100)
				format_relative(stream, s_aregs[code & 3], u16(disp) | u32(opcodes.r8(pc++)) << 16);
			else
				format_relative(stream, s_aregs[code & 3], disp);
		}
	}
	else if (code == 0b01111)
		dasm_abs16(stream, pc, opcodes);
	else if (code == 0b01110)
		dasm_abs24(stream, pc, opcodes);
	else if (code < 0b10100 && !indirect)
	{
		switch (size)
		{
		case 0: // .B
			util::stream_format(stream, "R%d%c", code & 1, BIT(code, 1) ? 'L' : 'H');
			break;

		case 1: // .W
			util::stream_format(stream, "R%d", (code & 3) ^ 2);
			break;

		case 2: // .L
		case 3: // X
			if (BIT(code, 1))
				util::stream_format(stream, "R%dR%d", code & 3, code & 1);
			else
				stream << "<invalid>";
			break;

		default:
			stream << "<invalid>";
			break;
		}
	}
	else
		stream << "<invalid>";
	if (indirect)
		stream << ']';
}

void m32c_disassembler::dasm_immediate_mode(std::ostream &stream, offs_t &pc, const m32c_disassembler::data_buffer &opcodes, u8 code, u8 size, bool indirect, int count, bool signd) const
{
	unsigned offset = 0;
	switch (code)
	{
	case 0b00100: case 0b00101: case 0b00110: case 0b00111:
		offset = 1;
		break;

	case 0b01000: case 0b01001: case 0b01111:
		offset = 2;
		break;

	case 0b01010: case 0b01011: case 0b01110:
		offset = 3;
		break;

	default:
		break;
	}

	// Immediate bytes come after relative displacements
	switch (size)
	{
	case 0: case 3:
		if (signd)
		{
			for (int i = 0; i < count; ++i)
			{
				format_imm_signed(stream, s8(opcodes.r8(pc + offset++)));
				stream << ", ";
			}
		}
		else
		{
			for (int i = 0; i < count; ++i)
			{
				format_imm8(stream, opcodes.r8(pc + offset++));
				stream << ", ";
			}
		}
		dasm_operand(stream, pc, opcodes, code, size, indirect);
		pc += count;
		break;

	case 1:
		if (signd)
		{
			for (int i = 0; i < count; ++i)
			{
				format_imm_signed(stream, s16(opcodes.r16(pc + offset)));
				stream << ", ";
				offset += 2;
			}
		}
		else
		{
			for (int i = 0; i < count; ++i)
			{
				format_imm16(stream, opcodes.r16(pc + offset));
				stream << ", ";
				offset += 2;
			}
		}
		dasm_operand(stream, pc, opcodes, code, size, indirect);
		pc += count << 1;
		break;

	case 2:
		assert(count == 1);
		if (signd)
			format_imm_signed(stream, s32(opcodes.r32(pc + offset)));
		else
			format_imm32(stream, opcodes.r32(pc + offset));
		stream << ", ";
		dasm_operand(stream, pc, opcodes, code, size, indirect);
		pc += 4;
		break;
	}
}

void m32c_disassembler::dasm_00000001(std::ostream &stream, offs_t &pc, const m32c_disassembler::data_buffer &opcodes, bool indirect_src, bool indirect_dest) const
{
	u8 op2 = opcodes.r8(pc++);
	if (op2 < 0x80 || op2 >= 0xe0 || (op2 & 0x0e) > 0x08)
		stream << "und";
	else
	{
		u8 op3 = opcodes.r8(pc++);
		if (op2 >= 0xd0)
		{
			if (!BIT(op2, 0))
			{
				util::stream_format(stream, "%-11s%d, ", s_bit_ops[BIT(op3, 3, 3)], BIT(op3, 0, 3));
				dasm_operand(stream, pc, opcodes, (op2 & 0x0e) << 1 | BIT(op3, 6, 2), 0, indirect_src);
			}
			else if (!BIT(op3, 6))
			{
				if (BIT(op3, 4))
				{
					util::stream_format(stream, "%-11s%s, ", "stc", BIT(op3, 3) ? s_sregs_16bit[op3 & 0x07] : s_sregs_dma[op3 & 0x07]);
					dasm_operand(stream, pc, opcodes, (op2 & 0x0e) << 1 | BIT(op3, 6, 2), BIT(op3, 3) ? 1 : 2, indirect_dest);
				}
				else
				{
					util::stream_format(stream, "%-11s", "ldc");
					dasm_operand(stream, pc, opcodes, (op2 & 0x0e) << 1 | BIT(op3, 6, 2), BIT(op3, 3) ? 1 : 2, indirect_src);
					util::stream_format(stream, ", %s", BIT(op3, 3) ? s_sregs_16bit[op3 & 0x07] : s_sregs_dma[op3 & 0x07]);
				}
			}
			else
				stream << "und";
		}
		else if ((op3 & 0x0f) < 0x0c && !BIT(op3, 0))
		{
			util::stream_format(stream, "%-11s", util::string_format("%s.%c", s_dadd_ops[BIT(op3, 1, 3)], BIT(op2, 0) ? 'W' : 'B'));
			dasm_operand(stream, pc, opcodes, (op2 & 0x70) >> 2 | BIT(op3, 4, 2), op2 & 1, indirect_src);
			stream << ", ";
			dasm_operand(stream, pc, opcodes, (op2 & 0x0e) << 1 | BIT(op3, 6, 2), op2 & 1, indirect_dest);
		}
		else if ((op3 & 0x0f) == 0b0111 && !BIT(op2, 0))
		{
			util::stream_format(stream, "%-11s", "exts.b");
			dasm_operand(stream, pc, opcodes, (op2 & 0x70) >> 2 | BIT(op3, 4, 2), 0, indirect_src);
			stream << ", ";
			dasm_operand(stream, pc, opcodes, (op2 & 0x0e) << 1 | BIT(op3, 6, 2), 1, indirect_dest);
		}
		else if ((op3 & 0x0f) == 0b1011 && !BIT(op2, 0))
		{
			util::stream_format(stream, "%-11s", "extz");
			dasm_operand(stream, pc, opcodes, (op2 & 0x70) >> 2 | BIT(op3, 4, 2), 0, indirect_src);
			stream << ", ";
			dasm_operand(stream, pc, opcodes, (op2 & 0x0e) << 1 | BIT(op3, 6, 2), 1, indirect_dest);
		}
		else if ((op3 & 0x0f) == 0x09)
		{
			util::stream_format(stream, "%-11s", util::string_format("tst.%c", BIT(op2, 0) ? 'w' : 'b'));
			dasm_operand(stream, pc, opcodes, (op2 & 0x70) >> 2 | BIT(op3, 4, 2), op2 & 1, indirect_src);
			stream << ", ";
			dasm_operand(stream, pc, opcodes, (op2 & 0x0e) << 1 | BIT(op3, 6, 2), op2 & 1, indirect_dest);
		}
		else if ((op3 & 0x0e) == 0x0c)
		{
			util::stream_format(stream, "%-11s", util::string_format("%s.%c", BIT(op3, 0) ? "min" : "max", BIT(op2, 0) ? 'w' : 'b'));
			dasm_operand(stream, pc, opcodes, (op2 & 0x70) >> 2 | BIT(op3, 4, 2), op2 & 1, indirect_src);
			stream << ", ";
			dasm_operand(stream, pc, opcodes, (op2 & 0x0e) << 1 | BIT(op3, 6, 2), op2 & 1, indirect_dest);
		}
		else if ((op2 & 0x70) == 0x00 && (op3 & 0x0f) == 0x0e)
		{
			if ((op3 & 0x30) == 0x30)
			{
				util::stream_format(stream, "%-11s", util::string_format("clip.%c", BIT(op2, 0) ? 'w' : 'b'));
				dasm_immediate_mode(stream, pc, opcodes, (op2 & 0x0e) << 1 | BIT(op3, 6, 2), BIT(op2, 0), indirect_dest, 2, true);
			}
			else
			{
				util::stream_format(stream, "%-11s", util::string_format("%sad%c.%c ", BIT(op3, 5) ? "d" : "", BIT(op3, 4) ? 'd' : 'c', BIT(op2, 0) ? 'w' : 'b'));
				dasm_immediate_mode(stream, pc, opcodes, (op2 & 0x0e) << 1 | BIT(op3, 6, 2), BIT(op2, 0), indirect_dest, 1, !BIT(op3, 5));
			}
		}
		else if ((op2 & 0x70) == 0x10 && (op3 & 0x0f) == 0x0e && (op3 & 0x30) != 0x30)
		{
			util::stream_format(stream, "%-11s", util::string_format("%ss%cb.%c", BIT(op3, 5) ? "d" : "", BIT(op3, 4) ? 'u' : 'b', BIT(op2, 0) ? 'w' : 'b'));
			dasm_operand(stream, pc, opcodes, (op2 & 0x70) >> 2 | BIT(op3, 4, 2), op2 & 1, indirect_src);
			stream << ", ";
			dasm_operand(stream, pc, opcodes, (op2 & 0x0e) << 1 | BIT(op3, 6, 2), op2 & 1, indirect_dest);
		}
		else if ((op2 & 0x61) == 0x20 && (op3 & 0x0f) == 0x0e)
		{
			util::stream_format(stream, "%-11s", util::string_format("mov%c%c", BIT(op3, 4) ? 'h' : 'l', BIT(op3, 5) ? 'h' : 'l'));
			if (BIT(op2, 4))
			{
				stream << "R0L, ";
				dasm_operand(stream, pc, opcodes, (op2 & 0x0e) << 1 | BIT(op3, 6, 2), 0, indirect_dest);
			}
			else
			{
				dasm_operand(stream, pc, opcodes, (op2 & 0x0e) << 1 | BIT(op3, 6, 2), 0, indirect_src);
				stream << ", R0L";
			}
		}
		else if ((op2 & 0x71) == 0x01 && (op3 & 0x2f) == 0x0f)
		{
			util::stream_format(stream, "%-11s", BIT(op3, 4) ? "mul.l" : "mulu.l");
			dasm_operand(stream, pc, opcodes, (op2 & 0x0e) << 1 | BIT(op3, 6, 2), 2, indirect_src);
			stream << ", R2R0";
		}
		else if ((op2 & 0x70) == 0x00 && (op3 & 0x2f) == 0x2f)
		{
			util::stream_format(stream, "%-11s", util::string_format("%s.%c", BIT(op3, 4) ? "min" : "max", BIT(op2, 0) ? 'w' : 'b'));
			dasm_immediate_mode(stream, pc, opcodes, (op2 & 0x0e) << 1 | BIT(op3, 6, 2), BIT(op2, 0), indirect_dest, 1, true);
		}
		else if ((op2 & 0x71) == 0x21 && (op3 & 0x0f) == 0x0f && (op3 & 0x30) != 0x30)
		{
			util::stream_format(stream, "%-11s", util::string_format("div%s.l", BIT(op3, 5) ? "x" : BIT(op3, 4) ? "" : "u"));
			dasm_operand(stream, pc, opcodes, (op2 & 0x0e) << 1 | BIT(op3, 6, 2), 2, indirect_src);
		}
		else
			stream << "und";
	}
}

void m32c_disassembler::dasm_general(std::ostream &stream, offs_t &pc, offs_t &flags, const m32c_disassembler::data_buffer &opcodes, u8 op1, bool indirect_src, bool indirect_dest) const
{
	const u8 op2 = opcodes.r8(pc++);
	switch (op2 & 0x0f)
	{
	case 0b0000:
		util::stream_format(stream, "%-11s", BIT(op1, 0) ? "sub.l" : "subx");
		dasm_operand(stream, pc, opcodes, (op1 & 0x70) >> 2 | BIT(op2, 4, 2), BIT(op1, 0) ? 2 : 0, indirect_src);
		stream << ", ";
		dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), 2, indirect_dest);
		break;

	case 0b0001:
		if (BIT(op1, 0))
		{
			util::stream_format(stream, "%-11s", "cmp.l");
			dasm_operand(stream, pc, opcodes, (op1 & 0x70) >> 2 | BIT(op2, 4, 2), BIT(op1, 0) ? 2 : 0, indirect_src);
			stream << ", ";
			dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), 2, indirect_dest);
		}
		else switch ((op1 & 0x70) >> 2 | BIT(op2, 4, 2))
		{
		case 0b00000:
			util::stream_format(stream, "%-11s", "jmpi.a");
			dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), 2, indirect_src);
			break;

		case 0b00001: case 0b00101: case 0b01001: case 0b01101:
			util::stream_format(stream, "%-11s", util::string_format("%sx", s_long_ops[BIT(op1, 4, 2)]));
			dasm_immediate_mode(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), 3, indirect_dest, 1, true);
			break;

		case 0b00010: case 0b00110: case 0b01010: case 0b10010:
			util::stream_format(stream, "%-11s", util::string_format("%s.l", s_long_shift_ops[BIT(op1, 4, 3)]));
			dasm_immediate_mode(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), 3, indirect_dest, 1, true);
			break;

		case 0b00011: case 0b00111: case 0b01011: case 0b01111:
			util::stream_format(stream, "%-11s", util::string_format("%s.l", s_long_ops[BIT(op1, 4, 2)]));
			dasm_immediate_mode(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), 2, indirect_dest, 1, (op1 & 0x30) != 0x30);
			break;

		case 0b00100:
			util::stream_format(stream, "%-11s", "jsri.a");
			dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), 2, indirect_src);
			flags |= STEP_OVER;
			break;

		case 0b01000:
			util::stream_format(stream, "%-11s", "push.l");
			dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), 2, indirect_src);
			break;

		case 0b01100:
			util::stream_format(stream, "%-11s", "pusha");
			dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), 2, indirect_src);
			break;

		case 0b10000: case 0b10001:
			util::stream_format(stream, "%-11sR1H, ", BIT(op2, 4) ? "sha.l" : "shl.l");
			dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), 2, indirect_dest);
			break;
		}
		break;

	case 0b0010:
		util::stream_format(stream, "%-11s", BIT(op1, 0) ? "add.l" : "addx");
		dasm_operand(stream, pc, opcodes, (op1 & 0x70) >> 2 | BIT(op2, 4, 2), BIT(op1, 0) ? 2 : 0, indirect_src);
		stream << ", ";
		dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), 2, indirect_dest);
		break;

	case 0b0011:
		if (BIT(op1, 0))
		{
			util::stream_format(stream, "%-11s", "mov.l");
			dasm_operand(stream, pc, opcodes, (op1 & 0x70) >> 2 | BIT(op2, 4, 2), 2, indirect_src);
			stream << ", ";
			dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), 2, indirect_dest);
		}
		else if (!BIT(op1, 4))
		{
			util::stream_format(stream, "%-11s", util::string_format("index%c%s.%c ", BIT(op2, 5) ? 'w' : 'b', BIT(op1, 6) ? "s" : BIT(op1, 5) ? "d" : "", BIT(op1, 0) ? 'w' : 'b'));
			dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), op1 & 1, indirect_src);
		}
		else if (!BIT(op1, 5) || BIT(op2, 5))
		{
			util::stream_format(stream, "%-11s", util::string_format("indexl%s.%c ", BIT(op1, 5) ? "d" : BIT(op2, 5) ? "" : "s", BIT(op1, 0) ? 'w' : 'b'));
			dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), 2, indirect_src);
		}
		else if ((op1 & 0x0c) == 0 && (!BIT(op1, 1) || BIT(op2, 6)))
		{
			const bool signd = BIT(op2, 6);
			util::stream_format(stream, "%-11s", util::string_format("div%s.%c ", signd ? (BIT(op1, 1) ? "x" : "") : "u", BIT(op2, 4) ? 'w' : 'b'));
			if (BIT(op2, 4))
			{
				if (signd)
					format_imm_signed(stream, s16(opcodes.r16(pc)));
				else
					format_imm16(stream, opcodes.r16(pc));
				pc += 2;
			}
			else
			{
				if (signd)
					format_imm_signed(stream, s8(opcodes.r8(pc)));
				else
					format_imm8(stream, opcodes.r8(pc++));
			}
		}
		else if (op1 == 0xb2 && op2 == 0x03)
			stream << "wait";
		else if (op1 == 0xb6 && (op2 & 0xe0) == 0)
		{
			util::stream_format(stream, "%-11s", "add.l");
			if (BIT(op2, 4))
			{
				format_imm_signed(stream, s16(opcodes.r16(pc)));
				pc += 2;
			}
			else
				format_imm_signed(stream, s8(opcodes.r8(pc++)));
			stream << ", SP";
		}
		else if (op1 == 0xb6 && op2 == 0x53)
		{
			util::stream_format(stream, "%-11s", "push.l");
			format_imm32(stream, opcodes.r32(pc));
			pc += 4;
		}
		else if (op1 == 0xb6 && (op2 & 0xef) == 0xc3)
		{
			util::stream_format(stream, "%-11s", BIT(op2, 4) ? "stctx" : "ldctx");
			dasm_abs16(stream, pc, opcodes);
			stream << ", ";
			dasm_abs24(stream, pc, opcodes);
		}
		else if ((op1 & 0x78) == 0x30 && (op2 & 0xe0) == 0x80)
			util::stream_format(stream, "%s.%c", s_str_ops[BIT(op1, 1, 2)], BIT(op2, 4) ? 'W' : 'B');
		else if (op1 == 0xb8 && !BIT(op2, 5))
			util::stream_format(stream, "%s.%c", s_sstr_ops[BIT(op2, 6, 2)], BIT(op2, 4) ? 'W' : 'B');
		else
			stream << "und";
		break;

	case 0b0100: case 0b1100:
		util::stream_format(stream, "%-11s", util::string_format("mul%s.%c", BIT(op2, 3) ? "" : "u", BIT(op1, 0) ? 'w' : 'b'));
		dasm_operand(stream, pc, opcodes, (op1 & 0x70) >> 2 | BIT(op2, 4, 2), op1 & 1, indirect_src);
		stream << ", ";
		dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), op1 & 1, indirect_dest);
		break;

	case 0b0101: case 0b1101:
		util::stream_format(stream, "%-11s", util::string_format("%s.%c", BIT(op2, 3) ? "and" : "or", BIT(op1, 0) ? 'w' : 'b'));
		dasm_operand(stream, pc, opcodes, (op1 & 0x70) >> 2 | BIT(op2, 4, 2), op1 & 1, indirect_src);
		stream << ", ";
		dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), op1 & 1, indirect_dest);
		break;

	case 0b0110:
		util::stream_format(stream, "%-11s", util::string_format("cmp.%c", BIT(op1, 0) ? 'w' : 'b'));
		dasm_operand(stream, pc, opcodes, (op1 & 0x70) >> 2 | BIT(op2, 4, 2), op1 & 1, indirect_src);
		stream << ", ";
		dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), op1 & 1, indirect_dest);
		break;

	case 0b1000: case 0b1010:
		util::stream_format(stream, "%-11s", util::string_format("%s.%c", BIT(op2, 1) ? "sub" : "add", BIT(op1, 0) ? 'w' : 'b'));
		dasm_operand(stream, pc, opcodes, (op1 & 0x70) >> 2 | BIT(op2, 4, 2), op1 & 1, indirect_src);
		stream << ", ";
		dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), op1 & 1, indirect_dest);
		break;

	case 0b1001: case 0b1011:
		util::stream_format(stream, "%-11s", util::string_format("%s.%c", BIT(op2, 1) ? "mov" : "xor", BIT(op1, 0) ? 'w' : 'b'));
		dasm_operand(stream, pc, opcodes, (op1 & 0x70) >> 2 | BIT(op2, 4, 2), op1 & 1, indirect_src);
		stream << ", ";
		dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), op1 & 1, indirect_dest);
		break;

	case 0b1110:
		switch ((op1 & 0x70) >> 2 | BIT(op2, 4, 2))
		{
		case 0b00000: case 0b00001: case 0b00101:
			util::stream_format(stream, "%-11s", util::string_format("div%s.%c", BIT(op1, 4) ? "x" : BIT(op2, 4) ? "" : "u", BIT(op1, 0) ? 'w' : 'b'));
			dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), op1 & 1, indirect_src);
			break;

		case 0b00010: case 0b00011: case 0b00110:
			util::stream_format(stream, "%-11s", util::string_format("%s.%c", BIT(op1, 4) ? "cmp" : BIT(op2, 4) ? "sub" : "add", BIT(op1, 0) ? 'w' : 'b'));
			dasm_immediate_mode(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), op1 & 1, indirect_dest, 1, true);
			break;

		case 0b00100: case 0b00111:
			util::stream_format(stream, "%-11s", util::string_format("%s.%c", BIT(op2, 5) ? "tst" : "xor", BIT(op1, 0) ? 'w' : 'b'));
			dasm_immediate_mode(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), op1 & 1, indirect_dest, 1, false);
			break;

		case 0b01000: case 0b01100:
			util::stream_format(stream, "%-11s", util::string_format("%s.%c", BIT(op1, 4) ? "dec" : "inc", BIT(op1, 0) ? 'w' : 'b'));
			dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), op1 & 1, indirect_dest);
			break;

		case 0b01001: case 0b01101:
			util::stream_format(stream, "%-11s", util::string_format("%s.%c", BIT(op1, 4) ? "adcf" : "not", BIT(op1, 0) ? 'w' : 'b'));
			dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), op1 & 1, indirect_dest);
			break;

		case 0b01010: case 0b01110:
			util::stream_format(stream, "%-11s", util::string_format("ro%cc.%c", BIT(op1, 4) ? 'l' : 'r', BIT(op1, 0) ? 'w' : 'b'));
			dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), op1 & 1, indirect_dest);
			break;

		case 0b01011: case 0b01111:
			util::stream_format(stream, "%-11sR1H, ", util::string_format("sh%c.%c", BIT(op1, 4) ? 'a' : 'l', BIT(op1, 0) ? 'w' : 'b'));
			dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), op1 & 1, indirect_dest);
			break;

		case 0b10000:
			util::stream_format(stream, "%-11s", util::string_format("push.%c", BIT(op1, 0) ? 'w' : 'b'));
			dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), op1 & 1, indirect_src);
			break;

		case 0b10001:
			util::stream_format(stream, "%-11s", util::string_format("exts.%c", BIT(op1, 0) ? 'w' : 'b'));
			dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), op1 & 1, indirect_dest);
			break;

		case 0b10010:
			util::stream_format(stream, "%-11s", util::string_format("bitindex.%c", BIT(op1, 0) ? 'w' : 'b'));
			dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), op1 & 1, indirect_dest);
			break;

		case 0b10011:
			if (BIT(op1, 0))
			{
				util::stream_format(stream, "%-11s", "mulex");
				dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), 1, indirect_dest);
			}
			else
				stream << "und";
			break;
		}
		break;

	case 0b1111:
		if ((op1 & 0x60) == 0)
		{
			const u8 imm1111_op = (op1 & 0x10) >> 2 | BIT(op2, 4, 2);
			util::stream_format(stream, "%-11s", util::string_format("%s.%c", s_imm1111_ops[imm1111_op], BIT(op1, 0) ? 'w' : 'b'));
			dasm_immediate_mode(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), op1 & 1, indirect_dest, BIT(op1, 4) && (op2 & 0x30) == 0x30 ? 2 : 1, imm1111_op == 0b0001);
		}
		else if ((op1 & 0x60) == 0x20 && (op2 & 0x30) == 0)
		{
			util::stream_format(stream, "%-11s", util::string_format("mov.%c ", BIT(op1, 0) ? 'w' : 'b'));
			if (!BIT(op1, 4))
			{
				dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), op1 & 1, indirect_src);
				stream << ", ";
			}
			format_relative(stream, "SP", s8(opcodes.r8(pc++)));
			if (BIT(op1, 4))
			{
				stream << ", ";
				dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), op1 & 1, indirect_dest);
			}
		}
		else if ((op1 & 0x70) == 0x20)
		{
			if ((op2 & 0x30) == 0x30)
				util::stream_format(stream, "%-11sR1H, ", util::string_format("rot.%c", BIT(op1, 0) ? 'w' : 'b'));
			else
				util::stream_format(stream, "%-11s", util::string_format("%s.%c", BIT(op2, 5) ? "neg" : "abs", BIT(op1, 0) ? 'w' : 'b'));
			dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), op1 & 1, indirect_dest);
		}
		else if ((op1 & 0x70) == 0x30 && (op2 & 0x30) == 0x20)
		{
			util::stream_format(stream, "%-11s", util::string_format("pop.%c ", BIT(op1, 0) ? 'w' : 'b'));
			dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), op1 & 1, indirect_dest);
		}
		else if ((op1 & 0x71) == 0x41 && !BIT(op2, 5))
		{
			if (BIT(op2, 4))
			{
				util::stream_format(stream, "%-11s", "jsri.w");
				flags |= STEP_OVER;
			}
			else
				util::stream_format(stream, "%-11s", "jmpi.w");
			dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), op1 & 1, indirect_src);
		}
		else
			stream << "und";
		break;

	default:
		stream << "und";
		break;
	}
}

void m32c_disassembler::dasm_1101(std::ostream &stream, offs_t &pc, const m32c_disassembler::data_buffer &opcodes, u8 op1, bool indirect_src, bool indirect_dest) const
{
	u8 op2 = opcodes.r8(pc++);
	if ((op2 & 0x38) != 0x08 && !BIT(op1, 0))
	{
		if ((op2 & 0x38) == 0x10)
		{
			// CND byte comes after the destination
			u8 cnd;
			switch ((op1 & 0x0e) << 1 | BIT(op2, 6, 2))
			{
			case 0b00100: case 0b00101: case 0b00110: case 0b00111:
				cnd = opcodes.r8(pc + 1);
				break;

			case 0b01000: case 0b01001: case 0b01111:
				cnd = opcodes.r8(pc + 2);
				break;

			case 0b01010: case 0b01011: case 0b01110:
				cnd = opcodes.r8(pc + 3);
				break;

			default:
				cnd = opcodes.r8(pc);
				break;
			}
			if ((cnd & 0xf7) < 0x07)
			{
				util::stream_format(stream, "%-11s%d, ", util::string_format("bm%s", s_cnds[cnd]), op2 & 0x07);
				dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), op1 & 1, indirect_dest);
			}
			else
			{
				util::stream_format(stream, "%-11s%d, ", "bmcnd", op2 & 0x07);
				dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), op1 & 1, indirect_dest);
				stream << ", ";
				format_imm8(stream, cnd);
			}
			++pc; // consume CND
		}
		else
		{
			util::stream_format(stream, "%-11s%d, ", s_btst_ops[BIT(op2, 3, 3)], op2 & 0x07);
			dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), op1 & 1, (op2 & 0x38) == 0 ? indirect_src : indirect_dest);
		}
	}
	else if ((op2 & 0x38) == 0x08 && (op2 & 0x06) != 0x06)
	{
		util::stream_format(stream, "%-11s", util::string_format("xchg.%c", BIT(op1, 0) ? 'w' : 'b'));
		if (BIT(op2, 1))
			util::stream_format(stream, "%s, ", s_aregs[op2 & 1]);
		else if (BIT(op1, 0))
			util::stream_format(stream, "R%d, ", bitswap<2>(op2, 2, 0));
		else
			util::stream_format(stream, "R%d%c, ", op2 & 1, BIT(op2, 2) ? 'H' : 'L');
		dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), op1 & 1, indirect_dest);
	}
	else if ((op2 & 0x38) == 0x10)
	{
		assert(BIT(op1, 0));
		util::stream_format(stream, "%-11s%s, ", "stc", s_sregs_24bit[op2 & 0x07]);
		dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), 2, indirect_dest);
	}
	else if ((op2 & 0x3c) == 0x18 && !BIT(op1, 3))
	{
		util::stream_format(stream, "%-11s", "mova");
		dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), op1 & 1, indirect_src);
		if (BIT(op2, 1))
			util::stream_format(stream, ", %s", s_aregs[op2 & 1]);
		else
			util::stream_format(stream, ", R%dR%d", (op2 & 1) + 2, op2 & 1);
	}
	else if ((op1 & 0xfd) == 0xd1 && (op2 & 0x78) == 0x28)
		util::stream_format(stream, "%-11s%s", BIT(op1, 1) ? "popc" : "pushc", BIT(op2, 7) ? s_sregs_16bit[op2 & 0x07] : s_sregs_24bit[op2 & 0x07]);
	else if (op1 == 0xd5 && (op2 & 0x38) == 0x28)
	{
		if (op2 >= 0xc0)
			util::stream_format(stream, "%-11s#%d", "ldipl", op2 & 0x07);
		else
		{
			util::stream_format(stream, "%-11s", "ldc");
			if (BIT(op2, 7))
			{
				format_imm16(stream, opcodes.r16(pc));
				util::stream_format(stream, ", %s", s_sregs_16bit[op2 & 0x07]);
				pc += 2;
			}
			else
			{
				format_imm24(stream, opcodes.r16(pc) | u32(opcodes.r8(pc + 2)) << 16);
				util::stream_format(stream, ", %s", BIT(op2, 6) ? s_sregs_dma[op2 & 0x07] : s_sregs_24bit[op2 & 0x07]);
				pc += 3;
			}
		}
	}
	else if ((op1 & 0x0c) == 0 && (op2 & 0xf8) == 0xe8)
		util::stream_format(stream, "%-11s%c", BIT(op1, 1) ? "fclr" : "fset", "CDZSBOIU"[op2 & 0x07]);
	else if ((op2 & 0x30) == 0x30)
	{
		util::stream_format(stream, "%-11s", util::string_format("sc%s", s_cnds[op2 & 0x0f]));
		dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), 1, indirect_dest);
	}
	else if (op1 == 0xd9 && (op2 & 0xb8) == 0x28 && (op2 & 0x07) != 0x07)
		util::stream_format(stream, "%-11sC", util::string_format("bm%s", s_cnds[bitswap<4>(op2, 6, 2, 1, 0)]));
	else
		stream << "und";
}

void m32c_disassembler::dasm_111x(std::ostream &stream, offs_t &pc, offs_t &flags, const m32c_disassembler::data_buffer &opcodes, u8 op1, bool indirect_dest) const
{
	u8 op2 = opcodes.r8(pc++);
	if ((op2 & 0x30) == 0 || ((op2 & 0x30) == 0x20 && !BIT(op1, 4)))
	{
		util::stream_format(stream, "%-11s#", util::string_format("%s.%c", BIT(op2, 5) ? "rot" : BIT(op1, 4) ? "sha" : "shl", BIT(op1, 0) ? 'w' : 'b'));
		// Shift count is sign-magnitude coded
		if (BIT(op2, 3))
			stream << '-';
		util::stream_format(stream, "%d, ", 1 + (op2 & 0x07));
		dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), op1 & 1, indirect_dest);
	}
	else if ((op2 & 0x30) == 0x10 && BIT(op1, 4))
	{
		if (BIT(op2, 3))
			util::stream_format(stream, "%-11s#%d, ", util::string_format("sbjnz.%c", BIT(op1, 0) ? 'w' : 'b'), 8 - (op2 & 0x07));
		else
			util::stream_format(stream, "%-11s#%d, ", util::string_format("adjnz.%c", BIT(op1, 0) ? 'w' : 'b'), op2 & 0x07);
		dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), op1 & 1, indirect_dest);
		stream << ", ";
		format_label(stream, pc + s8(opcodes.r8(pc)));
		flags |= STEP_COND;
		++pc;
	}
	else if ((op2 & 0x30) == 0x30 && BIT(op1, 4))
	{
		if ((op1 & 0x11) == 0x11)
			stream << "und";
		else
		{
			util::stream_format(stream, "%-11s#%d, ", "add.l", int(op2 & 0x0f) - (BIT(op2, 3) ? 16 : 0));
			dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), 2, indirect_dest);
		}
	}
	else
	{
		util::stream_format(stream, "%-11s", util::string_format("%s.%c", BIT(op2, 5) ? (BIT(op2, 4) ? "add" : "mov") : "cmp", BIT(op1, 0) ? 'w' : 'b'));
		util::stream_format(stream, "#%d, ", int(op2 & 0x0f) - (BIT(op2, 3) ? 16 : 0));
		dasm_operand(stream, pc, opcodes, (op1 & 0x0e) << 1 | BIT(op2, 6, 2), op1 & 1, indirect_dest);
	}
}

offs_t m32c_disassembler::disassemble(std::ostream &stream, offs_t pc, const m32c_disassembler::data_buffer &opcodes, const m32c_disassembler::data_buffer &params)
{
	offs_t pc0 = pc;
	offs_t flags = SUPPORTED;

	u8 op1 = opcodes.r8(pc++);
	bool indirect_src = false;
	bool indirect_dest = false;
	if ((op1 & 0xb7) == 0x01 && op1 != 0x01)
	{
		indirect_src = BIT(op1, 6);
		indirect_dest = BIT(op1, 3);
		op1 = opcodes.r8(pc++);
	}
	switch (op1)
	{
	case 0x00: case 0x08:
		stream << "brk";
		if (op1 == 0x08)
			stream << '2';
		flags |= STEP_OVER;
		break;

	case 0x01:
		dasm_00000001(stream, pc, opcodes, indirect_src, indirect_dest);
		break;

	case 0x02: case 0x03:
	case 0x10: case 0x11: case 0x12: case 0x13:
	case 0x20: case 0x21: case 0x22: case 0x23:
	case 0x30: case 0x31: case 0x32: case 0x33:
		util::stream_format(stream, "%-11s", util::string_format("mov.%c", BIT(op1, 0) ? 'w' : 'b'));
		if (BIT(op1, 1))
			stream << "#0";
		else
		{
			stream << "R0";
			if (!BIT(op1, 0))
				stream << 'L';
		}
		stream << ", ";
		dasm_operand(stream, pc, opcodes, s_short_code[BIT(op1, 4, 2)], op1 & 1, indirect_dest);
		break;

	case 0x04: case 0x05: case 0x0c: case 0x0d:
	case 0x14: case 0x15: case 0x1c: case 0x1d:
	case 0x24: case 0x25: case 0x2c: case 0x2d:
	case 0x34: case 0x35: case 0x3c: case 0x3d:
		util::stream_format(stream, "%-11s", util::string_format("%s.%c", BIT(op1, 3) ? "tst" : "mov", BIT(op1, 0) ? 'w' : 'b'));
		dasm_immediate_mode(stream, pc, opcodes, s_short_code[BIT(op1, 4, 2)], op1 & 1, indirect_dest, 1, false);
		break;

	case 0x06: case 0x07: case 0x0e: case 0x0f:
	case 0x16: case 0x17: case 0x1e: case 0x1f:
	case 0x26: case 0x27: case 0x2e: case 0x2f:
	case 0x36: case 0x37: case 0x3e: case 0x3f:
	case 0x46: case 0x47:
	case 0x56: case 0x57:
	case 0x66: case 0x67:
	case 0x76: case 0x77:
		util::stream_format(stream, "%-11s", util::string_format("%s.%c", BIT(op1, 6) ? "cmp" : BIT(op1, 3) ? "sub" : "add", BIT(op1, 0) ? 'w' : 'b'));
		dasm_immediate_mode(stream, pc, opcodes, s_short_code[BIT(op1, 4, 2)], op1 & 1, indirect_dest, 1, true);
		break;

	case 0x0a: case 0x0b:
	case 0x1a: case 0x1b:
	case 0x2a: case 0x2b:
	case 0x3a: case 0x3b:
		util::stream_format(stream, "%-11s%d, ", "btst", bitswap<3>(op1, 5, 4, 0));
		if (indirect_src)
		{
			stream << '[';
			dasm_abs16(stream, pc, opcodes);
			stream << ']';
		}
		else
			dasm_abs16(stream, pc, opcodes);
		break;

	case 0x18: case 0x19:
	case 0x28: case 0x29:
	case 0x38: case 0x39:
	case 0x50: case 0x51:
	case 0x60: case 0x61:
	case 0x70: case 0x71:
		util::stream_format(stream, "%-11s", util::string_format("%s.%c", BIT(op1, 6) ? "cmp" : "mov", BIT(op1, 0) ? 'w' : 'b'));
		dasm_operand(stream, pc, opcodes, s_short_code[BIT(op1, 4, 2)], op1 & 1, indirect_src);
		stream << ", R0";
		if (!BIT(op1, 0))
			stream << 'L';
		break;

	case 0x42: case 0x43:
	case 0x52: case 0x53:
	case 0x62: case 0x63:
	case 0x72: case 0x73:
		util::stream_format(stream, "%-11s#%d, SP", "add.l", 1 + bitswap<3>(op1, 5, 4, 0));
		break;

	case 0x44: case 0x45: case 0x4c: case 0x4d:
	case 0x54: case 0x55: case 0x5c: case 0x5d:
	case 0x64: case 0x65: case 0x6c: case 0x6d:
	case 0x74: case 0x75: case 0x7c: case 0x7d:
		util::stream_format(stream, "%-11s", util::string_format("%s.%c", BIT(op1, 3) ? "and" : "or", BIT(op1, 0) ? 'w' : 'b'));
		dasm_immediate_mode(stream, pc, opcodes, s_short_code[BIT(op1, 4, 2)], op1 & 1, indirect_dest, 1, false);
		break;

	case 0x4a: case 0x4b:
	case 0x5a: case 0x5b:
	case 0x6a: case 0x6b:
	case 0x7a: case 0x7b:
		util::stream_format(stream, "%-11s", "jmp.s");
		format_label(stream, pc + 1 + bitswap<3>(op1, 5, 4, 0));
		break;

	case 0x4e: case 0x4f:
	case 0x5e: case 0x5f:
	case 0x6e: case 0x6f:
	case 0x7e: case 0x7f:
		util::stream_format(stream, "%-11s", util::string_format("mov.%c", BIT(op1, 0) ? 'w' : 'b'));
		dasm_operand(stream, pc, opcodes, s_short_code[BIT(op1, 4, 2)], op1 & 1, indirect_src);
		stream << ", R1";
		if (!BIT(op1, 0))
			stream << 'L';
		break;

	case 0x58: case 0x59:
	case 0x68: case 0x69:
	case 0x78: case 0x79:
		util::stream_format(stream, "%-11s", "mov.l");
		dasm_operand(stream, pc, opcodes, s_short_code[BIT(op1, 4, 2)], 2, indirect_src);
		util::stream_format(stream, ", %s", s_aregs[op1 & 1]);
		break;

	case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87: case 0x88: case 0x89:
	case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97: case 0x98: case 0x99:
	case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7: case 0xa8: case 0xa9:
	case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7: case 0xb8: case 0xb9:
	case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7: case 0xc8: case 0xc9:
		dasm_general(stream, pc, flags, opcodes, op1, indirect_src, indirect_dest);
		break;

	case 0x8a: case 0x8b:
	case 0x9a: case 0x9b:
	case 0xaa: case 0xab:
	case 0xba:
	case 0xca: case 0xcb:
	case 0xda: case 0xdb:
	case 0xea: case 0xeb:
	case 0xfa:
		util::stream_format(stream, "%-11s", util::string_format("j%s", s_cnds[bitswap<4>(op1, 6, 5, 4, 0)]));
		format_label(stream, pc + s8(opcodes.r8(pc)));
		++pc;
		flags |= STEP_COND;
		break;

	case 0x8c: case 0x8d:
	case 0xac: case 0xad:
		util::stream_format(stream, "%-11s#%d, %s", "add.l", BIT(op1, 5) ? 2 : 1, s_aregs[op1 & 1]);
		break;

	case 0x8e:
	{
		util::stream_format(stream, "%-11s", "popm");
		u8 dest = opcodes.r8(pc++);
		if (dest == 0)
			stream << "null";
		else
		{
			for (int i = 0; i < 8 && dest != 0; ++i, dest >>= 1)
			{
				if (BIT(dest, 0))
				{
					if (i < 4)
						util::stream_format(stream, "R%d", i);
					else
						stream << s_aregs[i & 3];
					if (dest != 0x01)
						stream << ", ";
				}
			}
		}
		break;
	}

	case 0x8f:
	{
		util::stream_format(stream, "%-11s", "pushm");
		u8 src = opcodes.r8(pc++);
		if (src == 0)
			stream << "null";
		else
		{
			for (int i = 7; i >= 0 && src != 0; --i, src >>= 1)
			{
				if (BIT(src, 0))
				{
					if (i < 4)
						util::stream_format(stream, "R%d", i);
					else
						stream << s_aregs[i & 3];
					if (src != 0x01)
						stream << ", ";
				}
			}
		}
		break;
	}

	case 0x9c: case 0x9d:
	case 0xbc: case 0xbd:
		if (BIT(op1, 5))
		{
			util::stream_format(stream, "%-11s", "mov.l");
			format_imm24(stream, opcodes.r16(pc) | u32(opcodes.r8(pc + 2)) << 16);
			pc += 3;
		}
		else
		{
			util::stream_format(stream, "%-11s", "mov.w");
			format_imm16(stream, opcodes.r16(pc));
			pc += 2;
		}
		util::stream_format(stream, ", %s", s_aregs[op1 & 1]);
		break;

	case 0x9e: case 0x9f:
		if (BIT(op1, 0))
			stream << 'f';
		stream << "reit";
		flags |= STEP_OUT;
		break;

	case 0xae:
		util::stream_format(stream, "%-11s", "push.b");
		format_imm8(stream, opcodes.r8(pc++));
		break;

	case 0xaf:
		util::stream_format(stream, "%-11s", "push.w");
		format_imm16(stream, opcodes.r16(pc));
		pc += 2;
		break;

	case 0xbb:
		util::stream_format(stream, "%-11s", "jmp.b");
		format_label(stream, pc + s8(opcodes.r8(pc)));
		++pc;
		break;

	case 0xbe:
	{
		u8 imm6 = opcodes.r8(pc++);
		util::stream_format(stream, "%-11s", "int");
		if ((imm6 & 0x03) != 0)
			stream << "<invalid>";
		else
			format_imm8(stream, imm6 >> 2);
		flags |= STEP_OVER;
		break;
	}

	case 0xbf:
		stream << "into";
		break;

	case 0xcc:
		util::stream_format(stream, "%-11s", "jmp.a");
		dasm_abs24(stream, pc, opcodes);
		break;

	case 0xcd:
		util::stream_format(stream, "%-11s", "jsr.a");
		dasm_abs24(stream, pc, opcodes);
		flags |= STEP_OVER;
		break;

	case 0xce: case 0xcf:
		if (BIT(op1, 0))
		{
			util::stream_format(stream, "%-11s", "jsr.w");
			flags |= STEP_OVER;
		}
		else
			util::stream_format(stream, "%-11s", "jmp.w");
		format_label(stream, pc + s16(opcodes.r16(pc)));
		pc += 2;
		break;

	case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7: case 0xd8: case 0xd9:
		dasm_1101(stream, pc, opcodes, op1, indirect_src, indirect_dest);
		break;

	case 0xdc:
		util::stream_format(stream, "%-11s", "jmps");
		format_imm8(stream, opcodes.r8(pc++));
		break;

	case 0xdd:
		util::stream_format(stream, "%-11s", "jsrs");
		format_imm8(stream, opcodes.r8(pc++));
		flags |= STEP_OVER;
		break;

	case 0xde:
		stream << "nop";
		break;

	case 0xdf:
		stream << "rts";
		flags |= STEP_OVER;
		break;

	case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7: case 0xe8: case 0xe9:
	case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7: case 0xf8: case 0xf9:
		dasm_111x(stream, pc, flags, opcodes, op1, indirect_dest);
		break;

	case 0xec:
		util::stream_format(stream, "%-11s", "enter");
		format_imm_signed(stream, opcodes.r8(pc++)); // actually unsigned
		break;

	case 0xfc:
		util::stream_format(stream, "%-11s", "exitd");
		flags |= STEP_OUT;
		break;

	case 0xff:
	default:
		stream << "und";
		break;
	}

	return (pc - pc0) | flags;
}

