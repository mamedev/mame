// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\
 ARCompact disassembler

\*********************************/

#include "emu.h"
#include "arcompactdasm_internal.h"


/************************************************************************************************************************************
*                                                                                                                                   *
* individual opcode handlers (disassembly)                                                                                          *
*                                                                                                                                   *
************************************************************************************************************************************/


int arcompact_disassembler::handle04_f_a_b_c_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext, int ignore_dst, int b_reserved)
{
	//           PP
	// 0010 0bbb 00ii iiii FBBB CCCC CCAA AAAA
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	uint8_t breg = common32_get_breg(op);
	bool F = common32_get_F(op);
	uint8_t creg = common32_get_creg(op);
	uint8_t areg = common32_get_areg(op);

	util::stream_format(stream, "%s%s", optext, flagbit[F ? 1:0]);

	if ((!b_reserved) && (breg == DASM_REG_LIMM))
	{
		limm = dasm_get_limm_32bit_opcode(pc, opcodes);
		size = 8;
		got_limm = 1;
	}

	if (creg == DASM_REG_LIMM)
	{
		if (!got_limm)
		{
			limm = dasm_get_limm_32bit_opcode(pc, opcodes);
			size = 8;
		}
	}

	// areg can be LIMM too, but in that case LIMM indicates 'no destination' rather than an actual LIMM value following

	if (ignore_dst == 0)
	{
		if (areg != DASM_REG_LIMM)  util::stream_format(stream, " %s <-", regnames[areg]);
		else util::stream_format(stream, " <no dst> <-");
	}
	else if (ignore_dst == 1) // certain opcode types ignore the 'a' field entirely, it should be set to 0.
	{
			if (areg) util::stream_format(stream, " <reserved %d> <-", areg);
	}
	else if (ignore_dst == 2) // for multiply operations areg should always be set to LIMM
	{
		if (areg != DASM_REG_LIMM) util::stream_format(stream, " <invalid %d> <-", areg);
		else  util::stream_format(stream, " <mulres> <-");
	}

	if (!b_reserved)
	{
		if (breg == DASM_REG_LIMM)
			util::stream_format(stream, " 0x%08x,", limm);
		else
			util::stream_format(stream, " %s,", regnames[breg]);
	}
	else
	{
		if (breg) util::stream_format(stream, "<reserved %d>,", breg);
	}

	if (creg == DASM_REG_LIMM)
		util::stream_format(stream, " 0x%08x", limm);
	else
		util::stream_format(stream, " %s", regnames[creg]);

	return size;
}

// like p00 but with 'u6' istead of C
int arcompact_disassembler::handle04_f_a_b_u6_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext, int ignore_dst, int b_reserved)
{
	//           PP
	// 0010 0bbb 01ii iiii FBBB uuuu uuAA AAAA
	int size = 4;
	uint32_t limm = 0;
//  int got_limm = 0;

	uint8_t breg = common32_get_breg(op);
	bool F = common32_get_F(op);
	uint32_t u = common32_get_u6(op);
	uint8_t areg = common32_get_areg(op);

	util::stream_format(stream, "%s%s", optext, flagbit[F ? 1:0]);

	if ((!b_reserved) && (breg == DASM_REG_LIMM))
	{
		limm = dasm_get_limm_32bit_opcode(pc, opcodes);
		size = 8;
//      got_limm = 1;
	}

	// areg can be LIMM too, but in that case LIMM indicates 'no destination' rather than an actual LIMM value following

	if (ignore_dst == 0)
	{
		if (areg != DASM_REG_LIMM)  util::stream_format(stream, " %s <-", regnames[areg]);
		else util::stream_format(stream, " <no dst> <-");
	}
	else if (ignore_dst == 1) // certain opcode types ignore the 'a' field entirely, it should be set to 0.
	{
			if (areg) util::stream_format(stream, " <reserved %d> <-", areg);
	}
	else if (ignore_dst == 2) // for multiply operations areg should always be set to LIMM
	{
		if (areg != DASM_REG_LIMM) util::stream_format(stream, " <invalid %d> <-", areg);
		else  util::stream_format(stream, " <mulres> <-");
	}

	if (!b_reserved)
	{
		if (breg == DASM_REG_LIMM)
			util::stream_format(stream, " 0x%08x,", limm);
		else
			util::stream_format(stream, " %s,", regnames[breg]);
	}
	else
	{
		if (breg) util::stream_format(stream, "<reserved %d>,", breg);
	}

	util::stream_format(stream, " 0x%02x", u);

	return size;
}


int arcompact_disassembler::handle04_f_b_b_s12_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext, int b_reserved)
{
	int size = 4;
	uint32_t limm;
	//int got_limm = 0;

	uint8_t breg = common32_get_breg(op);
	bool F = common32_get_F(op);
	uint32_t S = common32_get_s12(op);

	util::stream_format(stream, "%s%s", optext, flagbit[F ? 1:0]);

	if (!b_reserved)
	{
		if (breg == DASM_REG_LIMM)
		{
			limm = dasm_get_limm_32bit_opcode(pc, opcodes);
			size = 8;
			//got_limm = 1;
			util::stream_format(stream, " 0x%08x ", limm);

		}
		else
		{
			util::stream_format(stream, " %s, ", regnames[breg]);
		}
	}

	util::stream_format(stream, "0x%02x", S);
	return size;
}

int arcompact_disassembler::handle04_cc_f_b_b_c_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext, int b_reserved)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	uint8_t breg = common32_get_breg(op);
	bool F = common32_get_F(op);
	uint8_t condition = common32_get_condition(op);
	uint8_t creg = common32_get_creg(op);

	util::stream_format(stream, "%s%s%s", optext, conditions[condition], flagbit[F ? 1:0]);

	if (!b_reserved)
	{
		if (breg == DASM_REG_LIMM)
		{
			limm = dasm_get_limm_32bit_opcode(pc, opcodes);
			size = 8;
			got_limm = 1;
			util::stream_format(stream, " 0x%08x ", limm);

		}
		else
		{
			util::stream_format(stream, " %s, ", regnames[breg]);
		}
	}

	if (creg == DASM_REG_LIMM)
	{
		if (!got_limm)
		{
			limm = dasm_get_limm_32bit_opcode(pc, opcodes);
			size = 8;
		}
		util::stream_format(stream, "0x%08x", limm);
	}
	else
	{
		util::stream_format(stream, "%s", regnames[creg]);
	}

	return size;
}

int arcompact_disassembler::handle04_cc_f_b_b_u6_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext, int b_reserved)
{
	int size = 4;
	uint32_t limm;
	//int got_limm = 0;

	uint8_t breg = common32_get_breg(op);
	bool F = common32_get_F(op);
	uint8_t condition = common32_get_condition(op);
	uint32_t u = common32_get_u6(op);

	util::stream_format(stream, "%s%s%s", optext, conditions[condition], flagbit[F ? 1:0]);

	if (!b_reserved)
	{
		if (breg == DASM_REG_LIMM)
		{
			limm = dasm_get_limm_32bit_opcode(pc, opcodes);
			size = 8;
			//got_limm = 1;
			util::stream_format(stream, " 0x%08x ", limm);

		}
		else
		{
			util::stream_format(stream, " %s, ", regnames[breg]);
		}
	}

	util::stream_format(stream, "0x%02x", u);

	return size;
}

int arcompact_disassembler::handle04_p11_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext, int b_reserved)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handle04_cc_f_b_b_c_helper_dasm(stream, pc, op, opcodes, optext, b_reserved);
		case 0x01: return handle04_cc_f_b_b_u6_helper_dasm(stream, pc, op, opcodes, optext, b_reserved);
	}
	return 0;
}


int arcompact_disassembler::handle04_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext, int ignore_dst, int b_reserved)
{
	uint8_t p = common32_get_p(op);

	switch (p)
	{
		case 0x00: return handle04_f_a_b_c_helper_dasm(stream, pc, op, opcodes, optext, ignore_dst, b_reserved);
		case 0x01: return handle04_f_a_b_u6_helper_dasm(stream, pc, op, opcodes, optext, ignore_dst, b_reserved);
		case 0x02: return handle04_f_b_b_s12_helper_dasm(stream, pc, op, opcodes, optext, b_reserved);
		case 0x03: return handle04_p11_helper_dasm(stream, pc, op, opcodes, optext, b_reserved);
	}

	return 0;
}


int arcompact_disassembler::handle::dasm_illegal(std::ostream& stream, offs_t pc, uint8_t param1, uint8_t param2, uint32_t op, const data_buffer& opcodes)
{
	util::stream_format(stream, "<illegal 0x%02x_%02x> (%08x)\n", param1, param2, op);
	return 4;
}

int arcompact_disassembler::handle::dasm_illegal(std::ostream& stream, offs_t pc, uint8_t param1, uint8_t param2, uint8_t param3, uint32_t op, const data_buffer& opcodes)
{
	util::stream_format(stream, "<illegal 0x%02x_%02x_%02x> (%08x)\n", param1, param2, param3, op);
	return 4;
}

int arcompact_disassembler::handle::dasm_illegal(std::ostream& stream, offs_t pc, uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4, uint32_t op, const data_buffer& opcodes)
{
	util::stream_format(stream, "<illegal 0x%02x_%02x_%02x_%02x> (%08x)\n", param1, param2, param3, param4, op);
	return 4;
}

int arcompact_disassembler::handle::dasm_reserved(std::ostream& stream, offs_t pc, uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4, uint32_t op, const data_buffer& opcodes)
{
	util::stream_format(stream, "<reserved 0x%02x_%02x_%02x_%02x> (%08x)\n", param1, param2, param3, param4, op);
	return 4;
}
