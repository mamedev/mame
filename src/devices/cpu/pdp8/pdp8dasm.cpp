// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
    First-gen DEC PDP-8 disassembler

    Written by Ryan Holtz
*/

#include "emu.h"
#include "pdp8dasm.h"

u32 pdp8_disassembler::opcode_alignment() const
{
	return 2;
}

offs_t pdp8_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	uint16_t op = opcodes.r16(pc);
	uint8_t opcode = (op >> 011) & 07;
	uint16_t current_page = pc & 07600;
	uint16_t zero_addr = op & 0177;
	uint16_t current_addr = current_page | zero_addr;
	bool indirect = (op & 0400) ? true : false;
	bool zero_page = (op & 0200) ? false : true;

	switch (opcode)
	{
		case 0:
			util::stream_format(stream, "AND %c %05o", indirect ? 'I' : ' ', zero_page ? zero_addr : current_addr);
			break;
		case 1:
			util::stream_format(stream, "TAD %c %05o", indirect ? 'I' : ' ', zero_page ? zero_addr : current_addr);
			break;
		case 2:
			util::stream_format(stream, "ISZ %c %05o", indirect ? 'I' : ' ', zero_page ? zero_addr : current_addr);
			break;
		case 3:
			util::stream_format(stream, "DCA %c %05o", indirect ? 'I' : ' ', zero_page ? zero_addr : current_addr);
			break;
		case 4:
			util::stream_format(stream, "JMS %c %05o", indirect ? 'I' : ' ', zero_page ? zero_addr : current_addr);
			break;
		case 5:
			util::stream_format(stream, "JMP %c %05o", indirect ? 'I' : ' ', zero_page ? zero_addr : current_addr);
			break;
		case 6:
			util::stream_format(stream, "IOT %03o %01o", (op >> 03) & 077, op & 07);
			break;
		case 7:
		{
			bool group2 = ((op & 0401) == 0400);
			if (!group2)
			{
				if (!(op & 0377))
				{
					util::stream_format(stream, "NOP ");
				}
				else
				{
					if (op & 0200)
					{
						util::stream_format(stream, "CLA ");
					}
					if (op & 0100)
					{
						util::stream_format(stream, "CLL ");
					}
					if (op & 040)
					{
						util::stream_format(stream, "CMA ");
					}
					if (op & 020)
					{
						util::stream_format(stream, "CML ");
					}
					if (op & 01)
					{
						util::stream_format(stream, "IAC ");
					}
					if (op & 010)
					{
						if (op & 02)
						{
							util::stream_format(stream, "RTR ");
						}
						else
						{
							util::stream_format(stream, "RAR ");
						}
					}
					if (op & 04)
					{
						if (op & 02)
						{
							util::stream_format(stream, "RTL ");
						}
						else
						{
							util::stream_format(stream, "RAL ");
						}
					}
				}
			}
			else
			{
				if (!(op & 0377))
				{
					util::stream_format(stream, "NOP ");
				}
				else
				{
					if (op & 010)
					{
						if (!(op & 0160))
						{
							util::stream_format(stream, "SKP ");
						}
						else
						{
							if (op & 0100)
							{
								util::stream_format(stream, "SPA ");
							}
							if (op & 040)
							{
								util::stream_format(stream, "SNA ");
							}
							if (op & 020)
							{
								util::stream_format(stream, "SZL ");
							}
						}
					}
					else
					{
						if (op & 0100)
						{
							util::stream_format(stream, "SMA ");
						}
						if (op & 040)
						{
							util::stream_format(stream, "SZA ");
						}
						if (op & 020)
						{
							util::stream_format(stream, "SNL ");
						}
					}
					if (op & 0200)
					{
						util::stream_format(stream, "CLA ");
					}
					if (op & 04)
					{
						util::stream_format(stream, "OSR ");
					}
					if (op & 02)
					{
						util::stream_format(stream, "HLT ");
					}
				}
			}
		}
	}

	return 2 | SUPPORTED;
}
