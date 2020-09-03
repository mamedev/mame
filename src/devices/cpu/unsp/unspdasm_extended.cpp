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
		// 2 param form

		// Ext Register Ra = Ra op Rb
		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rb = (ximm & 0x000f) >> 0;
		uint8_t ra = (ximm & 0x0e00) >> 9;
		ra |= (ximm & 0x0100) >> 5;

		switch (aluop)
		{
		case 0x00: // add
			// A += B
			util::stream_format(stream, "(Extended group 0) %s += %s", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
																	 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
			break;

		case 0x01: // adc
			// A += B, Carry
			util::stream_format(stream, "(Extended group 0) %s += %s, carry", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
																			, (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
			break;

		case 0x02: // sub
			// A -= B
			util::stream_format(stream, "(Extended group 0) %s -= %s", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
																	 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
			break;

		case 0x03: // sbc
			// A -= B, Carry
			util::stream_format(stream, "(Extended group 0) %s -= %s, carry", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
																			, (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
			break;

		case 0x04: // cmp
			// CMP A,B
			util::stream_format(stream, "(Extended group 0) cmp %s, %s", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
																	   , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
			break;

		case 0x06: // neg
			// A = -B
			util::stream_format(stream, "(Extended group 0) %s = -%s", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
																	 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
			break;

		case 0x08: // xor
			// A ^= B
			util::stream_format(stream, "(Extended group 0) %s ^= %s", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
																	 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
			break;

		case 0x09: // load
			// A = B
			util::stream_format(stream, "(Extended group 0) %s = %s", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
																	, (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
			break;

		case 0x0a: // or
			// A |= B
			util::stream_format(stream, "(Extended group 0) %s |= %s", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
																	 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
			break;

		case 0x0b: // and
			// A &= B
			util::stream_format(stream, "(Extended group 0) %s &= %s", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
																	 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
			break;

		case 0x0c: // test
			// TEST A,B
			util::stream_format(stream, "(Extended group 0) test %s, %s", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
																		, (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
			break;

		case 0x0d: // store
			// B = A
			util::stream_format(stream, "(Extended group 0) %s = %s", (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
																	, (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]);
			break;

		case 0x05: // invalid
		case 0x07: // invalid
		case 0x0e: // invalid
		case 0x0f: // invalid
			util::stream_format(stream, "(Extended group 0) <INVALID Ra = Ra op Rb form>");
			break;
		}

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

			int start = rx;
			int end = (start-(size-1))&7;

			util::stream_format(stream, "(Extended group) push %s, %s to [%s]",
					extregs[start],
					extregs[end],
					(rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
		}
		else
		{
			uint8_t rb =   (ximm & 0x000f) >> 0;
			uint8_t size = (ximm & 0x7000) >> 12;
			uint8_t rx   = (ximm & 0x0e00) >> 9;
			
			if (size == 0) size = 8;

			int start = (rx + 1)&7;
			int end = (start+(size-1))&7;

			util::stream_format(stream, "(Extended group) pop %s, %s from [%s]",
					extregs[start],
					extregs[end],
					(rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
		}
		return UNSP_DASM_OK;
	}
	case 0x04:  case 0x14:
	{
		// 3 param form

		// Ra=Rb op IMM16

		// A = Ra
		// B = Rb
		// C = IMM16

		len = 3;
		uint16_t imm16_2 = opcodes.r16(pc + 2);
		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rb = (ximm & 0x000f) >> 0;
		uint8_t ra = (ximm & 0x0e00) >> 9;
		ra |= (ximm & 0x0100) >> 5;

		switch (aluop)
		{
		case 0x00: // add
			// A = B + C
			util::stream_format(stream, "(Extended group 1) %s = %s + %04x", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
																		   , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
																		   , imm16_2);
			break;

		case 0x01: // adc
			// A = B + C, Carry
			util::stream_format(stream, "(Extended group 1) %s = %s + %04x, carry", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
																				  , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
																				  , imm16_2);
			break;

		case 0x02: // sub
			// A = B - C
			util::stream_format(stream, "(Extended group 1) %s = %s - %04x", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
																		   , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
																		   , imm16_2);
			break;

		case 0x03: // sbc
			// A = B - C, Carry
			util::stream_format(stream, "(Extended group 1) %s = %s - %04x, carry", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
																				  , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
																				  , imm16_2);
			break;

		case 0x04: // cmp
			// CMP B,C
			util::stream_format(stream, "(Extended group 1) cmp %s, %04x", (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
																		 , imm16_2);
			break;

		case 0x06: // neg
			// A = -C
			util::stream_format(stream, "(Extended group 1) %s = -%04x", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
															   , imm16_2);
			break;

		case 0x08: // xor
			// A = B ^ C
			util::stream_format(stream, "(Extended group 1) %s = %s ^ %04x", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
																		   , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
																		   , imm16_2);
			break;

		case 0x09: // load
			// A = C
			util::stream_format(stream, "(Extended group 1) %s = %04x", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
																	  , imm16_2);
			break;

		case 0x0a: // or
			// A = B | C
			util::stream_format(stream, "(Extended group 1) %s = %s | %04x", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
																		   , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
																		   , imm16_2);
			break;

		case 0x0b: // and
			// A = B & C
			util::stream_format(stream, "(Extended group 1) %s = %s & %04x", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
																		   , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
																		   , imm16_2);
			break;

		case 0x0c: // test
			// TEST B,C
			util::stream_format(stream, "(Extended group 1) test %s, %04x", (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
																		  , imm16_2);
			break;

		case 0x0d: // store, C = B, invalid for this mode (would store to immediate)
		case 0x05: // invalid
		case 0x07: // invalid
		case 0x0e: // invalid
		case 0x0f: // invalid
			util::stream_format(stream, "(Extended group 1) <INVALID Ra=Rb op IMM16 form>");
			break;
		}

		return UNSP_DASM_OK;
	}

	case 0x06:
	case 0x16:
	{
		// 3 param form

		// Ra = Rb op [A16]

		// A = Ra
		// B = Rb
		// C = [A16]

		len = 3;
		uint16_t imm16_2 = opcodes.r16(pc + 2);
		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rb = (ximm & 0x000f) >> 0;
		uint8_t ra = (ximm & 0x0e00) >> 9;
		ra |= (ximm & 0x0100) >> 5;


		switch (aluop)
		{
		case 0x00: // add
			// A = B + C
			util::stream_format(stream, "(Extended group 2) %s = %s + [%04x]", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
																			 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
																			 , imm16_2);
			break;

		case 0x01: // adc
			// A = B + C, Carry
			util::stream_format(stream, "(Extended group 2) %s = %s + [%04x], carry", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
																					, (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
																					, imm16_2);
			break;

		case 0x02: // sub
			// A = B - C
			util::stream_format(stream, "(Extended group 2) %s = %s - [%04x]", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
																			 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
																			 , imm16_2);
			break;

		case 0x03: // sbc
			// A = B - C, Carry
			util::stream_format(stream, "(Extended group 2) %s = %s - [%04x], carry", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
																					, (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
																					, imm16_2);
			break;

		case 0x04: // cmp
			// CMP B,C
			util::stream_format(stream, "(Extended group 2) cmp %s, [%04x]", (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
																		   , imm16_2);
			break;

		case 0x06: // neg
			// A = -C
			util::stream_format(stream, "(Extended group 2) %s = -[%04x]", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
																		 , imm16_2);
			break;

		case 0x08: // xor
			// A = B ^ C
			util::stream_format(stream, "(Extended group 2) %s = %s ^ [%04x]", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
																			 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
																			 , imm16_2);
			break;

		case 0x09: // load
			// A = C
			util::stream_format(stream, "(Extended group 2) %s = [%04x]", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
																		, imm16_2);
			break;

		case 0x0a: // or
			// A = B | C
			util::stream_format(stream, "(Extended group 2) %s = %s | [%04x]", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
																			 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
																			 , imm16_2);
			break;

		case 0x0b: // and
			// A = B & C
			util::stream_format(stream, "(Extended group 2) %s = %s & [%04x]", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
																			 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
																			 , imm16_2);
			break;

		case 0x0c: // test
			// TEST B,C
			util::stream_format(stream, "(Extended group 2) test %s, [%04x]", (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
															   , imm16_2);
			break;

		case 0x0d: // store
			// C = B
			util::stream_format(stream, "(Extended group 2) [%04x] = %s", imm16_2
																		, (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
			break;

		case 0x05: // invalid
		case 0x07: // invalid
		case 0x0e: // invalid
		case 0x0f: // invalid
			util::stream_format(stream, "(Extended group 2) <INVALID Ra = Rb op [A16] form>");
			break;
		}


		return UNSP_DASM_OK;
	}

	case 0x07:
	case 0x17:
	{
		// 3 param form

		//[A16] = Ra op Rb

		// A = [A16]
		// B = Rb
		// C = Ra

		len = 3;
		uint16_t imm16_2 = opcodes.r16(pc + 2);
		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rb = (ximm & 0x000f) >> 0;
		uint8_t ra = (ximm & 0x0e00) >> 9;
		ra |= (ximm & 0x0100) >> 5;

		switch (aluop)
		{
		case 0x00: // add
			// A = B + C
			util::stream_format(stream, "(Extended group 3) [%04x] = %s + %s", imm16_2
																			 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
																			 , (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]);
			break;

		case 0x01: // adc
			// A = B + C, Carry
			util::stream_format(stream, "(Extended group 3) [%04x] = %s + %s, carry", imm16_2
																					, (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
																					, (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]);
			break;

		case 0x02: // sub
			// A = B - C
			util::stream_format(stream, "(Extended group 3) [%04x] = %s - %s", imm16_2
																			 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
																			 , (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]);
			break;

		case 0x03: // sbc
			// A = B - C, Carry
			util::stream_format(stream, "(Extended group 3) [%04x] = %s - %s, carry", imm16_2
																					, (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
																					, (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]);
			break;

		case 0x04: // cmp  (is this even a valid form? [A16] not even used)
			// CMP B,C
			util::stream_format(stream, "(Extended group 3) cmp %s, %s", (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
																	   , (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]);
			break;

		case 0x06: // neg
			// A = -C
			util::stream_format(stream, "(Extended group 3) [%04x] = -%s", imm16_2
																		 , (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]);
			break;

		case 0x08: // xor
			// A = B ^ C
			util::stream_format(stream, "(Extended group 3) [%04x] = %s ^ %s", imm16_2
																			 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
																			 , (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]);
			break;

		case 0x09: // load
			// A = C
			util::stream_format(stream, "(Extended group 3) [%04x] = %s", imm16_2
																		, (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]);
			break;

		case 0x0a: // or
			// A = B | C
			util::stream_format(stream, "(Extended group 3) [%04x] = %s | %s", imm16_2
																			 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
																			 , (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]);
			break;

		case 0x0b: // and
			// A = B & C
			util::stream_format(stream, "(Extended group 3) [%04x] = %s & %s", imm16_2
																			 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
																			 , (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]);
			break;

		case 0x0c: // test  (is this even a valid form? [A16] not even used)
			// TEST B,C
			util::stream_format(stream, "(Extended group 3) test %s, %s", (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
																		, (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]);
			break;

		case 0x0d: // store  (is this even a valid form? [A16] not even used)
			// C = B
			util::stream_format(stream, "(Extended group 3) [%04x] = %s", imm16_2
																		, (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
			break;

		case 0x05: // invalid
		case 0x07: // invalid
		case 0x0e: // invalid
		case 0x0f: // invalid
			util::stream_format(stream, "(Extended group 3) <INVALID [A16] = Ra op Rb form>");
			break;
		}

		return UNSP_DASM_OK;
	}

	case 0x08: case 0x09:
	{
		// Ext Indirect Rx=Rx op [Ry@]

		// A = Rx
		// B = [Ry@]

		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t ry = (ximm & 0x0007) >> 0;
		uint8_t form = (ximm & 0x0018) >> 3;
		uint8_t rx = (ximm & 0x0e00) >> 9;

		switch (aluop)
		{
		case 0x00: // add
			// A += B
			util::stream_format(stream, "(Extended group 4) %s += ", extregs[rx]);
			util::stream_format(stream, forms[form], extregs[ry]);
			break;

		case 0x01: // adc
			// A += B, Carry
			util::stream_format(stream, "(Extended group 4) %s += ", extregs[rx]);
			util::stream_format(stream, forms[form], extregs[ry]);
			util::stream_format(stream, ", carry");
			break;

		case 0x02: // sub
			// A -= B
			util::stream_format(stream, "(Extended group 4) %s -= ", extregs[rx]);
			util::stream_format(stream, forms[form], extregs[ry]);
			break;

		case 0x03: // sbc
			// A -= B, Carry
			util::stream_format(stream, "(Extended group 4) %s -= ", extregs[rx]);
			util::stream_format(stream, forms[form], extregs[ry]);
			util::stream_format(stream, ", carry");
			break;

		case 0x04: // cmp
			// CMP A,B
			util::stream_format(stream, "(Extended group 4) cmp %s, ", extregs[rx]);
			util::stream_format(stream, forms[form], extregs[ry]);
			break;

		case 0x06: // neg
			// A = -B
			util::stream_format(stream, "(Extended group 4) %s = -", extregs[rx]);
			util::stream_format(stream, forms[form], extregs[ry]);
			break;

		case 0x08: // xor
			// A ^= B
			util::stream_format(stream, "(Extended group 4) %s ^= ", extregs[rx]);
			util::stream_format(stream, forms[form], extregs[ry]);
			break;

		case 0x09: // load
			// A = B
			util::stream_format(stream, "(Extended group 4) %s = ", extregs[rx]);
			util::stream_format(stream, forms[form], extregs[ry]);
			break;

		case 0x0a: // or
			// A |= B
			util::stream_format(stream, "(Extended group 4) %s |= ", extregs[rx]);
			util::stream_format(stream, forms[form], extregs[ry]);
			break;

		case 0x0b: // and
			// A &= B
			util::stream_format(stream, "(Extended group 4) %s &= ", extregs[rx]);
			util::stream_format(stream, forms[form], extregs[ry]);
			break;

		case 0x0c: // test
			// TEST A,B
			util::stream_format(stream, "(Extended group 4) test %s, ", extregs[rx]);
			util::stream_format(stream, forms[form], extregs[ry]);
			break;

		case 0x0d: // store
			// B = A
			util::stream_format(stream, "(Extended group 4) ");
			util::stream_format(stream, forms[form], extregs[ry]);
			util::stream_format(stream, " = %s", extregs[rx]);
			break;

		case 0x05: // invalid
		case 0x07: // invalid
		case 0x0e: // invalid
		case 0x0f: // invalid
			util::stream_format(stream, "(Extended group 4) <INVALID Indirect Rx=Rx op [Ry@] form>");
			break;
		}

		return UNSP_DASM_OK;
	}
	case 0x0a: case 0x0b:
	{
		// Ext DS_Indirect Rx=Rx op ds:[Ry@]

		// A = Rx
		// B = ds[Ry@]

		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t ry = (ximm & 0x0007) >> 0;
		uint8_t form = (ximm & 0x0018) >> 3;
		uint8_t rx = (ximm & 0x0e00) >> 9;

		switch (aluop)
		{
		case 0x00: // add
			// A += B
			util::stream_format(stream, "(Extended group 5) %s += ds:", extregs[rx]);
			util::stream_format(stream, forms[form], extregs[ry]);
			break;

		case 0x01: // adc
			// A += B, Carry
			util::stream_format(stream, "(Extended group 5) %s += ds:", extregs[rx]);
			util::stream_format(stream, forms[form], extregs[ry]);
			util::stream_format(stream, ", carry");
			break;

		case 0x02: // sub
			// A -= B
			util::stream_format(stream, "(Extended group 5) %s -= ds:", extregs[rx]);
			util::stream_format(stream, forms[form], extregs[ry]);
			break;

		case 0x03: // sbc
			// A -= B, Carry
			util::stream_format(stream, "(Extended group 5) %s -= ds:", extregs[rx]);
			util::stream_format(stream, forms[form], extregs[ry]);
			util::stream_format(stream, ", carry");
			break;

		case 0x04: // cmp
			// CMP A,B
			util::stream_format(stream, "(Extended group 5) cmp %s, ds:", extregs[rx]);
			util::stream_format(stream, forms[form], extregs[ry]);
			break;

		case 0x06: // neg
			// A = -B
			util::stream_format(stream, "(Extended group 5) %s = -ds:", extregs[rx]);
			util::stream_format(stream, forms[form], extregs[ry]);
			break;

		case 0x08: // xor
			// A ^= B
			util::stream_format(stream, "(Extended group 5) %s ^= ds:", extregs[rx]);
			util::stream_format(stream, forms[form], extregs[ry]);
			break;

		case 0x09: // load
			// A = B
			util::stream_format(stream, "(Extended group 5) %s = ds:", extregs[rx]);
			util::stream_format(stream, forms[form], extregs[ry]);
			break;

		case 0x0a: // or
			// A |= B
			util::stream_format(stream, "(Extended group 5) %s |= ds:", extregs[rx]);
			util::stream_format(stream, forms[form], extregs[ry]);
			break;

		case 0x0b: // and
			// A &= B
			util::stream_format(stream, "(Extended group 5) %s &= ds:", extregs[rx]);
			util::stream_format(stream, forms[form], extregs[ry]);
			break;

		case 0x0c: // test
			// TEST A,B
			util::stream_format(stream, "(Extended group 5) test %s, ds:", extregs[rx]);
			util::stream_format(stream, forms[form], extregs[ry]);
			break;

		case 0x0d: // store
			// B = A
			util::stream_format(stream, "(Extended group 5) ds:");
			util::stream_format(stream, forms[form], extregs[ry]);
			util::stream_format(stream, " = %s", extregs[rx]);
			break;

		case 0x05: // invalid
		case 0x07: // invalid
		case 0x0e: // invalid
		case 0x0f: // invalid
			util::stream_format(stream, "(Extended group 5) <INVALID DS_Indirect Rx=Rx op ds:[Ry@] form>");
			break;
		}

		return UNSP_DASM_OK;
	}
	case 0x18: case 0x19: case 0x1a: case 0x1b:
	{
		// Ext IM6 Rx=Rx op IM6

		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rx = (ximm & 0x0e00) >> 9;
		uint8_t imm6 = (ximm & 0x003f) >> 0;

		// A = Rx
		// B = IM6

		switch (aluop)
		{
		case 0x00: // add
			// A += B
			util::stream_format(stream, "(Extended group 6) %s += %02x", extregs[rx], imm6 );
			break;

		case 0x01: // adc
			// A += B, Carry
			util::stream_format(stream, "(Extended group 6) %s += %02x, carry", extregs[rx], imm6 );
			break;

		case 0x02: // sub
			// A -= B
			util::stream_format(stream, "(Extended group 6) %s -= %02x", extregs[rx], imm6 );
			break;

		case 0x03: // sbc
			// A -= B, Carry
			util::stream_format(stream, "(Extended group 6) %s -= %02x, carry", extregs[rx], imm6 );
			break;

		case 0x04: // cmp
			// CMP A,B
			util::stream_format(stream, "(Extended group 6) cmp %s, %02x", extregs[rx], imm6 );
			break;

		case 0x06: // neg
			// A = -B
			util::stream_format(stream, "(Extended group 6) %s = -%02x", extregs[rx], imm6 );
			break;

		case 0x08: // xor
			// A ^= B
			util::stream_format(stream, "(Extended group 6) %s ^= %02x", extregs[rx], imm6 );
			break;

		case 0x09: // load
			// A = B
			util::stream_format(stream, "(Extended group 6) %s = %02x", extregs[rx], imm6 );
			break;

		case 0x0a: // or
			// A |= B
			util::stream_format(stream, "(Extended group 6) %s |= %02x", extregs[rx], imm6 );
			break;

		case 0x0b: // and
			// A &= B
			util::stream_format(stream, "(Extended group 6) %s &= %02x", extregs[rx], imm6 );
			break;

		case 0x0c: // test
			// TEST A,B
			util::stream_format(stream, "(Extended group 6) test %s, %02x", extregs[rx], imm6 );
			break;

		case 0x0d: // store,  B = A (invalid for this type)
		case 0x05: // invalid
		case 0x07: // invalid
		case 0x0e: // invalid
		case 0x0f: // invalid
			util::stream_format(stream, "(Extended group 6) <INVALID IM6 Rx=Rx op IM6 form>");
			break;
		}

		return UNSP_DASM_OK;
	}

	case 0x0c: case 0x0d: case 0x0e: case 0x0f:
	{
		// Ext Base+Disp6 Rx=Rx op [BP+IM6]

		// A = Rx
		// B = [BP+IM6]

		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rx = (ximm & 0x0e00) >> 9;
		uint8_t imm6 = (ximm & 0x003f) >> 0;

		switch (aluop)
		{
		case 0x00: // add
			// A += B
			util::stream_format(stream, "(Extended group 7) %s += [BP+%02x]", extregs[rx], imm6 );
			break;

		case 0x01: // adc
			// A += B, Carry
			util::stream_format(stream, "(Extended group 7) %s += [BP+%02x], carry", extregs[rx], imm6 );
			break;

		case 0x02: // sub
			// A -= B
			util::stream_format(stream, "(Extended group 7) %s -= [BP+%02x]", extregs[rx], imm6 );
			break;

		case 0x03: // sbc
			// A -= B, Carry
			util::stream_format(stream, "(Extended group 7) %s -= [BP+%02x], carry", extregs[rx], imm6 );
			break;

		case 0x04: // cmp
			// CMP A,B
			util::stream_format(stream, "(Extended group 7) cmp %s, [BP+%02x]", extregs[rx], imm6 );
			break;

		case 0x06: // neg
			// A = -B
			util::stream_format(stream, "(Extended group 7) %s = -[BP+%02x]", extregs[rx], imm6 );
			break;

		case 0x08: // xor
			// A ^= B
			util::stream_format(stream, "(Extended group 7) %s ^= [BP+%02x]", extregs[rx], imm6 );
			break;

		case 0x09: // load
			// A = B
			util::stream_format(stream, "(Extended group 7) %s = [BP+%02x]", extregs[rx], imm6 );
			break;

		case 0x0a: // or
			// A |= B
			util::stream_format(stream, "(Extended group 7) %s |= [BP+%02x]", extregs[rx], imm6 );
			break;

		case 0x0b: // and
			// A &= B
			util::stream_format(stream, "(Extended group 7) %s &= [BP+%02x]", extregs[rx], imm6 );
			break;

		case 0x0c: // test
			// TEST A,B
			util::stream_format(stream, "(Extended group 7) test %s, [BP+%02x]", extregs[rx], imm6 );
			break;

		case 0x0d: // store
			// B = A
			util::stream_format(stream, "(Extended group 7) [BP+%02x] = %s", imm6, extregs[rx] );
			break;

		case 0x05: // invalid
		case 0x07: // invalid
		case 0x0e: // invalid
		case 0x0f: // invalid
			util::stream_format(stream, "(Extended group 7) <INVALID Base+Disp6 Rx=Rx op [BP+IM6] form>");
			break;
		}

		return UNSP_DASM_OK;
	}

	case 0x1c: case 0x1d: case 0x1e: case 0x1f:
	{
		// Ext A6 Rx=Rx op [A6]

		// A = Rx
		// B = [A6]

		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rx = (ximm & 0x0e00) >> 9;
		uint8_t a6 = (ximm & 0x003f) >> 0;

		switch (aluop)
		{
		case 0x00: // add
			// A += B
			util::stream_format(stream, "(Extended group 8) %s += [%02x]", extregs[rx], a6 );
			break;

		case 0x01: // adc
			// A += B, Carry
			util::stream_format(stream, "(Extended group 8) %s += [%02x], carry", extregs[rx], a6 );
			break;

		case 0x02: // sub
			// A -= B
			util::stream_format(stream, "(Extended group 8) %s -= [%02x]", extregs[rx], a6 );
			break;

		case 0x03: // sbc
			// A -= B, Carry
			util::stream_format(stream, "(Extended group 8) %s -= [%02x], carry", extregs[rx], a6 );
			break;

		case 0x04: // cmp
			// CMP A,B
			util::stream_format(stream, "(Extended group 8) cmp %s, [%02x]", extregs[rx], a6 );
			break;

		case 0x06: // neg
			// A = -B
			util::stream_format(stream, "(Extended group 8) %s = -[%02x]", extregs[rx], a6 );
			break;

		case 0x08: // xor
			// A ^= B
			util::stream_format(stream, "(Extended group 8) %s ^= [%02x]", extregs[rx], a6 );
			break;

		case 0x09: // load
			// A = B
			util::stream_format(stream, "(Extended group 8) %s = [%02x]", extregs[rx], a6 );
			break;

		case 0x0a: // or
			// A |= B
			util::stream_format(stream, "(Extended group 8) %s |= [%02x]", extregs[rx], a6 );
			break;

		case 0x0b: // and
			// A &= B
			util::stream_format(stream, "(Extended group 8) %s &= [%02x]", extregs[rx], a6 );
			break;

		case 0x0c: // test
			// TEST A,B
			util::stream_format(stream, "(Extended group 8) test %s, [%02x]", extregs[rx], a6 );
			break;

		case 0x0d: // store
			// B = A
			util::stream_format(stream, "(Extended group 8) [%02x] = %s", a6, extregs[rx] );
			break;

		case 0x05: // invalid
		case 0x07: // invalid
		case 0x0e: // invalid
		case 0x0f: // invalid
			util::stream_format(stream, "(Extended group 8) <INVALID A6 Rx=Rx op [A6] form>");
			break;
		}

		return UNSP_DASM_OK;
	}
	}

	util::stream_format(stream, "<UNKNOWN EXTENDED>");
	return UNSP_DASM_OK;
}
