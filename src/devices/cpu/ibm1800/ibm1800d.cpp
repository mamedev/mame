// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    IBM 1130/1800 disassembler

    The 1800 has a few more instructions than the 1130 (and some later 1800
    clones have further extensions to the instruction set). The most
    significant difference, however, is that the index registers are part
    of core storage on the 1130 but not memory-mapped on the 1800; some
    1130 programs take advantage of this fact, and IBM's diagnostics use it
    to distinguish the two processors.

    Endianness note: the architectural format for double-word operands in
    memory requires the upper 16 bits to be located at an even address and
    the lower 16 bits at the next higher address. However, in the 1442
    packed binary card format, words are punched with the lower 8 bits in
    positions 12 to 5 of odd-numbered columns and the upper 8 bits in the
    same positions of the even-numbered columns to their right. Punched
    card dumps will therefore likely need byte-swapping to be correctly
    parsed by tools such as unidasm. Similar byte-swapping might also be
    necessary for disk pack dumps.

    The 1130/1800 assembler source format requires that the format and tag
    fields be placed in specific columns to the left of the operand field.
    This is particularly significant for shift instructions, whose count
    is obtained either from the operand field or from an index register.

***************************************************************************/

#include "emu.h"
#include "ibm1800d.h"

namespace {

const char *const s_memory_ops[32] =
{
	"", "XIO", "", "",
	"", "STS", "", "",
	"BSI", "", "", "",
	"LDX", "STX", "MDX", "",
	"A", "AD", "S", "SD",
	"M", "D", "CMP", "DCM",
	"LD", "LDD", "STO", "STD",
	"AND", "OR", "EOR", ""
};

const char *const s_shift_ops[8] =
{
	"SLA", "SLCA", "SLT", "SLC",
	"SRA", "", "SRT", "RTE"
};

} // anonymous namespace

ibm1800_disassembler::ibm1800_disassembler(bool type_1130)
	: util::disasm_interface()
	, m_1130(type_1130)
{
}

ibm1130_disassembler::ibm1130_disassembler()
	: ibm1800_disassembler(true)
{
}

u32 ibm1800_disassembler::opcode_alignment() const
{
	return 1;
}

void ibm1800_disassembler::format_cond(std::ostream &stream, u8 cond)
{
	if (cond == 0)
	{
		stream << '0';
		return;
	}

	for (int b = 5; b != 0; --b)
	{
		if (BIT(cond, b))
			stream << "OCE+-Z"[b];
	}
}

void ibm1800_disassembler::format_disp(std::ostream &stream, int disp)
{
	if (disp < 0)
	{
		stream << '-';
		disp = -disp;
	}
	if (unsigned(disp) > 9)
		stream << '/';
	util::stream_format(stream, "%X", unsigned(disp));
}

offs_t ibm1800_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u16 inst = opcodes.r16(pc);

	if ((inst & 0xf400) == 0x1000 && (inst & 0x08c0) != 0x0840)
	{
		// Shift instructions
		if (inst == 0x1000)
			stream << "NOP";
		else if (inst == 0x18d0)
			stream << "XCH";
		else
		{
			util::stream_format(stream, "%-5s", s_shift_ops[bitswap<3>(inst, 11, 7, 6)]);
			if (BIT(inst, 8, 2) != 0)
				util::stream_format(stream, " %d", BIT(inst, 8, 2));
			else
				util::stream_format(stream, "   %d", inst & 0x003f);
		}
		return 1 | SUPPORTED;
	}
	else if ((inst & 0xff00) == 0x2000)
	{
		// LDS sets or resets carry and overflow indicators
		util::stream_format(stream, "%-8s%d", "LDS", inst & 0x0003);
		return 1 | SUPPORTED;
	}
	else if ((inst & 0xff00) == 0x3000)
	{
		u8 disp = inst & 0x00ff;
		if (disp != 0)
			util::stream_format(stream, "%-8s/%02X", "WAIT", disp);
		else
			stream << "WAIT";
		return 1 | SUPPORTED;
	}
	else if ((inst & 0xff80) == 0x4800)
	{
		// Single word skip instructions
		util::stream_format(stream, "%-8s", BIT(inst, 6) ? "BOSC" : "SKP");
		format_cond(stream, inst & 0x003f);
		return 1 | STEP_COND | SUPPORTED;
	}
	else if ((inst & 0xfc00) == 0x4c00)
	{
		// Long branch instructions
		bool bsc = false;
		switch (inst & 0x007f)
		{
		case 0x00:
			util::stream_format(stream, "%-5s", "B");
			break;

		case 0x01:
			util::stream_format(stream, "%-5s", "BO");
			break;

		case 0x02:
			util::stream_format(stream, "%-5s", "BC");
			break;

		case 0x04:
			util::stream_format(stream, "%-5s", "BOD");
			break;

		case 0x08:
			util::stream_format(stream, "%-5s", "BNP");
			break;

		case 0x10:
			util::stream_format(stream, "%-5s", "BNN");
			break;

		case 0x18:
			util::stream_format(stream, "%-5s", "BZ");
			break;

		case 0x20:
			util::stream_format(stream, "%-5s", "BNZ");
			break;

		case 0x28:
			util::stream_format(stream, "%-5s", "BN");
			break;

		case 0x30:
			util::stream_format(stream, "%-5s", "BP");
			break;

		default:
			util::stream_format(stream, "%-5s", BIT(inst, 6) ? "BOSC" : "BSC");
			bsc = true;
			break;
		}
		util::stream_format(stream, "%c%c ", BIT(inst, 7) ? 'I' : bsc || (inst & 0x003f) == 0 ? 'L' : ' ', BIT(inst, 8, 2) != 0 ? '0' + BIT(inst, 8, 2) : ' ');
		if (BIT(inst, 8, 2) != 0)
			format_disp(stream, s16(opcodes.r16(pc + 1)));
		else
			util::stream_format(stream, "/%04X", opcodes.r16(pc + 1));
		if (bsc && (inst & 0x003f) != 0)
		{
			stream << ',';
			format_cond(stream, inst & 0x003f);
		}
		return 2 | ((inst & 0x003f) != 0 ? STEP_COND : 0) | ((inst & 0x0380) == 0x0080 ? STEP_OUT : 0) | SUPPORTED;
	}
	else if ((inst & 0xff00) == 0x7400)
	{
		// Special long-format version of MDX which takes an 8-bit signed displacement and does not allow indirection
		util::stream_format(stream, "%-8s/%04X,", "MDM", opcodes.r16(pc + 1));
		format_disp(stream, s8(inst & 0x00ff));
		return 2 | STEP_COND | SUPPORTED;
	}
	else
	{
		const char *op = s_memory_ops[BIT(inst, 11, 5)];
		if (*op != '\0' && ((inst & 0xf000) != 0xb000 || !m_1130))
		{
			util::stream_format(stream, "%-5s", op);
			offs_t flags = (inst & 0xf800) == 0x4000 ? STEP_OVER : (inst & 0xf800) == 0x7000 || (inst & 0xf000) == 0xb000 ? STEP_COND : 0;
			if (BIT(inst, 10))
			{
				// Long format
				util::stream_format(stream, "%c%c ", BIT(inst, 7) ? 'I' : 'L', BIT(inst, 8, 2) != 0 ? '0' + BIT(inst, 8, 2) : ' ');
				u16 addr = opcodes.r16(pc + 1);
				if (BIT(inst, 8, 2) != 0 && (inst & 0xe880) != 0x6080 && (inst & 0xf800) != 0x6800)
					format_disp(stream, s16(addr));
				else if (m_1130 && addr <= 0x0003 && addr != 0x0000)
					util::stream_format(stream, "XR%d", addr);
				else
					util::stream_format(stream, "/%04X", addr);
				// This extra field can be used on the 1800 for the storage protect status variation of STS (/40, /41, /C0, /C1)
				if ((inst & 0x007f) != 0)
					util::stream_format(stream, ",/%02X", inst & 0x007f);
				return 2 | flags | SUPPORTED;
			}
			else if (BIT(inst, 8, 2) != 0)
			{
				// Short format with indexing
				util::stream_format(stream, " %d ", BIT(inst, 8, 2));
				if ((inst & 0xf800) == 0x6800)
					util::stream_format(stream, "/%04X", (pc + 1 + s8(inst & 0x00ff)) & 0xffff);
				else
					format_disp(stream, s8(inst & 0x00ff));
				return 1 | flags | SUPPORTED;
			}
			else
			{
				// Short format without indexing
				util::stream_format(stream, "   /%04X", (((inst & 0xf800) == 0x6000 ? 0 : pc + 1) + s8(inst & 0x00ff)) & 0xffff);
				return 1 | ((inst & 0xf800) == 0x7000 ? 0 : flags) | SUPPORTED; // MDX branches unconditionally in this mode
			}
		}
		else
		{
			util::stream_format(stream, "%-8s/%04X", "DC", inst);
			return 1 | SUPPORTED;
		}
	}
}
