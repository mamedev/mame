// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
/***************************************************************************

    dsp56dsm.cpp
    Disassembler for the portable Motorola/Freescale DSP56156 emulator.
    Written by Andrew Gardner

***************************************************************************/

#include "emu.h"
#include "opcode.h"
#include "dsp56dsm.h"

u32 dsp56156_disassembler::opcode_alignment() const
{
	return 1;
}

/*****************************/
/* Main disassembly function */
/*****************************/
offs_t dsp56156_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	const uint16_t w0 = opcodes.r16(pc);
	const uint16_t w1 = opcodes.r16(pc+1);

	// Decode and disassemble.
	DSP_56156::Opcode op(w0, w1);
	stream << op.disassemble();

	const unsigned size = op.size();
	return (size | SUPPORTED);
}
