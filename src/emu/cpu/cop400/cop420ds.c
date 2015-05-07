// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    cop420ds.c

    National Semiconductor COP420 Emulator.

***************************************************************************/

#include "emu.h"

CPU_DISASSEMBLE( cop420 )
{
	UINT8 opcode = oprom[0];
	UINT8 next_opcode = oprom[1];
	UINT16 address;
	UINT32 flags = 0;
	int bytes = 1;

	if ((opcode >= 0x80 && opcode <= 0xBE) || (opcode >= 0xC0 && opcode <= 0xFE))
	{
		if ((pc & 0x3E0) >= 0x80 && (pc & 0x3E0) < 0x100) //JP pages 2,3
		{
			address = (UINT16)((pc & 0x380) | (opcode & 0x7F));
			sprintf(buffer, "JP %x", address);
		}
		else
		{
			if ((opcode & 0xC0) == 0xC0) //JP other pages
			{
				address = (UINT16)((pc & 0x3C0) | (opcode & 0x3F));
				sprintf(buffer, "JP %x", address);
			}
			else                    //JSRP
			{
				address = (UINT16)(0x80 | (opcode & 0x3F));
				sprintf(buffer, "JSRP %x", address);
				flags = DASMFLAG_STEP_OVER;
			}
		}
	}
	else if (opcode >= 0x08 && opcode <= 0x0F)
	{
		sprintf(buffer, "LBI 0,%u", ((opcode & 0xF) + 1) & 0xF);
	}
	else if (opcode >= 0x18 && opcode <= 0x1F)
	{
		sprintf(buffer, "LBI 1,%u", ((opcode & 0xF) + 1) & 0xF);
	}
	else if (opcode >= 0x28 && opcode <= 0x2F)
	{
		sprintf(buffer, "LBI 2,%u", ((opcode & 0xF) + 1) & 0xF);
	}
	else if (opcode >= 0x38 && opcode <= 0x3F)
	{
		sprintf(buffer, "LBI 3,%u", ((opcode & 0xF) + 1) & 0xF);
	}
	else if (opcode >= 0x51 && opcode <= 0x5F)
	{
		sprintf(buffer, "AISC %u", opcode & 0xF);
	}
	else if (opcode >= 0x60 && opcode <= 0x63)
	{
		address = ((opcode & 0x03) << 8) | next_opcode;
		sprintf(buffer, "JMP %x", address);
		bytes = 2;
	}
	else if (opcode >= 0x68 && opcode <= 0x6B)
	{
		address = ((opcode & 0x03) << 8) | next_opcode;
		sprintf(buffer, "JSR %x", address);
		flags = DASMFLAG_STEP_OVER;
		bytes = 2;
	}
	else if (opcode >= 0x70 && opcode <= 0x7F)
	{
		sprintf(buffer, "STII %u", opcode & 0xF);
	}
	else
	{
		switch (opcode)
		{
		case 0:
			sprintf(buffer, "CLRA");
			break;

		case 1:
			sprintf(buffer, "SKMBZ 0");
			break;

		case 2:
			sprintf(buffer, "XOR");
			break;

		case 3:
			sprintf(buffer, "SKMBZ 2");
			break;

		case 4:
			sprintf(buffer, "XIS 0");
			break;

		case 5:
			sprintf(buffer, "LD 0");
			break;

		case 6:
			sprintf(buffer, "X 0");
			break;

		case 7:
			sprintf(buffer, "XDS 0");
			break;

		case 0x10:
			sprintf(buffer, "CASC");
			break;

		case 0x11:
			sprintf(buffer, "SKMBZ 1");
			break;

		case 0x12:
			sprintf(buffer, "XABR");
			break;

		case 0x13:
			sprintf(buffer, "SKMBZ 3");
			break;

		case 0x14:
			sprintf(buffer, "XIS 1");
			break;

		case 0x15:
			sprintf(buffer, "LD 1");
			break;

		case 0x16:
			sprintf(buffer, "X 1");
			break;

		case 0x17:
			sprintf(buffer, "XDS 1");
			break;

		case 0x20:
			sprintf(buffer, "SKC");
			break;

		case 0x21:
			sprintf(buffer, "SKE");
			break;

		case 0x22:
			sprintf(buffer, "SC");
			break;

		case 0x23:
			bytes = 2;

			if (next_opcode <= 0x3f)
			{
				address = (UINT16)(next_opcode & 0x3F);
				sprintf(buffer, "LDD %x,%x", ((address & 0x30) >> 4),address & 0x0F);
			}
			else if (next_opcode >= 0x80 && next_opcode <= 0xbf)
			{
				address = (UINT16)(next_opcode & 0x3F);
				sprintf(buffer, "XAD %x,%x", ((address & 0x30) >> 4),address & 0x0F);
			}
			else
			{
				sprintf(buffer, "Invalid");
			}
			break;

		case 0x24:
			sprintf(buffer, "XIS 2");
			break;

		case 0x25:
			sprintf(buffer, "LD 2");
			break;

		case 0x26:
			sprintf(buffer, "X 2");
			break;

		case 0x27:
			sprintf(buffer, "XDS 2");
			break;

		case 0x30:
			sprintf(buffer, "ASC");
			break;

		case 0x31:
			sprintf(buffer, "ADD");
			break;

		case 0x32:
			sprintf(buffer, "RC");
			break;

		case 0x33:
			bytes = 2;

			if (next_opcode >= 0x50 && next_opcode <= 0x5F)
			{
				sprintf(buffer, "OGI %u", next_opcode & 0xF);
			}
			else if (next_opcode >= 0x60 && next_opcode <= 0x6F)
			{
				sprintf(buffer, "LEI %u", next_opcode & 0xF);
			}
			else if (next_opcode >= 0x80 && next_opcode <= 0x8F)
			{
				sprintf(buffer, "LBI 0,%u", next_opcode & 0xF);
			}
			else if (next_opcode >= 0x90 && next_opcode <= 0x9F)
			{
				sprintf(buffer, "LBI 1,%u", next_opcode & 0xF);
			}
			else if (next_opcode >= 0xA0 && next_opcode <= 0xAF)
			{
				sprintf(buffer, "LBI 2,%u", next_opcode & 0xF);
			}
			else if (next_opcode >= 0xB0 && next_opcode <= 0xBF)
			{
				sprintf(buffer, "LBI 3,%u", next_opcode & 0xF);
			}
			else
			{
				switch (next_opcode)
				{
				case 0x01:
					sprintf(buffer, "SKGBZ 0");
					break;

				case 0x03:
					sprintf(buffer, "SKGBZ 2");
					break;

				case 0x11:
					sprintf(buffer, "SKGBZ 1");
					break;

				case 0x13:
					sprintf(buffer, "SKGBZ 3");
					break;

				case 0x21:
					sprintf(buffer, "SKGZ");
					break;

				case 0x28:
					sprintf(buffer, "ININ");
					break;

				case 0x29:
					sprintf(buffer, "INIL");
					break;

				case 0x2A:
					sprintf(buffer, "ING");
					break;

				case 0x2C:
					sprintf(buffer, "CQMA");
					break;

				case 0x2E:
					sprintf(buffer, "INL");
					break;

				case 0x3A:
					sprintf(buffer, "OMG");
					break;

				case 0x3C:
					sprintf(buffer, "CAMQ");
					break;

				case 0x3E:
					sprintf(buffer, "OBD");
					break;

				default:
					sprintf(buffer, "Invalid");
					break;
				}
			}
			break;

		case 0x34:
			sprintf(buffer, "XIS 3");
			break;

		case 0x35:
			sprintf(buffer, "LD 3");
			break;

		case 0x36:
			sprintf(buffer, "X 3");
			break;

		case 0x37:
			sprintf(buffer, "XDS 3");
			break;

		case 0x40:
			sprintf(buffer, "COMP");
			break;

		case 0x41:
			sprintf(buffer, "SKT");
			break;

		case 0x42:
			sprintf(buffer, "RMB 2");
			break;

		case 0x43:
			sprintf(buffer, "RMB 3");
			break;

		case 0x44:
			sprintf(buffer, "NOP");
			break;

		case 0x45:
			sprintf(buffer, "RMB 1");
			break;

		case 0x46:
			sprintf(buffer, "SMB 2");
			break;

		case 0x47:
			sprintf(buffer, "SMB 1");
			break;

		case 0x48:
			sprintf(buffer, "RET");
			flags = DASMFLAG_STEP_OUT;
			break;

		case 0x49:
			sprintf(buffer, "RETSK");
			flags = DASMFLAG_STEP_OUT;
			break;

		case 0x4A:
			sprintf(buffer, "ADT");
			break;

		case 0x4B:
			sprintf(buffer, "SMB 3");
			break;

		case 0x4C:
			sprintf(buffer, "RMB 0");
			break;

		case 0x4D:
			sprintf(buffer, "SMB 0");
			break;

		case 0x4E:
			sprintf(buffer, "CBA");
			break;

		case 0x4F:
			sprintf(buffer, "XAS");
			break;

		case 0x50:
			sprintf(buffer, "CAB");
			break;

		case 0xBF:
			sprintf(buffer, "LQID");
			break;

		case 0xFF:
			sprintf(buffer, "JID");
			break;

		default:
			sprintf(buffer, "Invalid");
			break;
		}
	}

	return bytes | flags | DASMFLAG_SUPPORTED;
}
