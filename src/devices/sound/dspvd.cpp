// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha DSPV
//
// Audio dsp dedicated to acoustic simulation
//
// Disassembler, placeholder

#include "emu.h"
#include "dspvd.h"

dspv_disassembler::dspv_disassembler()
{
}

u32 dspv_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t dspv_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u16 opc = opcodes.r16(pc);

	util::stream_format(stream, "dc.w %04x", opc);

	return 1 | SUPPORTED;
}
