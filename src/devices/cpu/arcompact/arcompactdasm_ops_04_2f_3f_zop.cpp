// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\
 ARCompact disassembler

 ALU Operations, 0x04, [0x00-0x1F]

\*********************************/

#include "emu.h"
#include "arcompactdasm_internal.h"

int arcompact_disassembler::handle::dasm32_SLEEP(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	util::stream_format( stream, "SLEEP (%08x)", op);
	return 4;
}

int arcompact_disassembler::handle::dasm32_SWI(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	util::stream_format( stream, "SWI / TRAP0 (%08x)", op);
	return 4;
}

int arcompact_disassembler::handle::dasm32_SYNC(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	util::stream_format( stream, "SYNC (%08x)", op);
	return 4;
}

int arcompact_disassembler::handle::dasm32_RTIE(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	util::stream_format( stream, "RTIE (%08x)", op);
	return 4;
}

int arcompact_disassembler::handle::dasm32_BRK(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	util::stream_format( stream, "BRK (%08x)", op);
	return 4;
}
