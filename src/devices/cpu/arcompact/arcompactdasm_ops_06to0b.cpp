// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\
 ARCompact disassembler

\*********************************/

#include "emu.h"
#include "arcompactdasm_internal.h"

int arcompact_disassembler::handle::dasm32_ARC_EXT06(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	util::stream_format( stream, "op a,b,c (06 ARC ext) (%08x)", op );
	return 4;
}

int arcompact_disassembler::handle::dasm32_USER_EXT07(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	util::stream_format( stream, "op a,b,c (07 User ext) (%08x)", op );
	return 4;
}

int arcompact_disassembler::handle::dasm32_USER_EXT08(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	util::stream_format( stream, "op a,b,c (08 User ext) (%08x)", op );
	return 4;
}

int arcompact_disassembler::handle::dasm32_MARKET_EXT09(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	util::stream_format( stream, "op a,b,c (09 Market ext) (%08x)", op );
	return 4;
}

int arcompact_disassembler::handle::dasm32_MARKET_EXT0a(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	util::stream_format( stream, "op a,b,c (0a Market ext) (%08x)",  op );
	return 4;
}

int arcompact_disassembler::handle::dasm32_MARKET_EXT0b(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	util::stream_format( stream, "op a,b,c (0b Market ext) (%08x)",  op );
	return 4;
}
