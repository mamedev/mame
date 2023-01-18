// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\

 ARCompact disassembler

\*********************************/

#include "emu.h"

#include "arcompactdasm.h"

#define GET_01_01_01_BRANCH_ADDR \
	int32_t address = (op & 0x00fe0000) >> 17; \
	address |= ((op & 0x00008000) >> 15) << 7; \
	if (address & 0x80) address = -0x80 + (address & 0x7f); \
	op &= ~ 0x00fe800f;


#define GROUP_0e_GET_h \
	h =  ((op & 0x0007) << 3); \
	h |= ((op & 0x00e0) >> 5); \
	op &= ~0x00e7;
#define COMMON32_GET_breg \
	int b_temp = (op & 0x07000000) >> 24; op &= ~0x07000000; \
	int B_temp = (op & 0x00007000) >> 12; op &= ~0x00007000; \
	int breg = b_temp | (B_temp << 3);
#define COMMON32_GET_creg \
	int creg = (op & 0x00000fc0) >> 6; op &= ~0x00000fc0;
#define COMMON32_GET_u6 \
	int u = (op & 0x00000fc0) >> 6; op &= ~0x00000fc0;
#define COMMON32_GET_areg \
	int areg = (op & 0x0000003f) >> 0; op &= ~0x0000003f;
#define COMMON32_GET_areg_reserved \
	int ares = (op & 0x0000003f) >> 0; op &= ~0x0000003f;
#define COMMON32_GET_F \
	int F = (op & 0x00008000) >> 15; op &= ~0x00008000;
#define COMMON32_GET_p \
	int p = (op & 0x00c00000) >> 22; op &= ~0x00c00000;

#define COMMON32_GET_s12 \
		int S_temp = (op & 0x0000003f) >> 0; op &= ~0x0000003f; \
		int s_temp = (op & 0x00000fc0) >> 6; op &= ~0x00000fc0; \
		int S = s_temp | (S_temp<<6);
#define COMMON32_GET_CONDITION \
		uint8_t condition = op & 0x0000001f;  op &= ~0x0000001f;


#define COMMON16_GET_breg \
	breg =  ((op & 0x0700) >>8); \
	op &= ~0x0700;
#define COMMON16_GET_creg \
	creg =  ((op & 0x00e0) >>5); \
	op &= ~0x00e0;
#define COMMON16_GET_areg \
	areg =  ((op & 0x0007) >>0); \
	op &= ~0x0007;
#define COMMON16_GET_u3 \
	u =  ((op & 0x0007) >>0); \
	op &= ~0x0007;
#define COMMON16_GET_u5 \
	u =  ((op & 0x001f) >>0); \
	op &= ~0x001f;
#define COMMON16_GET_u8 \
	u =  ((op & 0x00ff) >>0); \
	op &= ~0x00ff;
#define COMMON16_GET_u7 \
	u =  ((op & 0x007f) >>0); \
	op &= ~0x007f;
#define COMMON16_GET_s9 \
	s =  ((op & 0x01ff) >>0); \
	op &= ~0x01ff;
// registers used in 16-bit opcodes hae a limited range
// and can only address registers r0-r3 and r12-r15

#define REG_16BIT_RANGE(_reg_) \
	if (_reg_>3) _reg_+= 8;

// this is as messed up as the rest of the 16-bit alignment in LE mode...

#define LIMM_REG 62
#define GET_LIMM \
	limm = (opcodes.r16(pc+4) << 16) | opcodes.r16(pc+6);

#define GET_LIMM16 \
	limm = (opcodes.r16(pc+2) << 16) | opcodes.r16(pc+4);

#define PC_ALIGNED32 \
	(pc&0xfffffffc)


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
	COMMON32_GET_CONDITION

	util::stream_format(stream, "B%s(%s) %08x", delaybit[n], conditions[condition], PC_ALIGNED32 + (address * 2));
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

	util::stream_format(stream, "B%s %08x", delaybit[n], PC_ALIGNED32 + (address * 2));
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

	COMMON32_GET_CONDITION

	util::stream_format(stream, "BL%s(%s) %08x", delaybit[n], conditions[condition], PC_ALIGNED32 + (address *2));
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

	util::stream_format(stream, "BL%s %08x", delaybit[n], PC_ALIGNED32 + (address *2));
	if (res)  util::stream_format(stream, "(reserved bit set)");

	return size;
}



int arcompact_disassembler::handle01_01_00_helper(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext)
{
	int size = 4;

	// Branch on Compare / Bit Test - Register-Register
	// 00001 bbb sssssss 1 S BBB CCCCCC N 0 iiii
	GET_01_01_01_BRANCH_ADDR


	COMMON32_GET_creg
	COMMON32_GET_breg;
	int n = (op & 0x00000020) >> 5; op &= ~0x00000020;

	op &= ~0x07007fe0;

	if ((breg != LIMM_REG) && (creg != LIMM_REG))
	{
		util::stream_format( stream, "%s%s %s, %s to 0x%08x", optext, delaybit[n], regnames[breg], regnames[creg], PC_ALIGNED32 + (address * 2) );
	}
	else
	{
		uint32_t limm;
		GET_LIMM;
		size = 8;

		if ((breg == LIMM_REG) && (creg != LIMM_REG))
		{
			util::stream_format( stream, "%s%s 0x%08x, %s to 0x%08x", optext, delaybit[n], limm, regnames[creg], PC_ALIGNED32 + (address * 2) );
		}
		else if ((creg == LIMM_REG) && (breg != LIMM_REG))
		{
			util::stream_format( stream, "%s%s %s, 0x%08x to 0x%08x", optext, delaybit[n], regnames[breg], limm, PC_ALIGNED32 + (address * 2) );
		}
		else
		{
			// b and c are LIMM? invalid??
			util::stream_format( stream, "%s%s 0x%08x, 0x%08x (illegal?) to 0x%08x", optext, delaybit[n], limm, limm, PC_ALIGNED32 + (address * 2) );

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
	GET_01_01_01_BRANCH_ADDR

	COMMON32_GET_u6
	COMMON32_GET_breg;
	int n = (op & 0x00000020) >> 5; op &= ~0x00000020;

	op &= ~0x07007fe0;

	util::stream_format(stream, "%s%s %s, 0x%02x %08x (%08x)", optext, delaybit[n], regnames[breg], u, PC_ALIGNED32 + (address * 2), op & ~0xf8fe800f);

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


int arcompact_disassembler::handle_dasm32_LD_r_o(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	// bitpos
	// 1111 1111 1111 1111 0000 0000 0000 0000
	// fedc ba98 7654 3210 fedc ba98 7654 3210
	// fields
	// 0001 0bbb ssss ssss SBBB DaaZ ZXAA AAAA
	int size = 4;

	COMMON32_GET_areg
	int X = (op & 0x00000040) >> 6;  //op &= ~0x00000040;
	int Z = (op & 0x00000180) >> 7;  //op &= ~0x00000180;
	int a = (op & 0x00000600) >> 9;  //op &= ~0x00000600;
	int D = (op & 0x00000800) >> 11;// op &= ~0x00000800;
	int S = (op & 0x00008000) >> 15;// op &= ~0x00008000;
	int s = (op & 0x00ff0000) >> 16;// op &= ~0x00ff0000;
	COMMON32_GET_breg;

	int sdat = s | (S << 8); // todo - signed

	uint32_t limm = 0;
	if (breg == LIMM_REG)
	{
		GET_LIMM;
		size = 8;
	}

	util::stream_format(stream, "LD");
	util::stream_format(stream, "%s", datasize[Z]);
	util::stream_format(stream, "%s", dataextend[X]);
	util::stream_format(stream, "%s", addressmode[a]);
	util::stream_format(stream, "%s", cachebit[D]);
	util::stream_format(stream, " ");
	util::stream_format(stream, "%s <- ", regnames[areg]);
	util::stream_format(stream, "[");
	if (breg == LIMM_REG) util::stream_format(stream, "(%08x), ", limm);
	else util::stream_format(stream, "%s, ", regnames[breg]);
	util::stream_format(stream, "%03x", sdat);
	util::stream_format(stream, "]");

	return size;
}

int arcompact_disassembler::handle_dasm32_ST_r_o(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;
	// bitpos
	// 1111 1111 1111 1111 0000 0000 0000 0000
	// fedc ba98 7654 3210 fedc ba98 7654 3210
	// fields
	// 0001 1bbb ssss ssss SBBB CCCC CCDa aZZR
	int S = (op & 0x00008000) >> 15;// op &= ~0x00008000;
	int s = (op & 0x00ff0000) >> 16;// op &= ~0x00ff0000;

	COMMON32_GET_breg;
	int sdat = s | (S << 8); // todo - signed

	int R = (op & 0x00000001) >> 0; op &= ~0x00000001;
	int Z = (op & 0x00000006) >> 1; op &= ~0x00000006;
	int a = (op & 0x00000018) >> 3; op &= ~0x00000018;
	int D = (op & 0x00000020) >> 5; op &= ~0x00000020;
	COMMON32_GET_creg

	if (breg == LIMM_REG)
	{
		GET_LIMM;
		size = 8;
		got_limm = 1;
	}


	util::stream_format(stream, "ST");
	util::stream_format(stream, "%s", datasize[Z]);
	util::stream_format(stream, "%s", addressmode[a]);
	util::stream_format(stream, "%s", cachebit[D]);
	util::stream_format(stream, " ");

	util::stream_format(stream, "[");
	if (breg == LIMM_REG) util::stream_format(stream, "(%08x), ", limm);
	else util::stream_format(stream, "%s, ", regnames[breg]);
	util::stream_format(stream, "%03x", sdat);
	util::stream_format(stream, "] <- ");

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM;
			size = 8;
		}
		util::stream_format(stream, "(%08x)", limm);

	}
	else
	{
		util::stream_format(stream, "%s", regnames[creg]);
	}

	if (R) util::stream_format(stream, "(reserved bit set)");


	return size;
}

int arcompact_disassembler::handle04_p00_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext, int ignore_dst, int b_reserved)
{
	//           PP
	// 0010 0bbb 00ii iiii FBBB CCCC CCAA AAAA
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg
	COMMON32_GET_areg

	util::stream_format(stream, "%s", optext);
	util::stream_format(stream, "%s", flagbit[F]);
	//  util::stream_format(stream, " p(%d)", p);


	if ((!b_reserved) && (breg == LIMM_REG))
	{
		GET_LIMM;
		size = 8;
		got_limm = 1;
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM;
			size = 8;
		}
	}

	// areg can be LIMM too, but in that case LIMM indicates 'no destination' rather than an actual LIMM value following

	if (ignore_dst == 0)
	{
		if (areg != LIMM_REG)  util::stream_format(stream, " %s <-", regnames[areg]);
		else util::stream_format(stream, " <no dst> <-");
	}
	else if (ignore_dst == 1) // certain opcode types ignore the 'a' field entirely, it should be set to 0.
	{
			if (areg) util::stream_format(stream, " <reserved %d> <-", areg);
	}
	else if (ignore_dst == 2) // for multiply operations areg should always be set to LIMM
	{
		if (areg != LIMM_REG) util::stream_format(stream, " <invalid %d> <-", areg);
		else  util::stream_format(stream, " <mulres> <-");
	}

	if (!b_reserved)
	{
		if (breg == LIMM_REG)
			util::stream_format(stream, " 0x%08x,", limm);
		else
			util::stream_format(stream, " %s,", regnames[breg]);
	}
	else
	{
		if (breg) util::stream_format(stream, "<reserved %d>,", breg);
	}

	if (creg == LIMM_REG)
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

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6
	COMMON32_GET_areg

	util::stream_format(stream, "%s", optext);
	util::stream_format(stream, "%s", flagbit[F]);
	//  util::stream_format(stream, " p(%d)", p);


	if ((!b_reserved) && (breg == LIMM_REG))
	{
		GET_LIMM;
		size = 8;
//      got_limm = 1;
	}

	// areg can be LIMM too, but in that case LIMM indicates 'no destination' rather than an actual LIMM value following

	if (ignore_dst == 0)
	{
		if (areg != LIMM_REG)  util::stream_format(stream, " %s <-", regnames[areg]);
		else util::stream_format(stream, " <no dst> <-");
	}
	else if (ignore_dst == 1) // certain opcode types ignore the 'a' field entirely, it should be set to 0.
	{
			if (areg) util::stream_format(stream, " <reserved %d> <-", areg);
	}
	else if (ignore_dst == 2) // for multiply operations areg should always be set to LIMM
	{
		if (areg != LIMM_REG) util::stream_format(stream, " <invalid %d> <-", areg);
		else  util::stream_format(stream, " <mulres> <-");
	}

	if (!b_reserved)
	{
		if (breg == LIMM_REG)
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

	COMMON32_GET_breg;
	COMMON32_GET_F
	COMMON32_GET_s12;

	util::stream_format(stream, "%s", optext);
	util::stream_format(stream, "%s", flagbit[F]);
	//  util::stream_format(stream, " p(%d)", p);


	if (!b_reserved)
	{
		if (breg == LIMM_REG)
		{
			GET_LIMM;
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

	COMMON32_GET_breg;
	COMMON32_GET_F
	COMMON32_GET_CONDITION;
	COMMON32_GET_creg

	util::stream_format(stream, "%s", optext);
	util::stream_format(stream, "%s", flagbit[F]);
	//  util::stream_format(stream, " p(%d)", p);

	if (!b_reserved)
	{
		if (breg == LIMM_REG)
		{
			GET_LIMM;
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


	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM;
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

	COMMON32_GET_breg;
	COMMON32_GET_F
	COMMON32_GET_CONDITION;
	COMMON32_GET_u6

	util::stream_format(stream, "%s", optext);
	util::stream_format(stream, "%s", flagbit[F]);
	//  util::stream_format(stream, " p(%d)", p);

	if (!b_reserved)
	{
		if (breg == LIMM_REG)
		{
			GET_LIMM;
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
	COMMON32_GET_p;

	switch (p)
	{
		case 0x00: return handle04_p00_helper_dasm(stream, pc, op, opcodes, optext, ignore_dst, b_reserved);
		case 0x01: return handle04_p01_helper_dasm(stream, pc, op, opcodes, optext, ignore_dst, b_reserved);
		case 0x02: return handle04_p10_helper_dasm(stream, pc, op, opcodes, optext, b_reserved);
		case 0x03: return handle04_p11_helper_dasm(stream, pc, op, opcodes, optext, b_reserved);
	}

	return 0;
}

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
	COMMON32_GET_breg; // breg is reserved
	COMMON32_GET_p;

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
		COMMON32_GET_s12
		if (S & 0x800) S = -0x800 + (S&0x7ff);

		util::stream_format(stream, "LP (start %08x, end %08x)", pc + 4, pc + S*2);
	}
	else if (p == 0x03) // Loop conditional
	{ // 0010 0RRR 1110 1000 0RRR uuuu uu1Q QQQQ
		COMMON32_GET_u6
		COMMON32_GET_CONDITION
		util::stream_format(stream, "LP<%s> (start %08x, end %08x)", conditions[condition], pc + 4, PC_ALIGNED32 + u*2);

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

	COMMON32_GET_p;
	COMMON32_GET_breg;
	COMMON32_GET_F

	util::stream_format(stream, "LR");
	if (F) util::stream_format(stream, ".<F set, illegal>");
//  util::stream_format(stream, " p(%d)", p);



	if (breg == LIMM_REG)
	{
		util::stream_format(stream, "<no dest>"); // illegal encoding?
	}
	else
	{
		util::stream_format(stream, " %s, ", regnames[breg]);
	}



	if (p == 0)
	{
		COMMON32_GET_creg
		COMMON32_GET_areg_reserved

		if (creg == LIMM_REG)
		{
			if (!got_limm)
			{
				GET_LIMM;
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
		COMMON32_GET_u6
		COMMON32_GET_areg_reserved

		int auxreg = u;
		PRINT_AUX_REGNAME

		if (ares) util::stream_format(stream, "reserved(%02x) ", ares);
	}
	else if (p == 2)
	{
		COMMON32_GET_s12;

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

	COMMON32_GET_p;
	COMMON32_GET_breg;
	COMMON32_GET_F

	util::stream_format(stream, "SR");
	if (F) util::stream_format(stream, ".<F set, illegal>");
//  util::stream_format(stream, " p(%d)", p);



	if (breg == LIMM_REG)
	{
		GET_LIMM;
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
		COMMON32_GET_creg
		COMMON32_GET_areg_reserved

		if (creg == LIMM_REG)
		{
			if (!got_limm)
			{
				GET_LIMM;
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
		COMMON32_GET_u6
		COMMON32_GET_areg_reserved

		int auxreg = u;
		PRINT_AUX_REGNAME

		if (ares) util::stream_format(stream, " (reserved %02x) ", ares);


	}
	else if (p == 2)
	{
		COMMON32_GET_s12;

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


int arcompact_disassembler::handle04_2f_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext)
{
	//
	// 0010 0bbb pp10 1111 FBBB CCCC CCII IIII
	int size = 4;

	COMMON32_GET_p;
	COMMON32_GET_breg;
	COMMON32_GET_F

	util::stream_format(stream, "%s", optext);
	util::stream_format(stream, "%s", flagbit[F]);
//  util::stream_format(stream, " p(%d)", p);

	if (breg == LIMM_REG)
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
		COMMON32_GET_creg

		if (creg == LIMM_REG)
		{
			uint32_t limm;
			GET_LIMM;
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
		COMMON32_GET_u6

		util::stream_format(stream, "U(0x%02x) ", u);
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


int arcompact_disassembler::handle_dasm32_ASL_single(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_2f_helper_dasm(stream, pc, op, opcodes, "ASL");
} // ASL

int arcompact_disassembler::handle_dasm32_ASR_single(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_2f_helper_dasm(stream, pc, op, opcodes, "ASR");
} // ASR

int arcompact_disassembler::handle_dasm32_LSR_single(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_2f_helper_dasm(stream, pc, op, opcodes, "LSR");
} // LSR

int arcompact_disassembler::handle_dasm32_ROR_single(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_2f_helper_dasm(stream, pc, op, opcodes, "ROR");
} // ROR

int arcompact_disassembler::handle_dasm32_RRC(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_2f_helper_dasm(stream, pc, op, opcodes, "RCC");
} // RCC

int arcompact_disassembler::handle_dasm32_SEXB(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_2f_helper_dasm(stream, pc, op, opcodes, "SEXB");
} // SEXB

int arcompact_disassembler::handle_dasm32_SEXW(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_2f_helper_dasm(stream, pc, op, opcodes, "SEXW");
} // SEXW

int arcompact_disassembler::handle_dasm32_EXTB(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_2f_helper_dasm(stream, pc, op, opcodes, "EXTB");
} // EXTB


int arcompact_disassembler::handle_dasm32_EXTW(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_2f_helper_dasm(stream, pc, op, opcodes, "EXTW");
} // EXTW



int arcompact_disassembler::handle_dasm32_ABS(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_2f_helper_dasm(stream, pc, op, opcodes, "ABS");
} // ABS

int arcompact_disassembler::handle_dasm32_NOT(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_2f_helper_dasm(stream, pc, op, opcodes, "NOT");
} // NOT

int arcompact_disassembler::handle_dasm32_RLC(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_2f_helper_dasm(stream, pc, op, opcodes, "RCL");
} // RLC

int arcompact_disassembler::handle_dasm32_EX(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle04_2f_helper_dasm(stream, pc, op, opcodes, "EX");
} // EX



int arcompact_disassembler::handle_dasm32_SLEEP(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	util::stream_format( stream, "SLEEP (%08x)", op);
	return 4;
}

int arcompact_disassembler::handle_dasm32_SWI(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	util::stream_format( stream, "SWI / TRAP0 (%08x)", op);
	return 4;
}

int arcompact_disassembler::handle_dasm32_SYNC(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	util::stream_format( stream, "SYNC (%08x)", op);
	return 4;
}

int arcompact_disassembler::handle_dasm32_RTIE(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	util::stream_format( stream, "RTIE (%08x)", op);
	return 4;
}

int arcompact_disassembler::handle_dasm32_BRK(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	util::stream_format( stream, "BRK (%08x)", op);
	return 4;
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
	COMMON32_GET_breg;
	int D = (op & 0x00008000) >> 15; op &= ~0x00008000;
	COMMON32_GET_creg
	COMMON32_GET_areg

	util::stream_format(stream, "%s", addressmode[mode]);
	util::stream_format(stream, "%s", cachebit[D]);

	util::stream_format(stream, " %s. ", regnames[areg]);

	if (breg == LIMM_REG)
	{
		GET_LIMM;
		size = 8;
		got_limm = 1;
		util::stream_format(stream, "[%08x, ", limm);

	}
	else
	{
		util::stream_format(stream, "[%s, ", regnames[breg]);
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM;
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

	COMMON32_GET_p;
	COMMON32_GET_breg;
	COMMON32_GET_F

	util::stream_format(stream, "%s", optext);
	util::stream_format(stream, "%s", flagbit[F]);
//  util::stream_format(stream, " p(%d)", p);


	util::stream_format(stream, " %s, ", regnames[breg]);

	if (p == 0)
	{
		COMMON32_GET_creg

		if (creg == LIMM_REG)
		{
			uint32_t limm;
			GET_LIMM;
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
		COMMON32_GET_u6
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



int arcompact_disassembler::handle0c_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext, int format)
{
	int areg, breg, creg;

	COMMON16_GET_areg;
	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(areg);
	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);


	if (format==0) util::stream_format(stream, "%s %s <- [%s, %s]", optext, regnames[areg], regnames[breg], regnames[creg]);
	else util::stream_format(stream, "%s %s <- %s, %s", optext, regnames[areg], regnames[breg], regnames[creg]);

	return 2;
}


int arcompact_disassembler::handle_dasm_LD_S_a_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0c_helper_dasm(stream, pc, op, opcodes, "LD_S", 0);
}

int arcompact_disassembler::handle_dasm_LDB_S_a_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0c_helper_dasm(stream, pc, op, opcodes, "LDB_S", 0);
}

int arcompact_disassembler::handleop_dasm_LDW_S_a_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0c_helper_dasm(stream, pc, op, opcodes, "LDW_S", 0);
}

int arcompact_disassembler::handle_dasm_ADD_S_a_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0c_helper_dasm(stream, pc, op, opcodes, "ADD_S", 1);
}


int arcompact_disassembler::handle0d_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext)
{
	int u, breg, creg;

	COMMON16_GET_u3;
	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	util::stream_format(stream, "%s %s <- [%s, 0x%02x]", optext, regnames[creg], regnames[breg], u);
	return 2;
}


int arcompact_disassembler::handle_dasm_ADD_S_c_b_u3(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0d_helper_dasm(stream, pc, op, opcodes, "ADD_S");
}

int arcompact_disassembler::handle_dasm_SUB_S_c_b_u3(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0d_helper_dasm(stream, pc, op, opcodes, "SUB_S");
}

int arcompact_disassembler::handle_dasm_ASL_S_c_b_u3(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0d_helper_dasm(stream, pc, op, opcodes, "ASL_S");
}

int arcompact_disassembler::handle_dasm_ASR_S_c_b_u3(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0d_helper_dasm(stream, pc, op, opcodes, "ASR_S");
}



int arcompact_disassembler::handle0e_0x_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext, int revop)
{
	int h,breg;
	int size = 2;

	GROUP_0e_GET_h;
	COMMON16_GET_breg;
	REG_16BIT_RANGE(breg);

	if (h == LIMM_REG)
	{
		uint32_t limm;
		GET_LIMM16;
		size = 6;
		if (!revop) util::stream_format( stream, "%s %s <- 0x%08x", optext, regnames[breg], limm);
		else util::stream_format( stream, "%s 0x%08x <- %s", optext, limm, regnames[breg]);
	}
	else
	{
		if (!revop) util::stream_format( stream, "%s %s <- %s", optext, regnames[breg], regnames[h]);
		else util::stream_format( stream, "%s %s <- %s", optext, regnames[h], regnames[breg]);

	}

	return size;

}

int arcompact_disassembler::handle_dasm_ADD_S_b_b_h_or_limm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0e_0x_helper_dasm(stream, pc, op, opcodes, "ADD_S", 0);
}

int arcompact_disassembler::handle_dasm_MOV_S_b_h_or_limm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0e_0x_helper_dasm(stream, pc, op, opcodes, "MOV_S", 0);
}

int arcompact_disassembler::handle_dasm_CMP_S_b_h_or_limm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0e_0x_helper_dasm(stream, pc, op, opcodes, "CMP_S", 0);
}

int arcompact_disassembler::handle_dasm_MOV_S_hob(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0e_0x_helper_dasm(stream, pc, op, opcodes, "MOV_S", 1);
}



int arcompact_disassembler::handle0f_00_0x_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext)
{
	int breg;

	COMMON16_GET_breg;
	REG_16BIT_RANGE(breg);

	util::stream_format( stream, "%s %s", optext, regnames[breg]);

	return 2;

}



int arcompact_disassembler::handle_dasm_J_S_b(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_00_0x_helper_dasm(stream, pc, op, opcodes, "J_S");
}

int arcompact_disassembler::handle_dasm_J_S_D_b(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_00_0x_helper_dasm(stream, pc, op, opcodes, "J_S.D");
}

int arcompact_disassembler::handle_dasm_JL_S_b(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_00_0x_helper_dasm(stream, pc, op, opcodes, "JL_S");
}

int arcompact_disassembler::handle_dasm_JL_S_D_b(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_00_0x_helper_dasm(stream, pc, op, opcodes, "JL_S.D");
}

int arcompact_disassembler::handle_dasm_SUB_S_NE_b_b_b(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_00_0x_helper_dasm(stream, pc, op, opcodes, "SUB_S.NE");
}





// Zero parameters (ZOP)
int arcompact_disassembler::handle_dasm_NOP_S(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	util::stream_format( stream, "NOP_S"); return 2;
}

int arcompact_disassembler::handle_dasm_UNIMP_S(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	util::stream_format( stream, "UNIMP_S"); return 2;
} // Unimplemented Instruction, same as illegal, but recommended to fill blank space

int arcompact_disassembler::handle_dasm_JEQ_S_blink(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	util::stream_format( stream, "JEQ_S [blink]"); return 2;
}

int arcompact_disassembler::handle_dasm_JNE_S_blink(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	util::stream_format( stream, "JNE_S [blink]"); return 2;
}

int arcompact_disassembler::handle_dasm_J_S_blink(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	util::stream_format( stream, "J_S [blink]"); return 2;
}

int arcompact_disassembler::handle_dasm_J_S_D_blink(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	util::stream_format( stream, "J_S.D [blink]"); return 2;
}






int arcompact_disassembler::handle0f_0x_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext, int nodst)
{
	int breg, creg;

	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	if (nodst==0) util::stream_format(stream, "%s %s <- %s", optext, regnames[breg], regnames[creg]);
	else if (nodst==1) util::stream_format(stream, "%s <no dst>, %s, %s", optext, regnames[breg], regnames[creg]);
	else if (nodst==2) util::stream_format(stream, "%s <mulres>, %s, %s", optext, regnames[breg], regnames[creg]);

	return 2;
}

int arcompact_disassembler::handle_dasm_SUB_S_b_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "SUB_S",0);
}

int arcompact_disassembler::handle_dasm_AND_S_b_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "AND_S",0);
}

int arcompact_disassembler::handle_dasm_OR_S_b_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "OR_S",0);
}

int arcompact_disassembler::handle_dasm_BIC_S_b_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "BIC_S",0);
}

int arcompact_disassembler::handle_dasm_XOR_S_b_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "XOR_S",0);
}

int arcompact_disassembler::handle_dasm_TST_S_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "TST_S",1);
}

int arcompact_disassembler::handle_dasm_MUL64_S_0_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "MUL64_S",2);
} // actual destination is special multiply registers

int arcompact_disassembler::handle_dasm_SEXB_S_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "SEXB_S",0);
}

int arcompact_disassembler::handle_dasm_SEXW_S_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "SEXW_S",0);
}

int arcompact_disassembler::handle_dasm_EXTB_S_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "EXTB_S",0);
}

int arcompact_disassembler::handle_dasm_EXTW_S_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "EXTW_S",0);
}

int arcompact_disassembler::handle_dasm_ABS_S_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "ABS_S",0);
}

int arcompact_disassembler::handle_dasm_NOT_S_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "NOT_S",0);
}

int arcompact_disassembler::handle_dasm_NEG_S_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "NEG_S",0);
}

int arcompact_disassembler::handle_dasm_ADD1_S_b_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "ADD1_S",0);
}

int arcompact_disassembler::handle_dasm_ADD2_S_b_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "ADD2_S",0);
}

int arcompact_disassembler::handle_dasm_ADD3_S_b_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "ADD3_S",0);
}

int arcompact_disassembler::handle_dasm_ASL_S_b_b_c_multiple(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "ASL_S",0);
}

int arcompact_disassembler::handle_dasm_LSR_S_b_b_c_multiple(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "LSR_S",0);
}

int arcompact_disassembler::handle_dasm_ASR_S_b_b_c_multiple(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "ASR_S",0);
}

int arcompact_disassembler::handle_dasm_ASL_S_b_c_single(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "ASL1_S",0);
}

int arcompact_disassembler::handle_dasm_ASR_S_b_c_single(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "ASR1_S",0);
}

int arcompact_disassembler::handle_dasm_LSR_S_b_c_single(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "LSR1_S",0);
}



int arcompact_disassembler::handle_dasm_TRAP_S_u6(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)  // special
{ // 0111 1uuu uuu1 1110
	int u = (op & 0x07e0)>>5;
	util::stream_format( stream, "TRAP_S %02x",u);
	return 2;
}

int arcompact_disassembler::handle_dasm_BRK_S(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)  // special
{
	int u = (op & 0x07e0)>>5; op &= ~0x07e0;

	if (u == 0x003f)
	{
		util::stream_format(stream, "BRK_S");
	}
	else
	{
		util::stream_format(stream, "<illegal BRK_S %02x>",u);
	}
	return 2;
}


int arcompact_disassembler::handle_ld_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext, int shift, int swap)
{
	int breg, creg, u;

	COMMON16_GET_breg;
	COMMON16_GET_creg;
	COMMON16_GET_u5;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	u <<= shift;

	if (!swap) util::stream_format(stream, "%s %s, [%s, 0x%02x] (%04x)", optext, regnames[creg], regnames[breg], u, op);
	else  util::stream_format(stream, "%s [%s, 0x%02x], %s (%04x)", optext, regnames[breg], u, regnames[creg], op);
	return 2;

}


int arcompact_disassembler::handle_dasm_LD_S_c_b_u7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle_ld_helper_dasm(stream, pc, op, opcodes, "LD_S", 2, 0);
}

int arcompact_disassembler::handle_dasm_LDB_S_c_b_u5(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle_ld_helper_dasm(stream, pc, op, opcodes, "LDB_S", 0, 0);
}

int arcompact_disassembler::handle_dasm_LDW_S_c_b_u6(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle_ld_helper_dasm(stream, pc, op, opcodes, "LDW_S", 1, 0);
}

int arcompact_disassembler::handle_dasm_LDW_S_X_c_b_u6(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle_ld_helper_dasm(stream, pc, op, opcodes, "LDW_S.X", 1, 0);
}

int arcompact_disassembler::handle_dasm_ST_S_c_b_u7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle_ld_helper_dasm(stream, pc, op, opcodes, "ST_S", 2, 1);
}

int arcompact_disassembler::handle_dasm_STB_S_c_b_u5(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle_ld_helper_dasm(stream, pc, op, opcodes, "STB_S", 0, 1);
}

int arcompact_disassembler::handle_dasm_STW_S_c_b_u6(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle_ld_helper_dasm(stream, pc, op, opcodes, "STW_S", 1, 1);
}


int arcompact_disassembler::handle_l7_0x_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext)
{
	int breg, u;

	COMMON16_GET_breg;
	COMMON16_GET_u5;

	REG_16BIT_RANGE(breg);

	util::stream_format(stream, "%s %s, 0x%02x", optext, regnames[breg], u);

	return 2;

}

int arcompact_disassembler::handle_dasm_ASL_S_b_b_u5(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle_l7_0x_helper_dasm(stream, pc, op, opcodes, "ASL_S");
}

int arcompact_disassembler::handle_dasm_LSR_S_b_b_u5(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle_l7_0x_helper_dasm(stream, pc, op, opcodes, "LSR_S");
}

int arcompact_disassembler::handle_dasm_ASR_S_b_b_u5(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle_l7_0x_helper_dasm(stream, pc, op, opcodes, "ASR_S");
}

int arcompact_disassembler::handle_dasm_SUB_S_b_b_u5(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle_l7_0x_helper_dasm(stream, pc, op, opcodes, "SUB_S");
}

int arcompact_disassembler::handle_dasm_BSET_S_b_b_u5(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle_l7_0x_helper_dasm(stream, pc, op, opcodes, "BSET_S");
}

int arcompact_disassembler::handle_dasm_BCLR_S_b_b_u5(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle_l7_0x_helper_dasm(stream, pc, op, opcodes, "BCLR_S");
}

int arcompact_disassembler::handle_dasm_BMSK_S_b_b_u5(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle_l7_0x_helper_dasm(stream, pc, op, opcodes, "BSMK_S");
}

int arcompact_disassembler::handle_dasm_BTST_S_b_u5(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle_l7_0x_helper_dasm(stream, pc, op, opcodes, "BTST_S");
}


// op bits remaining for 0x18_xx subgroups 0x071f

int arcompact_disassembler::handle18_0x_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext, int st, int format)
{
	int breg, u;

	COMMON16_GET_breg;
	COMMON16_GET_u5;

	REG_16BIT_RANGE(breg);

	util::stream_format(stream, "%s %s ", optext, regnames[breg]);
	if (st==1) util::stream_format(stream, "-> ");
	else util::stream_format(stream, "<- ");

	if (format==0) util::stream_format(stream, "[SP, 0x%02x]", u*4);
	else  util::stream_format(stream, "SP, 0x%02x", u*4);


	return 2;
}

int arcompact_disassembler::handle_dasm_LD_S_b_sp_u7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle18_0x_helper_dasm(stream, pc, op, opcodes, "LD_S", 0,0);
}

int arcompact_disassembler::handle_dasm_LDB_S_b_sp_u7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle18_0x_helper_dasm(stream, pc, op, opcodes, "LDB_S", 0,0);
}

int arcompact_disassembler::handle_dasm_ST_S_b_sp_u7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle18_0x_helper_dasm(stream, pc, op, opcodes, "ST_S", 1,0);
}

int arcompact_disassembler::handle_dasm_STB_S_b_sp_u7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle18_0x_helper_dasm(stream, pc, op, opcodes, "STB_S", 1,0);
}

int arcompact_disassembler::handle_dasm_ADD_S_b_sp_u7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle18_0x_helper_dasm(stream, pc, op, opcodes, "ADD_S", 1,1); // check format
}

// op bits remaining for 0x18_05_xx subgroups 0x001f
int arcompact_disassembler::handle_dasm_ADD_S_sp_sp_u7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int u;
	COMMON16_GET_u5;

	util::stream_format( stream, "ADD_S SP, SP, 0x%02x", u*4);
	return 2;

}

int arcompact_disassembler::handle_dasm_SUB_S_sp_sp_u7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int u;
	COMMON16_GET_u5;

	util::stream_format( stream, "SUB_S SP, SP, 0x%02x", u*4);
	return 2;
}

// op bits remaining for 0x18_06_xx subgroups 0x0700
int arcompact_disassembler::handle_dasm_POP_S_b(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int breg;
	COMMON16_GET_breg
	REG_16BIT_RANGE(breg)

	util::stream_format(stream, "POP_S %s", regnames[breg]);

	return 2;
}

int arcompact_disassembler::handle_dasm_POP_S_blink(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int res = (op & 0x0700) >> 8;
	op &= ~0x0700; // all bits now used

	if (res)
		util::stream_format(stream, "POP_S [BLINK] (Reserved Bits set %04x)", op);
	else
		util::stream_format(stream, "POP_S [BLINK]");

	return 2;
}

// op bits remaining for 0x18_07_xx subgroups 0x0700
int arcompact_disassembler::handle_dasm_PUSH_S_b(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int breg;
	COMMON16_GET_breg
	REG_16BIT_RANGE(breg)

	util::stream_format(stream, "PUSH_S %s", regnames[breg]);

	return 2;
}


int arcompact_disassembler::handle_dasm_PUSH_S_blink(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int res = (op & 0x0700) >> 8;
	op &= ~0x0700; // all bits now used

	if (res)
		util::stream_format(stream, "PUSH_S [BLINK] (Reserved Bits set %04x)", op);
	else
		util::stream_format(stream, "PUSH_S [BLINK]");

	return 2;
}


int arcompact_disassembler::handle19_0x_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext, int shift, int format)
{
	int s;

	COMMON16_GET_s9;
	// todo, signed
	s <<= shift;


	util::stream_format(stream, "%s %s, ", optext, regnames[0]);
	if (format == 0)
	{
		util::stream_format(stream, "[GP, %03x]", s);
	}
	else
	{
		util::stream_format(stream, "GP, %03x", s);
	}

	return 2;
}

int arcompact_disassembler::handle_dasm_LD_S_r0_gp_s11(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle19_0x_helper_dasm(stream, pc, op, opcodes, "LD_S", 2, 0);
}

int arcompact_disassembler::handle_dasm_LDB_S_r0_gp_s9(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle19_0x_helper_dasm(stream, pc, op, opcodes, "LDB_S", 0, 0);
}

int arcompact_disassembler::handle_dasm_LDW_S_r0_gp_s10(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle19_0x_helper_dasm(stream, pc, op, opcodes, "LDW_S", 1, 0);
}

int arcompact_disassembler::handle_dasm_ADD_S_r0_gp_s11(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle19_0x_helper_dasm(stream, pc, op, opcodes, "ADD_S", 2, 1);
}


int arcompact_disassembler::handle_dasm_LD_S_b_pcl_u10(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int breg, u;
	COMMON16_GET_breg;
	COMMON16_GET_u8;
	REG_16BIT_RANGE(breg);

	util::stream_format(stream, "MOV_S %s, [PCL, %03x]", regnames[breg], u*4);

	return 2;
}

int arcompact_disassembler::handle_dasm_MOV_S_b_u8(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int breg, u;
	COMMON16_GET_breg;
	COMMON16_GET_u8;
	REG_16BIT_RANGE(breg);

	util::stream_format(stream, "MOV_S %s <- 0x%02x", regnames[breg], u);
	return 2;
}

int arcompact_disassembler::handle_dasm_ADD_S_b_b_u7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int breg, u;
	COMMON16_GET_breg;
	COMMON16_GET_u7;
	REG_16BIT_RANGE(breg);

	util::stream_format(stream, "ADD_S %s <- %s, %02x", regnames[breg], regnames[breg], u);
	return 2;
}

int arcompact_disassembler::handle_dasm_CMP_S_b_u7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int breg, u;
	COMMON16_GET_breg;
	COMMON16_GET_u7;
	REG_16BIT_RANGE(breg);

	util::stream_format(stream, "CMP_S %s, %02x", regnames[breg], u);
	return 2;
}

int arcompact_disassembler::handle1d_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext)
{
	int breg;
	COMMON16_GET_breg;
	REG_16BIT_RANGE(breg);

	int s = (op & 0x007f) >> 0; op &= ~0x007f;
	if (s & 0x40) s = -0x40 + (s & 0x3f);

	util::stream_format(stream, "%s %s, 0 to 0x%08x", optext, regnames[breg], PC_ALIGNED32 + s*2);
	return 2;
}


int arcompact_disassembler::handle_dasm_BREQ_S_b_0_s8(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle1d_helper_dasm(stream, pc, op, opcodes,"BREQ_S");
}

int arcompact_disassembler::handle_dasm_BRNE_S_b_0_s8(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle1d_helper_dasm(stream, pc, op, opcodes,"BRNE_S");
}



int arcompact_disassembler::handle1e_0x_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext)
{
	int s = (op & 0x01ff) >> 0; op &= ~0x01ff;
	if (s & 0x100) s = -0x100 + (s & 0xff);

	util::stream_format(stream, "%s %08x", optext, PC_ALIGNED32 + s*2);
	return 2;
}



int arcompact_disassembler::handle_dasm_B_S_s10(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle1e_0x_helper_dasm(stream, pc, op, opcodes, "B_S");
}

int arcompact_disassembler::handle_dasm_BEQ_S_s10(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle1e_0x_helper_dasm(stream, pc, op, opcodes, "BEQ_S");
}

int arcompact_disassembler::handle_dasm_BNE_S_s10(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle1e_0x_helper_dasm(stream, pc, op, opcodes, "BNE_S");
}


int arcompact_disassembler::handle1e_03_0x_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext)
{
	int s = (op & 0x003f) >> 0; op &= ~0x003f;
	if (s & 0x020) s = -0x20 + (s & 0x1f);

	util::stream_format(stream, "%s %08x", optext, PC_ALIGNED32 + s*2);
	return 2;
}

int arcompact_disassembler::handle_dasm_BGT_S_s7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle1e_03_0x_helper_dasm(stream, pc, op, opcodes, "BGT_S");
}

int arcompact_disassembler::handle_dasm_BGE_S_s7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle1e_03_0x_helper_dasm(stream, pc, op, opcodes, "BGE_S");
}

int arcompact_disassembler::handle_dasm_BLT_S_s7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle1e_03_0x_helper_dasm(stream, pc, op, opcodes, "BLT_S");
}

int arcompact_disassembler::handle_dasm_BLE_S_s7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle1e_03_0x_helper_dasm(stream, pc, op, opcodes, "BLE_S");
}

int arcompact_disassembler::handle_dasm_BHI_S_s7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle1e_03_0x_helper_dasm(stream, pc, op, opcodes, "BHI_S");
}

int arcompact_disassembler::handle_dasm_BHS_S_s7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle1e_03_0x_helper_dasm(stream, pc, op, opcodes, "BHS_S");
}

int arcompact_disassembler::handle_dasm_BLO_S_s7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle1e_03_0x_helper_dasm(stream, pc, op, opcodes, "BLO_S");
}

int arcompact_disassembler::handle_dasm_BLS_S_s7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle1e_03_0x_helper_dasm(stream, pc, op, opcodes, "BLS_S");
}


int arcompact_disassembler::handle_dasm_BL_S_s13(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int s = (op & 0x07ff) >> 0; op &= ~0x07ff;
	if (s & 0x400) s = -0x400 + (s & 0x3ff);

	util::stream_format(stream, "BL_S %08x", PC_ALIGNED32 + (s*4));
	return 2;
}

/************************************************************************************************************************************
*                                                                                                                                   *
* illegal opcode handlers (disassembly)                                                                                             *
*                                                                                                                                   *
************************************************************************************************************************************/



int arcompact_disassembler::handle_dasm_illegal(std::ostream& stream, offs_t pc, uint8_t param1, uint8_t param2, uint16_t op, const data_buffer& opcodes)
{
	return 2;
}
int arcompact_disassembler::handle_dasm_illegal(std::ostream& stream, offs_t pc, uint8_t param1, uint8_t param2, uint8_t param3, uint16_t op, const data_buffer& opcodes)
{
	return 2;
}
int arcompact_disassembler::handle_dasm_illegal(std::ostream& stream, offs_t pc, uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4, uint16_t op, const data_buffer& opcodes)
{
	util::stream_format(stream, "<illegal 0x%02x_%02x_%02x_%02x> (%04x)\n", param1, param2, param3, param4, op);
	return 2;
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
