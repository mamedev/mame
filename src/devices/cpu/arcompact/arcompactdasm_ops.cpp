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

int arcompact_disassembler::handle_dasm32_B_cc_D_s21(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	int size = 4;
	// Branch Conditionally
	// 0000 0sss ssss sss0 SSSS SSSS SSNQ QQQQ
	int32_t address = (op & 0x07fe0000) >> 17;
	address |= ((op & 0x0000ffc0) >> 6) << 10;
	if (address & 0x80000) address = -0x80000 + (address & 0x7ffff);
	int n = (op & 0x00000020) >> 5; op &= ~0x00000020;
	DASM_COMMON32_GET_CONDITION

	util::stream_format(stream, "B%s(%s) %08x", delaybit[n], conditions[condition], DASM_PC_ALIGNED32 + (address * 2));
	return size;
}

int arcompact_disassembler::handle_dasm32_B_D_s25(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	int size = 4;
	// Branch Unconditionally Far
	// 0000 0sss ssss sss1 SSSS SSSS SSNR TTTT
	int32_t address = (op & 0x07fe0000) >> 17;
	address |= ((op & 0x0000ffc0) >> 6) << 10;
	address |= ((op & 0x0000000f) >> 0) << 20;
	if (address & 0x800000) address = -0x800000 + (address & 0x7fffff);
	int n = (op & 0x00000020) >> 5; op &= ~0x00000020;
	int res =  (op & 0x00000010) >> 4; op &= ~0x00000010;

	util::stream_format(stream, "B%s %08x", delaybit[n], DASM_PC_ALIGNED32 + (address * 2));
	if (res)  util::stream_format(stream, "(reserved bit set)");

	return size;
}

int arcompact_disassembler::handle_dasm32_BL_cc_d_s21(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	int size = 4;

	// Branch and Link Conditionally
	// 00001 sssssssss 00 SSSSSSSSSS N QQQQQ
	int32_t address =   (op & 0x07fc0000) >> 17;
	address |=        ((op & 0x0000ffc0) >> 6) << 10;
	if (address & 0x800000) address = -0x800000 + (address&0x7fffff);
	int n = (op & 0x00000020) >> 5; op &= ~0x00000020;

	DASM_COMMON32_GET_CONDITION

	util::stream_format(stream, "BL%s(%s) %08x", delaybit[n], conditions[condition], DASM_PC_ALIGNED32 + (address *2));
	return size;
}

int arcompact_disassembler::handle_dasm32_BL_d_s25(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	int size = 4;
	// Branch and Link Unconditionally Far
	// 00001 sssssssss 10  SSSSSSSSSS N R TTTT
	int32_t address =   (op & 0x07fc0000) >> 17;
	address |=        ((op & 0x0000ffc0) >> 6) << 10;
	address |=        ((op & 0x0000000f) >> 0) << 20;
	if (address & 0x800000) address = -0x800000 + (address&0x7fffff);
	int n = (op & 0x00000020) >> 5; op &= ~0x00000020;
	int res =  (op & 0x00000010) >> 4; op &= ~0x00000010;

	util::stream_format(stream, "BL%s %08x", delaybit[n], DASM_PC_ALIGNED32 + (address *2));
	if (res)  util::stream_format(stream, "(reserved bit set)");

	return size;
}



int arcompact_disassembler::handle01_01_00_helper(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext)
{
	int size = 4;

	// Branch on Compare / Bit Test - Register-Register
	// 00001 bbb sssssss 1 S BBB CCCCCC N 0 iiii
	DASM_GET_01_01_01_BRANCH_ADDR


	DASM_COMMON32_GET_creg
	DASM_COMMON32_GET_breg;
	int n = (op & 0x00000020) >> 5; op &= ~0x00000020;

	op &= ~0x07007fe0;

	if ((breg != DASM_LIMM_REG) && (creg != DASM_LIMM_REG))
	{
		util::stream_format( stream, "%s%s %s, %s to 0x%08x", optext, delaybit[n], regnames[breg], regnames[creg], DASM_PC_ALIGNED32 + (address * 2) );
	}
	else
	{
		uint32_t limm;
		DASM_GET_LIMM;
		size = 8;

		if ((breg == DASM_LIMM_REG) && (creg != DASM_LIMM_REG))
		{
			util::stream_format( stream, "%s%s 0x%08x, %s to 0x%08x", optext, delaybit[n], limm, regnames[creg], DASM_PC_ALIGNED32 + (address * 2) );
		}
		else if ((creg == DASM_LIMM_REG) && (breg != DASM_LIMM_REG))
		{
			util::stream_format( stream, "%s%s %s, 0x%08x to 0x%08x", optext, delaybit[n], regnames[breg], limm, DASM_PC_ALIGNED32 + (address * 2) );
		}
		else
		{
			// b and c are LIMM? invalid??
			util::stream_format( stream, "%s%s 0x%08x, 0x%08x (illegal?) to 0x%08x", optext, delaybit[n], limm, limm, DASM_PC_ALIGNED32 + (address * 2) );

		}
	}

	return size;
}


// register - register cases
int arcompact_disassembler::handle_dasm32_BREQ_reg_reg(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle01_01_00_helper( stream, pc, op, opcodes, "BREQ");
}

int arcompact_disassembler::handle_dasm32_BRNE_reg_reg(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle01_01_00_helper( stream, pc, op, opcodes, "BRNE");
}

int arcompact_disassembler::handle_dasm32_BRLT_reg_reg(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle01_01_00_helper( stream, pc, op, opcodes, "BRLT");
}

int arcompact_disassembler::handle_dasm32_BRGE_reg_reg(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle01_01_00_helper( stream, pc, op, opcodes, "BRGE");
}

int arcompact_disassembler::handle_dasm32_BRLO_reg_reg(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle01_01_00_helper( stream, pc, op, opcodes, "BRLO");
}

int arcompact_disassembler::handle_dasm32_BRHS_reg_reg(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle01_01_00_helper( stream, pc, op, opcodes, "BRHS");
}

int arcompact_disassembler::handle_dasm32_BBIT0_reg_reg(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle01_01_00_helper( stream, pc, op, opcodes, "BBIT0");
}

int arcompact_disassembler::handle_dasm32_BBIT1_reg_reg(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle01_01_00_helper( stream, pc, op, opcodes, "BBIT1");
}


int arcompact_disassembler::handle01_01_01_helper(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext)
{
	int size = 4;

	// using 'b' as limm here makes no sense (comparing a long immediate against a short immediate) so I assume it isn't
	// valid?

	// Branch on Compare / Bit Test - Register-Immediate
	// 0000 1bbb ssss sss1 SBBB uuuu uuN1 iiii
	DASM_GET_01_01_01_BRANCH_ADDR

	DASM_COMMON32_GET_u6
	DASM_COMMON32_GET_breg;
	int n = (op & 0x00000020) >> 5; op &= ~0x00000020;

	op &= ~0x07007fe0;

	util::stream_format(stream, "%s%s %s, 0x%02x %08x (%08x)", optext, delaybit[n], regnames[breg], u, DASM_PC_ALIGNED32 + (address * 2), op & ~0xf8fe800f);

	return size;
}

// register -immediate cases
int arcompact_disassembler::handle_dasm32_BREQ_reg_imm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle01_01_01_helper(stream, pc, op, opcodes, "BREQ");
}

int arcompact_disassembler::handle_dasm32_BRNE_reg_imm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle01_01_01_helper(stream, pc, op, opcodes, "BRNE");
}

int arcompact_disassembler::handle_dasm32_BRLT_reg_imm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle01_01_01_helper(stream, pc, op, opcodes, "BRLT");
}

int arcompact_disassembler::handle_dasm32_BRGE_reg_imm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle01_01_01_helper(stream, pc, op, opcodes, "BRGE");
}

int arcompact_disassembler::handle_dasm32_BRLO_reg_imm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle01_01_01_helper(stream, pc, op, opcodes, "BRLO");
}

int arcompact_disassembler::handle_dasm32_BRHS_reg_imm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle01_01_01_helper(stream, pc, op, opcodes, "BRHS");
}

int arcompact_disassembler::handle_dasm32_BBIT0_reg_imm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle01_01_01_helper(stream, pc, op, opcodes, "BBIT0");
}

int arcompact_disassembler::handle_dasm32_BBIT1_reg_imm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle01_01_01_helper(stream, pc, op, opcodes, "BBIT1");
}


int arcompact_disassembler::handle04_p00_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext, int ignore_dst, int b_reserved)
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
int arcompact_disassembler::handle04_p01_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext, int ignore_dst, int b_reserved)
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


int arcompact_disassembler::handle04_p10_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext, int b_reserved)
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

int arcompact_disassembler::handle04_p11_m0_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext, int b_reserved)
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

int arcompact_disassembler::handle04_p11_m1_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext, int b_reserved)
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
		case 0x00: return handle04_p11_m0_helper_dasm(stream, pc, op, opcodes, optext, b_reserved);
		case 0x01: return handle04_p11_m1_helper_dasm(stream, pc, op, opcodes, optext, b_reserved);
	}
	return 0;
}


int arcompact_disassembler::handle04_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext, int ignore_dst, int b_reserved)
{
	DASM_COMMON32_GET_p;

	switch (p)
	{
		case 0x00: return handle04_p00_helper_dasm(stream, pc, op, opcodes, optext, ignore_dst, b_reserved);
		case 0x01: return handle04_p01_helper_dasm(stream, pc, op, opcodes, optext, ignore_dst, b_reserved);
		case 0x02: return handle04_p10_helper_dasm(stream, pc, op, opcodes, optext, b_reserved);
		case 0x03: return handle04_p11_helper_dasm(stream, pc, op, opcodes, optext, b_reserved);
	}

	return 0;
}



int arcompact_disassembler::handle_dasm32_Jcc(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "J", 1,1);
}



int arcompact_disassembler::handle_dasm32_Jcc_D(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "J.D", 1,1);
}

int arcompact_disassembler::handle_dasm32_JLcc(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "JL", 1,1);
}

int arcompact_disassembler::handle_dasm32_JLcc_D(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "JL.D", 1,1);
}




int arcompact_disassembler::handle_dasm32_LP(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes) // LPcc (loop setup)
{
	DASM_COMMON32_GET_breg; // breg is reserved
	DASM_COMMON32_GET_p;

	if (p == 0x00)
	{
		util::stream_format(stream, "<illegal LPcc, p = 0x00)");
	}
	else if (p == 0x01)
	{
		util::stream_format(stream, "<illegal LPcc, p = 0x01)");
	}
	else if (p == 0x02) // Loop unconditional
	{ // 0010 0RRR 1010 1000 0RRR ssss ssSS SSSS
		DASM_COMMON32_GET_s12
		if (S & 0x800) S = -0x800 + (S&0x7ff);

		util::stream_format(stream, "LP (start %08x, end %08x)", pc + 4, pc + S*2);
	}
	else if (p == 0x03) // Loop conditional
	{ // 0010 0RRR 1110 1000 0RRR uuuu uu1Q QQQQ
		DASM_COMMON32_GET_u6
		DASM_COMMON32_GET_CONDITION
		util::stream_format(stream, "LP<%s> (start %08x, end %08x)", conditions[condition], pc + 4, DASM_PC_ALIGNED32 + u*2);

		int unused = (op & 0x00000020)>>5;
		if (unused==0)  util::stream_format(stream, "(unused bit not set)");

	}

	if (breg) util::stream_format(stream, "(reseved B bits set %02x)", breg);

	return 4;
}

#define PRINT_AUX_REGNAME \
		if ((auxreg >= 0) && (auxreg < 0x420)) \
		{ \
			if (strcmp(auxregnames[auxreg],"unusedreg")) \
				util::stream_format(stream, "[%s]", auxregnames[auxreg]); \
			else \
				util::stream_format(stream, "[%03x]", auxreg); \
		} \
		else \
			util::stream_format(stream, "[%03x]", auxreg);

int arcompact_disassembler::handle_dasm32_LR(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)  // Load FROM Auxiliary register TO register
{
	//           pp        F
	// 0010 0bbb 0010 1010 0BBB CCCC CCRR RRRR
	// 0010 0bbb 0010 1010 0BBB 1111 10RR RRRR
	// 0010 0bbb 0110 1010 0BBB uuuu uu00 0000
	// 0010 0bbb 1010 1010 0BBB ssss ssSS SSSS


	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	DASM_COMMON32_GET_p;
	DASM_COMMON32_GET_breg;
	DASM_COMMON32_GET_F

	util::stream_format(stream, "LR");
	if (F) util::stream_format(stream, ".<F set, illegal>");
//  util::stream_format(stream, " p(%d)", p);



	if (breg == DASM_LIMM_REG)
	{
		util::stream_format(stream, "<no dest>"); // illegal encoding?
	}
	else
	{
		util::stream_format(stream, " %s, ", regnames[breg]);
	}



	if (p == 0)
	{
		DASM_COMMON32_GET_creg
		DASM_COMMON32_GET_areg_reserved

		if (creg == DASM_LIMM_REG)
		{
			if (!got_limm)
			{
				DASM_GET_LIMM;
				size = 8;
			}

			util::stream_format(stream, "(%08x) ", limm);

		}
		else
		{
			util::stream_format(stream, "C(%s) ", regnames[creg]);
		}

		if (ares) util::stream_format(stream, "reserved(%02x) ", ares);
	}
	else if (p == 1)
	{
		DASM_COMMON32_GET_u6
		DASM_COMMON32_GET_areg_reserved

		int auxreg = u;
		PRINT_AUX_REGNAME

		if (ares) util::stream_format(stream, "reserved(%02x) ", ares);
	}
	else if (p == 2)
	{
		DASM_COMMON32_GET_s12;

		int auxreg = S;
		PRINT_AUX_REGNAME

	}
	else if (p == 3)
	{
		util::stream_format(stream, " <mode 3, illegal>");
	}

	return size;
}

int arcompact_disassembler::handle_dasm32_SR(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)  // Store TO Auxiliary register FROM register
{
	// code at ~ 40073DFE in leapster bios is manually setting up a loop this way
	// rather than using the lPcc opcode

	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	DASM_COMMON32_GET_p;
	DASM_COMMON32_GET_breg;
	DASM_COMMON32_GET_F

	util::stream_format(stream, "SR");
	if (F) util::stream_format(stream, ".<F set, illegal>");
//  util::stream_format(stream, " p(%d)", p);



	if (breg == DASM_LIMM_REG)
	{
		DASM_GET_LIMM;
		size = 8;
		got_limm = 1;
		util::stream_format(stream, " %08x -> ", limm);

	}
	else
	{
		util::stream_format(stream, " %s -> ", regnames[breg]);
	}



	if (p == 0)
	{
		DASM_COMMON32_GET_creg
		DASM_COMMON32_GET_areg_reserved

		if (creg == DASM_LIMM_REG)
		{
			if (!got_limm)
			{
				DASM_GET_LIMM;
				size = 8;
			}

			util::stream_format(stream, "[%08x]", limm);

		}
		else
		{
			util::stream_format(stream, "[%s]", regnames[creg]);


		}

		if (ares) util::stream_format(stream, " (reserved %02x) ", ares);


	}
	else if (p == 1)
	{
		DASM_COMMON32_GET_u6
		DASM_COMMON32_GET_areg_reserved

		int auxreg = u;
		PRINT_AUX_REGNAME

		if (ares) util::stream_format(stream, " (reserved %02x) ", ares);


	}
	else if (p == 2)
	{
		DASM_COMMON32_GET_s12;

		int auxreg = S;

		PRINT_AUX_REGNAME

	}
	else if (p == 3)
	{
		util::stream_format(stream, " <mode 3, illegal>");
	}

	return size;}


int arcompact_disassembler::handle_dasm32_FLAG(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	// leapster bios uses formats for FLAG that are not defined, bug I guess work anyway (P modes 0 / 1)
	return handle04_helper_dasm(stream, pc, op, opcodes, "FLAG", 1,1);
}






// format on these is..

// 0010 0bbb aa11 0ZZX DBBB CCCC CCAA AAAA
// note, bits  11 0ZZX are part of the sub-opcode # already - this is a special encoding
int arcompact_disassembler::handle04_3x_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, int dsize, int extend)
{
	int size = 4;
	uint32_t limm=0;
	int got_limm = 0;

	util::stream_format(stream, "LD");
	util::stream_format(stream, "%s", datasize[dsize]);
	util::stream_format(stream, "%s", dataextend[extend]);

	int mode = (op & 0x00c00000) >> 22; op &= ~0x00c00000;
	DASM_COMMON32_GET_breg;
	int D = (op & 0x00008000) >> 15; op &= ~0x00008000;
	DASM_COMMON32_GET_creg
	DASM_COMMON32_GET_areg

	util::stream_format(stream, "%s", addressmode[mode]);
	util::stream_format(stream, "%s", cachebit[D]);

	util::stream_format(stream, " %s. ", regnames[areg]);

	if (breg == DASM_LIMM_REG)
	{
		DASM_GET_LIMM;
		size = 8;
		got_limm = 1;
		util::stream_format(stream, "[%08x, ", limm);

	}
	else
	{
		util::stream_format(stream, "[%s, ", regnames[breg]);
	}

	if (creg == DASM_LIMM_REG)
	{
		if (!got_limm)
		{
			DASM_GET_LIMM;
			size = 8;
		}
		util::stream_format(stream, "(%08x)]", limm);

	}
	else
	{
		util::stream_format(stream, "%s]", regnames[creg]);
	}


	return size;



}

int arcompact_disassembler::handle_dasm32_LD_0(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_3x_helper_dasm(stream, pc, op, opcodes,0,0);
}

// ZZ value of 0x0 with X of 1 is illegal
int arcompact_disassembler::handle_dasm32_LD_1(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_3x_helper_dasm(stream, pc, op, opcodes,0,1);
}

int arcompact_disassembler::handle_dasm32_LD_2(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_3x_helper_dasm(stream, pc, op, opcodes,1,0);
}

int arcompact_disassembler::handle_dasm32_LD_3(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_3x_helper_dasm(stream, pc, op, opcodes,1,1);
}

int arcompact_disassembler::handle_dasm32_LD_4(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_3x_helper_dasm(stream, pc, op, opcodes,2,0);
}

int arcompact_disassembler::handle_dasm32_LD_5(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_3x_helper_dasm(stream, pc, op, opcodes,2,1);
}

// ZZ value of 0x3 is illegal
int arcompact_disassembler::handle_dasm32_LD_6(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_3x_helper_dasm(stream, pc, op, opcodes,3,0);
}

int arcompact_disassembler::handle_dasm32_LD_7(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_3x_helper_dasm(stream, pc, op, opcodes,3,1);
}







int arcompact_disassembler::handle_dasm32_ASL_multiple(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "ASL", 0,0);
}

int arcompact_disassembler::handle_dasm32_LSR_multiple(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "LSR", 0,0);
}

int arcompact_disassembler::handle_dasm32_ASR_multiple(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "ASR", 0,0);
}

int arcompact_disassembler::handle_dasm32_ROR_multiple(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "ROR", 0,0);
}

int arcompact_disassembler::handle_dasm32_MUL64(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "MUL64", 2,0);
} // special

int arcompact_disassembler::handle_dasm32_MULU64(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "MULU64", 2,0);
} // special

int arcompact_disassembler::handle_dasm32_ADDS(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "ADDS", 0,0);
}

int arcompact_disassembler::handle_dasm32_SUBS(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "SUBS", 0,0);
}

int arcompact_disassembler::handle_dasm32_DIVAW(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "DIVAW", 0,0);
}




int arcompact_disassembler::handle_dasm32_ASLS(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "ASLS", 0,0);
}

int arcompact_disassembler::handle_dasm32_ASRS(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "ASRS", 0,0);
}


int arcompact_disassembler::handle_dasm32_ADDSDW(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "ADDSDW", 0,0);
}

int arcompact_disassembler::handle_dasm32_SUBSDW(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_helper_dasm(stream, pc, op, opcodes, "SUBSDW", 0,0);
}




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



int arcompact_disassembler::handle_dasm32_ARC_EXT06(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	util::stream_format( stream, "op a,b,c (06 ARC ext) (%08x)", op );
	return 4;
}

int arcompact_disassembler::handle_dasm32_USER_EXT07(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	util::stream_format( stream, "op a,b,c (07 User ext) (%08x)", op );
	return 4;
}

int arcompact_disassembler::handle_dasm32_USER_EXT08(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	util::stream_format( stream, "op a,b,c (08 User ext) (%08x)", op );
	return 4;
}

int arcompact_disassembler::handle_dasm32_MARKET_EXT09(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	util::stream_format( stream, "op a,b,c (09 Market ext) (%08x)", op );
	return 4;
}

int arcompact_disassembler::handle_dasm32_MARKET_EXT0a(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	util::stream_format( stream, "op a,b,c (0a Market ext) (%08x)",  op );
	return 4;
}

int arcompact_disassembler::handle_dasm32_MARKET_EXT0b(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	util::stream_format( stream, "op a,b,c (0b Market ext) (%08x)",  op );
	return 4;
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
