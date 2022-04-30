// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    cop420ds.cpp

    National Semiconductor COP420 disassembler.

***************************************************************************/

#include "emu.h"
#include "cop420ds.h"

u32 cop420_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t cop420_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	uint8_t opcode = opcodes.r8(pc);
	uint8_t next_opcode = opcodes.r8(pc+1);
	uint16_t address;
	uint32_t flags = 0;
	int bytes = 1;

	if ((opcode >= 0x80 && opcode <= 0xBE) || (opcode >= 0xC0 && opcode <= 0xFE))
	{
		int page = pc >> 6;

		if (page == 2 || page == 3) //JP pages 2,3
		{
			address = (uint16_t)((pc & 0x380) | (opcode & 0x7F));
			util::stream_format(stream, "JP %03X", address);
		}
		else
		{
			if ((opcode & 0xC0) == 0xC0) //JP other pages
			{
				address = (uint16_t)((pc & 0x3C0) | (opcode & 0x3F));
				util::stream_format(stream, "JP %03X", address);
			}
			else                    //JSRP
			{
				address = (uint16_t)(0x80 | (opcode & 0x3F));
				util::stream_format(stream, "JSRP %03X", address);
				flags = STEP_OVER;
			}
		}
	}
	else if (opcode >= 0x08 && opcode <= 0x0F)
	{
		util::stream_format(stream, "LBI 0,%u", ((opcode & 0xF) + 1) & 0xF);
	}
	else if (opcode >= 0x18 && opcode <= 0x1F)
	{
		util::stream_format(stream, "LBI 1,%u", ((opcode & 0xF) + 1) & 0xF);
	}
	else if (opcode >= 0x28 && opcode <= 0x2F)
	{
		util::stream_format(stream, "LBI 2,%u", ((opcode & 0xF) + 1) & 0xF);
	}
	else if (opcode >= 0x38 && opcode <= 0x3F)
	{
		util::stream_format(stream, "LBI 3,%u", ((opcode & 0xF) + 1) & 0xF);
	}
	else if (opcode >= 0x51 && opcode <= 0x5F)
	{
		util::stream_format(stream, "AISC %u", opcode & 0xF);
		flags = STEP_COND;
	}
	else if (opcode >= 0x60 && opcode <= 0x63)
	{
		address = ((opcode & 0x03) << 8) | next_opcode;
		util::stream_format(stream, "JMP %03X", address);
		bytes = 2;
	}
	else if (opcode >= 0x68 && opcode <= 0x6B)
	{
		address = ((opcode & 0x03) << 8) | next_opcode;
		util::stream_format(stream, "JSR %03X", address);
		flags = STEP_OVER;
		bytes = 2;
	}
	else if (opcode >= 0x70 && opcode <= 0x7F)
	{
		util::stream_format(stream, "STII %u", opcode & 0xF);
	}
	else
	{
		switch (opcode)
		{
		case 0:
			util::stream_format(stream, "CLRA");
			break;

		case 1:
			util::stream_format(stream, "SKMBZ 0");
			flags = STEP_COND;
			break;

		case 2:
			util::stream_format(stream, "XOR");
			break;

		case 3:
			util::stream_format(stream, "SKMBZ 2");
			flags = STEP_COND;
			break;

		case 4:
			util::stream_format(stream, "XIS 0");
			flags = STEP_COND;
			break;

		case 5:
			util::stream_format(stream, "LD 0");
			break;

		case 6:
			util::stream_format(stream, "X 0");
			break;

		case 7:
			util::stream_format(stream, "XDS 0");
			flags = STEP_COND;
			break;

		case 0x10:
			util::stream_format(stream, "CASC");
			flags = STEP_COND;
			break;

		case 0x11:
			util::stream_format(stream, "SKMBZ 1");
			flags = STEP_COND;
			break;

		case 0x12:
			util::stream_format(stream, "XABR");
			break;

		case 0x13:
			util::stream_format(stream, "SKMBZ 3");
			flags = STEP_COND;
			break;

		case 0x14:
			util::stream_format(stream, "XIS 1");
			flags = STEP_COND;
			break;

		case 0x15:
			util::stream_format(stream, "LD 1");
			break;

		case 0x16:
			util::stream_format(stream, "X 1");
			break;

		case 0x17:
			util::stream_format(stream, "XDS 1");
			flags = STEP_COND;
			break;

		case 0x20:
			util::stream_format(stream, "SKC");
			flags = STEP_COND;
			break;

		case 0x21:
			util::stream_format(stream, "SKE");
			flags = STEP_COND;
			break;

		case 0x22:
			util::stream_format(stream, "SC");
			break;

		case 0x23:
			bytes = 2;

			if (next_opcode <= 0x3f)
			{
				address = (uint16_t)(next_opcode & 0x3F);
				util::stream_format(stream, "LDD %u,%u", ((address & 0x30) >> 4),address & 0x0F);
			}
			else if (next_opcode >= 0x80 && next_opcode <= 0xbf)
			{
				address = (uint16_t)(next_opcode & 0x3F);
				util::stream_format(stream, "XAD %u,%u", ((address & 0x30) >> 4),address & 0x0F);
			}
			else
			{
				util::stream_format(stream, "Invalid");
			}
			break;

		case 0x24:
			util::stream_format(stream, "XIS 2");
			flags = STEP_COND;
			break;

		case 0x25:
			util::stream_format(stream, "LD 2");
			break;

		case 0x26:
			util::stream_format(stream, "X 2");
			break;

		case 0x27:
			util::stream_format(stream, "XDS 2");
			flags = STEP_COND;
			break;

		case 0x30:
			util::stream_format(stream, "ASC");
			flags = STEP_COND;
			break;

		case 0x31:
			util::stream_format(stream, "ADD");
			break;

		case 0x32:
			util::stream_format(stream, "RC");
			break;

		case 0x33:
			bytes = 2;

			if (next_opcode >= 0x50 && next_opcode <= 0x5F)
			{
				util::stream_format(stream, "OGI %u", next_opcode & 0xF);
			}
			else if (next_opcode >= 0x60 && next_opcode <= 0x6F)
			{
				util::stream_format(stream, "LEI %u", next_opcode & 0xF);
			}
			else if (next_opcode >= 0x80 && next_opcode <= 0x8F)
			{
				util::stream_format(stream, "LBI 0,%u", next_opcode & 0xF);
			}
			else if (next_opcode >= 0x90 && next_opcode <= 0x9F)
			{
				util::stream_format(stream, "LBI 1,%u", next_opcode & 0xF);
			}
			else if (next_opcode >= 0xA0 && next_opcode <= 0xAF)
			{
				util::stream_format(stream, "LBI 2,%u", next_opcode & 0xF);
			}
			else if (next_opcode >= 0xB0 && next_opcode <= 0xBF)
			{
				util::stream_format(stream, "LBI 3,%u", next_opcode & 0xF);
			}
			else
			{
				switch (next_opcode)
				{
				case 0x01:
					util::stream_format(stream, "SKGBZ 0");
					flags = STEP_COND;
					break;

				case 0x03:
					util::stream_format(stream, "SKGBZ 2");
					flags = STEP_COND;
					break;

				case 0x11:
					util::stream_format(stream, "SKGBZ 1");
					flags = STEP_COND;
					break;

				case 0x13:
					util::stream_format(stream, "SKGBZ 3");
					flags = STEP_COND;
					break;

				case 0x21:
					util::stream_format(stream, "SKGZ");
					flags = STEP_COND;
					break;

				case 0x28:
					util::stream_format(stream, "ININ");
					break;

				case 0x29:
					util::stream_format(stream, "INIL");
					break;

				case 0x2A:
					util::stream_format(stream, "ING");
					break;

				case 0x2C:
					util::stream_format(stream, "CQMA");
					break;

				case 0x2E:
					util::stream_format(stream, "INL");
					break;

				case 0x3A:
					util::stream_format(stream, "OMG");
					break;

				case 0x3C:
					util::stream_format(stream, "CAMQ");
					break;

				case 0x3E:
					util::stream_format(stream, "OBD");
					break;

				default:
					util::stream_format(stream, "Invalid");
					break;
				}
			}
			break;

		case 0x34:
			util::stream_format(stream, "XIS 3");
			flags = STEP_COND;
			break;

		case 0x35:
			util::stream_format(stream, "LD 3");
			break;

		case 0x36:
			util::stream_format(stream, "X 3");
			break;

		case 0x37:
			util::stream_format(stream, "XDS 3");
			flags = STEP_COND;
			break;

		case 0x40:
			util::stream_format(stream, "COMP");
			break;

		case 0x41:
			util::stream_format(stream, "SKT");
			flags = STEP_COND;
			break;

		case 0x42:
			util::stream_format(stream, "RMB 2");
			break;

		case 0x43:
			util::stream_format(stream, "RMB 3");
			break;

		case 0x44:
			util::stream_format(stream, "NOP");
			break;

		case 0x45:
			util::stream_format(stream, "RMB 1");
			break;

		case 0x46:
			util::stream_format(stream, "SMB 2");
			break;

		case 0x47:
			util::stream_format(stream, "SMB 1");
			break;

		case 0x48:
			util::stream_format(stream, "RET");
			flags = STEP_OUT;
			break;

		case 0x49:
			util::stream_format(stream, "RETSK");
			flags = STEP_OUT;
			break;

		case 0x4A:
			util::stream_format(stream, "ADT");
			break;

		case 0x4B:
			util::stream_format(stream, "SMB 3");
			break;

		case 0x4C:
			util::stream_format(stream, "RMB 0");
			break;

		case 0x4D:
			util::stream_format(stream, "SMB 0");
			break;

		case 0x4E:
			util::stream_format(stream, "CBA");
			break;

		case 0x4F:
			util::stream_format(stream, "XAS");
			break;

		case 0x50:
			util::stream_format(stream, "CAB");
			break;

		case 0xBF:
			util::stream_format(stream, "LQID");
			break;

		case 0xFF:
			util::stream_format(stream, "JID");
			break;

		default:
			util::stream_format(stream, "Invalid");
			break;
		}
	}

	return bytes | flags | SUPPORTED;
}
