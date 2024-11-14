// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\
 ARCompact disassembler

 ALU Operations, 0x04, [0x00-0x1F]

\*********************************/

#include "emu.h"
#include "arcompactdasm_internal.h"


// MOV is a special case because 'a' is completely ignored even where
// it would usually be the destination.
// as a result, b is always the destination, and LIMM handling is different
// it also has an official NOP alias for no destination + no flag cases
int arcompact_disassembler::handle04_MOV_f_a_b_c_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	int size = 4;
	uint8_t breg = common32_get_breg(op);
	bool F = common32_get_F(op);
	uint8_t creg = common32_get_creg(op);
	util::stream_format(stream, "MOV%s %s,", flagbit[F ? 1:0], regnames[breg]);
	if (creg == DASM_REG_LIMM)
	{
		uint32_t limm = dasm_get_limm_32bit_opcode(pc, opcodes);
		size = 8;
		util::stream_format(stream, " 0x%08x", limm);
	}
	else
	{
		util::stream_format(stream, " %s", regnames[creg]);
	}
	return size;
}

int arcompact_disassembler::handle04_MOV_f_a_b_u6_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	int size = 4;
	uint8_t breg = common32_get_breg(op);
	bool F = common32_get_F(op);
	uint32_t u = common32_get_u6(op);
	// if there's no destination and no flags being set, this is a NOP
	if ((F == 0) & (breg == DASM_REG_LIMM))
	{
		util::stream_format(stream, "NOP");
	}
	else
	{
		util::stream_format(stream, "MOV%s %s, 0x%08x", flagbit[F ? 1:0], regnames[breg], u);
	}
	return size;
}


int arcompact_disassembler::handle04_MOV_f_b_b_s12_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	uint8_t breg = common32_get_breg(op);
	bool F = common32_get_F(op);
	uint32_t S = common32_get_s12(op);
	util::stream_format(stream, "MOV%s %s, 0x%08x", flagbit[F ? 1:0], regnames[breg], S);
	return 4;
}

int arcompact_disassembler::handle04_MOV_cc_f_b_b_c_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	int size = 4;
	uint8_t breg = common32_get_breg(op);
	bool F = common32_get_F(op);
	uint8_t condition = common32_get_condition(op);
	uint8_t creg = common32_get_creg(op);
	util::stream_format(stream, "MOV%s%s %s, ", conditions[condition], flagbit[F ? 1:0], regnames[breg]);
	if (creg == DASM_REG_LIMM)
	{
		uint32_t limm = dasm_get_limm_32bit_opcode(pc, opcodes);
		size = 8;
		util::stream_format(stream, "0x%08x", limm);
	}
	else
	{
		util::stream_format(stream, "%s", regnames[creg]);
	}
	return size;
}

int arcompact_disassembler::handle04_MOV_cc_f_b_b_u6_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	uint8_t breg = common32_get_breg(op);
	bool F = common32_get_F(op);
	uint8_t condition = common32_get_condition(op);
	uint32_t u = common32_get_u6(op);
	util::stream_format(stream, "MOV%s%s %s, 0x%08x", conditions[condition], flagbit[F ? 1:0], regnames[breg], u);
	return 4;
}

int arcompact_disassembler::handle04_MOV_p11_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	int M = (op & 0x00000020) >> 5;
	switch (M)
	{
		case 0x00: return handle04_MOV_cc_f_b_b_c_helper_dasm(stream, pc, op, opcodes);
		case 0x01: return handle04_MOV_cc_f_b_b_u6_helper_dasm(stream, pc, op, opcodes);
	}
	return 0;
}


int arcompact_disassembler::handle04_MOV_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	uint8_t p = common32_get_p(op);
	switch (p)
	{
		case 0x00: return handle04_MOV_f_a_b_c_helper_dasm(stream, pc, op, opcodes);
		case 0x01: return handle04_MOV_f_a_b_u6_helper_dasm(stream, pc, op, opcodes);
		case 0x02: return handle04_MOV_f_b_b_s12_helper_dasm(stream, pc, op, opcodes);
		case 0x03: return handle04_MOV_p11_helper_dasm(stream, pc, op, opcodes);
	}

	return 0;
}


int arcompact_disassembler::handle::dasm32_ADD(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "ADD", 0,0);
}

int arcompact_disassembler::handle::dasm32_ADC(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "ADC", 0,0);
}

int arcompact_disassembler::handle::dasm32_SUB(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "SUB", 0,0);
}

int arcompact_disassembler::handle::dasm32_SBC(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "SBC", 0,0);
}

int arcompact_disassembler::handle::dasm32_AND(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "AND", 0,0);
}

int arcompact_disassembler::handle::dasm32_OR(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "OR", 0,0);
}

int arcompact_disassembler::handle::dasm32_BIC(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "BIC", 0,0);
}

int arcompact_disassembler::handle::dasm32_XOR(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "XOR", 0,0);
}

int arcompact_disassembler::handle::dasm32_MAX(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "MAX", 0,0);
}

int arcompact_disassembler::handle::dasm32_MIN(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "MIN", 0,0);
}

int arcompact_disassembler::handle::dasm32_MOV(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_MOV_helper_dasm(stream, pc, op, opcodes);
}

int arcompact_disassembler::handle::dasm32_TST(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "TST", 1,0);
}

int arcompact_disassembler::handle::dasm32_CMP(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "CMP", 1,0);
}

int arcompact_disassembler::handle::dasm32_RCMP(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "RCMP", 1,0);
}

int arcompact_disassembler::handle::dasm32_RSUB(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "RSUB", 0,0);
}

int arcompact_disassembler::handle::dasm32_BSET(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "BSET", 0,0);
}

int arcompact_disassembler::handle::dasm32_BCLR(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "BCLR", 0,0);
}

int arcompact_disassembler::handle::dasm32_BTST(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "BTST", 0,0);
}

int arcompact_disassembler::handle::dasm32_BXOR(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "BXOR", 0,0);
}

int arcompact_disassembler::handle::dasm32_BMSK(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "BMSK", 0,0);
}

int arcompact_disassembler::handle::dasm32_ADD1(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "ADD1", 0,0);
}

int arcompact_disassembler::handle::dasm32_ADD2(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "ADD2", 0,0);
}

int arcompact_disassembler::handle::dasm32_ADD3(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "ADD3", 0,0);
}

int arcompact_disassembler::handle::dasm32_SUB1(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "SUB1", 0,0);
}

int arcompact_disassembler::handle::dasm32_SUB2(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "SUB2", 0,0);
}

int arcompact_disassembler::handle::dasm32_SUB3(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "SUB3", 0,0);
}

int arcompact_disassembler::handle::dasm32_MPY(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "MPY", 0,0);
} // *

int arcompact_disassembler::handle::dasm32_MPYH(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "MPYH", 0,0);
} // *

int arcompact_disassembler::handle::dasm32_MPYHU(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "MPYHU", 0,0);
} // *

int arcompact_disassembler::handle::dasm32_MPYU(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "MPYU", 0,0);
} // *

int arcompact_disassembler::handle::dasm32_Jcc(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "J", 1,1);
}

int arcompact_disassembler::handle::dasm32_Jcc_D(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "J.D", 1,1);
}

int arcompact_disassembler::handle::dasm32_JLcc(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "JL", 1,1);
}

int arcompact_disassembler::handle::dasm32_JLcc_D(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "JL.D", 1,1);
}

int arcompact_disassembler::handle::dasm32_LP(std::ostream& stream, offs_t pc, uint32_t op, const data_buffer& opcodes) // LPcc (loop setup)
{
	//uint8_t breg = common32_get_breg(op); // breg is reserved
	uint8_t p = common32_get_p(op);

	if (p == 0x00)
	{
		util::stream_format(stream, "<illegal LPcc, p = 0x00>");
	}
	else if (p == 0x01)
	{
		util::stream_format(stream, "<illegal LPcc, p = 0x01>");
	}
	else if (p == 0x02) // Loop unconditional
	{ // 0010 0RRR 1010 1000 0RRR ssss ssSS SSSS
		uint32_t S = common32_get_s12(op);
		util::stream_format(stream, "LP (start %08x, end %08x)", pc + 4, (pc & 0xfffffffc) + S * 2);
	}
	else if (p == 0x03) // Loop conditional
	{ // 0010 0RRR 1110 1000 0RRR uuuu uu1Q QQQQ
		uint32_t u = common32_get_u6(op);
		uint8_t condition = common32_get_condition(op);
		util::stream_format(stream, "LP<%s> (start %08x, end %08x)", conditions[condition], pc + 4, (pc & 0xfffffffc) + u * 2);
	}

	return 4;
}

void arcompact_disassembler::output_aux_regname(std::ostream& stream, uint32_t auxreg)
{
	if ((auxreg >= 0) && (auxreg < 0x420))
	{
		if (strcmp(auxregnames[auxreg], "unusedreg"))
			util::stream_format(stream, "[%s]", auxregnames[auxreg]);
		else
			util::stream_format(stream, "[%03x]", auxreg);
	}
	else
		util::stream_format(stream, "[%03x]", auxreg);
}

int arcompact_disassembler::handle::dasm32_LR(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)  // Load FROM Auxiliary register TO register
{
	//           pp        F
	// 0010 0bbb 0010 1010 0BBB CCCC CCRR RRRR
	// 0010 0bbb 0010 1010 0BBB 1111 10RR RRRR
	// 0010 0bbb 0110 1010 0BBB uuuu uu00 0000
	// 0010 0bbb 1010 1010 0BBB ssss ssSS SSSS
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	uint8_t p = common32_get_p(op);
	uint8_t breg = common32_get_breg(op);
	bool F = common32_get_F(op);

	util::stream_format(stream, "LR");
	if (F) util::stream_format(stream, ".<F set, illegal>");
//  util::stream_format(stream, " p(%d)", p);

	if (breg == DASM_REG_LIMM)
	{
		util::stream_format(stream, "<no dest>"); // illegal encoding?
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
			if (!got_limm)
			{
				limm = dasm_get_limm_32bit_opcode(pc, opcodes);
				size = 8;
			}

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
		output_aux_regname(stream, u);
	}
	else if (p == 2)
	{
		uint32_t S = common32_get_s12(op);
		output_aux_regname(stream, S);
	}
	else if (p == 3)
	{
		util::stream_format(stream, " <mode 3, illegal>");
	}

	return size;
}

int arcompact_disassembler::handle::dasm32_SR(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)  // Store TO Auxiliary register FROM register
{
	// code at ~ 40073DFE in leapster bios is manually setting up a loop this way
	// rather than using the lPcc opcode
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	uint8_t p = common32_get_p(op);
	uint8_t breg = common32_get_breg(op);
	bool F = common32_get_F(op);

	util::stream_format(stream, "SR");
	if (F) util::stream_format(stream, ".<F set, illegal>");
//  util::stream_format(stream, " p(%d)", p);

	if (breg == DASM_REG_LIMM)
	{
		limm = dasm_get_limm_32bit_opcode(pc, opcodes);
		size = 8;
		got_limm = 1;
		util::stream_format(stream, " 0x%08x -> ", limm);

	}
	else
	{
		util::stream_format(stream, " %s -> ", regnames[breg]);
	}

	if (p == 0)
	{
		uint8_t creg = common32_get_creg(op);

		if (creg == DASM_REG_LIMM)
		{
			if (!got_limm)
			{
				limm = dasm_get_limm_32bit_opcode(pc, opcodes);
				size = 8;
			}
			util::stream_format(stream, "[0x%08x]", limm);

		}
		else
		{
			util::stream_format(stream, "[%s]", regnames[creg]);
		}
	}
	else if (p == 1)
	{
		uint32_t u = common32_get_u6(op);
		output_aux_regname(stream, u);
	}
	else if (p == 2)
	{
		uint32_t S = common32_get_s12(op);
		output_aux_regname(stream, S);
	}
	else if (p == 3)
	{
		util::stream_format(stream, " <mode 3, illegal>");
	}
	return size;
}


int arcompact_disassembler::handle::dasm32_FLAG(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	// leapster bios uses formats for FLAG that are explicitly defined and are considered redundant
	return handle04_helper_dasm(stream, pc, op, opcodes, "FLAG", 1,1);
}
