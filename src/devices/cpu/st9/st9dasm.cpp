// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    SGS-Thomson/STMicroelectronics ST9 Family disassembler

    This architecture loosely resembles that of Zilog's Super8 and Z8, but
    adds many 16-bit operations and indirect addressing modes. Instructions
    listed as ALD and ALDW in opcode maps are actually mode prefixes
    for common ALU instructions.

    ST9+ extends the program space to 22 bits by using segmentation and
    also adds several instructions to facilitate stack frame linkage.

    ETRAP and ERET are listed in the ST9+ Opcode Map but are otherwise
    undocumented. These instructions were likely reserved for special
    debugging tools.

***************************************************************************/

#include "emu.h"
#include "st9dasm.h"

st9_disassembler::st9_disassembler()
	: util::disasm_interface()
{
}

u32 st9_disassembler::opcode_alignment() const
{
	return 1;
}

u32 st9p_disassembler::interface_flags() const
{
	return PAGED;
}

u32 st9p_disassembler::page_address_bits() const
{
	return 16;
}

namespace {

const char *const s_ald[16] =
{
	"or", "and",
	"sbc", "adc",
	"add", "sub",
	"xor", nullptr,
	"tcm", "cp",
	"tm", nullptr,
	nullptr, nullptr,
	nullptr, "ld"
};

const char *const s_aldw[16] =
{
	"orw", "andw",
	"sbcw", "adcw",
	"addw", "subw",
	"xorw", nullptr,
	"tcmw", "cpw",
	"tmw", nullptr,
	nullptr, nullptr,
	nullptr, "ldw"
};

const char *const s_unary_inst[16] =
{
	nullptr, nullptr,
	"popu", "pushu",
	"dec", "inc",
	nullptr, "da",
	"cpl", "clr",
	"rol", "rlc",
	"ror", "rrc",
	"sra", "swap"
};

const char *const s_cc[16] =
{
	"f", "lt", "le", "ule", "ov", "mi", "eq", "c",
	"", "ge", "gt", "ugt", "nov", "pl", "ne", "nc"
};

const char *const s_group_e[16] =
{
	"P0DR", "P1DR",
	"P2DR", "P3DR",
	"P4DR", "P5DR",
	"CICR", "FLAGR",
	"RP0R", "RP1R",
	"PPR", "MODER",
	"USPHR", "USPLR",
	"SSPHR", "SSPLR"
};

} // anonymous namespace

void st9_disassembler::format_dir(std::ostream &stream, u8 r) const
{
	if ((r & 0xf0) == 0xd0)
		util::stream_format(stream, "r%d", r & 0x0f);
	else if ((r & 0xf0) == 0xe0)
		stream << s_group_e[r & 0x0f]; // R224-R239
	else
		util::stream_format(stream, "R%02X", r);
}

void st9_disassembler::format_dirw(std::ostream &stream, u8 rr) const
{
	if ((rr & 0xf0) == 0xd0)
		util::stream_format(stream, "rr%d", rr & 0x0f);
	else if (rr == 0xee)
		stream << "SSPR"; // RR238
	else if (rr == 0xec)
		stream << "USPR"; // RR236
	else if (rr == 0xe8)
		stream << "RPR";
	else
		util::stream_format(stream, "RR%02X", rr);
}

void st9_disassembler::format_imm(std::ostream &stream, u8 n) const
{
	stream << '#';
	if (n >= 0xa0)
		stream << '0';
	util::stream_format(stream, "%02Xh", n);
}

void st9_disassembler::format_immw(std::ostream &stream, u16 nn) const
{
	stream << '#';
	if (nn >= 0xa000)
		stream << '0';
	util::stream_format(stream, "%04Xh", nn);
}

void st9_disassembler::format_disp(std::ostream &stream, s8 n) const
{
	util::stream_format(stream, "%d", n);
}

void st9_disassembler::format_dispw(std::ostream &stream, u16 nn) const
{
	if (nn >= 0xa000)
		stream << '0';
	util::stream_format(stream, "%04Xh", nn);
}

void st9_disassembler::format_label(std::ostream &stream, u16 nn) const
{
	if (nn >= 0xa000)
		stream << '0';
	util::stream_format(stream, "%04Xh", nn);
}

offs_t st9_disassembler::dasm_unknown(std::ostream &stream, u8 opc) const
{
	util::stream_format(stream, "%-8s", ".byte");
	if (opc >= 0xa0)
		stream << '0';
	util::stream_format(stream, "%02Xh", opc);
	return 1 | SUPPORTED;
}

offs_t st9_disassembler::dasm_06(std::ostream &stream, u8 byte1, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	const char *const aldw = s_aldw[byte1 >> 4];
	if (aldw != nullptr)
	{
		util::stream_format(stream, "%-8s", aldw);
		if (BIT(byte1, 0))
		{
			format_disp(stream, opcodes.r8(pc + 2));
			util::stream_format(stream, "(rr%d)", byte1 & 0x0e);
			stream << ',';
			format_immw(stream, opcodes.r8(pc + 3));
			return 5 | SUPPORTED;
		}
		else
		{
			format_dispw(stream, opcodes.r16(pc + 2));
			util::stream_format(stream, "(rr%d)", byte1 & 0x0e);
			stream << ',';
			format_immw(stream, opcodes.r8(pc + 4));
			return 6 | SUPPORTED;
		}
	}
	else
		return dasm_unknown(stream, 0x06);
}

offs_t st9_disassembler::dasm_0f(std::ostream &stream, u8 byte1, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	if (BIT(byte1, 4))
	{
		const u8 byte2 = opcodes.r8(pc + 2);
		util::stream_format(stream, "%-8sr%d.%d,r%d.%s%d", "bor", byte1 & 0x0f, byte1 >> 5, byte2 & 0x0f, BIT(byte2, 4) ? "!" : "", byte2 >> 5);
		return 3 | SUPPORTED;
	}
	else
	{
		util::stream_format(stream, "%-8sr%d.%d", "bset", byte1 & 0x0f, byte1 >> 5);
		return 2 | SUPPORTED;
	}
}

offs_t st9_disassembler::dasm_1f(std::ostream &stream, u8 byte1, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	if (BIT(byte1, 4))
	{
		const u8 byte2 = opcodes.r8(pc + 2);
		util::stream_format(stream, "%-8sr%d.%d,r%d.%s%d", "band", byte1 & 0x0f, byte1 >> 5, byte2 & 0x0f, BIT(byte2, 4) ? "!" : "", byte2 >> 5);
		return 3 | SUPPORTED;
	}
	else
	{
		util::stream_format(stream, "%-8sr%d.%d", "bres", byte1 & 0x0f, byte1 >> 5);
		return 2 | SUPPORTED;
	}
}

offs_t st9_disassembler::dasm_26(std::ostream &stream, u8 byte1, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	const char *const ald = s_ald[byte1 >> 4];
	if (ald != nullptr)
	{
		util::stream_format(stream, "%-8s", ald);
		if (BIT(byte1, 0))
		{
			format_disp(stream, opcodes.r8(pc + 2));
			util::stream_format(stream, "(rr%d)", byte1 & 0x0e);
			stream << ',';
			format_dir(stream, opcodes.r8(pc + 3));
			return 4 | SUPPORTED;
		}
		else
		{
			format_dispw(stream, opcodes.r16(pc + 2));
			util::stream_format(stream, "(rr%d)", byte1 & 0x0e);
			stream << ',';
			format_dir(stream, opcodes.r8(pc + 4));
			return 5 | SUPPORTED;
		}
	}
	else
		return dasm_unknown(stream, 0x26);
}

offs_t st9_disassembler::dasm_2f(std::ostream &stream, u8 byte1, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	if (!BIT(byte1, 0))
	{
		util::stream_format(stream, "%-8s", "sraw");
		format_dirw(stream, byte1);
		return 2 | SUPPORTED;
	}
	else if (s_ald[byte1 >> 4] != nullptr)
	{
		util::stream_format(stream, "%-8s", s_ald[byte1 >> 4]);
		format_label(stream, opcodes.r16(pc + 3));
		stream << ',';
		format_imm(stream, opcodes.r8(pc + 2));
		return 5 | SUPPORTED;
	}
	else
		return dasm_unknown(stream, 0x2f);
}

offs_t st9_disassembler::dasm_36(std::ostream &stream, u8 byte1, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	if (!BIT(byte1, 0))
	{
		util::stream_format(stream, "%-8s", "rrcw");
		format_dirw(stream, byte1);
		return 2 | SUPPORTED;
	}
	else if (s_aldw[byte1 >> 4] != nullptr)
	{
		util::stream_format(stream, "%-8s", s_aldw[byte1 >> 4]);
		format_label(stream, opcodes.r16(pc + 4));
		stream << ',';
		format_immw(stream, opcodes.r16(pc + 2));
		return 6 | SUPPORTED;
	}
	else
		return dasm_unknown(stream, 0x36);
}

offs_t st9_disassembler::dasm_3f(std::ostream &stream, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	return dasm_unknown(stream, 0x3f);
}

offs_t st9p_disassembler::dasm_3f(std::ostream &stream, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	const u8 byte1 = opcodes.r8(pc + 1);
	if (BIT(byte1, 6))
	{
		util::stream_format(stream, "%-8s%02Xh,", BIT(byte1, 7) ? "jps" : "calls", byte1 & 0x3f);
		format_label(stream, opcodes.r16(pc + 2));
		return 4 | (BIT(byte1, 7) ? 0 : STEP_OVER) | SUPPORTED;
	}
	else
		return dasm_unknown(stream, 0x3f);
}

offs_t st9_disassembler::dasm_60(std::ostream &stream, u8 byte1, u8 byte2) const
{
	const char *const ald = BIT(byte1, 4) ? s_ald[byte2 >> 4] : s_aldw[byte2 >> 4];
	if (ald != nullptr)
	{
		util::stream_format(stream, "%-8s", ald);
		if (BIT(byte1, 0))
			util::stream_format(stream, "rr%d(rr%d),", (byte1 & 0xe0) >> 4, byte1 & 0x0e);
		util::stream_format(stream, "%s%d", BIT(byte1, 4) ? "r" : "rr", byte2 & 0x0f);
		if (!BIT(byte1, 0))
			util::stream_format(stream, ",rr%d(rr%d)", (byte1 & 0xe0) >> 4, byte1 & 0x0e);
		return 3 | SUPPORTED;
	}
	else
		return dasm_unknown(stream, 0x60);
}

offs_t st9_disassembler::dasm_6f(std::ostream &stream, u8 byte1, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	if (BIT(byte1, 4))
	{
		const u8 byte2 = opcodes.r8(pc + 2);
		util::stream_format(stream, "%-8sr%d.%d,r%d.%s%d", "bxor", byte1 & 0x0f, byte1 >> 5, byte2 & 0x0f, BIT(byte2, 4) ? "!" : "", byte2 >> 5);
		return 3 | SUPPORTED;
	}
	else
	{
		util::stream_format(stream, "%-8sr%d.%d", "bcpl", byte1 & 0x0f, byte1 >> 5);
		return 2 | SUPPORTED;
	}
}

offs_t st9_disassembler::dasm_72(std::ostream &stream, u8 byte1, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	const char *const ald = s_ald[byte1 >> 4];
	if (ald != nullptr)
	{
		util::stream_format(stream, "%-8s", ald);
		if (!BIT(byte1, 0))
			util::stream_format(stream, "(rr%d),", byte1 & 0x0e);
		format_dir(stream, opcodes.r8(pc + 2));
		if (BIT(byte1, 0))
			util::stream_format(stream, ",(rr%d)", byte1 & 0x0e);
		return 3 | SUPPORTED;
	}
	else
		return dasm_unknown(stream, 0x72);
}

offs_t st9_disassembler::dasm_73(std::ostream &stream, u8 byte1, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	const char *const ald = s_ald[byte1 >> 4];
	if (ald != nullptr && !BIT(byte1, 0))
	{
		util::stream_format(stream, "%-8s(", ald);
		format_dirw(stream, opcodes.r8(pc + 2));
		util::stream_format(stream, "),(rr%d)", byte1 & 0x0e);
		return 3 | SUPPORTED;
	}
	else
		return dasm_unknown(stream, 0x73);
}

offs_t st9p_disassembler::dasm_73(std::ostream &stream, u8 byte1, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	if ((byte1 & 0x71) == 0x41)
	{
		util::stream_format(stream, "%-8s(", BIT(byte1, 7) ? "jps" : "calls");
		format_dirw(stream, opcodes.r8(pc + 2));
		util::stream_format(stream, "),(rr%d)", byte1 & 0x0e);
		return 3 | (BIT(byte1, 7) ? 0 : STEP_OVER) | SUPPORTED;
	}
	else
		return st9_disassembler::dasm_73(stream, byte1, pc, opcodes);
}

offs_t st9_disassembler::dasm_74(std::ostream &stream, u8 byte1) const
{
	if (BIT(byte1, 0))
	{
		util::stream_format(stream, "%-8s(", "call");
		format_dirw(stream, byte1 & 0xfe);
		stream << ')';
		return 2 | STEP_OVER | SUPPORTED;
	}
	else
	{
		util::stream_format(stream, "%-8s", "pushw");
		format_dirw(stream, byte1);
		return 2 | SUPPORTED;
	}
}

offs_t st9_disassembler::dasm_75(std::ostream &stream, u8 byte1) const
{
	if (BIT(byte1, 0))
		return dasm_unknown(stream, 0x75);
	else
	{
		util::stream_format(stream, "%-8s", "popw");
		format_dirw(stream, byte1);
		return 2 | SUPPORTED;
	}
}

offs_t st9p_disassembler::dasm_75(std::ostream &stream, u8 byte1) const
{
	if (BIT(byte1, 0))
	{
		util::stream_format(stream, "%-8s", "unlink");
		format_dirw(stream, byte1 & 0xfe);
		return 2 | SUPPORTED;
	}
	else
		return st9_disassembler::dasm_75(stream, byte1);
}

offs_t st9_disassembler::dasm_7e(std::ostream &stream, u8 byte1, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	const char *const aldw = s_aldw[byte1 >> 4];
	if (aldw != nullptr && !BIT(byte1, 0))
	{
		util::stream_format(stream, "%-8s", aldw);
		format_dirw(stream, opcodes.r8(pc + 2));
		util::stream_format(stream, ",(rr%d)", byte1 & 0x0e);
		return 3 | SUPPORTED;
	}
	else
		return dasm_unknown(stream, 0x7e);
}

offs_t st9_disassembler::dasm_7f(std::ostream &stream, u8 byte1, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	const char *const ald = s_ald[byte1 >> 4];
	if (ald != nullptr)
	{
		util::stream_format(stream, "%-8s", ald);
		if (BIT(byte1, 0))
		{
			format_dir(stream, opcodes.r8(pc + 3));
			stream << ',';
			format_disp(stream, opcodes.r8(pc + 2));
			util::stream_format(stream, "(rr%d)", byte1 & 0x0e);
			return 4 | SUPPORTED;
		}
		else
		{
			format_dir(stream, opcodes.r8(pc + 4));
			stream << ',';
			format_dispw(stream, opcodes.r16(pc + 2));
			util::stream_format(stream, "(rr%d)", byte1 & 0x0e);
			return 5 | SUPPORTED;
		}
	}
	else
		return dasm_unknown(stream, 0x7f);
}

offs_t st9_disassembler::dasm_86(std::ostream &stream, u8 byte1, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	const char *const aldw = s_aldw[byte1 >> 4];
	if (aldw != nullptr)
	{
		util::stream_format(stream, "%-8s", aldw);
		if (BIT(byte1, 0))
		{
			const u8 byte3 = opcodes.r8(pc + 3);
			if (!BIT(byte3, 0))
			{
				format_dirw(stream, byte3);
				stream << ',';
			}
			format_disp(stream, opcodes.r8(pc + 2));
			util::stream_format(stream, "(rr%d)", byte1 & 0x0e);
			if (BIT(byte3, 0))
			{
				stream << ',';
				format_dirw(stream, byte3 & 0xfe);
			}
			return 4 | SUPPORTED;
		}
		else
		{
			const u8 byte4 = opcodes.r8(pc + 4);
			if (!BIT(byte4, 0))
			{
				format_dirw(stream, byte4);
				stream << ',';
			}
			format_dispw(stream, opcodes.r16(pc + 2));
			util::stream_format(stream, "(rr%d)", byte1 & 0x0e);
			if (BIT(byte4, 0))
			{
				stream << ',';
				format_dirw(stream, byte4 & 0xfe);
			}
			return 5 | SUPPORTED;
		}
	}
	else
		return dasm_unknown(stream, 0x86);
}

offs_t st9_disassembler::dasm_8f(std::ostream &stream, u8 byte1, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	if (!BIT(byte1, 0))
	{
		util::stream_format(stream, "%-8s", "rlcw");
		format_dirw(stream, byte1);
		return 2 | SUPPORTED;
	}
	else switch (byte1)
	{
	case 0x01: case 0x03:
	{
		const u8 byte2 = opcodes.r8(pc + 2);
		util::stream_format(stream, "%-8s", BIT(byte1, 1) ? "peau" : "pea");
		if (BIT(byte2, 0))
			format_dispw(stream, opcodes.r16(pc + 3));
		else
			format_disp(stream, opcodes.r8(pc + 3));
		stream << '(';
		format_dirw(stream, byte2 & 0xfe);
		stream << ')';
		return (BIT(byte2, 0) ? 5 : 4) | SUPPORTED;
	}

	case 0xc1: case 0xc3:
		util::stream_format(stream, "%-8s", BIT(byte1, 1) ? "pushuw" : "pushw");
		format_immw(stream, opcodes.r8(pc + 2));
		return 4 | SUPPORTED;

	case 0xf1: case 0xf3:
		util::stream_format(stream, "%-8s", BIT(byte1, 1) ? "pushu" : "push");
		format_imm(stream, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	default:
		return dasm_unknown(stream, 0x8f);
	}
}

offs_t st9_disassembler::dasm_96(std::ostream &stream, u8 byte1, u8 byte2) const
{
	const char *const aldw = s_aldw[byte2 >> 4];
	if (aldw != nullptr && !BIT(byte1, 0))
	{
		util::stream_format(stream, "%-8s(r%d),", aldw, byte2 & 0x0f);
		format_dirw(stream, byte1);
		return 3 | SUPPORTED;
	}
	else
		return dasm_unknown(stream, 0x96);
}

offs_t st9_disassembler::dasm_a6(std::ostream &stream, u8 byte1, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	const char *const aldw = s_aldw[byte1 >> 4];
	if (aldw != nullptr)
	{
		util::stream_format(stream, "%-8s", aldw);
		format_dirw(stream, opcodes.r8(pc + 2));
		util::stream_format(stream, ",(r%d)", byte1);
		return 3 | SUPPORTED;
	}
	else
		return dasm_unknown(stream, 0xa6);
}

offs_t st9_disassembler::dasm_b4(std::ostream &stream, u8 byte1, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	const char *const ald = s_ald[byte1 >> 4];
	if (ald != nullptr)
	{
		util::stream_format(stream, "%-8s", ald);
		if (!BIT(byte1, 0))
			util::stream_format(stream, "(rr%d)+,", byte1 & 0x0e);
		format_dir(stream, opcodes.r8(pc + 2));
		if (BIT(byte1, 0))
			util::stream_format(stream, ",(rr%d)+", byte1 & 0x0e);
		return 3 | SUPPORTED;
	}
	else
		return dasm_unknown(stream, 0xb4);
}

offs_t st9_disassembler::dasm_b6(std::ostream &stream, u8 opc, u8 byte1, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	if (BIT(byte1, 0))
		return dasm_unknown(stream, opc);
	else
	{
		util::stream_format(stream, "%-8s", BIT(opc, 0) ? "popuw" : "pushuw");
		format_dirw(stream, byte1);
		return 2 | SUPPORTED;
	}
}

offs_t st9p_disassembler::dasm_b6(std::ostream &stream, u8 opc, u8 byte1, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	if (BIT(byte1, 0))
	{
		util::stream_format(stream, "%-8s", BIT(opc, 0) ? "unlinku" : "linku");
		format_dirw(stream, byte1 & 0xfe);
		if (BIT(opc, 0))
			return 2 | SUPPORTED;
		else
		{
			stream << ',';
			format_imm(stream, opcodes.r8(pc + 2));
			return 3 | SUPPORTED;
		}
	}
	else
		return st9_disassembler::dasm_b6(stream, opc, byte1, pc, opcodes);
}

offs_t st9_disassembler::dasm_be(std::ostream &stream, u8 byte1, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	const char *const aldw = s_aldw[byte1 >> 4];
	if (aldw != nullptr)
	{
		util::stream_format(stream, "%-8s(rr%d),", aldw, byte1 & 0x0e);
		if (BIT(byte1, 0))
		{
			format_dirw(stream, opcodes.r8(pc + 2));
			return 3 | SUPPORTED;
		}
		else
		{
			format_immw(stream, opcodes.r16(pc + 2));
			return 4 | SUPPORTED;
		}
	}
	else
		return dasm_unknown(stream, 0xbe);
}

offs_t st9_disassembler::dasm_bf(std::ostream &stream, u8 byte1, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	if (!BIT(byte1, 0))
	{
		util::stream_format(stream, "%-8s", "ldw");
		format_dirw(stream, byte1);
		stream << ',';
		format_immw(stream, opcodes.r16(pc + 2));
		return 4 | SUPPORTED;
	}
	else if (byte1 == 0x01)
	{
		stream << "halt";
		return 2 | SUPPORTED;
	}
	else
		return dasm_unknown(stream, 0xbf);
}

offs_t st9_disassembler::dasm_c2(std::ostream &stream, u8 opc, u8 byte1, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	const char *const ald = BIT(opc, 0) ? s_aldw[byte1 >> 4] : s_ald[byte1 >> 4];
	if (ald != nullptr)
	{
		util::stream_format(stream, "%-8s", ald);
		if (!BIT(byte1, 0))
			util::stream_format(stream, "-(rr%d),", byte1 & 0x0e);
		if (BIT(opc, 0))
			format_dirw(stream, opcodes.r8(pc + 2));
		else
			format_dir(stream, opcodes.r8(pc + 2));
		if (BIT(byte1, 0))
			util::stream_format(stream, ",-(rr%d)", byte1 & 0x0e);
		return 3 | SUPPORTED;
	}
	else
		return dasm_unknown(stream, opc);
}

offs_t st9_disassembler::dasm_c4(std::ostream &stream, u8 opc, u8 byte1, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	const char *const ald = s_ald[byte1 >> 4];
	if (ald != nullptr)
	{
		util::stream_format(stream, "%-8s", ald);
		if (!BIT(opc, 0))
			util::stream_format(stream, "r%d,", byte1 & 0x0f);
		format_label(stream, opcodes.r16(pc + 2));
		if (BIT(opc, 0))
			util::stream_format(stream, ",r%d", byte1 & 0x0f);
		return 4 | SUPPORTED;
	}
	else
		return dasm_unknown(stream, opc);
}

offs_t st9_disassembler::dasm_c6(std::ostream &stream, u8 byte1, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	if (BIT(byte1, 0))
	{
		util::stream_format(stream, "%-8s", "ext");
		format_dirw(stream, byte1 & 0xfe);
		return 2 | SUPPORTED;
	}
	else
	{
		util::stream_format(stream, "%-8s", "dwjnz");
		format_dirw(stream, byte1);
		stream << ',';
		format_label(stream, pc + 3 + s8(opcodes.r8(pc + 2)));
		return 3 | STEP_COND | SUPPORTED;
	}
}

offs_t st9_disassembler::dasm_c7(std::ostream &stream, u8 byte1, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	switch (byte1 & 0x07)
	{
	case 0:
		util::stream_format(stream, "%-8s", "srp");
		format_imm(stream, byte1 >> 3);
		return 2 | SUPPORTED;

	case 4:
		util::stream_format(stream, "%-8s", "srp0");
		format_imm(stream, byte1 >> 3);
		return 2 | SUPPORTED;

	case 5:
		util::stream_format(stream, "%-8s", "srp1");
		format_imm(stream, byte1 >> 3);
		return 2 | SUPPORTED;

	case 2: case 6:
		util::stream_format(stream, "%-8s", "spp");
		format_imm(stream, byte1 >> 2);
		return 2 | SUPPORTED;

	default:
		return dasm_unknown(stream, 0xc7);
	}
}

offs_t st9_disassembler::dasm_cf(std::ostream &stream, u8 opc, u8 byte1) const
{
	if (BIT(byte1, 0))
		return dasm_unknown(stream, opc);
	else
	{
		util::stream_format(stream, "%-8s", BIT(opc, 4) ? "incw" : "decw");
		format_dirw(stream, byte1);
		return 2 | SUPPORTED;
	}
}

offs_t st9_disassembler::dasm_d4(std::ostream &stream, u8 byte1, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	if (BIT(byte1, 0))
		return dasm_unknown(stream, 0xd4);
	else
	{
		util::stream_format(stream, "%-8s(", "jp");
		format_dirw(stream, byte1);
		stream << ')';
		return 2 | SUPPORTED;
	}
}

offs_t st9p_disassembler::dasm_d4(std::ostream &stream, u8 byte1, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	if (BIT(byte1, 0))
	{
		util::stream_format(stream, "%-8s", "link");
		format_dirw(stream, byte1 & 0xfe);
		stream << ',';
		format_imm(stream, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;
	}
	else
		return st9_disassembler::dasm_d4(stream, byte1, pc, opcodes);
}

offs_t st9_disassembler::dasm_d5(std::ostream &stream, u8 byte1, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	const char *const aldw = s_aldw[byte1 >> 4];
	if (aldw != nullptr)
	{
		util::stream_format(stream, "%-8s", aldw);
		if (!BIT(byte1, 0))
			util::stream_format(stream, "(rr%d)+,", byte1 & 0x0e);
		format_dirw(stream, opcodes.r8(pc + 2));
		if (BIT(byte1, 0))
			util::stream_format(stream, ",(rr%d)+", byte1 & 0x0e);
		return 3 | SUPPORTED;
	}
	else
		return dasm_unknown(stream, 0xd5);
}

offs_t st9_disassembler::dasm_e2(std::ostream &stream, u8 byte1, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	const char *const aldw = s_aldw[byte1 >> 4];
	if (aldw != nullptr)
	{
		util::stream_format(stream, "%-8s", aldw);
		if (!BIT(byte1, 0))
			util::stream_format(stream, "rr%d,", byte1 & 0x0e);
		format_label(stream, opcodes.r16(pc + 2));
		if (BIT(byte1, 0))
			util::stream_format(stream, ",rr%d", byte1 & 0x0e);
		return 4 | SUPPORTED;
	}
	else
		return dasm_unknown(stream, 0xe2);
}

offs_t st9_disassembler::dasm_e6(std::ostream &stream, u8 byte1, u8 byte2) const
{
	const char *const ald = s_ald[byte2 >> 4];
	if (ald != nullptr)
	{
		util::stream_format(stream, "%-8s(r%d),", ald, byte2 & 0x0f);
		format_dir(stream, byte1);
		return 3 | SUPPORTED;
	}
	else
		return dasm_unknown(stream, 0xe6);
}

offs_t st9_disassembler::dasm_e7(std::ostream &stream, u8 byte1, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	const char *const ald = s_ald[byte1 >> 4];
	if (ald != nullptr)
	{
		util::stream_format(stream, "%-8s", ald);
		format_dir(stream, opcodes.r8(pc + 2));
		util::stream_format(stream, ",(r%d)", byte1 & 0x0f);
		return 3 | SUPPORTED;
	}
	else
		return dasm_unknown(stream, 0xe7);
}

offs_t st9_disassembler::dasm_ef(std::ostream &stream, u8 byte1, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	if (BIT(byte1, 0))
	{
		if (byte1 == 0x01)
		{
			stream << "wfi";
			return 2 | SUPPORTED;
		}
		else if (byte1 == 0x05)
		{
			stream << "eret";
			return 2 | STEP_OUT | SUPPORTED;
		}
		else
			return dasm_unknown(stream, 0xef);
	}
	else
	{
		util::stream_format(stream, "%-8s", "ldw");
		format_dirw(stream, opcodes.r8(pc + 2));
		stream << ',';
		format_dirw(stream, byte1);
		return 3 | SUPPORTED;
	}
}

offs_t st9_disassembler::dasm_f2(std::ostream &stream, u8 byte1, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	if (BIT(byte1, 4))
	{
		const u8 byte2 = opcodes.r8(pc + 2);
		util::stream_format(stream, "%-8sr%d.%d,r%d.%s%d", "bld", byte1 & 0x0f, byte1 >> 5, byte2 & 0x0f, BIT(byte2, 4) ? "!" : "", byte2 >> 5);
		return 3 | SUPPORTED;
	}
	else
	{
		util::stream_format(stream, "%-8sr%d.%d", "btset", byte1 & 0x0f, byte1 >> 5);
		return 2 | SUPPORTED;
	}
}

offs_t st9_disassembler::dasm_f3(std::ostream &stream, u8 byte1, offs_t pc, const st9_disassembler::data_buffer &opcodes) const
{
	const char *const ald = s_ald[byte1 >> 4];
	// Byte 1 for LD (rr),#N is documented as [XTN=F|dst,0], but actual code uses dst,1 instead
	if (ald != nullptr /*&& !BIT(byte1, 0)*/)
	{
		util::stream_format(stream, "%-8s(rr%d),", ald, byte1 & 0x0e);
		format_imm(stream, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;
	}
	else
		return dasm_unknown(stream, 0xf3);
}

offs_t st9_disassembler::dasm_f6(std::ostream &stream, u8 byte1) const
{
	if (BIT(byte1, 0))
		return dasm_unknown(stream, 0xf6);
	else
	{
		util::stream_format(stream, "%-8s(rr%d).%d", "btset", byte1 & 0x0e, byte1 >> 5);
		return 2 | SUPPORTED;
	}
}

offs_t st9p_disassembler::dasm_f6(std::ostream &stream, u8 byte1) const
{
	if (byte1 == 0x01)
	{
		stream << "rets";
		return 2 | STEP_OUT | SUPPORTED;
	}
	else
		return st9_disassembler::dasm_f6(stream, byte1);
}

offs_t st9_disassembler::disassemble(std::ostream &stream, offs_t pc, const st9_disassembler::data_buffer &opcodes, const st9_disassembler::data_buffer &params)
{
	const u8 opc = opcodes.r8(pc);

	switch (opc)
	{
	case 0x00:
		stream << "ei";
		return 1 | SUPPORTED;

	case 0x01:
		stream << "scf";
		return 1 | SUPPORTED;

	case 0x02: case 0x12: case 0x22: case 0x32: case 0x42: case 0x52: case 0x62: case 0x82: case 0x92: case 0xa2:
	{
		const u8 byte1 = opcodes.r8(pc + 1);
		const char *const ald = s_ald[opc >> 4];
		assert(ald != nullptr);
		util::stream_format(stream, "%-8sr%d,r%d", ald, byte1 >> 4, byte1 & 0x0f);
		return 2 | SUPPORTED;
	}

	case 0x03: case 0x13: case 0x23: case 0x33: case 0x43: case 0x53: case 0x63: case 0x83: case 0x93: case 0xa3:
	{
		const u8 byte1 = opcodes.r8(pc + 1);
		const char *const ald = s_ald[opc >> 4];
		assert(ald != nullptr);
		util::stream_format(stream, "%-8sr%d,(r%d)", ald, byte1 >> 4, byte1 & 0x0f);
		return 2 | SUPPORTED;
	}

	case 0x04: case 0x14: case 0x24: case 0x34: case 0x44: case 0x54: case 0x64: case 0x84: case 0x94: case 0xa4: case 0xf4:
	{
		const char *const ald = s_ald[opc >> 4];
		assert(ald != nullptr);
		util::stream_format(stream, "%-8s", ald);
		format_dir(stream, opcodes.r8(pc + 2));
		stream << ',';
		format_dir(stream, opcodes.r8(pc + 1));
		return 3 | SUPPORTED;
	}

	case 0x05: case 0x15: case 0x25: case 0x35: case 0x45: case 0x55: case 0x65: case 0x85: case 0x95: case 0xa5: case 0xf5:
	{
		const char *const ald = s_ald[opc >> 4];
		assert(ald != nullptr);
		util::stream_format(stream, "%-8s", ald);
		format_dir(stream, opcodes.r8(pc + 1));
		stream << ',';
		format_imm(stream, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;
	}

	case 0x06:
		return dasm_06(stream, opcodes.r8(pc + 1), pc, opcodes);

	case 0x07: case 0x17: case 0x27: case 0x37: case 0x47: case 0x57: case 0x67: case 0x87: case 0x97: case 0xa7:
	{
		const u8 byte1 = opcodes.r8(pc + 1);
		const char *const aldw = s_aldw[opc >> 4];
		assert(aldw != nullptr);
		util::stream_format(stream, "%-8s", aldw);
		if (BIT(byte1, 0))
		{
			format_dirw(stream, byte1 & 0xfe);
			stream << ',';
			format_immw(stream, opcodes.r16(pc + 2));
			return 4 | SUPPORTED;
		}
		else
		{
			format_dirw(stream, opcodes.r8(pc + 2));
			stream << ',';
			format_dirw(stream, byte1);
			return 3 | SUPPORTED;
		}
	}

	case 0x08: case 0x18: case 0x28: case 0x38: case 0x48: case 0x58: case 0x68: case 0x78:
	case 0x88: case 0x98: case 0xa8: case 0xb8: case 0xc8: case 0xd8: case 0xe8: case 0xf8:
		util::stream_format(stream, "%-8sr%d,", "ld", opc >> 4);
		format_dir(stream, opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0x09: case 0x19: case 0x29: case 0x39: case 0x49: case 0x59: case 0x69: case 0x79:
	case 0x89: case 0x99: case 0xa9: case 0xb9: case 0xc9: case 0xd9: case 0xe9: case 0xf9:
		util::stream_format(stream, "%-8s", "ld");
		format_dir(stream, opcodes.r8(pc + 1));
		util::stream_format(stream, ",r%d", opc >> 4);
		return 2 | SUPPORTED;

	case 0x0a: case 0x1a: case 0x2a: case 0x3a: case 0x4a: case 0x5a: case 0x6a: case 0x7a:
	case 0x8a: case 0x9a: case 0xaa: case 0xba: case 0xca: case 0xda: case 0xea: case 0xfa:
		util::stream_format(stream, "%-8sr%d,", "djnz", opc >> 4);
		format_label(stream, pc + 2 + s8(opcodes.r8(pc + 1)));
		return 2 | STEP_COND | SUPPORTED;

	case 0x0b: case 0x1b: case 0x2b: case 0x3b: case 0x4b: case 0x5b: case 0x6b: case 0x7b:
	case 0x8b: case 0x9b: case 0xab: case 0xbb: case 0xcb: case 0xdb: case 0xeb: case 0xfb:
		util::stream_format(stream, "jr%-6s", s_cc[opc >> 4]);
		format_label(stream, pc + 2 + s8(opcodes.r8(pc + 1)));
		return 2 | ((opc & 0x70) != 0 ? STEP_COND : 0) | SUPPORTED;

	case 0x0c: case 0x1c: case 0x2c: case 0x3c: case 0x4c: case 0x5c: case 0x6c: case 0x7c:
	case 0x8c: case 0x9c: case 0xac: case 0xbc: case 0xcc: case 0xdc: case 0xec: case 0xfc:
		util::stream_format(stream, "%-8sr%d,", "ld", opc >> 4);
		format_imm(stream, opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0x0d: case 0x1d: case 0x2d: case 0x3d: case 0x4d: case 0x5d: case 0x6d: case 0x7d:
	case 0x8d: case 0x9d: case 0xad: case 0xbd: case 0xcd: case 0xdd: case 0xed: case 0xfd:
		util::stream_format(stream, "jp%-6s", s_cc[opc >> 4]);
		format_label(stream, opcodes.r16(pc + 1));
		return 3 | ((opc & 0x70) != 0 ? STEP_COND : 0) | SUPPORTED;

	case 0x0e: case 0x1e: case 0x2e: case 0x3e: case 0x4e: case 0x5e: case 0x6e: case 0x8e: case 0x9e: case 0xae:
	{
		const u8 byte1 = opcodes.r8(pc + 1);
		const char *const aldw = s_aldw[opc >> 4];
		assert(aldw != nullptr);
		util::stream_format(stream, "%-8s", aldw);
		if (BIT(byte1, 4))
			stream << '(';
		util::stream_format(stream, "rr%d", (byte1 & 0xe0) >> 4);
		if (BIT(byte1, 4))
			stream << ')';
		stream << ',';
		if (BIT(byte1, 0))
			stream << '(';
		util::stream_format(stream, "rr%d", byte1 & 0x0e);
		if (BIT(byte1, 0))
			stream << ')';
		return 2 | SUPPORTED;
	}

	case 0x0f:
		return dasm_0f(stream, opcodes.r8(pc + 1), pc, opcodes);

	case 0x10:
		stream << "di";
		return 1 | SUPPORTED;

	case 0x11:
		stream << "rcf";
		return 1 | SUPPORTED;

	case 0x16:
		util::stream_format(stream, "%-8s", "xch");
		format_dir(stream, opcodes.r8(pc + 2));
		stream << ',';
		format_dir(stream, opcodes.r8(pc + 1));
		return 3 | SUPPORTED;

	case 0x1f:
		return dasm_1f(stream, opcodes.r8(pc + 1), pc, opcodes);

	case 0x20: case 0x30: case 0x40: case 0x50: case 0x70: case 0x80: case 0x90: case 0xa0: case 0xb0: case 0xc0: case 0xd0: case 0xe0: case 0xf0:
	{
		const char *const inst = s_unary_inst[opc >> 4];
		assert(inst != nullptr);
		util::stream_format(stream, "%-8s", inst);
		format_dir(stream, opcodes.r8(pc + 1));
		return 2 | SUPPORTED;
	}

	case 0x21: case 0x31: case 0x41: case 0x51: case 0x71: case 0x81: case 0x91: case 0xa1: case 0xb1: case 0xc1: case 0xd1: case 0xe1: case 0xf1:
	{
		const char *const inst = s_unary_inst[opc >> 4];
		assert(inst != nullptr);
		util::stream_format(stream, "%-8s(", inst);
		format_dir(stream, opcodes.r8(pc + 1));
		stream << ')';
		return 2 | SUPPORTED;
	}

	case 0x26:
		return dasm_26(stream, opcodes.r8(pc + 1), pc, opcodes);

	case 0x2f:
		return dasm_2f(stream, opcodes.r8(pc + 1), pc, opcodes);

	case 0x36:
		return dasm_36(stream, opcodes.r8(pc + 1), pc, opcodes);

	case 0x3f:
		return dasm_3f(stream, pc, opcodes);

	case 0x46:
		stream << "ret";
		return 1 | STEP_OUT | SUPPORTED;

	case 0x4f: case 0x5f:
	{
		const u8 byte1 = opcodes.r8(pc + 1);
		util::stream_format(stream, "%-8srr%d,r%d", BIT(opc, 4) ? "div" : "mul", byte1 >> 4, byte1 & 0x0f);
		return 2 | SUPPORTED;
	}

	case 0x56:
	{
		const u8 byte2 = opcodes.r8(pc + 2);
		util::stream_format(stream, "%-8srr%d,rr%d,", "divws", byte2 >> 4, byte2 & 0x0f);
		format_dirw(stream, opcodes.r8(pc + 1));
		return 3 | SUPPORTED;
	}

	case 0x60:
		return dasm_60(stream, opcodes.r8(pc + 1), opcodes.r8(pc + 2));

	case 0x61:
		stream << "ccf";
		return 1 | SUPPORTED;

	case 0x66: case 0x76:
		util::stream_format(stream, "%-8s", BIT(opc, 4) ? "pop" : "push");
		format_dir(stream, opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0x6f:
		return dasm_6f(stream, opcodes.r8(pc + 1), pc, opcodes);

	case 0x72:
		return dasm_72(stream, opcodes.r8(pc + 1), pc, opcodes);

	case 0x73:
		return dasm_73(stream, opcodes.r8(pc + 1), pc, opcodes);

	case 0x74:
		return dasm_74(stream, opcodes.r8(pc + 1));

	case 0x75:
		return dasm_75(stream, opcodes.r8(pc + 1));

	case 0x77: case 0xf7:
		util::stream_format(stream, "%-8s(", BIT(opc, 7) ? "push" : "pop");
		format_dir(stream, opcodes.r8(pc + 1));
		stream << ')';
		return 2 | SUPPORTED;

	case 0x7e:
		return dasm_7e(stream, opcodes.r8(pc + 1), pc, opcodes);

	case 0x7f:
		return dasm_7f(stream, opcodes.r8(pc + 1), pc, opcodes);

	case 0x86:
		return dasm_86(stream, opcodes.r8(pc + 1), pc, opcodes);

	case 0x8f:
		return dasm_8f(stream, opcodes.r8(pc + 1), pc, opcodes);

	case 0x96:
		return dasm_96(stream, opcodes.r8(pc + 1), opcodes.r8(pc + 1));

	case 0x9f:
	{
		const u8 byte1 = opcodes.r8(pc + 1);
		util::stream_format(stream, "%-8sr%d,(rr%d),", BIT(byte1, 4) ? "cpjt" : "cpjf", byte1 & 0x0f, (byte1 & 0xe0) >> 4);
		format_label(stream, pc + 3 + s8(opcodes.r8(pc + 2)));
		return 3 | STEP_COND | SUPPORTED;
	}

	case 0xa6:
		return dasm_a6(stream, opcodes.r8(pc + 1), pc, opcodes);

	case 0xaf:
	{
		const u8 byte1 = opcodes.r8(pc + 1);
		util::stream_format(stream, "%-8sr%d.%d,", BIT(byte1, 4) ? "btjf" : "btjt", byte1 & 0x0f, byte1 >> 5);
		format_label(stream, pc + 3 + s8(opcodes.r8(pc + 2)));
		return 3 | STEP_COND | SUPPORTED;
	}

	case 0xb2: case 0xb3:
	{
		const u8 byte1 = opcodes.r8(pc + 1);
		util::stream_format(stream, "%-8s", "ld");
		if (BIT(opc, 0))
			util::stream_format(stream, "r%d,", byte1 >> 4);
		format_disp(stream, opcodes.r8(pc + 2));
		util::stream_format(stream, "(r%d)", byte1 & 0x0f);
		if (!BIT(opc, 0))
			util::stream_format(stream, ",r%d", byte1 >> 4);
		return 3 | SUPPORTED;
	}

	case 0xb4:
		return dasm_b4(stream, opcodes.r8(pc + 1), pc, opcodes);

	case 0xb5:
	{
		const u8 byte1 = opcodes.r8(pc + 1);
		if (BIT(byte1, 0))
			util::stream_format(stream, "%-8s(rr%d),r%d", "ld", byte1 & 0x0e, byte1 >> 4);
		else
			util::stream_format(stream, "%-8sr%d,(rr%d)", "ld", byte1 >> 4, byte1 & 0x0e);
		return 2 | SUPPORTED;
	}

	case 0xb6: case 0xb7:
		return dasm_b6(stream, opc, opcodes.r8(pc + 1), pc, opcodes);

	case 0xbe:
		return dasm_be(stream, opcodes.r8(pc + 1), pc, opcodes);

	case 0xbf:
		return dasm_bf(stream, opcodes.r8(pc + 1), pc, opcodes);

	case 0xc2: case 0xc3:
		return dasm_c2(stream, opc, opcodes.r8(pc + 1), pc, opcodes);

	case 0xc4: case 0xc5:
		return dasm_c4(stream, opc, opcodes.r8(pc + 1), pc, opcodes);

	case 0xc6:
		return dasm_c6(stream, opcodes.r8(pc + 1), pc, opcodes);

	case 0xc7:
		return dasm_c7(stream, opcodes.r8(pc + 1), pc, opcodes);

	case 0xce:
		stream << "etrap";
		return 1 | STEP_OVER | SUPPORTED;

	case 0xcf: case 0xdf:
		return dasm_cf(stream, opc, opcodes.r8(pc + 1));

	case 0xd2:
		util::stream_format(stream, "%-8s", "call");
		format_label(stream, opcodes.r16(pc + 1));
		return 3 | STEP_OVER | SUPPORTED;

	case 0xd3:
		stream << "iret";
		return 1 | STEP_OUT | SUPPORTED;

	case 0xd4:
		return dasm_d4(stream, opcodes.r8(pc + 1), pc, opcodes);

	case 0xd5:
		return dasm_d5(stream, opcodes.r8(pc + 1), pc, opcodes);

	case 0xd6:
	{
		const u8 byte1 = opcodes.r8(pc + 1);
		util::stream_format(stream, "ld%c%-5c(rr%d)+,(rr%d)+", "pd"[BIT(byte1, 4)], "pd"[BIT(byte1, 0)], (byte1 & 0xe0) >> 4, byte1 & 0x0e);
		return 2 | SUPPORTED;
	}

	case 0xd7:
	{
		const u8 byte1 = opcodes.r8(pc + 1);
		util::stream_format(stream, "%-8s", "ld");
		if (BIT(byte1, 0))
			util::stream_format(stream, "(r%d)+,(rr%d)+", byte1 >> 4, byte1 & 0x0e);
		else
			util::stream_format(stream, "(rr%d)+,(r%d)+", byte1 & 0x0e, byte1 >> 4);
		return 2 | SUPPORTED;
	}

	case 0xde:
	{
		const u8 byte1 = opcodes.r8(pc + 1);
		util::stream_format(stream, "%-8s", "ldw");
		if (!BIT(byte1, 4))
			util::stream_format(stream, "rr%d,", byte1 >> 4);
		format_disp(stream, opcodes.r8(pc + 2));
		util::stream_format(stream, "(r%d)", byte1 & 0x0f);
		if (BIT(byte1, 4))
			util::stream_format(stream, ",rr%d", (byte1 & 0xe0) >> 4);
		return 3 | SUPPORTED;
	}

	case 0xe2:
		return dasm_e2(stream, opcodes.r8(pc + 1), pc, opcodes);

	case 0xe3:
	{
		const u8 byte1 = opcodes.r8(pc + 1);
		util::stream_format(stream, "%-8s", "ldw");
		if (BIT(byte1, 4))
			stream << '(';
		util::stream_format(stream, "rr%d", (byte1 & 0xe0) >> 4);
		if (BIT(byte1, 4))
			stream << ')';
		stream << ',';
		if (BIT(byte1, 0))
			stream << '(';
		util::stream_format(stream, "rr%d", byte1 & 0x0e);
		if (BIT(byte1, 0))
			stream << ')';
		return 2 | SUPPORTED;
	}

	case 0xe4:
	{
		const u8 byte1 = opcodes.r8(pc + 1);
		util::stream_format(stream, "%-8sr%d,(r%d)", "ld", byte1 >> 4, byte1 & 0x0f);
		return 2 | SUPPORTED;
	}

	case 0xe5:
	{
		const u8 byte1 = opcodes.r8(pc + 1);
		util::stream_format(stream, "%-8s(r%d),r%d", "ld", byte1 >> 4, byte1 & 0x0f);
		return 2 | SUPPORTED;
	}

	case 0xe6:
		return dasm_e6(stream, opcodes.r8(pc + 1), opcodes.r8(pc + 2));

	case 0xe7:
		return dasm_e7(stream, opcodes.r8(pc + 1), pc, opcodes);

	case 0xee:
		stream << "spm";
		return 1 | SUPPORTED;

	case 0xef:
		return dasm_ef(stream, opcodes.r8(pc + 1), pc, opcodes);

	case 0xf2:
		return dasm_f2(stream, opcodes.r8(pc + 1), pc, opcodes);

	case 0xf3:
		return dasm_f3(stream, opcodes.r8(pc + 1), pc, opcodes);

	case 0xf6:
		return dasm_f6(stream, opcodes.r8(pc + 1));

	case 0xfe:
		stream << "sdm";
		return 1 | SUPPORTED;

	case 0xff:
		stream << "nop";
		return 1 | SUPPORTED;

	default:
		return dasm_unknown(stream, opc);
	}
}
