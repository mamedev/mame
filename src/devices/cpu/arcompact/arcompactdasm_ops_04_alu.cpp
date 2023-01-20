// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\

 ARCompact disassembler

 ALU Operations, 0x04, [0x00-0x1F]

\*********************************/

#include "emu.h"

#include "arcompactdasm.h"

int arcompact_disassembler::handle_dasm32_ADD(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "ADD", 0,0);
}

int arcompact_disassembler::handle_dasm32_ADC(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "ADC", 0,0);
}

int arcompact_disassembler::handle_dasm32_SUB(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "SUB", 0,0);
}

int arcompact_disassembler::handle_dasm32_SBC(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "SBC", 0,0);
}

int arcompact_disassembler::handle_dasm32_AND(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "AND", 0,0);
}

int arcompact_disassembler::handle_dasm32_OR(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "OR", 0,0);
}

int arcompact_disassembler::handle_dasm32_BIC(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "BIC", 0,0);
}

int arcompact_disassembler::handle_dasm32_XOR(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "XOR", 0,0);
}

int arcompact_disassembler::handle_dasm32_MAX(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "MAX", 0,0);
}

int arcompact_disassembler::handle_dasm32_MIN(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "MIN", 0,0);
}


int arcompact_disassembler::handle_dasm32_MOV(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "MOV", 1,0);
}

int arcompact_disassembler::handle_dasm32_TST(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "TST", 1,0);
}

int arcompact_disassembler::handle_dasm32_CMP(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "CMP", 1,0);
}

int arcompact_disassembler::handle_dasm32_RCMP(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "RCMP", 1,0);
}

int arcompact_disassembler::handle_dasm32_RSUB(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "RSUB", 0,0);
}

int arcompact_disassembler::handle_dasm32_BSET(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "BSET", 0,0);
}

int arcompact_disassembler::handle_dasm32_BCLR(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "BCLR", 0,0);
}

int arcompact_disassembler::handle_dasm32_BTST(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "BTST", 0,0);
}

int arcompact_disassembler::handle_dasm32_BXOR(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "BXOR", 0,0);
}

int arcompact_disassembler::handle_dasm32_BMSK(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "BMSK", 0,0);
}

int arcompact_disassembler::handle_dasm32_ADD1(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "ADD1", 0,0);
}

int arcompact_disassembler::handle_dasm32_ADD2(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "ADD2", 0,0);
}

int arcompact_disassembler::handle_dasm32_ADD3(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "ADD3", 0,0);
}

int arcompact_disassembler::handle_dasm32_SUB1(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "SUB1", 0,0);
}

int arcompact_disassembler::handle_dasm32_SUB2(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "SUB2", 0,0);
}

int arcompact_disassembler::handle_dasm32_SUB3(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "SUB3", 0,0);
}

int arcompact_disassembler::handle_dasm32_MPY(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "MPY", 0,0);
} // *

int arcompact_disassembler::handle_dasm32_MPYH(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "MPYH", 0,0);
} // *

int arcompact_disassembler::handle_dasm32_MPYHU(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "MPYHU", 0,0);
} // *

int arcompact_disassembler::handle_dasm32_MPYU(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "MPYU", 0,0);
} // *
