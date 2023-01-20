// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\

 ARCompact disassembler

\*********************************/

#include "emu.h"

#include "arcompactdasm.h"

int arcompact_disassembler::handle05_2f_0x_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext)
{
	//
	// 0010 1bbb pp10 1111 FBBB CCCC CCII IIII when pp == 0x00
	// or
	// 0010 1bbb pp10 1111 FBBB UUUU UUII IIII when pp == 0x01
	// otherwise invalid

	int size = 4;

	DASM_COMMON32_GET_p;
	DASM_COMMON32_GET_breg;
	DASM_COMMON32_GET_F

	util::stream_format(stream, "%s", optext);
	util::stream_format(stream, "%s", flagbit[F]);
//  util::stream_format(stream, " p(%d)", p);


	util::stream_format(stream, " %s, ", regnames[breg]);

	if (p == 0)
	{
		DASM_COMMON32_GET_creg

		if (creg == DASM_LIMM_REG)
		{
			uint32_t limm;
			DASM_GET_LIMM;
			size = 8;
			util::stream_format(stream, "(%08x) ", limm);

		}
		else
		{
			util::stream_format(stream, "C(%s) ", regnames[creg]);
		}
	}
	else if (p == 1)
	{
		DASM_COMMON32_GET_u6
		util::stream_format(stream, "U(0x%02x) ", u);
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

int arcompact_disassembler::handle_dasm32_SWAP(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle05_2f_0x_helper_dasm(stream, pc, op, opcodes, "SWAP");
}

int arcompact_disassembler::handle_dasm32_NORM(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle05_2f_0x_helper_dasm(stream, pc, op, opcodes, "NORM");
}

int arcompact_disassembler::handle_dasm32_SAT16(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle05_2f_0x_helper_dasm(stream, pc, op, opcodes, "SAT16");
}

int arcompact_disassembler::handle_dasm32_RND16(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle05_2f_0x_helper_dasm(stream, pc, op, opcodes, "RND16");
}

int arcompact_disassembler::handle_dasm32_ABSSW(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle05_2f_0x_helper_dasm(stream, pc, op, opcodes, "ABSSW");
}

int arcompact_disassembler::handle_dasm32_ABSS(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle05_2f_0x_helper_dasm(stream, pc, op, opcodes, "ABSS");
}

int arcompact_disassembler::handle_dasm32_NEGSW(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle05_2f_0x_helper_dasm(stream, pc, op, opcodes, "NEGSW");
}

int arcompact_disassembler::handle_dasm32_NEGS(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle05_2f_0x_helper_dasm(stream, pc, op, opcodes, "NEGS");
}

int arcompact_disassembler::handle_dasm32_NORMW(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle05_2f_0x_helper_dasm(stream, pc, op, opcodes, "NORMW");
}
