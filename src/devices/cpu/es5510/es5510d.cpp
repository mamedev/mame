// license:BSD-3-Clause
// copyright-holders:Christian Brunschen
/***************************************************************************
 *
 *   es5510.c - Ensoniq ES5510 (ESP) emulation
 *   by Christian Brunschen
 *
 ***************************************************************************/

#include "emu.h"
#include "es5510d.h"

u32 es5510_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t es5510_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	return 1;
}
