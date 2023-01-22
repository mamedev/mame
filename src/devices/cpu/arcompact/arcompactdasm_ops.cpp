// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\

 ARCompact disassembler

\*********************************/

#include "emu.h"

#include "arcompactdasm.h"


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

	DASM_COMMON32_GET_breg;
	DASM_COMMON32_GET_F;
	DASM_COMMON32_GET_creg
	DASM_COMMON32_GET_areg

	util::stream_format(stream, "%s", optext);
	util::stream_format(stream, "%s", flagbit[F]);
	//  util::stream_format(stream, " p(%d)", p);


	if ((!b_reserved) && (breg == DASM_LIMM_REG))
	{
		DASM_GET_LIMM;
		size = 8;
		got_limm = 1;
	}

	if (creg == DASM_LIMM_REG)
	{
		if (!got_limm)
		{
			DASM_GET_LIMM;
			size = 8;
		}
	}

	// areg can be LIMM too, but in that case LIMM indicates 'no destination' rather than an actual LIMM value following

	if (ignore_dst == 0)
	{
		if (areg != DASM_LIMM_REG)  util::stream_format(stream, " %s <-", regnames[areg]);
		else util::stream_format(stream, " <no dst> <-");
	}
	else if (ignore_dst == 1) // certain opcode types ignore the 'a' field entirely, it should be set to 0.
	{
			if (areg) util::stream_format(stream, " <reserved %d> <-", areg);
	}
	else if (ignore_dst == 2) // for multiply operations areg should always be set to LIMM
	{
		if (areg != DASM_LIMM_REG) util::stream_format(stream, " <invalid %d> <-", areg);
		else  util::stream_format(stream, " <mulres> <-");
	}

	if (!b_reserved)
	{
		if (breg == DASM_LIMM_REG)
			util::stream_format(stream, " 0x%08x,", limm);
		else
			util::stream_format(stream, " %s,", regnames[breg]);
	}
	else
	{
		if (breg) util::stream_format(stream, "<reserved %d>,", breg);
	}

	if (creg == DASM_LIMM_REG)
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

	DASM_COMMON32_GET_breg;
	DASM_COMMON32_GET_F;
	DASM_COMMON32_GET_u6
	DASM_COMMON32_GET_areg

	util::stream_format(stream, "%s", optext);
	util::stream_format(stream, "%s", flagbit[F]);
	//  util::stream_format(stream, " p(%d)", p);


	if ((!b_reserved) && (breg == DASM_LIMM_REG))
	{
		DASM_GET_LIMM;
		size = 8;
//      got_limm = 1;
	}

	// areg can be LIMM too, but in that case LIMM indicates 'no destination' rather than an actual LIMM value following

	if (ignore_dst == 0)
	{
		if (areg != DASM_LIMM_REG)  util::stream_format(stream, " %s <-", regnames[areg]);
		else util::stream_format(stream, " <no dst> <-");
	}
	else if (ignore_dst == 1) // certain opcode types ignore the 'a' field entirely, it should be set to 0.
	{
			if (areg) util::stream_format(stream, " <reserved %d> <-", areg);
	}
	else if (ignore_dst == 2) // for multiply operations areg should always be set to LIMM
	{
		if (areg != DASM_LIMM_REG) util::stream_format(stream, " <invalid %d> <-", areg);
		else  util::stream_format(stream, " <mulres> <-");
	}

	if (!b_reserved)
	{
		if (breg == DASM_LIMM_REG)
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

	DASM_COMMON32_GET_breg;
	DASM_COMMON32_GET_F
	DASM_COMMON32_GET_s12;

	util::stream_format(stream, "%s", optext);
	util::stream_format(stream, "%s", flagbit[F]);
	//  util::stream_format(stream, " p(%d)", p);


	if (!b_reserved)
	{
		if (breg == DASM_LIMM_REG)
		{
			DASM_GET_LIMM;
			size = 8;
			//got_limm = 1;
			util::stream_format(stream, " 0x%08x ", limm);

		}
		else
		{
			util::stream_format(stream, " %s, ", regnames[breg]);
		}
	}
	else
	{
		if (breg) util::stream_format(stream, "reserved(%s), ", regnames[breg]);
	}

	util::stream_format(stream, "S(%02x)", S);
	return size;
}

int arcompact_disassembler::handle04_cc_f_b_b_c_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext, int b_reserved)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	DASM_COMMON32_GET_breg;
	DASM_COMMON32_GET_F
	DASM_COMMON32_GET_CONDITION;
	DASM_COMMON32_GET_creg

	util::stream_format(stream, "%s", optext);
	util::stream_format(stream, "%s", flagbit[F]);
	//  util::stream_format(stream, " p(%d)", p);

	if (!b_reserved)
	{
		if (breg == DASM_LIMM_REG)
		{
			DASM_GET_LIMM;
			size = 8;
			got_limm = 1;
			util::stream_format(stream, " 0x%08x ", limm);

		}
		else
		{
			util::stream_format(stream, " %s, ", regnames[breg]);
		}
	}
	else
	{
		if (breg) util::stream_format(stream, "reserved(%s), ", regnames[breg]);
	}


	util::stream_format(stream, " Cond<%s> ", conditions[condition]);


	if (creg == DASM_LIMM_REG)
	{
		if (!got_limm)
		{
			DASM_GET_LIMM;
			size = 8;
		}
		util::stream_format(stream, " 0x%08x ", limm);
	}
	else
	{
		util::stream_format(stream, "C(%s)", regnames[creg]);
	}

	return size;
}

int arcompact_disassembler::handle04_cc_f_b_b_u6_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext, int b_reserved)
{
	int size = 4;
	uint32_t limm;
	//int got_limm = 0;

	DASM_COMMON32_GET_breg;
	DASM_COMMON32_GET_F
	DASM_COMMON32_GET_CONDITION;
	DASM_COMMON32_GET_u6

	util::stream_format(stream, "%s", optext);
	util::stream_format(stream, "%s", flagbit[F]);
	//  util::stream_format(stream, " p(%d)", p);

	if (!b_reserved)
	{
		if (breg == DASM_LIMM_REG)
		{
			DASM_GET_LIMM;
			size = 8;
			//got_limm = 1;
			util::stream_format(stream, " 0x%08x ", limm);

		}
		else
		{
			util::stream_format(stream, " %s, ", regnames[breg]);
		}
	}
	else
	{
		if (breg) util::stream_format(stream, "reserved(%s), ", regnames[breg]);
	}


	util::stream_format(stream, " Cond<%s> ", conditions[condition]);


	util::stream_format(stream, "U(%02x)", u);

	return size;
}

int arcompact_disassembler::handle04_p11_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext, int b_reserved)
{
	int M = (op & 0x00000020) >> 5; op &= ~0x00000020;

	switch (M)
	{
		case 0x00: return handle04_cc_f_b_b_c_helper_dasm(stream, pc, op, opcodes, optext, b_reserved);
		case 0x01: return handle04_cc_f_b_b_u6_helper_dasm(stream, pc, op, opcodes, optext, b_reserved);
	}
	return 0;
}


int arcompact_disassembler::handle04_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext, int ignore_dst, int b_reserved)
{
	DASM_COMMON32_GET_p;

	switch (p)
	{
		case 0x00: return handle04_f_a_b_c_helper_dasm(stream, pc, op, opcodes, optext, ignore_dst, b_reserved);
		case 0x01: return handle04_f_a_b_u6_helper_dasm(stream, pc, op, opcodes, optext, ignore_dst, b_reserved);
		case 0x02: return handle04_f_b_b_s12_helper_dasm(stream, pc, op, opcodes, optext, b_reserved);
		case 0x03: return handle04_p11_helper_dasm(stream, pc, op, opcodes, optext, b_reserved);
	}

	return 0;
}


int arcompact_disassembler::handle_dasm_illegal(std::ostream& stream, offs_t pc, uint8_t param1, uint8_t param2, uint32_t op, const data_buffer& opcodes)
{
	return 4;

}
int arcompact_disassembler::handle_dasm_illegal(std::ostream& stream, offs_t pc, uint8_t param1, uint8_t param2, uint8_t param3, uint32_t op, const data_buffer& opcodes)
{
	return 4;

}
int arcompact_disassembler::handle_dasm_illegal(std::ostream& stream, offs_t pc, uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4, uint32_t op, const data_buffer& opcodes)
{
	return 4;

}

int arcompact_disassembler::handle_dasm_reserved(std::ostream& stream, offs_t pc, uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4, uint32_t op, const data_buffer& opcodes)
{
	return 4;
}
