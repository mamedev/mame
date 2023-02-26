// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\
 ARCompact disassembler

\*********************************/

#include "emu.h"
#include "arcompactdasm_internal.h"

// format on these is..

// 0010 0bbb aa11 0ZZX DBBB CCCC CCAA AAAA
// note, bits  11 0ZZX are part of the sub-opcode # already - this is a special encoding
int arcompact_disassembler::handle04_3x_helper_dasm(std::ostream& stream, offs_t pc, uint32_t op, const data_buffer& opcodes, int dsize, int extend)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	util::stream_format(stream, "LD%s%s", datasize[dsize], dataextend[extend]);

	int mode = (op & 0x00c00000) >> 22;
	uint8_t breg = common32_get_breg(op);
	int D = (op & 0x00008000) >> 15;
	uint8_t creg = common32_get_creg(op);
	uint8_t areg = common32_get_areg(op);

	util::stream_format(stream, "%s%s %s. ", addressmode[mode], cachebit[D], regnames[areg]);

	if (breg == DASM_REG_LIMM)
	{
		limm = dasm_get_limm_32bit_opcode(pc, opcodes);
		size = 8;
		got_limm = 1;
		util::stream_format(stream, "[0x%08x, ", limm);

	}
	else
	{
		util::stream_format(stream, "[%s, ", regnames[breg]);
	}

	if (creg == DASM_REG_LIMM)
	{
		if (!got_limm)
		{
			limm = dasm_get_limm_32bit_opcode(pc, opcodes);
			size = 8;
		}
		util::stream_format(stream, "0x%08x]", limm);
	}
	else
	{
		util::stream_format(stream, "%s]", regnames[creg]);
	}

	return size;
}

int arcompact_disassembler::handle::dasm32_LD_0(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_3x_helper_dasm(stream, pc, op, opcodes,0,0);
}

// ZZ value of 0x0 with X of 1 is illegal
int arcompact_disassembler::handle::dasm32_LD_1(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_3x_helper_dasm(stream, pc, op, opcodes,0,1);
}

int arcompact_disassembler::handle::dasm32_LD_2(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_3x_helper_dasm(stream, pc, op, opcodes,1,0);
}

int arcompact_disassembler::handle::dasm32_LD_3(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_3x_helper_dasm(stream, pc, op, opcodes,1,1);
}

int arcompact_disassembler::handle::dasm32_LD_4(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_3x_helper_dasm(stream, pc, op, opcodes,2,0);
}

int arcompact_disassembler::handle::dasm32_LD_5(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_3x_helper_dasm(stream, pc, op, opcodes,2,1);
}

// ZZ value of 0x3 is illegal
int arcompact_disassembler::handle::dasm32_LD_6(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_3x_helper_dasm(stream, pc, op, opcodes,3,0);
}

int arcompact_disassembler::handle::dasm32_LD_7(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_3x_helper_dasm(stream, pc, op, opcodes,3,1);
}
