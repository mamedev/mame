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

char const* const unsp_disassembler::jumps[] =
{
	"jb", "jae", "jge", "jl", "jne", "je", "jpl", "jmi",
	"jbe", "ja", "jle", "jg", "jvc", "jvs", "jmp", "<inv>"
};

offs_t unsp_disassembler::disassemble_jumps(std::ostream &stream, offs_t pc, const uint16_t op)
{
	uint32_t len = 1;
	const uint16_t op0 = (op >> 12) & 15;
	const uint16_t op1 = (op >> 6) & 7;
	const uint32_t opimm = op & 0x3f;

	if (op1 == 0)
	{
		util::stream_format(stream, "%s %04x", jumps[op0], pc + 1 + opimm);
		return UNSP_DASM_OK;
	}
	else if (op1 == 1)
	{
		util::stream_format(stream, "%s %04x", jumps[op0], pc + 1 - opimm);
		return UNSP_DASM_OK;
	}

	return UNSP_DASM_OK;
}
