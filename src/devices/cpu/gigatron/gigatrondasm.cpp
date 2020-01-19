// license:BSD-3-Clause
// copyright-holders:Sterophonick

// Gigatron Disassembler

#include "emu.h"
#include "gigatrondasm.h"

u32 gigatron_disassembler::opcode_alignment() const
{
	return 0;
}

offs_t gigatron_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	return 0;
}
