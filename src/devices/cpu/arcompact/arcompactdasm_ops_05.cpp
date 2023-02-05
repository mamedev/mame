// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\
 ARCompact disassembler
\*********************************/

#include "emu.h"
#include "arcompactdasm_internal.h"

int arcompact_disassembler::handle::dasm32_ASL_multiple(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "ASL", 0,0);
}

int arcompact_disassembler::handle::dasm32_LSR_multiple(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "LSR", 0,0);
}

int arcompact_disassembler::handle::dasm32_ASR_multiple(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "ASR", 0,0);
}

int arcompact_disassembler::handle::dasm32_ROR_multiple(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "ROR", 0,0);
}

int arcompact_disassembler::handle::dasm32_MUL64(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "MUL64", 2,0);
}

int arcompact_disassembler::handle::dasm32_MULU64(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "MULU64", 2,0);
}

int arcompact_disassembler::handle::dasm32_ADDS(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "ADDS", 0,0);
}

int arcompact_disassembler::handle::dasm32_SUBS(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "SUBS", 0,0);
}

int arcompact_disassembler::handle::dasm32_DIVAW(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "DIVAW", 0,0);
}

int arcompact_disassembler::handle::dasm32_ASLS(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "ASLS", 0,0);
}

int arcompact_disassembler::handle::dasm32_ASRS(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "ASRS", 0,0);
}

int arcompact_disassembler::handle::dasm32_ADDSDW(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "ADDSDW", 0,0);
}

int arcompact_disassembler::handle::dasm32_SUBSDW(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "SUBSDW", 0,0);
}
