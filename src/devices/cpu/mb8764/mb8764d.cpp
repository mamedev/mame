// license:BSD-3-Clause
// copyright-holders:superctr
/***************************************************************************

    Fujitsu MB8764 family DSP disassembler

    Mnemonics and operand syntax as in Fujitsu's assembler listings.

***************************************************************************/

#include "emu.h"
#include "mb8764d.h"

namespace {

constexpr char const *const ALU_NAMES[16] =
{
	"NOP", "ABS", "ADD", "NEG", "MLT", "SRA", "SUB", "SLA",
	"MSM", "AND", "MRD", "ORA", "SUM", "DIV", "RED", "COM"
};

constexpr char const *const REG_NAMES[2][6] =
{
	{ "EO", "A", "B", "D", "EXA", nullptr },
	{ "X", "Y", "C0", "C1", "PGM", "PGT" }
};

constexpr char const *const MASK_NAMES[19] =
{
	nullptr, nullptr, "TRC", "CLP", "OV", "ESS", "VS", "OF", "IF", "PCT",
	"DMM", "Y", "X", "B", "A", "DMC", "F1", "F0", "D"
};

} // anonymous namespace

u32 mb8764_disassembler::opcode_alignment() const
{
	return 1;
}

std::string mb8764_disassembler::alu(u32 op)
{
	switch (BIT(op, 20, 4))
	{
	case 0x0: case 0x1: case 0xc:
		return ALU_NAMES[BIT(op, 17, 3) << 1];
	case 0x8:
	{
		static constexpr u8 codes[4] = { 0x0, 0x4, 0x8, 0xa };
		return ALU_NAMES[codes[BIT(op, 18, 2)]];
	}
	default:
		return ALU_NAMES[BIT(op, 16, 4)];
	}
}

std::string mb8764_disassembler::regs(u32 field, bool group)
{
	std::string result;
	for (int i = 0; i < 6; i++)
	{
		if (BIT(field, 5 - i))
		{
			if (!result.empty())
				result += ':';
			result += REG_NAMES[group][i] ? REG_NAMES[group][i] : "?";
		}
	}
	return result;
}

std::string mb8764_disassembler::ram(u32 op)
{
	return util::string_format("$%02X%s%s", op & 0xff, BIT(op, 8) ? "(Y)" : "", BIT(op, 9) ? "" : "(E)");
}

std::string mb8764_disassembler::mask(u32 op)
{
	std::string result;
	for (int i = 18; i >= 0; i--)
	{
		if (BIT(op, i))
		{
			if (!result.empty())
				result += ':';
			result += MASK_NAMES[i] ? MASK_NAMES[i] : util::string_format("$%05X", 1 << i);
		}
	}
	return result;
}

std::string mb8764_disassembler::condition(u32 op)
{
	switch (op & 0xfc00)
	{
	case 0x1000: return "F0";
	case 0x4000: return "C0";
	case 0x8400: return "OV";
	case 0x8800: return "ZR";
	case 0x9000: return "IF";
	case 0xa000: return "OF";
	default:     return util::string_format("$%04X", op & 0xfc00);
	}
}

offs_t mb8764_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	const u32 op = opcodes.r32(pc) & 0xff'ffff;
	u32 flags = 0;

	switch (BIT(op, 20, 4))
	{
	case 0x0:
	case 0x1:
		util::stream_format(stream, "%s:%s $%02X%s,$%02X%s%s",
				BIT(op, 20) ? "LAB" : "LTB", alu(op),
				BIT(op, 9, 7), BIT(op, 16) ? "(X)" : "",
				op & 0x7f, BIT(op, 8) ? "(Y)" : "", BIT(op, 7) ? "" : "(E)");
		break;

	case 0x4:
	case 0x6:
		util::stream_format(stream, "MOV:%s %s,%s", alu(op), ram(op), regs(BIT(op, 10, 6), BIT(op, 21)));
		break;

	case 0x5:
		util::stream_format(stream, "NOP:%s", alu(op));
		break;

	case 0x7:
		util::stream_format(stream, "MOV:%s #$%02X,%s", alu(op), op & 0x3ff, regs(BIT(op, 10, 6), true));
		break;

	case 0x8:
	{
		std::string dest;
		if (BIT(op, 11))
			dest = ram(op);
		if (BIT(op, 12, 4))
		{
			if (!dest.empty())
				dest += ':';
			dest += regs(BIT(op, 10, 6) & 0x3c, false);
		}
		util::stream_format(stream, "MOV:%s %s,%s", alu(op), BIT(op, 17) ? "?" : BIT(op, 16) ? "A" : "D", dest);
		break;
	}

	case 0xc:
		if (BIT(op, 16))
			util::stream_format(stream, "MXY:%s #$%X,#$%X", alu(op), BIT(op, 9, 7), op & 0xff);
		else
			util::stream_format(stream, "LDI:%s #$%04X", alu(op), op & 0xffff);
		break;

	case 0xd:
		util::stream_format(stream, "JOC:%s $%02X,%s", alu(op), op & 0x3ff, condition(op));
		flags = STEP_COND;
		break;

	case 0xe:
		util::stream_format(stream, "%s %s", BIT(op, 19) ? "SET" : "CLR", mask(op & 0x7'ffff));
		break;

	case 0xf:
		if (BIT(op, 16))
		{
			switch (BIT(op, 17, 3))
			{
			case 0: util::stream_format(stream, "MOV #$%04X,EO", op & 0xffff); break;
			case 1: util::stream_format(stream, "MOV #$%03X,EA", op & 0xffff); break;
			case 2: util::stream_format(stream, "MOV #$%04X,D", op & 0xffff); break;
			case 3: util::stream_format(stream, "MOV #$%04X,B", op & 0xffff); break;
			case 5: util::stream_format(stream, "MOV #$%03X,EXA", op & 0x3ff); break;
			case 7: util::stream_format(stream, "DCW $%04X", op & 0xffff); break;
			default: util::stream_format(stream, "??? $%06X", op); break;
			}
		}
		else
		{
			switch (BIT(op, 12, 4))
			{
			case 0x0:
				util::stream_format(stream, "JSR:%s $%02X", alu(op), op & 0x3ff);
				flags = STEP_OVER;
				break;
			case 0x2:
				util::stream_format(stream, "RTS:%s", alu(op));
				flags = STEP_OUT;
				break;
			case 0x4:
				util::stream_format(stream, "AVP:%s #$%X", alu(op), op & 0xfff);
				break;
			case 0xe:
				util::stream_format(stream, "JMP:%s $%02X", alu(op), op & 0x3ff);
				break;
			default:
				util::stream_format(stream, "??? $%06X", op);
				break;
			}
		}
		break;

	default:
		util::stream_format(stream, "??? $%06X", op);
		break;
	}

	return 1 | flags | SUPPORTED;
}
