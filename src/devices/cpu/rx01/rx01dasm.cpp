// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    DEC RX01 microcode disassembler

    DEC's firmware listing is based on a modified PDP-8 cross-assembler.
    The syntax used here differs slightly to improve the disassembly of
    double-length instructions (which the PDP-8 generally lacks).

***************************************************************************/

#include "emu.h"
#include "rx01dasm.h"

rx01_disassembler::rx01_disassembler()
	: util::disasm_interface()
{
}

const char *const rx01_disassembler::s_0_or_1[2] = {
	"ZERO",
	"ONE"
};

const char *const rx01_disassembler::s_conditions[16] = {
	"RUN",
	"IOB3OT",
	"DATAIN",
	"INDX",
	"SR7",
	"COFL",
	"CRC16",
	"HOME",
	"WRTEN",
	"SEPCLK",
	"XIIBIT",
	"DEQSR7",
	"BAROFL",
	"MCEQSH",
	"BDATAO",
	"FLAGO"
};

const char *const rx01_disassembler::s_flag_control[4] = {
	"NOP",
	"OFF",
	"ON",
	"TOG"
};

u32 rx01_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t rx01_disassembler::disassemble(std::ostream &stream, offs_t pc, const rx01_disassembler::data_buffer &opcodes, const rx01_disassembler::data_buffer &params)
{
	u8 opcode = opcodes.r8(pc);

	if (BIT(opcode, 6))
	{
		// Branch instructions
		util::stream_format(stream, "%sBR %s ",
							BIT(opcode, 7) ? "W" : "",
							s_conditions[(opcode & 074) >> 2]);
		if ((opcode & 074) == 010 || (opcode & 074) == 020)
			stream << s_0_or_1[BIT(opcode, 1)];
		else
			stream << (BIT(opcode, 1) ? "T" : "F");
		if (BIT(opcode, 0))
		{
			stream << " IND";
			return 1 | STEP_COND | SUPPORTED;
		}
		else
		{
			util::stream_format(stream, " %04o", ((pc + 2) & 07400) | opcodes.r8(pc + 1));
			return 2 | STEP_COND | SUPPORTED;
		}
	}
	else if (BIT(opcode, 7))
	{
		if (BIT(opcode, 1))
		{
			stream << "JUMP ";
			if (BIT(opcode, 0))
			{
				util::stream_format(stream, "F%d IND", (opcode & 074) >> 2);
				return 1 | SUPPORTED;
			}
			else
			{
				util::stream_format(stream, "%04o", u16(opcode & 074) << 6 | opcodes.r8(pc + 1));
				return 2 | SUPPORTED;
			}
		}
		else
		{
			// Clock scratchpad address register
			util::stream_format(stream, "OPEN R%d", (opcode & 074) >> 2);
			return 1 | SUPPORTED;
		}
	}
	else switch (opcode & 074)
	{
	case 000: case 004: case 010: case 014: case 020: case 024: case 030:
		// IOB flip-flops for drive or interface
		util::stream_format(stream, "%s IOB%d", BIT(opcode, 1) ? "SET" : "CLR", (opcode & 074) >> 2);
		return 1 | SUPPORTED;

	case 034:
		// Drive select
		util::stream_format(stream, "UNIT %s", s_0_or_1[BIT(opcode, 1)]);
		return 1 | SUPPORTED;

	case 040:
		// Head load/unload
		util::stream_format(stream, "%sHD", BIT(opcode, 1) ? "LD" : "UN");
		return 1 | SUPPORTED;

	case 044:
		// Clock buffer address register
		if (BIT(opcode, 1))
			stream << "INCR BAR";
		else
			util::stream_format(stream, "CLR BAR %s", BIT(opcode, 0) ? "LONG" : "SHORT");
		return 1 | SUPPORTED;

	case 050:
		// Sector data buffer write
		util::stream_format(stream, "%s WRTBUF", BIT(opcode, 0) ? "START" : "FIN");
		return 1 | SUPPORTED;

	case 054:
		// Shift/preset CRC
		if (BIT(opcode, 0))
			util::stream_format(stream, "%sCRC", BIT(opcode, 1) ? "PRE" : "DAT");
		else
			util::stream_format(stream, "CRC %s", s_0_or_1[BIT(opcode, 1)]);
		return 1 | SUPPORTED;

	case 060:
		// Clock flag flip-flop (J/K)
		util::stream_format(stream, "%s FLAG", s_flag_control[opcode & 3]);
		return 1 | SUPPORTED;

	case 064:
		// Write to scratchpad
		stream << "LSP";
		return 1 | SUPPORTED;

	case 070:
		// Clock counter
		if (BIT(opcode, 1))
		{
			stream << "ICT";
			return 1 | SUPPORTED;
		}
		else if (BIT(opcode, 0))
		{
			stream << "ESP";
			return 1 | SUPPORTED;
		}
		else
		{
			util::stream_format(stream, "LCT %03o", opcodes.r8(pc + 1));
			return 2 | SUPPORTED;
		}

	case 074:
		// Clock shift register
		if (!BIT(opcode, 0))
			util::stream_format(stream, "ROTATE %s", s_0_or_1[BIT(opcode, 1)]);
		else if (BIT(opcode, 1))
			stream << "DATSR";
		else
			stream << "LSR";
		return 1 | SUPPORTED;

	default:
		// This should never happen
		util::stream_format(stream, ".BYTE %03o", opcode);
		return 1 | SUPPORTED;
	}
}
