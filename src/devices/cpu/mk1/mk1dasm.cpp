// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Mark 1 FORTH Computer microcode disassembler

***************************************************************************/

#include "emu.h"
#include "mk1dasm.h"

mk1_disassembler::mk1_disassembler()
	: util::disasm_interface()
{
}

u32 mk1_disassembler::opcode_alignment() const
{
	return 1;
}

namespace {

std::string_view s_srcs[8] = { "w", "ip", "tos", "rs", "[w]", "[ip]", "0", "f" };
std::string_view s_dsts[8] = { "w", "ip", "tos", "rs", "[w]", "op", "a", "b" };

// "b??" is the actual mnemonic Andrew Holme's assembler uses (condition is sign or less depending on operation)
std::string_view s_skips[4] = { "b??", "bcs", "bne", "bis" };

} // anonymous namespace

offs_t mk1_disassembler::disassemble(std::ostream &stream, offs_t pc, const mk1_disassembler::data_buffer &opcodes, const mk1_disassembler::data_buffer &params)
{
	const u8 uop = opcodes.r8(pc);

	if (uop < 0x80)
	{
		// Move LSB/MSB
		util::stream_format(stream, "mov %s%s, %s%s",
			s_dsts[BIT(uop, 0, 3)],
			BIT(uop, 2) ? "" : (BIT(uop, 6) ? ".h" : ".l"),
			s_srcs[BIT(uop, 3, 3)],
			BIT(uop, 5) ? "" : (BIT(uop, 6) ? ".h" : ".l"));
		return 1 | SUPPORTED;
	}
	else if (uop >= 0xc0)
	{
		// Conditional skip
		const u8 disp = BIT(uop, 0, 4);
		util::stream_format(stream, "%s %d ; $%03x", s_skips[BIT(uop, 4, 2)], disp, (pc + 1 + disp) & 0xfff);
		return 1 | STEP_COND | SUPPORTED;
	}
	else if (uop >= 0xb0)
	{
		// Jump indirect
		stream << "xop";
		return 1 | SUPPORTED;
	}
	else if (uop >= 0xa0)
	{
		// Set ALU function
		stream << "alu ";
		switch (BIT(uop, 0, 4))
		{
		case 0:
			stream << "add";
			break;

		case 1:
			stream << "adc";
			break;

		case 2:
			stream << "sub";
			break;

		case 3:
			stream << "sbb";
			break;

		case 4:
			stream << "asl";
			break;

		case 5:
			stream << "rol";
			break;

		case 8:
			stream << "a"; // was also to have been "0="
			break;

		case 9:
			stream << "b";
			break;

		case 10:
			stream << "and";
			break;

		case 11:
			stream << "or";
			break;

		case 12:
			stream << "not";
			break;

		case 13:
			stream << "xor";
			break;

		case 14:
			stream << "a=b";
			break;

		default:
			util::stream_format(stream, "%d", BIT(uop, 0, 4));
			break;
		}
		return 1 | SUPPORTED;
	}
	else if (uop >= 0x90)
	{
		// Jump direct
		util::stream_format(stream, "jmp %d", BIT(uop, 0, 4));
		return 1 | STEP_OUT | SUPPORTED;
	}
	else if (BIT(uop, 2))
	{
		// Disable/enable IRQ
		if (BIT(uop, 3))
			stream << "eni";
		else
			stream << "dis";
		return 1 | SUPPORTED;
	}
	else
	{
		// Increment/decrement
		if (BIT(uop, 3))
			stream << "inc ";
		else
			stream << "dec ";
		if (BIT(uop, 1))
			util::stream_format(stream, "%csp", BIT(uop, 0) ? 'r' : 'p');
		else
			stream << s_dsts[BIT(uop, 0)];
		return 1 | SUPPORTED;
	}
}
