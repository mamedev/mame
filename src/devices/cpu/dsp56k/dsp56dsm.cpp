// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
/***************************************************************************

    dsp56dsm.c
    Disassembler for the portable Motorola/Freescale dsp56k emulator.
    Written by Andrew Gardner

***************************************************************************/

#include "opcode.h"

#include "emu.h"
#include "dsp56k.h"

/*****************************/
/* Main disassembly function */
/*****************************/
static offs_t internal_disasm_dsp56k(cpu_device *device, std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, int options)
{
	const uint16_t w0 = oprom[0] | (oprom[1] << 8);
	const uint16_t w1 = oprom[2] | (oprom[3] << 8);

	// Decode and disassemble.
	DSP56K::Opcode op(w0, w1);
	stream << op.disassemble();

	const unsigned size = op.size();
	return (size | DASMFLAG_SUPPORTED);
}


CPU_DISASSEMBLE(dsp56k)
{
	std::ostringstream stream;
	offs_t result = internal_disasm_dsp56k(device, stream, pc, oprom, opram, options);
	std::string stream_str = stream.str();
	strcpy(buffer, stream_str.c_str());
	return result;
}
