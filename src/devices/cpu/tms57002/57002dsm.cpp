// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    57002dsm.cpp

    TMS57002 "DASP" emulator.

***************************************************************************/

#include "emu.h"
#include "57002dsm.h"

tms57002_disassembler::tms57002_disassembler()
{
}

std::string tms57002_disassembler::get_memadr(u32 opcode, char type)
{
	std::string buf;

	if (((opcode & 0x400) && (type == 'c')) || (!(opcode & 0x400) && (type == 'd')))
	{
		if (opcode & 0x100)
			buf = util::string_format("%c(%02x)", type, opcode & 0xff);
		else if (opcode & 0x80)
			buf = util::string_format("%c*+", type);
		else
			buf = util::string_format("%c*", type);
	}
	else if (opcode & 0x200)
		buf = util::string_format("%c*+", type);
	else
		buf = util::string_format("%c*", type);
	return buf;
}

u32 tms57002_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t tms57002_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	std::streampos original_pos = stream.tellp();
	u32 opcode = opcodes.r32(pc);
	u8 fa = opcode >> 18;
	if (fa == 0x3f)
	{
		switch ((opcode >> 11) & 0x7f) // category 3
		{

#define DASM3
#include "cpu/tms57002/tms57002.hxx"
#undef  DASM3

		default:
			util::stream_format(stream, "unk c3 %02x", (opcode >> 11) & 0x7f);
			break;
		}
	}
	else
	{
		switch (fa) // category 1
		{
		case 0x00:
			break;

#define DASM1
#include "cpu/tms57002/tms57002.hxx"
#undef  DASM1

		default:
			util::stream_format(stream, "unk c1 %02x", fa);
			break;
		}

		bool next_is_nop = ((opcode >> 11) & 0x7f) == 0x00;
		if (!next_is_nop && stream.tellp() != original_pos)
			stream << " ; ";

		switch ((opcode >> 11) & 0x7f) // category 2
		{
		case 0x00:
			if (stream.tellp() == original_pos)
				util::stream_format(stream, "nop");
			break;

#define DASM2
#include "cpu/tms57002/tms57002.hxx"
#undef  DASM2

		default:
			util::stream_format(stream, "unk c2 %02x", (opcode >> 11) & 0x7f);
			break;
		}
	}

	return 1;
}
