// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
/***************************************************************************

    dsp56dsm.c
    Disassembler for the portable Motorola/Freescale dsp56k emulator.
    Written by Andrew Gardner

***************************************************************************/

#include "emu.h"
#include "opcode.h"

#include "emu.h"
#include "dsp56k.h"

/*****************************/
/* Main disassembly function */
/*****************************/
CPU_DISASSEMBLE(dsp56k)
{
	const uint16_t w0 = oprom[0] | (oprom[1] << 8);
	const uint16_t w1 = oprom[2] | (oprom[3] << 8);

	// Decode and disassemble.
	DSP56K::Opcode op(w0, w1);
	stream << op.disassemble();

	const unsigned size = op.size();
	return (size | DASMFLAG_SUPPORTED);
}
