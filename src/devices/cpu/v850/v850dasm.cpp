// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    NEC/Renesas V850 disassembler

    The "V850E2" disassembler includes some instructions which are actually
    available only on V850E2S and V850E2M. More opcodes probably cause
    reserved instruction exceptions (RIE) than are decoded for even these
    advanced variants.

***************************************************************************/

#include "emu.h"
#include "v850dasm.h"

v850_disassembler::v850_disassembler()
	: util::disasm_interface()
{
}

v850es_disassembler::v850es_disassembler()
	: v850_disassembler()
{
}

v850e2_disassembler::v850e2_disassembler()
	: v850es_disassembler()
{
}

u32 v850_disassembler::opcode_alignment() const
{
	return 2;
}

const std::string_view v850_disassembler::s_regs[32] = {
	"zero", "r1", "r2", "sp", "gp", "tp", "r6", "r7",
	"r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
	"r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23",
	"r24", "r25", "r26", "r27", "r28", "r29", "ep", "lp"
};

const std::string_view v850_disassembler::s_dsp_ops[4] = {
	"satsubr", "satsub", "satadd", "mulh"
};

const std::string_view v850es_disassembler::s_szx_ops[4] = {
	"zxb", "sxb", "zxh", "sxh"
};

const std::string_view v850_disassembler::s_rr_ops[8] = {
	"or", "xor", "and", "tst", "subr", "sub", "add", "cmp"
};

const std::string_view v850_disassembler::s_imm5_ops[8] = {
	"mov", "satadd", "add", "cmp", "shr", "sar", "shl", "mulh"
};

const std::string_view v850_disassembler::s_bit_ops[4] = {
	"set1", "not1", "clr1", "tst1"
};

const std::string_view v850_disassembler::s_conds[16] = {
	"V", "C/L", "Z", "NH", "S/N", "T", "LT", "LE",
	"NV", "NC/NL", "NZ", "H", "NS/P", "SA", "GE", "GT"
};

const std::string_view v850_disassembler::s_bconds[16] = {
	"bv", "bc", "be", "bnh", "bn", "br", "blt", "ble",
	"bnv", "bnc", "bne", "bh", "bp", "bsa", "bge", "bgt"
};

void v850_disassembler::format_system_reg(std::ostream &stream, u8 sreg)
{
	switch (sreg)
	{
	case 0:
		stream << "EIPC";
		break;

	case 1:
		stream << "EIPSW";
		break;

	case 2:
		stream << "FEPC";
		break;

	case 3:
		stream << "FEPSW";
		break;

	case 4:
		stream << "ECR";
		break;

	case 5:
		stream << "PSW";
		break;

	default:
		util::stream_format(stream, "sr%d", sreg);
		break;
	}
}

void v850es_disassembler::format_system_reg(std::ostream &stream, u8 sreg)
{
	switch (sreg)
	{
	case 16:
		stream << "CTPC";
		break;

	case 17:
		stream << "CTPSW";
		break;

	case 20:
		stream << "CTBP";
		break;

	case 21:
		stream << "DIR";
		break;

	default:
		v850_disassembler::format_system_reg(stream, sreg);
		break;
	}
}

void v850e2_disassembler::format_system_reg(std::ostream &stream, u8 sreg)
{
	switch (sreg)
	{
	case 28:
		// V850E2S, V850E2M only
		stream << "EIWR";
		break;

	case 29:
		// V850E2S, V850E2M only
		stream << "FEWR";
		break;

	case 30:
		// V850E2S, V850E2M only
		stream << "DBWR";
		break;

	case 31:
		// V850E2S, V850E2M only
		stream << "BSEL";
		break;

	default:
		// all others are banked on V850E2S/E2M
		util::stream_format(stream, "sr%d", sreg);
		break;
	}
}

void v850_disassembler::format_imm5_signed(std::ostream &stream, u8 imm5)
{
	if (imm5 >= 0x20)
	{
		stream << '-';
		imm5 = 0x20 - imm5;
	}
	if (imm5 > 9)
		stream << "0x";
	util::stream_format(stream, "%X", imm5);
}

void v850_disassembler::format_imm16_signed(std::ostream &stream, u16 imm16)
{
	if (s16(imm16) < 0)
	{
		stream << '-';
		imm16 = -imm16;
	}
	if (imm16 > 9)
		stream << "0x";
	util::stream_format(stream, "%X", imm16);
}

void v850_disassembler::format_disp16_reg(std::ostream &stream, u16 disp16, u8 reg)
{
	if (reg == 0 && disp16 >= 0x8000)
		stream << "0xFFFF";
	else
	{
		if (s16(disp16) < 0)
		{
			stream << '-';
			disp16 = -disp16;
		}
		if (disp16 > 9)
			stream << "0x";
	}
	util::stream_format(stream, "%X[%s]", disp16, s_regs[reg]);
}

void v850_disassembler::format_disp8_ep(std::ostream &stream, u8 disp8)
{
	format_disp16_reg(stream, disp8, 30);
}

void v850e2_disassembler::format_disp23_reg(std::ostream &stream, u32 disp23, u8 reg)
{
	if (disp23 >= 0x400000)
	{
		if (reg == 0)
			disp23 |= 0xffc00000;
		else
		{
			stream << '-';
			disp23 = 0x800000 - disp23;
		}
	}
	util::stream_format(stream, "0x%X[%s]", disp23, s_regs[reg]);
}

void v850e2_disassembler::format_disp32_reg(std::ostream &stream, u32 disp32, u8 reg)
{
	util::stream_format(stream, "0x%08X[%s]", disp32, s_regs[reg]);
}

void v850es_disassembler::format_list12(std::ostream &stream, u16 list12)
{
	stream << '{';
	for (u8 reg = 20; list12 != 0; reg++)
	{
		if (BIT(list12, 11))
		{
			stream << s_regs[reg];
			list12 &= 0x7ff;
			if (list12 != 0)
				stream << ", ";
		}
		list12 <<= 1;
	}
	stream << '}';
}

offs_t v850_disassembler::dasm_000000(std::ostream &stream, u16 opcode, offs_t pc, const v850_disassembler::data_buffer &opcodes)
{
	if (opcode == 0)
		stream << "nop";
	else
		util::stream_format(stream, "%-8s%s, %s", "mov", s_regs[BIT(opcode, 0, 5)], s_regs[BIT(opcode, 11, 5)]);
	return 2 | SUPPORTED;
}

offs_t v850e2_disassembler::dasm_000000(std::ostream &stream, u16 opcode, offs_t pc, const v850e2_disassembler::data_buffer &opcodes)
{
	if ((opcode & 0xf800) != 0)
		return v850es_disassembler::dasm_000000(stream, opcode, pc, opcodes);
	else switch (BIT(opcode, 0, 5))
	{
	case 0b00000:
		stream << "nop";
		break;

	case 0b11100:
		// V850E2S, V850E2M only
		stream << "syncm";
		break;

	case 0b11101:
		// V850E2S, V850E2M only
		stream << "synce";
		break;

	case 0b11111:
		// V850E2S, V850E2M only
		stream << "syncp";
		break;

	default:
		stream << "rie";
		break;
	}
	return 2 | SUPPORTED;
}

offs_t v850_disassembler::dasm_000010(std::ostream &stream, u16 opcode, offs_t pc, const v850_disassembler::data_buffer &opcodes)
{
	util::stream_format(stream, "%-8s%s, %s", "divh", s_regs[BIT(opcode, 0, 5)], s_regs[BIT(opcode, 11, 5)]);
	return 2 | SUPPORTED;
}

offs_t v850es_disassembler::dasm_000010(std::ostream &stream, u16 opcode, offs_t pc, const v850es_disassembler::data_buffer &opcodes)
{
	if (opcode == 0xf840)
	{
		stream << "dbtrap";
		return 2 | STEP_OVER | SUPPORTED;
	}
	else if ((opcode & 0xf800) == 0)
	{
		util::stream_format(stream, "%-8s%s", "switch", s_regs[BIT(opcode, 0, 5)]);
		return 2 | SUPPORTED;
	}
	else
		return v850_disassembler::dasm_000010(stream, opcode, pc, opcodes);
}

offs_t v850e2_disassembler::dasm_000010(std::ostream &stream, u16 opcode, offs_t pc, const v850e2_disassembler::data_buffer &opcodes)
{
	if ((opcode & 0x001f) == 0)
	{
		if ((opcode & 0xe800) != 0 && !BIT(opcode, 15))
		{
			// V850E2S, V850E2M only
			util::stream_format(stream, "%-8s0x%X", "fetrap", 0x30 + BIT(opcode, 11, 4));
			return 2 | STEP_OVER | SUPPORTED;
		}
		else
		{
			stream << "rie";
			return 2 | SUPPORTED;
		}
	}
	else
		return v850es_disassembler::dasm_000010(stream, opcode, pc, opcodes);
}

offs_t v850_disassembler::dasm_000011(std::ostream &stream, u16 opcode, offs_t pc, const v850_disassembler::data_buffer &opcodes)
{
	util::stream_format(stream, "%-8s[%s]", "jmp", s_regs[BIT(opcode, 0, 5)]);
	return 2 | ((opcode & 0x001f) == 0x001f ? STEP_OUT : 0) | SUPPORTED;
}

offs_t v850es_disassembler::dasm_000011(std::ostream &stream, u16 opcode, offs_t pc, const v850_disassembler::data_buffer &opcodes)
{
	if ((opcode & 0xf800) != 0)
	{
		u8 disp4 = BIT(opcode, 0, 4);
		util::stream_format(stream, "%-8s", BIT(opcode, 4) ? "sld.hu" : "sld.bu");
		format_disp8_ep(stream, disp4);
		util::stream_format(stream, ", %s", s_regs[BIT(opcode, 0, 5)]);
		return 2 | SUPPORTED;
	}
	else
		return v850_disassembler::dasm_000011(stream, opcode, pc, opcodes);
}

offs_t v850_disassembler::dasm_0001(std::ostream &stream, u16 opcode, offs_t pc, const v850_disassembler::data_buffer &opcodes)
{
	util::stream_format(stream, "%-8s%s, %s", s_dsp_ops[BIT(opcode, 5, 2)], s_regs[BIT(opcode, 0, 5)], s_regs[BIT(opcode, 11, 5)]);
	return 2 | SUPPORTED;
}

offs_t v850es_disassembler::dasm_0001(std::ostream &stream, u16 opcode, offs_t pc, const v850_disassembler::data_buffer &opcodes)
{
	if ((opcode & 0xf800) == 0)
	{
		util::stream_format(stream, "%-8s%s", s_szx_ops[BIT(opcode, 5, 2)], s_regs[BIT(opcode, 0, 5)]);
		return 2 | SUPPORTED;
	}
	else
		return v850_disassembler::dasm_0001(stream, opcode, pc, opcodes);
}

offs_t v850_disassembler::dasm_010x(std::ostream &stream, u16 opcode, offs_t pc, const v850_disassembler::data_buffer &opcodes)
{
	u8 imm5 = BIT(opcode, 0, 5);
	util::stream_format(stream, "%-8s", s_imm5_ops[BIT(opcode, 5, 3)]);
	if (BIT(opcode, 7) && (opcode & 0x00c0) != 0x00c0)
		util::stream_format(stream, "%d", imm5);
	else
		format_imm5_signed(stream, imm5);
	util::stream_format(stream, ", %s", s_regs[BIT(opcode, 11, 5)]);
	return 2 | SUPPORTED;
}

offs_t v850es_disassembler::dasm_010x(std::ostream &stream, u16 opcode, offs_t pc, const v850es_disassembler::data_buffer &opcodes)
{
	if ((opcode & 0xf8c0) == 0)
	{
		util::stream_format(stream, "%-8s0x%02X", "callt", BIT(opcode, 0, 6));
		return 2 | STEP_OVER | SUPPORTED;
	}
	else
		return v850_disassembler::dasm_010x(stream, opcode, pc, opcodes);
}

offs_t v850e2_disassembler::dasm_010x(std::ostream &stream, u16 opcode, offs_t pc, const v850e2_disassembler::data_buffer &opcodes)
{
	if ((opcode & 0xf8e0) == 0x00e0)
	{
		s32 disp32 = opcodes.r32(pc + 2);
		if ((opcode & 0x001f) == 0)
		{
			util::stream_format(stream, "%-8s0x%08X", "jr", pc + disp32);
			return 6 | SUPPORTED;
		}
		else
		{
			util::stream_format(stream, "%-8s0x%08X, %s", "jarl", pc + disp32, s_regs[BIT(opcode, 0, 5)]);
			return 6 | STEP_OVER | SUPPORTED;
		}
	}
	else
		return v850es_disassembler::dasm_010x(stream, opcode, pc, opcodes);
}

offs_t v850_disassembler::dasm_1100(std::ostream &stream, u16 opcode, offs_t pc, const v850_disassembler::data_buffer &opcodes)
{
	u16 imm16 = opcodes.r16(pc + 2);
	if ((opcode & 0x0060) == 0x0040)
		util::stream_format(stream, "%-8s0x%04X", "movhi", imm16);
	else
	{
		util::stream_format(stream, "%-8s", BIT(opcode, 6) ? "satsubi" : BIT(opcode, 5) ? "movea" : "addi");
		format_imm16_signed(stream, imm16);
	}
	util::stream_format(stream, ", %s, %s", s_regs[BIT(opcode, 0, 5)], s_regs[BIT(opcode, 11, 5)]);
	return 4 | SUPPORTED;
}

offs_t v850es_disassembler::dasm_1100(std::ostream &stream, u16 opcode, offs_t pc, const v850es_disassembler::data_buffer &opcodes)
{
	if ((opcode & 0xf840) == 0x0040)
	{
		u32 opcode2 = opcodes.r16(pc + 2);
		u8 imm5 = BIT(opcode, 1, 5);
		util::stream_format(stream, "%-8s%s%X, ", "dispose", imm5 > 9 ? "0x" : "", imm5);
		format_list12(stream, (opcode2 & 0x0f00) | (opcode2 & 0xf000) >> 8 | (opcode2 & 0x00c0) >> 4 | (opcode & 0x0001) << 1 | (opcode2 & 0x0020) >> 5);
		if ((opcode2 & 0x001f) != 0)
		{
			util::stream_format(stream, ", [%s]", s_regs[BIT(opcode2, 0, 5)]);
			return 4 | STEP_OUT | SUPPORTED;
		}
		else
			return 4 | SUPPORTED;
	}
	else if ((opcode & 0xf860) == 0x0020)
	{
		u32 imm32 = opcodes.r32(pc + 2);
		util::stream_format(stream, "%-8s0x%08X, %s", "mov", imm32, s_regs[BIT(opcode, 0, 5)]);
		return 6 | SUPPORTED;
	}
	else
		return v850_disassembler::dasm_1100(stream, opcode, pc, opcodes);
}

offs_t v850_disassembler::dasm_1101(std::ostream &stream, u16 opcode, offs_t pc, const v850_disassembler::data_buffer &opcodes)
{
	u16 imm16 = opcodes.r16(pc + 2);
	if ((opcode & 0x0060) == 0x0060)
	{
		util::stream_format(stream, "%-8s", "mulhi");
		format_imm16_signed(stream, imm16);
		util::stream_format(stream, ", %s, %s", s_regs[BIT(opcode, 0, 5)], s_regs[BIT(opcode, 11, 5)]);
	}
	else
		util::stream_format(stream, "%-8s0x%04X, %s, %s", BIT(opcode, 6) ? "andi" : BIT(opcode, 5) ? "xori" : "ori",
							imm16, s_regs[BIT(opcode, 0, 5)], s_regs[BIT(opcode, 11, 5)]);
	return 4 | SUPPORTED;
}

offs_t v850e2_disassembler::dasm_1101(std::ostream &stream, u16 opcode, offs_t pc, const v850e2_disassembler::data_buffer &opcodes)
{
	if ((opcode & 0xf860) == 0x0060)
	{
		u32 disp32 = opcodes.r32(pc + 2);
		util::stream_format(stream, "%-8s", "jmp");
		format_disp32_reg(stream, disp32, BIT(opcode, 0, 5));
		return 6 | SUPPORTED;
	}
	else
		return v850es_disassembler::dasm_1101(stream, opcode, pc, opcodes);
}

offs_t v850_disassembler::dasm_11110(std::ostream &stream, u16 opcode1, u16 opcode2, offs_t pc, const v850_disassembler::data_buffer &opcodes)
{
	s32 disp22 = u32(opcode1 & 0x003f) << 16 | opcode2;
	if (BIT(opcode1, 5))
		disp22 -= 0x400000;
	if ((opcode1 & 0xf800) != 0)
	{
		util::stream_format(stream, "%-8s0x%08X, %s", "jarl", pc + disp22, s_regs[BIT(opcode1, 11, 5)]);
		return 4 | STEP_OVER | SUPPORTED;
	}
	else
	{
		util::stream_format(stream, "%-8s0x%08X", "jr", pc + disp22);
		return 4 | SUPPORTED;
	}
}

offs_t v850es_disassembler::dasm_11110(std::ostream &stream, u16 opcode1, u16 opcode2, offs_t pc, const v850es_disassembler::data_buffer &opcodes)
{
	if (!BIT(opcode2, 0))
		return v850_disassembler::dasm_11110(stream, opcode1, opcode2, pc, opcodes);
	else if ((opcode1 & 0xf800) != 0)
	{
		util::stream_format(stream, "%-8s", "ld.bu");
		format_disp16_reg(stream, (opcode2 & 0xfffe) | (opcode1 & 0x0020) >> 5, BIT(opcode1, 0, 5));
		util::stream_format(stream, ", %s", s_regs[BIT(opcode1, 11, 5)]);
		return 4 | SUPPORTED;
	}
	else
	{
		u8 imm5 = BIT(opcode1, 1, 5);
		util::stream_format(stream, "%-8s", "prepare");
		format_list12(stream, (opcode2 & 0x0f00) | (opcode2 & 0xf000) >> 8 | (opcode2 & 0x00c0) >> 4 | (opcode1 & 0x0001) << 1 | (opcode2 & 0x0020) >> 5);
		util::stream_format(stream, ", %s%X", imm5 > 9 ? "0x" : "", imm5);
		switch (opcode2 & 0x001a)
		{
		case 0b00000:
		default:
			// EP unchanged
			return 4 | SUPPORTED;

		case 0b00010:
			// SP to EP
			util::stream_format(stream, ", %s", s_regs[3]);
			return 4 | SUPPORTED;

		case 0b01010:
			// 16-bit immediate data sign-extended into EP
			stream << ", ";
			format_imm16_signed(stream, opcodes.r16(pc + 4));
			return 6 | SUPPORTED;

		case 0b10010:
			// 16-bit immediate data logically shifted left by 16 to EP
			util::stream_format(stream, ", 0x%04X0000", opcodes.r16(pc + 4));
			return 6 | SUPPORTED;

		case 0b11010:
			// 32-bit immediate data to EP
			util::stream_format(stream, "0x%08X", opcodes.r32(pc + 4));
			return 8 | SUPPORTED;
		}
	}
}

offs_t v850e2_disassembler::dasm_11110(std::ostream &stream, u16 opcode1, u16 opcode2, offs_t pc, const v850e2_disassembler::data_buffer &opcodes)
{
	if ((opcode1 & 0xf800) == 0)
	{
		// V850E2S, V850E2M only for these forms
		switch (BIT(opcode2, 0, 4))
		{
		case 0b0101:
			util::stream_format(stream, "%-8s", BIT(opcode1, 5) ? "ld.bu" : "ld.b");
			format_disp23_reg(stream, BIT(opcode2, 4, 7) | u32(opcodes.r16(pc + 4)) << 7, BIT(opcode1, 0, 5));
			util::stream_format(stream, ", %s", s_regs[BIT(opcode2, 11, 5)]);
			return 6 | SUPPORTED;

		case 0b0111:
			util::stream_format(stream, "%-8s", BIT(opcode1, 5) ? "ld.hu" : "ld.h");
			format_disp23_reg(stream, BIT(opcode2, 4, 7) | u32(opcodes.r16(pc + 4)) << 7, BIT(opcode1, 0, 5));
			util::stream_format(stream, ", %s", s_regs[BIT(opcode2, 11, 5)]);
			return 6 | SUPPORTED;

		case 0b1001:
			util::stream_format(stream, "%-8s", "ld.w");
			format_disp23_reg(stream, BIT(opcode2, 4, 7) | u32(opcodes.r16(pc + 4)) << 7, BIT(opcode1, 0, 5));
			util::stream_format(stream, ", %s", s_regs[BIT(opcode2, 11, 5)]);
			return 6 | SUPPORTED;

		case 0b1101:
			util::stream_format(stream, "%-8s%s, ", BIT(opcode1, 5) ? "st.h" : "st.b", s_regs[BIT(opcode2, 11, 5)]);
			format_disp23_reg(stream, BIT(opcode2, 4, 7) | u32(opcodes.r16(pc + 4)) << 7, BIT(opcode1, 0, 5));
			return 6 | SUPPORTED;

		case 0b1111:
			util::stream_format(stream, "%-8s%s, ", "st.w", s_regs[BIT(opcode2, 11, 5)]);
			format_disp23_reg(stream, BIT(opcode2, 4, 7) | u32(opcodes.r16(pc + 4)) << 7, BIT(opcode1, 0, 5));
			return 6 | SUPPORTED;
		}
	}
	return v850es_disassembler::dasm_11110(stream, opcode1, opcode2, pc, opcodes);
}

offs_t v850_disassembler::dasm_extended(std::ostream &stream, u16 opcode1, u16 opcode2, offs_t pc, const v850_disassembler::data_buffer &opcodes)
{
	switch (BIT(opcode2, 5, 6))
	{
	case 0b000000:
		util::stream_format(stream, "%-8s%s, %s", "setf", s_conds[BIT(opcode1, 0, 4)], s_regs[BIT(opcode1, 11, 5)]);
		return 4 | SUPPORTED;

	case 0b000001:
		util::stream_format(stream, "%-8s%s, ", "ldsr", s_regs[BIT(opcode1, 0, 5)]);
		format_system_reg(stream, BIT(opcode1, 11, 5));
		return 4 | SUPPORTED;

	case 0b000010:
		util::stream_format(stream, "%-8s", "stsr");
		format_system_reg(stream, BIT(opcode1, 0, 5));
		util::stream_format(stream, ", %s", s_regs[BIT(opcode1, 11, 5)]);
		return 4 | SUPPORTED;

	case 0b000100: case 0b000101: case 0b000110:
		util::stream_format(stream, "%-8s%s, %s", s_imm5_ops[BIT(opcode2, 5, 3)],
							s_regs[BIT(opcode1, 0, 5)], s_regs[BIT(opcode1, 11, 5)]);
		return 4 | SUPPORTED;

	case 0b001000:
		util::stream_format(stream, "%-8s0x%02X", "trap", BIT(opcode1, 0, 5));
		return 4 | STEP_OVER | SUPPORTED;

	case 0b001001:
		stream << "halt";
		return 4 | SUPPORTED;

	case 0b001010:
		stream << "reti";
		return 4 | STEP_OUT | SUPPORTED;

	case 0b001011:
		switch (BIT(opcode1, 13, 3))
		{
		case 0b000:
			stream << "di";
			return 4 | SUPPORTED;

		case 0b100:
			stream << "ei";
			return 4 | SUPPORTED;
		}
		[[fallthrough]];

	case 0b000011: case 0b000111: case 0b001100: case 0b001101: case 0b001110:
		util::stream_format(stream, "%-8s0x%04X%04X ; undefined", "dw", opcode2, opcode1);
		return 4 | SUPPORTED;

	default:
		util::stream_format(stream, "%-8s0x%04X%04X ; ILGOP", "dw", opcode2, opcode1);
		return 4 | SUPPORTED;
	}
}

offs_t v850es_disassembler::dasm_extended(std::ostream &stream, u16 opcode1, u16 opcode2, offs_t pc, const v850_disassembler::data_buffer &opcodes)
{
	if (BIT(opcode2, 0))
	{
		util::stream_format(stream, "%-8s", "ld.hu");
		format_disp16_reg(stream, opcode2 & 0xfffe, BIT(opcode1, 0, 5));
		util::stream_format(stream, ", %s", s_regs[BIT(opcode1, 11, 5)]);
		return 4 | SUPPORTED;
	}
	else switch (BIT(opcode2, 5, 6))
	{
	case 0b000111:
		util::stream_format(stream, "%-8s%s, [%s]", s_bit_ops[BIT(opcode2, 1, 2)],
							s_regs[BIT(opcode1, 11, 5)], s_regs[BIT(opcode1, 0, 5)]);
		return 4 | SUPPORTED;

	case 0b001010:
		switch (BIT(opcode2, 1, 2))
		{
		case 0b00:
			stream << "reti";
			return 4 | STEP_OUT | SUPPORTED;

		case 0b10:
			stream << "ctret";
			return 4 | STEP_OUT | SUPPORTED;

		case 0b11:
			stream << "dbret";
			return 4 | STEP_OUT | SUPPORTED;

		default:
			util::stream_format(stream, "%-8s0x%04X%04X ; undefined", "dw", opcode2, opcode1);
			return 4 | SUPPORTED;
		}

	case 0b001011:
		switch (BIT(opcode1, 11, 5))
		{
		case 0b00000:
			stream << "di";
			return 4 | SUPPORTED;

		case 0b10000:
			stream << "ei";
			return 4 | SUPPORTED;

		default:
			util::stream_format(stream, "%-8s0x%04X%04X ; undefined", "dw", opcode2, opcode1);
			return 4 | SUPPORTED;
		}

	case 0b010000:
		util::stream_format(stream, "%-8s%s, %s", "sasf", s_conds[BIT(opcode1, 0, 4)], s_regs[BIT(opcode1, 11, 5)]);
		return 4 | SUPPORTED;

	case 0b010001:
		util::stream_format(stream, "%-8s%s, %s, %s", BIT(opcode2, 1) ? "mulu" : "mul",
							s_regs[BIT(opcode1, 0, 5)], s_regs[BIT(opcode1, 11, 5)], s_regs[BIT(opcode2, 11, 5)]);
		return 4 | SUPPORTED;

	case 0b010010: case 0b010011:
	{
		u16 imm9 = (opcode1 & 0x001f) | (opcode2 & 0x003c) << 3;
		util::stream_format(stream, "%-8s", BIT(opcode2, 1) ? "mulu" : "mul");
		if (imm9 >= 0x100)
		{
			stream << '-';
			imm9 = 0x200 - imm9;
		}
		if (imm9 > 9)
			stream << "0x";
		util::stream_format(stream, "%X, %s, %s", imm9, s_regs[BIT(opcode1, 11, 5)], s_regs[BIT(opcode2, 11, 5)]);
		return 4 | SUPPORTED;
	}

	case 0b010100: case 0b010101:
		util::stream_format(stream, "%-8s%s, %s, %s", BIT(opcode2, 1) ? "divhu" : "divh",
							s_regs[BIT(opcode1, 0, 5)], s_regs[BIT(opcode1, 11, 5)], s_regs[BIT(opcode2, 11, 5)]);
		return 4 | SUPPORTED;

	case 0b010110: case 0b010111:
		util::stream_format(stream, "%-8s%s, %s, %s", BIT(opcode2, 1) ? "divu" : "div",
							s_regs[BIT(opcode1, 0, 5)], s_regs[BIT(opcode1, 11, 5)], s_regs[BIT(opcode2, 11, 5)]);
		return 4 | SUPPORTED;

	case 0b011000:
		util::stream_format(stream, "%-8s%s, ", "cmov", s_conds[BIT(opcode2, 1, 4)]);
		format_imm5_signed(stream, BIT(opcode1, 0, 5));
		util::stream_format(stream, ", %s, %s", s_regs[BIT(opcode1, 11, 5)], s_regs[BIT(opcode2, 11, 5)]);
		return 4 | SUPPORTED;

	case 0b011001:
		util::stream_format(stream, "%-8s%s, %s, %s", "cmov",
							s_conds[BIT(opcode2, 1, 4)], s_regs[BIT(opcode1, 11, 5)], s_regs[BIT(opcode2, 11, 5)]);
		return 4 | SUPPORTED;

	case 0b011010:
		switch (BIT(opcode2, 1, 4))
		{
		case 0b0000:
			util::stream_format(stream, "%-8s%s, %s", "bsw", s_regs[BIT(opcode1, 11, 5)], s_regs[BIT(opcode2, 11, 5)]);
			return 4 | SUPPORTED;

		case 0b0001:
			util::stream_format(stream, "%-8s%s, %s", "bsh", s_regs[BIT(opcode1, 11, 5)], s_regs[BIT(opcode2, 11, 5)]);
			return 4 | SUPPORTED;

		case 0b0010:
			util::stream_format(stream, "%-8s%s, %s", "hsw", s_regs[BIT(opcode1, 11, 5)], s_regs[BIT(opcode2, 11, 5)]);
			return 4 | SUPPORTED;
		}
		[[fallthrough]];

	case 0b000011: case 0b011011: case 0b011100: case 0b011101: case 0b011110:
		util::stream_format(stream, "%-8s0x%04X%04X ; undefined", "dw", opcode2, opcode1);
		return 4 | SUPPORTED;

	default:
		return v850_disassembler::dasm_extended(stream, opcode1, opcode2, pc, opcodes);
	}
}

offs_t v850e2_disassembler::dasm_extended(std::ostream &stream, u16 opcode1, u16 opcode2, offs_t pc, const v850e2_disassembler::data_buffer &opcodes)
{
	if (!BIT(opcode2, 0))
	{
		if (opcode2 == 0 && BIT(opcode1, 4))
		{
			util::stream_format(stream, "%-8s0x%02X, 0x%X", "rie", BIT(opcode1, 11, 5), BIT(opcode1, 0, 4));
			return 4 | SUPPORTED;
		}
		else if ((opcode2 & 0x07ff) == 0x0346)
		{
			util::stream_format(stream, "%-8s%s, %s", "hsh", s_regs[BIT(opcode1, 11, 5)], s_regs[BIT(opcode2, 11, 5)]);
			return 4 | SUPPORTED;
		}
		else switch (BIT(opcode2, 5, 6))
		{
		case 0b000100: case 0b000101: case 0b000110:
			util::stream_format(stream, "%-8s%s, %s", s_imm5_ops[BIT(opcode2, 5, 3)],
								s_regs[BIT(opcode1, 0, 5)], s_regs[BIT(opcode1, 11, 5)]);
			if (BIT(opcode2, 1))
				util::stream_format(stream, ", %s", s_regs[BIT(opcode2, 11, 5)]);
			return 4 | SUPPORTED;

		case 0b000111:
			switch (BIT(opcode2, 1, 4))
			{
			case 0b0000: case 0b0001: case 0b0010: case 0b0011:
				util::stream_format(stream, "%-8s%s, [%s]", s_bit_ops[BIT(opcode2, 1, 2)],
									s_regs[BIT(opcode1, 11, 5)], s_regs[BIT(opcode1, 0, 5)]);
				return 4 | SUPPORTED;

			case 0b0111:
				// V850E2S, V850E2M only
				util::stream_format(stream, "%-8s[%s], %s, %s", "caxi",
									s_regs[BIT(opcode1, 0, 5)], s_regs[BIT(opcode1, 11, 5)], s_regs[BIT(opcode2, 11, 5)]);
				return 4 | SUPPORTED;

			default:
				stream << "rie";
				return 4 | SUPPORTED;
			}

		case 0b001010:
			switch (BIT(opcode2, 1, 4))
			{
			case 0b0000:
				stream << "reti";
				return 4 | STEP_OUT | SUPPORTED;

			case 0b0010:
				stream << "ctret";
				return 4 | STEP_OUT | SUPPORTED;

			case 0b0100:
				// V850E2S, V850E2M only
				stream << "eiret";
				return 4 | STEP_OUT | SUPPORTED;

			case 0b0101:
				// V850E2S, V850E2M only
				stream << "feret";
				return 4 | STEP_OUT | SUPPORTED;

			default:
				stream << "rie";
				return 4 | SUPPORTED;
			}

		case 0b001011:
			switch (BIT(opcode1, 11, 5))
			{
			case 0b00000:
				stream << "di";
				return 4 | SUPPORTED;

			case 0b10000:
				stream << "ei";
				return 4 | SUPPORTED;

			case 0b11010:
				// V850E2S, V850E2M only
				util::stream_format(stream, "%-8s0x%02X", "syscall", (opcode1 & 0x001f) | (opcode2 & 0x3800) >> 6);
				return 4 | STEP_OVER | SUPPORTED;

			default:
				stream << "rie";
				return 4 | SUPPORTED;
			}

		case 0b010111:
			// V850E2S, V850E2M only
			util::stream_format(stream, "%-8s%s, %s, %s", BIT(opcode2, 1) ? "divqu" : "divq",
								s_regs[BIT(opcode1, 0, 5)], s_regs[BIT(opcode1, 11, 5)], s_regs[BIT(opcode2, 11, 5)]);
			return 4 | SUPPORTED;

		case 0b011011:
			switch (BIT(opcode2, 1, 4))
			{
			case 0b0000:
				util::stream_format(stream, "%-8s%s, %s", "sch0r", s_regs[BIT(opcode1, 11, 5)], s_regs[BIT(opcode2, 11, 5)]);
				return 4 | SUPPORTED;

			case 0b0001:
				util::stream_format(stream, "%-8s%s, %s", "sch1r", s_regs[BIT(opcode1, 11, 5)], s_regs[BIT(opcode2, 11, 5)]);
				return 4 | SUPPORTED;

			case 0b0010:
				util::stream_format(stream, "%-8s%s, %s", "sch0l", s_regs[BIT(opcode1, 11, 5)], s_regs[BIT(opcode2, 11, 5)]);
				return 4 | SUPPORTED;

			case 0b0011:
				util::stream_format(stream, "%-8s%s, %s", "sch1l", s_regs[BIT(opcode1, 11, 5)], s_regs[BIT(opcode2, 11, 5)]);
				return 4 | SUPPORTED;

			default:
				stream << "rie";
				return 4 | SUPPORTED;
			}

		case 0b011100: case 0b011101:
			if ((opcode2 & 0x001e) != 0x001a)
				util::stream_format(stream, "%-8s%s, %s, %s, %s", BIT(opcode2, 5) ? "adf" : "sbf", s_conds[BIT(opcode2, 1, 4)],
									s_regs[BIT(opcode1, 0, 5)], s_regs[BIT(opcode1, 11, 5)], s_regs[BIT(opcode2, 11, 5)]);
			else
				util::stream_format(stream, "%-8s%s, %s, %s", BIT(opcode2, 5) ? "satadd" : "satsub",
									s_regs[BIT(opcode1, 0, 5)], s_regs[BIT(opcode1, 11, 5)], s_regs[BIT(opcode2, 11, 5)]);
			return 4 | SUPPORTED;

		case 0b011110: case 0b011111:
			util::stream_format(stream, "%-8s%s, %s, %s, %s", BIT(opcode2, 5) ? "macu" : "mac",
								s_regs[BIT(opcode1, 0, 5)], s_regs[BIT(opcode1, 11, 5)],
								s_regs[BIT(opcode2, 11, 5)], s_regs[BIT(opcode2, 0, 5)]);
			return 4 | SUPPORTED;
		}
	}
	return v850es_disassembler::dasm_extended(stream, opcode1, opcode2, pc, opcodes);
}

offs_t v850_disassembler::disassemble(std::ostream &stream, offs_t pc, const v850_disassembler::data_buffer &opcodes, const v850_disassembler::data_buffer &params)
{
	u16 opcode = opcodes.r16(pc);

	switch (BIT(opcode, 7, 4))
	{
	default: // compiler warnings can be a nuisance
	case 0b0000:
		if (BIT(opcode, 6))
		{
			if (BIT(opcode, 5))
				return dasm_000011(stream, opcode, pc, opcodes);
			else
				return dasm_000010(stream, opcode, pc, opcodes);
		}
		else if (BIT(opcode, 5))
		{
			util::stream_format(stream, "%-8s%s, %s", "not", s_regs[BIT(opcode, 0, 5)], s_regs[BIT(opcode, 11, 5)]);
			return 2 | SUPPORTED;
		}
		else
			return dasm_000000(stream, opcode, pc, opcodes);

	case 0b0001:
		return dasm_0001(stream, opcode, pc, opcodes);

	case 0b0010: case 0b0011:
		util::stream_format(stream, "%-8s%s, %s", s_rr_ops[BIT(opcode, 5, 3)], s_regs[BIT(opcode, 0, 5)], s_regs[BIT(opcode, 11, 5)]);
		return 2 | SUPPORTED;

	case 0b0100: case 0b0101:
		return dasm_010x(stream, opcode, pc, opcodes);

	case 0b0110:
	{
		u8 disp7 = BIT(opcode, 0, 7);
		util::stream_format(stream, "%-8s", "sld.b");
		format_disp8_ep(stream, disp7);
		util::stream_format(stream, ", %s", s_regs[BIT(opcode, 11, 5)]);
		return 2 | SUPPORTED;
	}

	case 0b0111:
	{
		u8 disp7 = BIT(opcode, 0, 7);
		util::stream_format(stream, "%-8s", "sst.b");
		util::stream_format(stream, "%s, ", s_regs[BIT(opcode, 11, 5)]);
		format_disp8_ep(stream, disp7);
		return 2 | SUPPORTED;
	}

	case 0b1000:
	{
		u8 disp8 = BIT(opcode, 0, 7) << 1;
		util::stream_format(stream, "%-8s", "sld.h");
		format_disp8_ep(stream, disp8);
		util::stream_format(stream, ", %s", s_regs[BIT(opcode, 11, 5)]);
		return 2 | SUPPORTED;
	}

	case 0b1001:
	{
		u8 disp8 = BIT(opcode, 0, 7) << 1;
		util::stream_format(stream, "%-8s", "sst.h");
		util::stream_format(stream, "%s, ", s_regs[BIT(opcode, 11, 5)]);
		format_disp8_ep(stream, disp8);
		return 2 | SUPPORTED;
	}

	case 0b1010:
	{
		u8 disp8 = BIT(opcode, 1, 6) << 2;
		if (BIT(opcode, 0))
		{
			util::stream_format(stream, "%-8s", "sst.w");
			util::stream_format(stream, "%s, ", s_regs[BIT(opcode, 11, 5)]);
			format_disp8_ep(stream, disp8);
		}
		else
		{
			util::stream_format(stream, "%-8s", "sld.w");
			format_disp8_ep(stream, disp8);
			util::stream_format(stream, ", %s", s_regs[BIT(opcode, 11, 5)]);
		}
		return 2 | SUPPORTED;
	}

	case 0b1011:
	{
		s16 disp9 = s8((opcode & 0xf800) >> 8 | (opcode & 0x0070) >> 4) * 2;
		util::stream_format(stream, "%-8s0x%08X", s_bconds[BIT(opcode, 0, 4)], pc + disp9);
		return 2 | (BIT(opcode, 0, 4) != 0b0101 ? STEP_COND : 0) | SUPPORTED;
	}

	case 0b1100:
		return dasm_1100(stream, opcode, pc, opcodes);

	case 0b1101:
		return dasm_1101(stream, opcode, pc, opcodes);

	case 0b1110:
	{
		u16 disp16 = opcodes.r16(pc + 2);
		if (BIT(opcode, 6))
			util::stream_format(stream, "%-8s%s, ", BIT(opcode, 5) ? (BIT(disp16, 0) ? "st.w" : "st.h") : "st.b", s_regs[BIT(opcode, 11, 5)]);
		else
			util::stream_format(stream, "%-8s", BIT(opcode, 5) ? (BIT(disp16, 0) ? "ld.w" : "ld.h") : "ld.b");
		format_disp16_reg(stream, BIT(opcode, 5) ? disp16 & 0xfffe : disp16, BIT(opcode, 0, 5));
		if (!BIT(opcode, 6))
			util::stream_format(stream, ", %s", s_regs[BIT(opcode, 11, 5)]);
		return 4 | SUPPORTED;
	}

	case 0b1111:
		if (!BIT(opcode, 6))
			return dasm_11110(stream, opcode, opcodes.r16(pc + 2), pc, opcodes);
		else if (BIT(opcode, 5))
			return dasm_extended(stream, opcode, opcodes.r16(pc + 2), pc, opcodes);
		else
		{
			util::stream_format(stream, "%-8s%d, ", s_bit_ops[BIT(opcode, 14, 2)], BIT(opcode, 11, 3));
			format_disp16_reg(stream, opcodes.r16(pc + 2), BIT(opcode, 0, 5));
			return 4 | SUPPORTED;
		}
	}
}
