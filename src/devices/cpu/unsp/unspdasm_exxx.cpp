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

offs_t unsp_disassembler::disassemble_exxx_group(std::ostream& stream, offs_t pc, uint16_t op, uint16_t ximm)
{
	uint32_t len = 1;
	// several exxx opcodes have already been decoded as jumps by the time we get here
	util::stream_format(stream, "<DUNNO>");
	return UNSP_DASM_OK;
}

offs_t unsp_12_disassembler::disassemble_exxx_group(std::ostream& stream, offs_t pc, uint16_t op, uint16_t ximm)
{
	uint32_t len = 1;
	// several exxx opcodes have already been decoded as jumps by the time we get here

	//   Register BITOP  BITOP Rd,Rs            1 1 1 0   r r r 0   0 0 b b   0 r r r
	//   Register BITOP  BITOP Rd,offset        1 1 1 0   r r r 0   0 1 b b   o o o o
	//   Memory BITOP    BITOP [Rd], offset     1 1 1 0   r r r 1   1 0 b b   o o o o
	//   Memory BITOP    BITOP ds:[Rd], offset   1 1 1 0   r r r 1   1 1 b b   o o o o
	//   Memory BITOP    BITOP [Rd], Rs         1 1 1 0   r r r 1   0 0 b b   0 r r r
	//   Memory BITOP    BITOP ds:[Rd], Rs       1 1 1 0   r r r 1   0 1 b b   0 r r r

	if (((op & 0xf1c8) == 0xe000))
	{
		// Register BITOP  BITOP Rd,Rs
		uint8_t bitop = (op & 0x0030) >> 4;
		uint8_t rd =    (op & 0x0e00) >> 9;
		uint8_t rs =    (op & 0x0007) >> 0;
		util::stream_format(stream, "%s %s,%s", bitops[bitop], regs[rd], regs[rs]);
		return UNSP_DASM_OK;
	}
	else if (((op & 0xf1c0) == 0xe040))
	{
		// Register BITOP  BITOP Rd,offset
		uint8_t bitop =  (op & 0x0030) >> 4;
		uint8_t rd =     (op & 0x0e00) >> 9;
		uint8_t offset = (op & 0x000f) >> 0;
		util::stream_format(stream, "%s %s,%d", bitops[bitop], regs[rd], offset);
		return UNSP_DASM_OK;
	}
	else if (((op & 0xf1c0) == 0xe180))
	{
		// Memory BITOP    BITOP [Rd], offset
		uint8_t bitop =  (op & 0x0030) >> 4;
		uint8_t rd =     (op & 0x0e00) >> 9;
		uint8_t offset = (op & 0x000f) >> 0;
		util::stream_format(stream, "%s [%s],%d", bitops[bitop], regs[rd], offset);
		return UNSP_DASM_OK;
	}
	else if (((op & 0xf1c0) == 0xe1c0))
	{
		// Memory BITOP    BITOP ds:[Rd], offset
		uint8_t bitop =  (op & 0x0030) >> 4;
		uint8_t rd =     (op & 0x0e00) >> 9;
		uint8_t offset = (op & 0x000f) >> 0;
		util::stream_format(stream, "%s ds:[%s],%d", bitops[bitop], regs[rd], offset);
		return UNSP_DASM_OK;
	}
	else if (((op & 0xf1c8) == 0xe100))
	{
		// Memory BITOP    BITOP [Rd], Rs
		uint8_t bitop = (op & 0x0030) >> 4;
		uint8_t rd =    (op & 0x0e00) >> 9;
		uint8_t rs =    (op & 0x0007) >> 0;
		util::stream_format(stream, "%s [%s],%s", bitops[bitop], regs[rd], regs[rs]);
		return UNSP_DASM_OK;
	}
	else if (((op & 0xf1c8) == 0xe140))
	{
		// Memory BITOP    BITOP ds:[Rd], Rs
		uint8_t bitop = (op & 0x0030) >> 4;
		uint8_t rd =    (op & 0x0e00) >> 9;
		uint8_t rs =    (op & 0x0007) >> 0;
		util::stream_format(stream, "%s ds:[%s],%s", bitops[bitop], regs[rd], regs[rs]);
		return UNSP_DASM_OK;
	}
	else if (((op & 0xf0f8) == 0xe008))
	{
		// MUL operations
		// MUL      1 1 1 0*  r r r S*  0 0 0 0   1 r r r     (* = sign bit, fixed here)
		print_mul(stream, op); // MUL uu or MUL su (invalid?)
		return UNSP_DASM_OK;
	}
	else if (((op & 0xf080) == 0xe080))
	{
		// MULS     1 1 1 0*  r r r S*  1 s s s   s r r r    (* = sign bit, fixed here)

		// MULS uu or MULS su (invalid?)
		print_muls(stream, op);
		return UNSP_DASM_OK;
	}
	else if (((op & 0xf188) == 0xe108))
	{
		// 16 bit Shift    1 1 1 0   r r r 1   0 l l l   1 r r r
		uint8_t rd =    (op & 0x0e00) >> 9;
		uint8_t shift = (op & 0x0070) >> 4;
		uint8_t rs =    (op & 0x0007) >> 0;
		util::stream_format(stream, "%s = %s %s %s", regs[rd], regs[rd], lsft[shift], regs[rs]);
		return UNSP_DASM_OK;
	}

	util::stream_format(stream, "<DUNNO>");
	return UNSP_DASM_OK;
}
