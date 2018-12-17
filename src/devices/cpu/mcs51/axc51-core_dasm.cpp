// license:BSD-3-Clause
// copyright-holders:David Haywood
/*****************************************************************************

	AXC51-CORE (AppoTech Inc.)

	used in

	AX208 SoC

 *****************************************************************************/

#include "emu.h"
#include "axc51-core_dasm.h"

axc51core_disassembler::axc51core_disassembler() : mcs51_disassembler(default_names)
{
}


offs_t axc51core_disassembler::disassemble_op(std::ostream &stream, unsigned PC, offs_t pc, const data_buffer &opcodes, const data_buffer &params, uint8_t op)
{
	uint32_t flags = 0;
	uint8_t prm = 0;

	switch( op )
	{
		// unknown, probably the 16-bit extended ocpodes, see page 14 of AX208-SP-101-EN manual, encodings not specified!
		// note, the other AXC51-CORE based manuals don't seem to mention these, are they really AX208 specific?
		case 0xa5:
			prm = params.r8(PC++);
			util::stream_format(stream, "unknown ax208 a5 $%02X", prm);
			break;

		default:
			return mcs51_disassembler::disassemble_op(stream, PC, pc, opcodes, params, op);
	}

	return (PC - pc) | flags | SUPPORTED;
}


