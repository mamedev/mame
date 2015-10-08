// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
    First-gen DEC PDP-8 disassembler

    Written by Ryan Holtz
*/

#include "emu.h"

static char *output;

offs_t pdp8_dasm_one(char *buffer, offs_t pc, UINT16 op)
{
	UINT8 opcode = (op >> 011) & 07;
	UINT16 current_page = pc & 07600;
	UINT16 zero_addr = op & 0177;
	UINT16 current_addr = current_page | zero_addr;
	bool indirect = (op & 0400) ? true : false;
	bool zero_page = (op & 0200) ? false : true;

	output = buffer;

	switch (opcode)
	{
		case 0:
			output += sprintf(buffer, "AND %c %05o", indirect ? 'I' : ' ', zero_page ? zero_addr : current_addr);
			break;
		case 1:
			output += sprintf(buffer, "TAD %c %05o", indirect ? 'I' : ' ', zero_page ? zero_addr : current_addr);
			break;
		case 2:
			output += sprintf(buffer, "ISZ %c %05o", indirect ? 'I' : ' ', zero_page ? zero_addr : current_addr);
			break;
		case 3:
			output += sprintf(buffer, "DCA %c %05o", indirect ? 'I' : ' ', zero_page ? zero_addr : current_addr);
			break;
		case 4:
			output += sprintf(buffer, "JMS %c %05o", indirect ? 'I' : ' ', zero_page ? zero_addr : current_addr);
			break;
		case 5:
			output += sprintf(buffer, "JMP %c %05o", indirect ? 'I' : ' ', zero_page ? zero_addr : current_addr);
			break;
		case 6:
			output += sprintf(buffer, "IOT %03o %01o", (op >> 03) & 077, op & 07);
			break;
		case 7:
		{
			bool group2 = ((op & 0401) == 0400);
			if (!group2)
			{
				if (!(op & 0377))
				{
					output += sprintf(buffer, "NOP ");
				}
				else
				{
					if (op & 0200)
					{
						output += sprintf(buffer, "CLA ");
					}
					if (op & 0100)
					{
						output += sprintf(buffer, "CLL ");
					}
					if (op & 040)
					{
						output += sprintf(buffer, "CMA ");
					}
					if (op & 020)
					{
						output += sprintf(buffer, "CML ");
					}
					if (op & 01)
					{
						output += sprintf(buffer, "IAC ");
					}
					if (op & 010)
					{
						if (op & 02)
						{
							output += sprintf(buffer, "RTR ");
						}
						else
						{
							output += sprintf(buffer, "RAR ");
						}
					}
					if (op & 04)
					{
						if (op & 02)
						{
							output += sprintf(buffer, "RTL ");
						}
						else
						{
							output += sprintf(buffer, "RAL ");
						}
					}
				}
			}
			else
			{
				if (!(op & 0377))
				{
					output += sprintf(buffer, "NOP ");
				}
				else
				{
					if (op & 010)
					{
						if (!(op & 0160))
						{
							output += sprintf(buffer, "SKP ");
						}
						else
						{
							if (op & 0100)
							{
								output += sprintf(buffer, "SPA ");
							}
							if (op & 040)
							{
								output += sprintf(buffer, "SNA ");
							}
							if (op & 020)
							{
								output += sprintf(buffer, "SZL ");
							}
						}
					}
					else
					{
						if (op & 0100)
						{
							output += sprintf(buffer, "SMA ");
						}
						if (op & 040)
						{
							output += sprintf(buffer, "SZA ");
						}
						if (op & 020)
						{
							output += sprintf(buffer, "SNL ");
						}
					}
					if (op & 0200)
					{
						output += sprintf(buffer, "CLA ");
					}
					if (op & 04)
					{
						output += sprintf(buffer, "OSR ");
					}
					if (op & 02)
					{
						output += sprintf(buffer, "HLT ");
					}
				}
			}
		}
	}

	return 2 | DASMFLAG_SUPPORTED;
}

/*****************************************************************************/

CPU_DISASSEMBLE( pdp8 )
{
	UINT16 op = (*(UINT8 *)(opram + 0) << 8) |
				(*(UINT8 *)(opram + 1) << 0);
	return pdp8_dasm_one(buffer, pc, op);
}
