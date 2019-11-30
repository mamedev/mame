// license:GPL-2.0+
// copyright-holders:Segher Boessenkool, David Haywood
/*****************************************************************************

    SunPlus µ'nSP disassembler

    Copyright 2008-2017  Segher Boessenkool  <segher@kernel.crashing.org>
    Licensed under the terms of the GNU GPL, version 2
    http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

*****************************************************************************/

#include "emu.h"
#include "unspdasm.h"

offs_t unsp_disassembler::disassemble_extended_group(std::ostream& stream, offs_t pc, uint16_t op, uint16_t ximm, const data_buffer& opcodes)
{
	// shouldn't get here anyway
	int len = 1;
	util::stream_format(stream, "<UNKNOWN EXTENDED>");
	return UNSP_DASM_OK;
}

offs_t unsp_20_disassembler::disassemble_extended_group(std::ostream& stream, offs_t pc, uint16_t op, uint16_t ximm,const data_buffer &opcodes)
{
	uint32_t len = 2;

	switch ((ximm & 0x01f0) >> 4)
	{
	case 0x00: case 0x10:
	{
		// Ext Register Ra = Ra op Rb
		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rb = (ximm & 0x000f) >> 0;
		uint8_t ra = (ximm & 0x0e00) >> 9;
		ra |= (ximm & 0x0100) >> 5;

		util::stream_format(stream, "(Ext) %s = %s %s %s", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
														 , (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
														 , aluops[aluop]
														 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
		return UNSP_DASM_OK;
	}
	case 0x02:
	{
		// register decoding could be incorrect here

		// Ext Push/Pop
		if (ximm & 0x8000)
		{
			uint8_t rb =   (ximm & 0x000f) >> 0;
			uint8_t size = (ximm & 0x7000) >> 12;
			uint8_t rx   = (ximm & 0x0e00) >> 9;

			if (size == 0) size = 8;
			size -= 1;

			if ((rx-size) >= 0)
				util::stream_format(stream, "(Ext) push %s, %s to [%s]",
					   extregs[rx-size], extregs[rx], (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
			else
				util::stream_format(stream, "(Ext) push <BAD>");
		}
		else
		{
			uint8_t rb =   (ximm & 0x000f) >> 0;
			uint8_t size = (ximm & 0x7000) >> 12;
			uint8_t rx   = (ximm & 0x0e00) >> 9;

			if (size == 0) size = 8;
			size -= 1;

			if ((rx-size) >= 0)
				util::stream_format(stream, "(Ext) pop %s, %s from [%s]",
					   extregs[rx-size], extregs[rx], (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
			else
				util::stream_format(stream, "(Ext) pop <BAD>");
		}
		return UNSP_DASM_OK;
	}
	case 0x04:  case 0x14:
	{
		// Ra=Rb op IMM16
		len = 3;
		uint16_t imm16_2 = opcodes.r16(pc + 2);
		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rb = (ximm & 0x000f) >> 0;
		uint8_t ra = (ximm & 0x0e00) >> 9;
		ra |= (ximm & 0x0100) >> 5;

		util::stream_format(stream, "(Ext) %s = %s %s %04x", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
														   , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
														   , aluops[aluop]
														   , imm16_2);
		return UNSP_DASM_OK;
	}

	case 0x06:
	case 0x16:
	{
		// Ra=Rb op [A16]
		len = 3;
		uint16_t imm16_2 = opcodes.r16(pc + 2);
		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rb = (ximm & 0x000f) >> 0;
		uint8_t ra = (ximm & 0x0e00) >> 9;
		ra |= (ximm & 0x0100) >> 5;

		util::stream_format(stream, "(Ext) %s = %s %s [%04x]", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
															 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
															 , aluops[aluop]
															 , imm16_2);
		return UNSP_DASM_OK;
	}

	case 0x07:
	case 0x17:
	{
		//[A16] = Ra op Rb
		len = 3;
		uint16_t imm16_2 = opcodes.r16(pc + 2);
		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rb = (ximm & 0x000f) >> 0;
		uint8_t ra = (ximm & 0x0e00) >> 9;
		ra |= (ximm & 0x0100) >> 5;

		util::stream_format(stream, "(Ext) [0x4x] = %s %s %s", imm16_2
															 , (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
															 , aluops[aluop]
															 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
		return UNSP_DASM_OK;
	}

	case 0x08: case 0x09:
	{
		// Ext Indirect Rx=Rx op [Ry@]

		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t ry = (ximm & 0x0007) >> 0;
		uint8_t form = (ximm & 0x0018) >> 3;
		uint8_t rx = (ximm & 0x0e00) >> 9;

		util::stream_format(stream, "(Ext) %s=%s %s", extregs[rx], extregs[rx], aluops[aluop]);
		util::stream_format(stream, forms[form], extregs[ry]);

		return UNSP_DASM_OK;
	}
	case 0x0a: case 0x0b:
	{
		// Ext DS_Indirect Rx=Rx op ds:[Ry@]

		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t ry = (ximm & 0x0007) >> 0;
		uint8_t form = (ximm & 0x0018) >> 3;
		uint8_t rx = (ximm & 0x0e00) >> 9;

		util::stream_format(stream, "(Ext) %s=%s %s ds:", extregs[rx], extregs[rx], aluops[aluop]);
		util::stream_format(stream, forms[form], extregs[ry]);

		return UNSP_DASM_OK;
	}
	case 0x18: case 0x19: case 0x1a: case 0x1b:
	{
		// Ext IM6 Rx=Rx op IM6

		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rx = (ximm & 0x0e00) >> 9;
		uint8_t imm6 = (ximm & 0x003f) >> 0;

		util::stream_format(stream, "(Ext) %s=%s %s %02x", extregs[rx], extregs[rx], aluops[aluop], imm6 );
		return UNSP_DASM_OK;
	}

	case 0x0c: case 0x0d: case 0x0e: case 0x0f:
	{
		// Ext Base+Disp6 Rx=Rx op [BP+IM6]

		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rx = (ximm & 0x0e00) >> 9;
		uint8_t imm6 = (ximm & 0x003f) >> 0;

		util::stream_format(stream, "(Ext) %s=%s %s [BP+%02x]", extregs[rx], extregs[rx], aluops[aluop], imm6 );
		return UNSP_DASM_OK;
	}

	case 0x1c: case 0x1d: case 0x1e: case 0x1f:
	{
		// Ext A6 Rx=Rx op [A6]

		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rx = (ximm & 0x0e00) >> 9;
		uint8_t a6 = (ximm & 0x003f) >> 0;

		util::stream_format(stream, "(Ext) %s=%s %s [%02x]", extregs[rx], extregs[rx], aluops[aluop], a6 );
		return UNSP_DASM_OK;
	}
	}

	util::stream_format(stream, "<UNKNOWN EXTENDED>");
	return UNSP_DASM_OK;
}
