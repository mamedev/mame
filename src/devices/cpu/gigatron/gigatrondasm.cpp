// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Gigatron disassembler

***************************************************************************/

#include "emu.h"
#include "gigatrondasm.h"

gigatron_disassembler::gigatron_disassembler()
	: util::disasm_interface()
{
}

const char *const gigatron_disassembler::s_ops[7] = {
	"ld",
	"anda",
	"ora",
	"xora",
	"adda",
	"suba",
	"st"
};

const char *const gigatron_disassembler::s_jumps[8] = {
	"jmp",
	"bgt",
	"blt",
	"bne",
	"beq",
	"bge",
	"ble",
	"bra"
};

u32 gigatron_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t gigatron_disassembler::disassemble(std::ostream &stream, offs_t pc, const gigatron_disassembler::data_buffer &opcodes, const gigatron_disassembler::data_buffer &params)
{
	u16 inst = opcodes.r16(pc);
	u32 flags = 0;

	if (inst >= 0xe000)
	{
		// Jump instructions use special format
		util::stream_format(stream, "%-5s", s_jumps[(inst & 0x1c00) >> 10]);
		if ((inst & 0x1c00) == 0)
			stream << "y,";
		else if (inst < 0xfc00)
			flags = STEP_COND | step_over_extra(1);

		switch (inst & 0x0300)
		{
		case 0x0000:
			if ((inst & 0x1c00) == 0)
				util::stream_format(stream, "$%02x", inst & 0x00ff);
			else
				util::stream_format(stream, "$%04x", ((pc + 1) & 0x3f00) | (inst & 0x00ff));
			break;

		case 0x0100:
			util::stream_format(stream, "[$%02x]", inst & 0x00ff);
			break;

		case 0x0200:
			stream << "ac";
			break;

		case 0x0300:
			stream << "in";
			break;
		}
	}
	else if ((inst & 0xf300) == 0x0200)
		stream << "nop";
	else
	{
		if ((inst & 0xe300) == 0xc100)
		{
			// This was originally an undefined store mode
			util::stream_format(stream, "%-5s", "ctrl");
		}
		else
		{
			util::stream_format(stream, "%-5s", s_ops[(inst & 0xe000) >> 13]);

			// Bus data
			switch (inst & 0x0300)
			{
			case 0x0000:
				util::stream_format(stream, "$%02x", inst & 0x00ff);
				if (inst >= 0xc000)
					stream << ",";
				break;

			case 0x0100:
				break;

			case 0x0200:
				if (inst < 0xc000)
					stream << "ac"; // implicit for store instruction
				break;

			case 0x0300:
				stream << "in";
				if (inst >= 0xc000)
					stream << ",";
				break;
			}
		}

		// RAM source or store destination
		if (inst >= 0xc000 || (inst & 0x0300) == 0x0100)
		{
			if ((inst & 0xe300) != 0xc100)
				stream << "[";

			switch (inst & 0x1c00)
			{
			case 0x0000: case 0x1000: case 0x1400: case 0x1800:
				util::stream_format(stream, "$%02x", inst & 0x00ff);
				break;

			case 0x0400:
				stream << "x";
				break;

			case 0x0800:
				util::stream_format(stream, "y,$%02x", inst & 0x00ff);
				break;

			case 0x0c00:
				stream << "y,x";
				break;

			case 0x1c00:
				stream << "y,x++";
				break;
			}

			if ((inst & 0xe300) != 0xc100)
				stream << "]";
		}

		// Non-accumulator destinations
		if (BIT(inst, 12))
		{
			if (!BIT(inst, 11))
				util::stream_format(stream, ",%c", "xy"[BIT(inst, 10)]);
			else if (inst < 0xc000)
				stream << ",out";
		}
	}

	return 1 | flags | SUPPORTED;
}
