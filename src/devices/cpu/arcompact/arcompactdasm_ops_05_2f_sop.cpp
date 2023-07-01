// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\
 ARCompact disassembler

\*********************************/

#include "emu.h"
#include "arcompactdasm_internal.h"

int arcompact_disassembler::handle05_2f_0x_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext)
{
	//
	// 0010 1bbb pp10 1111 FBBB CCCC CCII IIII when pp == 0x00
	// or
	// 0010 1bbb pp10 1111 FBBB UUUU UUII IIII when pp == 0x01
	// otherwise invalid

	int size = 4;

	uint8_t p = common32_get_p(op);
	uint8_t breg = common32_get_breg(op);
	bool F = common32_get_F(op);

	util::stream_format(stream, "%s%s %s, ", optext, flagbit[F], regnames[breg]);

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
		util::stream_format(stream, "<05_2f illegal p=10>");
	}
	else if (p == 3)
	{
		util::stream_format(stream, "<05_2f illegal p=11>");
	}

	return size;
}

int arcompact_disassembler::handle::dasm32_SWAP(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle05_2f_0x_helper_dasm(stream, pc, op, opcodes, "SWAP");
}

int arcompact_disassembler::handle::dasm32_NORM(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle05_2f_0x_helper_dasm(stream, pc, op, opcodes, "NORM");
}

int arcompact_disassembler::handle::dasm32_SAT16(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle05_2f_0x_helper_dasm(stream, pc, op, opcodes, "SAT16");
}

int arcompact_disassembler::handle::dasm32_RND16(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle05_2f_0x_helper_dasm(stream, pc, op, opcodes, "RND16");
}

int arcompact_disassembler::handle::dasm32_ABSSW(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle05_2f_0x_helper_dasm(stream, pc, op, opcodes, "ABSSW");
}

int arcompact_disassembler::handle::dasm32_ABSS(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle05_2f_0x_helper_dasm(stream, pc, op, opcodes, "ABSS");
}

int arcompact_disassembler::handle::dasm32_NEGSW(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle05_2f_0x_helper_dasm(stream, pc, op, opcodes, "NEGSW");
}

int arcompact_disassembler::handle::dasm32_NEGS(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle05_2f_0x_helper_dasm(stream, pc, op, opcodes, "NEGS");
}

int arcompact_disassembler::handle::dasm32_NORMW(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle05_2f_0x_helper_dasm(stream, pc, op, opcodes, "NORMW");
}
