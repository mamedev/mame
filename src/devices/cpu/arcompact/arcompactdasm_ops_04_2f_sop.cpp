// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\
 ARCompact disassembler

 ALU Operations, 0x04, [0x00-0x1F]

\*********************************/

#include "emu.h"
#include "arcompactdasm_internal.h"

int arcompact_disassembler::handle04_2f_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext)
{
	//
	// 0010 0bbb pp10 1111 FBBB CCCC CCII IIII
	int size = 4;

	uint8_t p = common32_get_p(op);
	uint8_t breg = common32_get_breg(op);
	bool F = common32_get_F(op);

	util::stream_format(stream, "%s%s", optext, flagbit[F]);

	if (breg == DASM_REG_LIMM)
	{
		util::stream_format(stream, " <no dst>, ");
		// if using the 'EX' opcode this is illegal
	}
	else
	{
		util::stream_format(stream, " %s, ", regnames[breg]);
	}

	if (p == 0)
	{
		uint8_t creg = common32_get_creg(op);

		if (creg == DASM_REG_LIMM)
		{
			uint32_t limm;
			limm = dasm_get_limm_32bit_opcode(pc, opcodes);
			size = 8;
			util::stream_format(stream, "0x%08x ", limm);
		}
		else
		{
			util::stream_format(stream, "%s ", regnames[creg]);
		}
	}
	else if (p == 1)
	{
		uint32_t u = common32_get_u6(op);

		util::stream_format(stream, "0x%02x ", u);
	}
	else if (p == 2)
	{
		util::stream_format(stream, "<04_2f illegal p=10>");
	}
	else if (p == 3)
	{
		util::stream_format(stream, "<04_2f illegal p=11>");
	}

	return size;
}

int arcompact_disassembler::handle::dasm32_ASL_single(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_2f_helper_dasm(stream, pc, op, opcodes, "ASL");
}

int arcompact_disassembler::handle::dasm32_ASR_single(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_2f_helper_dasm(stream, pc, op, opcodes, "ASR");
}

int arcompact_disassembler::handle::dasm32_LSR_single(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_2f_helper_dasm(stream, pc, op, opcodes, "LSR");
}

int arcompact_disassembler::handle::dasm32_ROR_single(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_2f_helper_dasm(stream, pc, op, opcodes, "ROR");
}

int arcompact_disassembler::handle::dasm32_RRC(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_2f_helper_dasm(stream, pc, op, opcodes, "RCC");
}

int arcompact_disassembler::handle::dasm32_SEXB(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_2f_helper_dasm(stream, pc, op, opcodes, "SEXB");
}

int arcompact_disassembler::handle::dasm32_SEXW(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_2f_helper_dasm(stream, pc, op, opcodes, "SEXW");
}

int arcompact_disassembler::handle::dasm32_EXTB(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_2f_helper_dasm(stream, pc, op, opcodes, "EXTB");
}

int arcompact_disassembler::handle::dasm32_EXTW(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_2f_helper_dasm(stream, pc, op, opcodes, "EXTW");
}

int arcompact_disassembler::handle::dasm32_ABS(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_2f_helper_dasm(stream, pc, op, opcodes, "ABS");
}

int arcompact_disassembler::handle::dasm32_NOT(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_2f_helper_dasm(stream, pc, op, opcodes, "NOT");
}

int arcompact_disassembler::handle::dasm32_RLC(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_2f_helper_dasm(stream, pc, op, opcodes, "RLC");
}

int arcompact_disassembler::handle::dasm32_EX(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_2f_helper_dasm(stream, pc, op, opcodes, "EX");
}


