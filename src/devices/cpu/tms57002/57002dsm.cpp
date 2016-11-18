// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    57002dsm.c

    TMS57002 "DASP" emulator.

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "tms57002.h"

static std::string get_memadr(uint32_t opcode, char type)
{
	std::string buf;

	if(((opcode & 0x400) && (type == 'c')) || (!(opcode & 0x400) && (type == 'd'))) {
		if(opcode & 0x100)
			buf = util::string_format("%c(%02x)", type, opcode & 0xff);
		else if(opcode & 0x80)
			buf = util::string_format("%c*+", type);
		else
			buf = util::string_format("%c*", type);
	} else if(opcode & 0x200)
		buf = util::string_format("%c*+", type);
	else
		buf = util::string_format("%c*", type);
	return buf;
}


static offs_t internal_disasm_tms57002(cpu_device *device, std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, int options)
{
	std::streampos original_pos = stream.tellp();
	uint32_t opcode = opram[0] | (opram[1] << 8) | (opram[2] << 16);
	uint8_t fa = opcode >> 18;
	if(fa == 0x3f) {
		switch((opcode >> 11) & 0x7f) { // category 3

#define DASM3
#include "cpu/tms57002/tms57002.hxx"
#undef  DASM3

		default:
			util::stream_format(stream, "unk c3 %02x", (opcode >> 11) & 0x7f);
			break;
		}
	} else {
		switch(fa) { // category 1
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

		switch((opcode >> 11) & 0x7f) { // category 2
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


CPU_DISASSEMBLE(tms57002)
{
	std::ostringstream stream;
	offs_t result = internal_disasm_tms57002(device, stream, pc, oprom, opram, options);
	std::string stream_str = stream.str();
	strcpy(buffer, stream_str.c_str());
	return result;
}
