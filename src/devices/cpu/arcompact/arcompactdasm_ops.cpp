// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\

 ARCompact disassembler

\*********************************/

#include "emu.h"
#include <stdarg.h>

#include "arcompactdasm_ops.h"

char *output;

static void ATTR_PRINTF(1,2) print(const char *fmt, ...)
{
	va_list vl;

	va_start(vl, fmt);
	vsprintf(output, fmt, vl);
	va_end(vl);
}



#define GET_01_01_01_BRANCH_ADDR \
	INT32 address = (op & 0x00fe0000) >> 17; \
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
		UINT8 condition = op & 0x0000001f;  op &= ~0x0000001f;


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

#define GET_LIMM \
	limm = oprom[4] | (oprom[5] << 8); \
	limm |= (oprom[2] << 16) | (oprom[3] << 24);
#define PC_ALIGNED32 \
	(pc&0xfffffffc)


/************************************************************************************************************************************
*                                                                                                                                   *
* individual opcode handlers (disassembly)                                                                                          *
*                                                                                                                                   *
************************************************************************************************************************************/

int arcompact_handle00_00_dasm(DASM_OPS_32)
{
	int size = 4;
	// Branch Conditionally
	// 0000 0sss ssss sss0 SSSS SSSS SSNQ QQQQ
	INT32 address = (op & 0x07fe0000) >> 17;
	address |= ((op & 0x0000ffc0) >> 6) << 10;
	if (address & 0x80000) address = -0x80000 + (address & 0x7ffff);
	int n = (op & 0x00000020) >> 5; op &= ~0x00000020;
	COMMON32_GET_CONDITION

	output  += sprintf( output, "B%s(%s) %08x", delaybit[n], conditions[condition], PC_ALIGNED32 + (address * 2));
	return size;
}

int arcompact_handle00_01_dasm(DASM_OPS_32)
{
	int size = 4;
	// Branch Unconditionally Far
	// 0000 0sss ssss sss1 SSSS SSSS SSNR TTTT
	INT32 address = (op & 0x07fe0000) >> 17;
	address |= ((op & 0x0000ffc0) >> 6) << 10;
	address |= ((op & 0x0000000f) >> 0) << 20;
	if (address & 0x800000) address = -0x800000 + (address & 0x7fffff);
	int n = (op & 0x00000020) >> 5; op &= ~0x00000020;
	int res =  (op & 0x00000010) >> 4; op &= ~0x00000010;

	output  += sprintf( output, "B%s %08x", delaybit[n], PC_ALIGNED32 + (address * 2) );
	if (res)  output += sprintf(output, "(reserved bit set)");

	return size;
}

int arcompact_handle01_00_00dasm(DASM_OPS_32)
{
	int size = 4;

	// Branch and Link Conditionally
	// 00001 sssssssss 00 SSSSSSSSSS N QQQQQ
	INT32 address =   (op & 0x07fc0000) >> 17;
	address |=        ((op & 0x0000ffc0) >> 6) << 10;
	if (address & 0x800000) address = -0x800000 + (address&0x7fffff);
	int n = (op & 0x00000020) >> 5; op &= ~0x00000020;

	COMMON32_GET_CONDITION

	output  += sprintf( output, "BL%s(%s) %08x", delaybit[n], conditions[condition], PC_ALIGNED32 + (address *2) );
	return size;
}

int arcompact_handle01_00_01dasm(DASM_OPS_32)
{
	int size = 4;
	// Branch and Link Unconditionally Far
	// 00001 sssssssss 10  SSSSSSSSSS N R TTTT
	INT32 address =   (op & 0x07fc0000) >> 17;
	address |=        ((op & 0x0000ffc0) >> 6) << 10;
	address |=        ((op & 0x0000000f) >> 0) << 20;
	if (address & 0x800000) address = -0x800000 + (address&0x7fffff);
	int n = (op & 0x00000020) >> 5; op &= ~0x00000020;
	int res =  (op & 0x00000010) >> 4; op &= ~0x00000010;

	output  += sprintf( output, "BL%s %08x", delaybit[n], PC_ALIGNED32 + (address *2) );
	if (res)  output += sprintf(output, "(reserved bit set)");

	return size;
}



int arcompact_01_01_00_helper(DASM_OPS_32, const char* optext)
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
		print("%s%s %s, %s to 0x%08x", optext, delaybit[n], regnames[breg], regnames[creg], PC_ALIGNED32 + (address * 2) );
	}
	else
	{
		UINT32 limm;
		GET_LIMM_32;
		size = 8;

		if ((breg == LIMM_REG) && (creg != LIMM_REG))
		{
			print("%s%s 0x%08x, %s to 0x%08x", optext, delaybit[n], limm, regnames[creg], PC_ALIGNED32 + (address * 2) );
		}
		else if ((creg == LIMM_REG) && (breg != LIMM_REG))
		{
			print("%s%s %s, 0x%08x to 0x%08x", optext, delaybit[n], regnames[breg], limm, PC_ALIGNED32 + (address * 2) );
		}
		else
		{
			// b and c are LIMM? invalid??
			print("%s%s 0x%08x, 0x%08x (illegal?) to 0x%08x", optext, delaybit[n], limm, limm, PC_ALIGNED32 + (address * 2) );

		}
	}

	return size;
}


// register - register cases
int arcompact_handle01_01_00_00_dasm(DASM_OPS_32)  { return arcompact_01_01_00_helper( DASM_PARAMS, "BREQ"); }
int arcompact_handle01_01_00_01_dasm(DASM_OPS_32)  { return arcompact_01_01_00_helper( DASM_PARAMS, "BRNE"); }
int arcompact_handle01_01_00_02_dasm(DASM_OPS_32)  { return arcompact_01_01_00_helper( DASM_PARAMS, "BRLT"); }
int arcompact_handle01_01_00_03_dasm(DASM_OPS_32)  { return arcompact_01_01_00_helper( DASM_PARAMS, "BRGE"); }
int arcompact_handle01_01_00_04_dasm(DASM_OPS_32)  { return arcompact_01_01_00_helper( DASM_PARAMS, "BRLO"); }
int arcompact_handle01_01_00_05_dasm(DASM_OPS_32)  { return arcompact_01_01_00_helper( DASM_PARAMS, "BRHS"); }
int arcompact_handle01_01_00_0e_dasm(DASM_OPS_32)  { return arcompact_01_01_00_helper( DASM_PARAMS, "BBIT0");}
int arcompact_handle01_01_00_0f_dasm(DASM_OPS_32)  { return arcompact_01_01_00_helper( DASM_PARAMS, "BBIT1");}

int arcompact_01_01_01_helper(DASM_OPS_32, const char* optext)
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

	print("%s%s %s, 0x%02x %08x (%08x)", optext, delaybit[n], regnames[breg], u, PC_ALIGNED32 + (address * 2), op & ~0xf8fe800f);

	return size;
}

// register -immediate cases
int arcompact_handle01_01_01_00_dasm(DASM_OPS_32)  { return arcompact_01_01_01_helper(DASM_PARAMS, "BREQ"); }
int arcompact_handle01_01_01_01_dasm(DASM_OPS_32)  { return arcompact_01_01_01_helper(DASM_PARAMS, "BRNE"); }
int arcompact_handle01_01_01_02_dasm(DASM_OPS_32)  { return arcompact_01_01_01_helper(DASM_PARAMS, "BRLT"); }
int arcompact_handle01_01_01_03_dasm(DASM_OPS_32)  { return arcompact_01_01_01_helper(DASM_PARAMS, "BRGE"); }
int arcompact_handle01_01_01_04_dasm(DASM_OPS_32)  { return arcompact_01_01_01_helper(DASM_PARAMS, "BRLO"); }
int arcompact_handle01_01_01_05_dasm(DASM_OPS_32)  { return arcompact_01_01_01_helper(DASM_PARAMS, "BRHS"); }
int arcompact_handle01_01_01_0e_dasm(DASM_OPS_32)  { return arcompact_01_01_01_helper(DASM_PARAMS, "BBIT0"); }
int arcompact_handle01_01_01_0f_dasm(DASM_OPS_32)  { return arcompact_01_01_01_helper(DASM_PARAMS, "BBIT1"); }


int arcompact_handle02_dasm(DASM_OPS_32)
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

	UINT32 limm = 0;
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
	}

	output  += sprintf( output, "LD");
	output  += sprintf( output, "%s", datasize[Z]);
	output  += sprintf( output, "%s", dataextend[X]);
	output  += sprintf( output, "%s", addressmode[a]);
	output  += sprintf( output, "%s", cachebit[D]);
	output  += sprintf( output, " ");
	output  += sprintf( output, "%s <- ", regnames[areg]);
	output  += sprintf( output, "[");
	if (breg == LIMM_REG) output  += sprintf( output, "(%08x), ", limm);
	else output  += sprintf( output, "%s, ", regnames[breg]);
	output  += sprintf( output, "%03x", sdat);
	output  += sprintf( output, "]");

	return size;
}

int arcompact_handle03_dasm(DASM_OPS_32)
{
	int size = 4;
	UINT32 limm = 0;
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
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
	}


	output  += sprintf( output, "ST");
	output  += sprintf( output, "%s", datasize[Z]);
	output  += sprintf( output, "%s", addressmode[a]);
	output  += sprintf( output, "%s", cachebit[D]);
	output  += sprintf( output, " ");

	output  += sprintf( output, "[");
	if (breg == LIMM_REG) output  += sprintf( output, "(%08x), ", limm);
	else output  += sprintf( output, "%s, ", regnames[breg]);
	output  += sprintf( output, "%03x", sdat);
	output  += sprintf( output, "] <- ");

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		output += sprintf(output, "(%08x)", limm);

	}
	else
	{
		output += sprintf(output, "%s", regnames[creg]);
	}

	if (R) output  += sprintf( output, "(reserved bit set)");


	return size;
}

int arcompact_handle04_p00_helper_dasm(DASM_OPS_32, const char* optext, int ignore_dst, int b_reserved)
{
	//           PP
	// 0010 0bbb 00ii iiii FBBB CCCC CCAA AAAA
	int size = 4;
	UINT32 limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg
	COMMON32_GET_areg

	output += sprintf(output, "%s", optext);
	output += sprintf(output, "%s", flagbit[F]);
	//  output  += sprintf( output, " p(%d)", p);


	if ((!b_reserved) && (breg == LIMM_REG))
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
	}

	// areg can be LIMM too, but in that case LIMM indicates 'no destination' rather than an actual LIMM value following

	if (ignore_dst == 0)
	{
		if (areg != LIMM_REG)  output += sprintf(output, " %s <-", regnames[areg]);
		else output += sprintf(output, " <no dst> <-");
	}
	else if (ignore_dst == 1) // certain opcode types ignore the 'a' field entirely, it should be set to 0.
	{
			if (areg) output += sprintf(output, " <reserved %d> <-", areg);
	}
	else if (ignore_dst == 2) // for multiply operations areg should always be set to LIMM
	{
		if (areg != LIMM_REG) output += sprintf(output, " <invalid %d> <-", areg);
		else  output += sprintf(output, " <mulres> <-");
	}

	if (!b_reserved)
	{
		if (breg == LIMM_REG)
			output += sprintf(output, " 0x%08x,", limm);
		else
			output += sprintf(output, " %s,", regnames[breg]);
	}
	else
	{
		if (breg) output += sprintf(output, "<reserved %d>,", breg);
	}

	if (creg == LIMM_REG)
		output += sprintf(output, " 0x%08x", limm);
	else
		output += sprintf(output, " %s", regnames[creg]);

	return size;
}

// like p00 but with 'u6' istead of C
int arcompact_handle04_p01_helper_dasm(DASM_OPS_32, const char* optext, int ignore_dst, int b_reserved)
{
	//           PP
	// 0010 0bbb 01ii iiii FBBB uuuu uuAA AAAA
	int size = 4;
	UINT32 limm = 0;
//  int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6
	COMMON32_GET_areg

	output += sprintf(output, "%s", optext);
	output += sprintf(output, "%s", flagbit[F]);
	//  output  += sprintf( output, " p(%d)", p);


	if ((!b_reserved) && (breg == LIMM_REG))
	{
		GET_LIMM_32;
		size = 8;
//      got_limm = 1;
	}

	// areg can be LIMM too, but in that case LIMM indicates 'no destination' rather than an actual LIMM value following

	if (ignore_dst == 0)
	{
		if (areg != LIMM_REG)  output += sprintf(output, " %s <-", regnames[areg]);
		else output += sprintf(output, " <no dst> <-");
	}
	else if (ignore_dst == 1) // certain opcode types ignore the 'a' field entirely, it should be set to 0.
	{
			if (areg) output += sprintf(output, " <reserved %d> <-", areg);
	}
	else if (ignore_dst == 2) // for multiply operations areg should always be set to LIMM
	{
		if (areg != LIMM_REG) output += sprintf(output, " <invalid %d> <-", areg);
		else  output += sprintf(output, " <mulres> <-");
	}

	if (!b_reserved)
	{
		if (breg == LIMM_REG)
			output += sprintf(output, " 0x%08x,", limm);
		else
			output += sprintf(output, " %s,", regnames[breg]);
	}
	else
	{
		if (breg) output += sprintf(output, "<reserved %d>,", breg);
	}

	output += sprintf(output, " 0x%02x", u);

	return size;
}


int arcompact_handle04_p10_helper_dasm(DASM_OPS_32, const char* optext, int b_reserved)
{
	int size = 4;
	UINT32 limm;
	//int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F
	COMMON32_GET_s12;

	output += sprintf(output, "%s", optext);
	output += sprintf(output, "%s", flagbit[F]);
	//  output  += sprintf( output, " p(%d)", p);


	if (!b_reserved)
	{
		if (breg == LIMM_REG)
		{
			GET_LIMM_32;
			size = 8;
			//got_limm = 1;
			output += sprintf(output, " 0x%08x ", limm);

		}
		else
		{
			output += sprintf(output, " %s, ", regnames[breg]);
		}
	}
	else
	{
		if (breg) output += sprintf(output, "reserved(%s), ", regnames[breg]);
	}

	output += sprintf(output, "S(%02x)", S);
	return size;
}

int arcompact_handle04_p11_m0_helper_dasm(DASM_OPS_32, const char* optext, int b_reserved)
{
	int size = 4;
	UINT32 limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F
	COMMON32_GET_CONDITION;
	COMMON32_GET_creg

	output += sprintf(output, "%s", optext);
	output += sprintf(output, "%s", flagbit[F]);
	//  output  += sprintf( output, " p(%d)", p);

	if (!b_reserved)
	{
		if (breg == LIMM_REG)
		{
			GET_LIMM_32;
			size = 8;
			got_limm = 1;
			output += sprintf(output, " 0x%08x ", limm);

		}
		else
		{
			output += sprintf(output, " %s, ", regnames[breg]);
		}
	}
	else
	{
		if (breg) output += sprintf(output, "reserved(%s), ", regnames[breg]);
	}


	output += sprintf(output, " Cond<%s> ", conditions[condition]);


	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		output += sprintf(output, " 0x%08x ", limm);
	}
	else
	{
		output += sprintf(output, "C(%s)", regnames[creg]);
	}

	return size;
}

int arcompact_handle04_p11_m1_helper_dasm(DASM_OPS_32, const char* optext, int b_reserved)
{
	int size = 4;
	UINT32 limm;
	//int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F
	COMMON32_GET_CONDITION;
	COMMON32_GET_u6

	output += sprintf(output, "%s", optext);
	output += sprintf(output, "%s", flagbit[F]);
	//  output  += sprintf( output, " p(%d)", p);

	if (!b_reserved)
	{
		if (breg == LIMM_REG)
		{
			GET_LIMM_32;
			size = 8;
			//got_limm = 1;
			output += sprintf(output, " 0x%08x ", limm);

		}
		else
		{
			output += sprintf(output, " %s, ", regnames[breg]);
		}
	}
	else
	{
		if (breg) output += sprintf(output, "reserved(%s), ", regnames[breg]);
	}


	output += sprintf(output, " Cond<%s> ", conditions[condition]);


	output += sprintf(output, "U(%02x)", u);

	return size;
}

int arcompact_handle04_p11_helper_dasm(DASM_OPS_32, const char* optext, int b_reserved)
{
	int M = (op & 0x00000020) >> 5; op &= ~0x00000020;

	switch (M)
	{
		case 0x00: return arcompact_handle04_p11_m0_helper_dasm(DASM_PARAMS, optext, b_reserved);
		case 0x01: return arcompact_handle04_p11_m1_helper_dasm(DASM_PARAMS, optext, b_reserved);
	}
	return 0;
}


int arcompact_handle04_helper_dasm(DASM_OPS_32, const char* optext, int ignore_dst, int b_reserved)
{
	COMMON32_GET_p;

	switch (p)
	{
		case 0x00: return arcompact_handle04_p00_helper_dasm(DASM_PARAMS, optext, ignore_dst, b_reserved);
		case 0x01: return arcompact_handle04_p01_helper_dasm(DASM_PARAMS, optext, ignore_dst, b_reserved);
		case 0x02: return arcompact_handle04_p10_helper_dasm(DASM_PARAMS, optext, b_reserved);
		case 0x03: return arcompact_handle04_p11_helper_dasm(DASM_PARAMS, optext, b_reserved);
	}

	return 0;
}

int arcompact_handle04_00_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "ADD", 0,0);
}

int arcompact_handle04_01_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "ADC", 0,0);
}

int arcompact_handle04_02_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "SUB", 0,0);
}

int arcompact_handle04_03_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "SBC", 0,0);
}

int arcompact_handle04_04_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "AND", 0,0);
}

int arcompact_handle04_05_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "OR", 0,0);
}

int arcompact_handle04_06_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "BIC", 0,0);
}

int arcompact_handle04_07_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "XOR", 0,0);
}

int arcompact_handle04_08_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "MAX", 0,0);
}

int arcompact_handle04_09_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "MIN", 0,0);
}


int arcompact_handle04_0a_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "MOV", 1,0);
}

int arcompact_handle04_0b_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "TST", 1,0);
}

int arcompact_handle04_0c_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "CMP", 1,0);
}

int arcompact_handle04_0d_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "RCMP", 1,0);
}

int arcompact_handle04_0e_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "RSUB", 0,0);
}

int arcompact_handle04_0f_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "BSET", 0,0);
}

int arcompact_handle04_10_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "BCLR", 0,0);
}

int arcompact_handle04_11_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "BTST", 0,0);
}

int arcompact_handle04_12_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "BXOR", 0,0);
}

int arcompact_handle04_13_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "BMSK", 0,0);
}

int arcompact_handle04_14_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "ADD1", 0,0);
}

int arcompact_handle04_15_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "ADD2", 0,0);
}

int arcompact_handle04_16_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "ADD3", 0,0);
}

int arcompact_handle04_17_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "SUB1", 0,0);
}

int arcompact_handle04_18_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "SUB2", 0,0);
}

int arcompact_handle04_19_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "SUB3", 0,0);
}

int arcompact_handle04_1a_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "MPY", 0,0);
} // *

int arcompact_handle04_1b_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "MPYH", 0,0);
} // *

int arcompact_handle04_1c_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "MPYHU", 0,0);
} // *

int arcompact_handle04_1d_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "MPYU", 0,0);
} // *



int arcompact_handle04_20_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "J", 1,1);
}



int arcompact_handle04_21_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "J.D", 1,1);
}

int arcompact_handle04_22_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "JL", 1,1);
}

int arcompact_handle04_23_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "JL.D", 1,1);
}




int arcompact_handle04_28_dasm(DASM_OPS_32) // LPcc (loop setup)
{
	COMMON32_GET_breg; // breg is reserved
	COMMON32_GET_p;

	if (p == 0x00)
	{
		print("<illegal LPcc, p = 0x00)");
	}
	else if (p == 0x01)
	{
		print("<illegal LPcc, p = 0x01)");
	}
	else if (p == 0x02) // Loop unconditional
	{ // 0010 0RRR 1010 1000 0RRR ssss ssSS SSSS
		COMMON32_GET_s12
		if (S & 0x800) S = -0x800 + (S&0x7ff);

		output += sprintf(output, "LP (start %08x, end %08x)", pc + 4, pc + S*2);
	}
	else if (p == 0x03) // Loop conditional
	{ // 0010 0RRR 1110 1000 0RRR uuuu uu1Q QQQQ
		COMMON32_GET_u6
		COMMON32_GET_CONDITION
		output += sprintf(output, "LP<%s> (start %08x, end %08x)", conditions[condition], pc + 4, PC_ALIGNED32 + u*2);

		int unused = (op & 0x00000020)>>5;
		if (unused==0)  output += sprintf(output, "(unused bit not set)");

	}

	if (breg) output += sprintf(output, "(reseved B bits set %02x)", breg);

	return 4;
}

#define PRINT_AUX_REGNAME \
		if ((auxreg >= 0) && (auxreg < 0x420)) \
		{ \
			if (strcmp(auxregnames[auxreg],"unusedreg")) \
				output += sprintf(output, "[%s]", auxregnames[auxreg]); \
			else \
				output  += sprintf( output, "[%03x]", auxreg); \
		} \
		else \
			output  += sprintf( output, "[%03x]", auxreg);
int arcompact_handle04_2a_dasm(DASM_OPS_32)  // Load FROM Auxiliary register TO register
{
	//           pp        F
	// 0010 0bbb 0010 1010 0BBB CCCC CCRR RRRR
	// 0010 0bbb 0010 1010 0BBB 1111 10RR RRRR
	// 0010 0bbb 0110 1010 0BBB uuuu uu00 0000
	// 0010 0bbb 1010 1010 0BBB ssss ssSS SSSS


	int size = 4;
	UINT32 limm = 0;
	int got_limm = 0;

	COMMON32_GET_p;
	COMMON32_GET_breg;
	COMMON32_GET_F

	output  += sprintf( output, "LR");
	if (F) output  += sprintf( output, ".<F set, illegal>");
//  output  += sprintf( output, " p(%d)", p);



	if (breg == LIMM_REG)
	{
		output  += sprintf( output, "<no dest>" ); // illegal encoding?
	}
	else
	{
		output += sprintf(output, " %s, ", regnames[breg]);
	}



	if (p == 0)
	{
		COMMON32_GET_creg
		COMMON32_GET_areg_reserved

		if (creg == LIMM_REG)
		{
			if (!got_limm)
			{
				GET_LIMM_32;
				size = 8;
			}

			output  += sprintf( output, "(%08x) ", limm );

		}
		else
		{
			output  += sprintf( output, "C(%s) ", regnames[creg]);
		}

		if (ares) output  += sprintf( output, "reserved(%02x) ", ares );
	}
	else if (p == 1)
	{
		COMMON32_GET_u6
		COMMON32_GET_areg_reserved

		int auxreg = u;
		PRINT_AUX_REGNAME

		if (ares) output  += sprintf( output, "reserved(%02x) ", ares );
	}
	else if (p == 2)
	{
		COMMON32_GET_s12;

		int auxreg = S;
		PRINT_AUX_REGNAME

	}
	else if (p == 3)
	{
		output  += sprintf( output, " <mode 3, illegal>");
	}

	return size;
}

int arcompact_handle04_2b_dasm(DASM_OPS_32)  // Store TO Auxiliary register FROM register
{
	// code at ~ 40073DFE in leapster bios is manually setting up a loop this way
	// rather than using the lPcc opcode

	int size = 4;
	UINT32 limm = 0;
	int got_limm = 0;

	COMMON32_GET_p;
	COMMON32_GET_breg;
	COMMON32_GET_F

	output  += sprintf( output, "SR");
	if (F) output  += sprintf( output, ".<F set, illegal>");
//  output  += sprintf( output, " p(%d)", p);



	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		output  += sprintf( output, " %08x -> ", limm );

	}
	else
	{
		output += sprintf(output, " %s -> ", regnames[breg]);
	}



	if (p == 0)
	{
		COMMON32_GET_creg
		COMMON32_GET_areg_reserved

		if (creg == LIMM_REG)
		{
			if (!got_limm)
			{
				GET_LIMM_32;
				size = 8;
			}

			output  += sprintf( output, "[%08x]", limm );

		}
		else
		{
			output  += sprintf( output, "[%s]", regnames[creg]);


		}

		if (ares) output  += sprintf( output, " (reserved %02x) ", ares );


	}
	else if (p == 1)
	{
		COMMON32_GET_u6
		COMMON32_GET_areg_reserved

		int auxreg = u;
		PRINT_AUX_REGNAME

		if (ares) output  += sprintf( output, " (reserved %02x) ", ares );


	}
	else if (p == 2)
	{
		COMMON32_GET_s12;

		int auxreg = S;

		PRINT_AUX_REGNAME

	}
	else if (p == 3)
	{
		output  += sprintf( output, " <mode 3, illegal>");
	}

	return size;}


int arcompact_handle04_29_dasm(DASM_OPS_32)
{
	// leapster bios uses formats for FLAG that are not defined, bug I guess work anyway (P modes 0 / 1)
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "FLAG", 1,1);
}


int arcompact_handle04_2f_helper_dasm(DASM_OPS_32, const char* optext)
{
	//
	// 0010 0bbb pp10 1111 FBBB CCCC CCII IIII
	int size = 4;

	COMMON32_GET_p;
	COMMON32_GET_breg;
	COMMON32_GET_F

	output  += sprintf( output, "%s", optext);
	output  += sprintf( output, "%s", flagbit[F]);
//  output  += sprintf( output, " p(%d)", p);

	if (breg == LIMM_REG)
	{
		output += sprintf(output, " <no dst>, ");
		// if using the 'EX' opcode this is illegal
	}
	else
	{
		output += sprintf(output, " %s, ", regnames[breg]);
	}

	if (p == 0)
	{
		COMMON32_GET_creg

		if (creg == LIMM_REG)
		{
			UINT32 limm;
			GET_LIMM_32;
			size = 8;
			output  += sprintf( output, "(%08x) ", limm );

		}
		else
		{
			output  += sprintf( output, "C(%s) ", regnames[creg]);
		}
	}
	else if (p == 1)
	{
		COMMON32_GET_u6

		output  += sprintf( output, "U(0x%02x) ", u);
	}
	else if (p == 2)
	{
		output  += sprintf( output, "<04_2f illegal p=10>");
	}
	else if (p == 3)
	{
		output  += sprintf( output, "<04_2f illegal p=11>");
	}

	return size;
}


int arcompact_handle04_2f_00_dasm(DASM_OPS_32)  { return arcompact_handle04_2f_helper_dasm(DASM_PARAMS, "ASL"); } // ASL
int arcompact_handle04_2f_01_dasm(DASM_OPS_32)  { return arcompact_handle04_2f_helper_dasm(DASM_PARAMS, "ASR"); } // ASR
int arcompact_handle04_2f_02_dasm(DASM_OPS_32)  { return arcompact_handle04_2f_helper_dasm(DASM_PARAMS, "LSR"); } // LSR
int arcompact_handle04_2f_03_dasm(DASM_OPS_32)  { return arcompact_handle04_2f_helper_dasm(DASM_PARAMS, "ROR"); } // ROR
int arcompact_handle04_2f_04_dasm(DASM_OPS_32)  { return arcompact_handle04_2f_helper_dasm(DASM_PARAMS, "RCC"); } // RCC
int arcompact_handle04_2f_05_dasm(DASM_OPS_32)  { return arcompact_handle04_2f_helper_dasm(DASM_PARAMS, "SEXB"); } // SEXB
int arcompact_handle04_2f_06_dasm(DASM_OPS_32)  { return arcompact_handle04_2f_helper_dasm(DASM_PARAMS, "SEXW"); } // SEXW
int arcompact_handle04_2f_07_dasm(DASM_OPS_32)  { return arcompact_handle04_2f_helper_dasm(DASM_PARAMS, "EXTB"); } // EXTB

int arcompact_handle04_2f_08_dasm(DASM_OPS_32)  { return arcompact_handle04_2f_helper_dasm(DASM_PARAMS, "EXTW"); } // EXTW


int arcompact_handle04_2f_09_dasm(DASM_OPS_32)  { return arcompact_handle04_2f_helper_dasm(DASM_PARAMS, "ABS"); } // ABS
int arcompact_handle04_2f_0a_dasm(DASM_OPS_32)  { return arcompact_handle04_2f_helper_dasm(DASM_PARAMS, "NOT"); } // NOT
int arcompact_handle04_2f_0b_dasm(DASM_OPS_32)  { return arcompact_handle04_2f_helper_dasm(DASM_PARAMS, "RCL"); } // RLC
int arcompact_handle04_2f_0c_dasm(DASM_OPS_32)  { return arcompact_handle04_2f_helper_dasm(DASM_PARAMS, "EX"); } // EX


int arcompact_handle04_2f_3f_01_dasm(DASM_OPS_32)  { print("SLEEP (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_02_dasm(DASM_OPS_32)  { print("SWI / TRAP0 (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_03_dasm(DASM_OPS_32)  { print("SYNC (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_04_dasm(DASM_OPS_32)  { print("RTIE (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_05_dasm(DASM_OPS_32)  { print("BRK (%08x)", op); return 4;}





// format on these is..

// 0010 0bbb aa11 0ZZX DBBB CCCC CCAA AAAA
// note, bits  11 0ZZX are part of the sub-opcode # already - this is a special encoding
int arcompact_handle04_3x_helper_dasm(DASM_OPS_32, int dsize, int extend)
{
	int size = 4;
	UINT32 limm=0;
	int got_limm = 0;

	output += sprintf(output, "LD");
	output += sprintf(output, "%s", datasize[dsize]);
	output += sprintf(output, "%s", dataextend[extend]);

	int mode = (op & 0x00c00000) >> 22; op &= ~0x00c00000;
	COMMON32_GET_breg;
	int D = (op & 0x00008000) >> 15; op &= ~0x00008000;
	COMMON32_GET_creg
	COMMON32_GET_areg

	output += sprintf(output, "%s", addressmode[mode]);
	output += sprintf(output, "%s", cachebit[D]);

	output  += sprintf( output, " %s. ", regnames[areg]);

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		output  += sprintf( output, "[%08x, ", limm );

	}
	else
	{
		output += sprintf(output, "[%s, ", regnames[breg]);
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		output  += sprintf( output, "(%08x)]", limm );

	}
	else
	{
		output  += sprintf( output, "%s]", regnames[creg]);
	}


	return size;



}

int arcompact_handle04_30_dasm(DASM_OPS_32)  { return arcompact_handle04_3x_helper_dasm(DASM_PARAMS,0,0); }
// ZZ value of 0x0 with X of 1 is illegal
int arcompact_handle04_31_dasm(DASM_OPS_32)  { return arcompact_handle04_3x_helper_dasm(DASM_PARAMS,0,1); }
int arcompact_handle04_32_dasm(DASM_OPS_32)  { return arcompact_handle04_3x_helper_dasm(DASM_PARAMS,1,0); }
int arcompact_handle04_33_dasm(DASM_OPS_32)  { return arcompact_handle04_3x_helper_dasm(DASM_PARAMS,1,1); }
int arcompact_handle04_34_dasm(DASM_OPS_32)  { return arcompact_handle04_3x_helper_dasm(DASM_PARAMS,2,0); }
int arcompact_handle04_35_dasm(DASM_OPS_32)  { return arcompact_handle04_3x_helper_dasm(DASM_PARAMS,2,1); }
// ZZ value of 0x3 is illegal
int arcompact_handle04_36_dasm(DASM_OPS_32)  { return arcompact_handle04_3x_helper_dasm(DASM_PARAMS,3,0); }
int arcompact_handle04_37_dasm(DASM_OPS_32)  { return arcompact_handle04_3x_helper_dasm(DASM_PARAMS,3,1); }






int arcompact_handle05_00_dasm(DASM_OPS_32)  { return arcompact_handle04_helper_dasm(DASM_PARAMS, "ASL", 0,0); }
int arcompact_handle05_01_dasm(DASM_OPS_32)  { return arcompact_handle04_helper_dasm(DASM_PARAMS, "LSR", 0,0); }
int arcompact_handle05_02_dasm(DASM_OPS_32)  { return arcompact_handle04_helper_dasm(DASM_PARAMS, "ASR", 0,0); }
int arcompact_handle05_03_dasm(DASM_OPS_32)  { return arcompact_handle04_helper_dasm(DASM_PARAMS, "ROR", 0,0); }
int arcompact_handle05_04_dasm(DASM_OPS_32)  { return arcompact_handle04_helper_dasm(DASM_PARAMS, "MUL64", 2,0); } // special
int arcompact_handle05_05_dasm(DASM_OPS_32)  { return arcompact_handle04_helper_dasm(DASM_PARAMS, "MULU64", 2,0);} // special
int arcompact_handle05_06_dasm(DASM_OPS_32)  { return arcompact_handle04_helper_dasm(DASM_PARAMS, "ADDS", 0,0); }
int arcompact_handle05_07_dasm(DASM_OPS_32)  { return arcompact_handle04_helper_dasm(DASM_PARAMS, "SUBS", 0,0); }
int arcompact_handle05_08_dasm(DASM_OPS_32)  { return arcompact_handle04_helper_dasm(DASM_PARAMS, "DIVAW", 0,0); }



int arcompact_handle05_0a_dasm(DASM_OPS_32)  { return arcompact_handle04_helper_dasm(DASM_PARAMS, "ASLS", 0,0); }
int arcompact_handle05_0b_dasm(DASM_OPS_32)  { return arcompact_handle04_helper_dasm(DASM_PARAMS, "ASRS", 0,0); }

int arcompact_handle05_28_dasm(DASM_OPS_32)  { return arcompact_handle04_helper_dasm(DASM_PARAMS, "ADDSDW", 0,0); }
int arcompact_handle05_29_dasm(DASM_OPS_32)  { return arcompact_handle04_helper_dasm(DASM_PARAMS, "SUBSDW", 0,0); }



int arcompact_handle05_2f_0x_helper_dasm(DASM_OPS_32, const char* optext)
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

	output  += sprintf( output, "%s", optext);
	output  += sprintf( output, "%s", flagbit[F]);
//  output  += sprintf( output, " p(%d)", p);


	output += sprintf(output, " %s, ", regnames[breg]);

	if (p == 0)
	{
		COMMON32_GET_creg

		if (creg == LIMM_REG)
		{
			UINT32 limm;
			GET_LIMM_32;
			size = 8;
			output  += sprintf( output, "(%08x) ", limm );

		}
		else
		{
			output  += sprintf( output, "C(%s) ", regnames[creg]);
		}
	}
	else if (p == 1)
	{
		COMMON32_GET_u6
		output  += sprintf( output, "U(0x%02x) ", u);
	}
	else if (p == 2)
	{
		output  += sprintf( output, "<05_2f illegal p=10>");
	}
	else if (p == 3)
	{
		output  += sprintf( output, "<05_2f illegal p=11>");
	}

	return size;
}


int arcompact_handle05_2f_00_dasm(DASM_OPS_32)  { return arcompact_handle05_2f_0x_helper_dasm(DASM_PARAMS, "SWAP");  }
int arcompact_handle05_2f_01_dasm(DASM_OPS_32)  { return arcompact_handle05_2f_0x_helper_dasm(DASM_PARAMS, "NORM");  }
int arcompact_handle05_2f_02_dasm(DASM_OPS_32)  { return arcompact_handle05_2f_0x_helper_dasm(DASM_PARAMS, "SAT16"); }
int arcompact_handle05_2f_03_dasm(DASM_OPS_32)  { return arcompact_handle05_2f_0x_helper_dasm(DASM_PARAMS, "RND16"); }
int arcompact_handle05_2f_04_dasm(DASM_OPS_32)  { return arcompact_handle05_2f_0x_helper_dasm(DASM_PARAMS, "ABSSW"); }
int arcompact_handle05_2f_05_dasm(DASM_OPS_32)  { return arcompact_handle05_2f_0x_helper_dasm(DASM_PARAMS, "ABSS");  }
int arcompact_handle05_2f_06_dasm(DASM_OPS_32)  { return arcompact_handle05_2f_0x_helper_dasm(DASM_PARAMS, "NEGSW"); }
int arcompact_handle05_2f_07_dasm(DASM_OPS_32)  { return arcompact_handle05_2f_0x_helper_dasm(DASM_PARAMS, "NEGS");  }
int arcompact_handle05_2f_08_dasm(DASM_OPS_32)  { return arcompact_handle05_2f_0x_helper_dasm(DASM_PARAMS, "NORMW"); }


int arcompact_handle06_dasm(DASM_OPS_32)
{
	print("op a,b,c (06 ARC ext) (%08x)", op );
	return 4;
}

int arcompact_handle07_dasm(DASM_OPS_32)
{
	print("op a,b,c (07 User ext) (%08x)", op );
	return 4;
}

int arcompact_handle08_dasm(DASM_OPS_32)
{
	print("op a,b,c (08 User ext) (%08x)", op );
	return 4;
}

int arcompact_handle09_dasm(DASM_OPS_32)
{
	print("op a,b,c (09 Market ext) (%08x)", op );
	return 4;
}

int arcompact_handle0a_dasm(DASM_OPS_32)
{
	print("op a,b,c (0a Market ext) (%08x)",  op );
	return 4;
}

int arcompact_handle0b_dasm(DASM_OPS_32)
{
	print("op a,b,c (0b Market ext) (%08x)",  op );
	return 4;
}



int arcompact_handle0c_helper_dasm(DASM_OPS_16, const char* optext, int format)
{
	int areg, breg, creg;

	COMMON16_GET_areg;
	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(areg);
	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);


	if (format==0) print("%s %s <- [%s, %s]", optext, regnames[areg], regnames[breg], regnames[creg]);
	else print("%s %s <- %s, %s", optext, regnames[areg], regnames[breg], regnames[creg]);

	return 2;
}


int arcompact_handle0c_00_dasm(DASM_OPS_16)
{
	return arcompact_handle0c_helper_dasm(DASM_PARAMS, "LD_S", 0);
}

int arcompact_handle0c_01_dasm(DASM_OPS_16)
{
	return arcompact_handle0c_helper_dasm(DASM_PARAMS, "LDB_S", 0);
}

int arcompact_handle0c_02_dasm(DASM_OPS_16)
{
	return arcompact_handle0c_helper_dasm(DASM_PARAMS, "LDW_S", 0);
}

int arcompact_handle0c_03_dasm(DASM_OPS_16)
{
	return arcompact_handle0c_helper_dasm(DASM_PARAMS, "ADD_S", 1);
}


int arcompact_handle0d_helper_dasm(DASM_OPS_16, const char* optext)
{
	int u, breg, creg;

	COMMON16_GET_u3;
	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	print("%s %s <- [%s, 0x%02x]", optext, regnames[creg], regnames[breg], u);
	return 2;
}


int arcompact_handle0d_00_dasm(DASM_OPS_16)
{
	return arcompact_handle0d_helper_dasm(DASM_PARAMS, "ADD_S");
}

int arcompact_handle0d_01_dasm(DASM_OPS_16)
{
	return arcompact_handle0d_helper_dasm(DASM_PARAMS, "SUB_S");
}

int arcompact_handle0d_02_dasm(DASM_OPS_16)
{
	return arcompact_handle0d_helper_dasm(DASM_PARAMS, "ASL_S");
}

int arcompact_handle0d_03_dasm(DASM_OPS_16)
{
	return arcompact_handle0d_helper_dasm(DASM_PARAMS, "ASR_S");
}



int arcompact_handle0e_0x_helper_dasm(DASM_OPS_16, const char* optext, int revop)
{
	int h,breg;
	int size = 2;

	GROUP_0e_GET_h;
	COMMON16_GET_breg;
	REG_16BIT_RANGE(breg);

	if (h == LIMM_REG)
	{
		UINT32 limm;
		GET_LIMM;
		size = 6;
		if (!revop) print("%s %s <- 0x%08x", optext, regnames[breg], limm);
		else print("%s 0x%08x <- %s", optext, limm, regnames[breg]);
	}
	else
	{
		if (!revop) print("%s %s <- %s", optext, regnames[breg], regnames[h]);
		else print("%s %s <- %s", optext, regnames[h], regnames[breg]);

	}

	return size;

}

int arcompact_handle0e_00_dasm(DASM_OPS_16)
{
	return arcompact_handle0e_0x_helper_dasm(DASM_PARAMS, "ADD_S", 0);
}

int arcompact_handle0e_01_dasm(DASM_OPS_16)
{
	return arcompact_handle0e_0x_helper_dasm(DASM_PARAMS, "MOV_S", 0);
}

int arcompact_handle0e_02_dasm(DASM_OPS_16)
{
	return arcompact_handle0e_0x_helper_dasm(DASM_PARAMS, "CMP_S", 0);
}

int arcompact_handle0e_03_dasm(DASM_OPS_16)
{
	return arcompact_handle0e_0x_helper_dasm(DASM_PARAMS, "MOV_S", 1);
}



int arcompact_handle0f_00_0x_helper_dasm(DASM_OPS_16, const char* optext)
{
	int breg;

	COMMON16_GET_breg;
	REG_16BIT_RANGE(breg);

	print("%s %s", optext, regnames[breg]);

	return 2;

}



int arcompact_handle0f_00_00_dasm(DASM_OPS_16)  { return arcompact_handle0f_00_0x_helper_dasm(DASM_PARAMS, "J_S"); }
int arcompact_handle0f_00_01_dasm(DASM_OPS_16)  { return arcompact_handle0f_00_0x_helper_dasm(DASM_PARAMS, "J_S.D"); }
int arcompact_handle0f_00_02_dasm(DASM_OPS_16)  { return arcompact_handle0f_00_0x_helper_dasm(DASM_PARAMS, "JL_S");  }
int arcompact_handle0f_00_03_dasm(DASM_OPS_16)  { return arcompact_handle0f_00_0x_helper_dasm(DASM_PARAMS, "JL_S.D");  }
int arcompact_handle0f_00_06_dasm(DASM_OPS_16)  { return arcompact_handle0f_00_0x_helper_dasm(DASM_PARAMS, "SUB_S.NE"); }




// Zero parameters (ZOP)
int arcompact_handle0f_00_07_00_dasm(DASM_OPS_16)  { print("NOP_S"); return 2;}
int arcompact_handle0f_00_07_01_dasm(DASM_OPS_16)  { print("UNIMP_S"); return 2;} // Unimplemented Instruction, same as illegal, but recommended to fill blank space
int arcompact_handle0f_00_07_04_dasm(DASM_OPS_16)  { print("JEQ_S [blink]"); return 2;}
int arcompact_handle0f_00_07_05_dasm(DASM_OPS_16)  { print("JNE_S [blink]"); return 2;}
int arcompact_handle0f_00_07_06_dasm(DASM_OPS_16)  { print("J_S [blink]"); return 2;}
int arcompact_handle0f_00_07_07_dasm(DASM_OPS_16)  { print("J_S.D [blink]"); return 2;}





int arcompact_handle0f_0x_helper_dasm(DASM_OPS_16, const char* optext, int nodst)
{
	int breg, creg;

	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	if (nodst==0) print("%s %s <- %s", optext, regnames[breg], regnames[creg]);
	else if (nodst==1) print("%s <no dst>, %s, %s", optext, regnames[breg], regnames[creg]);
	else if (nodst==2) print("%s <mulres>, %s, %s", optext, regnames[breg], regnames[creg]);

	return 2;
}

int arcompact_handle0f_02_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "SUB_S",0);  }
int arcompact_handle0f_04_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "AND_S",0);  }
int arcompact_handle0f_05_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "OR_S",0);   }
int arcompact_handle0f_06_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "BIC_S",0);  }
int arcompact_handle0f_07_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "XOR_S",0);  }
int arcompact_handle0f_0b_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "TST_S",1);  }
int arcompact_handle0f_0c_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "MUL64_S",2);  } // actual destination is special multiply registers
int arcompact_handle0f_0d_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "SEXB_S",0); }
int arcompact_handle0f_0e_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "SEXW_S",0); }
int arcompact_handle0f_0f_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "EXTB_S",0); }
int arcompact_handle0f_10_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "EXTW_S",0); }
int arcompact_handle0f_11_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "ABS_S",0);  }
int arcompact_handle0f_12_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "NOT_S",0);  }
int arcompact_handle0f_13_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "NEG_S",0);  }
int arcompact_handle0f_14_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "ADD1_S",0); }
int arcompact_handle0f_15_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "ADD2_S",0); }
int arcompact_handle0f_16_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "ADD3_S",0); }
int arcompact_handle0f_18_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "ASL_S",0);  }
int arcompact_handle0f_19_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "LSR_S",0);  }
int arcompact_handle0f_1a_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "ASR_S",0);  }
int arcompact_handle0f_1b_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "ASL1_S",0); }
int arcompact_handle0f_1c_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "ASR1_S",0); }
int arcompact_handle0f_1d_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "LSR1_S",0); }


int arcompact_handle0f_1e_dasm(DASM_OPS_16)  // special
{ // 0111 1uuu uuu1 1110
	int u = (op & 0x07e0)>>5;
	print("TRAP_S %02x",u);
	return 2;
}

int arcompact_handle0f_1f_dasm(DASM_OPS_16)  // special
{
	int u = (op & 0x07e0)>>5; op &= ~0x07e0;

	if (u == 0x003f)
	{
		print("BRK_S");
	}
	else
	{
		print("<illegal BRK_S %02x>",u);
	}
	return 2;
}


int arcompact_handle_ld_helper_dasm(DASM_OPS_16, const char* optext, int shift, int swap)
{
	int breg, creg, u;

	COMMON16_GET_breg;
	COMMON16_GET_creg;
	COMMON16_GET_u5;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	u <<= shift;

	if (!swap) print("%s %s, [%s, 0x%02x] (%04x)", optext, regnames[creg], regnames[breg], u, op);
	else  print("%s [%s, 0x%02x], %s (%04x)", optext, regnames[breg], u, regnames[creg], op);
	return 2;

}


int arcompact_handle10_dasm(DASM_OPS_16)
{
	return arcompact_handle_ld_helper_dasm(DASM_PARAMS, "LD_S", 2, 0);
}

int arcompact_handle11_dasm(DASM_OPS_16)
{
	return arcompact_handle_ld_helper_dasm(DASM_PARAMS, "LDB_S", 0, 0);
}

int arcompact_handle12_dasm(DASM_OPS_16)
{
	return arcompact_handle_ld_helper_dasm(DASM_PARAMS, "LDW_S", 1, 0);
}

int arcompact_handle13_dasm(DASM_OPS_16)
{
	return arcompact_handle_ld_helper_dasm(DASM_PARAMS, "LDW_S.X", 1, 0);
}

int arcompact_handle14_dasm(DASM_OPS_16)
{
	return arcompact_handle_ld_helper_dasm(DASM_PARAMS, "ST_S", 2, 1);
}

int arcompact_handle15_dasm(DASM_OPS_16)
{
	return arcompact_handle_ld_helper_dasm(DASM_PARAMS, "STB_S", 0, 1);
}

int arcompact_handle16_dasm(DASM_OPS_16)
{
	return arcompact_handle_ld_helper_dasm(DASM_PARAMS, "STW_S", 1, 1);
}


int arcompact_handle_l7_0x_helper_dasm(DASM_OPS_16, const char* optext)
{
	int breg, u;

	COMMON16_GET_breg;
	COMMON16_GET_u5;

	REG_16BIT_RANGE(breg);

	print("%s %s, 0x%02x", optext, regnames[breg], u);

	return 2;

}

int arcompact_handle17_00_dasm(DASM_OPS_16)
{
	return arcompact_handle_l7_0x_helper_dasm(DASM_PARAMS, "ASL_S");
}

int arcompact_handle17_01_dasm(DASM_OPS_16)
{
	return arcompact_handle_l7_0x_helper_dasm(DASM_PARAMS, "LSR_S");
}

int arcompact_handle17_02_dasm(DASM_OPS_16)
{
	return arcompact_handle_l7_0x_helper_dasm(DASM_PARAMS, "ASR_S");
}

int arcompact_handle17_03_dasm(DASM_OPS_16)
{
	return arcompact_handle_l7_0x_helper_dasm(DASM_PARAMS, "SUB_S");
}

int arcompact_handle17_04_dasm(DASM_OPS_16)
{
	return arcompact_handle_l7_0x_helper_dasm(DASM_PARAMS, "BSET_S");
}

int arcompact_handle17_05_dasm(DASM_OPS_16)
{
	return arcompact_handle_l7_0x_helper_dasm(DASM_PARAMS, "BCLR_S");
}

int arcompact_handle17_06_dasm(DASM_OPS_16)
{
	return arcompact_handle_l7_0x_helper_dasm(DASM_PARAMS, "BSMK_S");
}

int arcompact_handle17_07_dasm(DASM_OPS_16)
{
	return arcompact_handle_l7_0x_helper_dasm(DASM_PARAMS, "BTST_S");
}


// op bits remaining for 0x18_xx subgroups 0x071f

int arcompact_handle18_0x_helper_dasm(DASM_OPS_16, const char* optext, int st, int format)
{
	int breg, u;

	COMMON16_GET_breg;
	COMMON16_GET_u5;

	REG_16BIT_RANGE(breg);

	output  += sprintf( output, "%s %s ", optext, regnames[breg]);
	if (st==1) output  += sprintf( output, "-> ");
	else output  += sprintf( output, "<- ");

	if (format==0) output  += sprintf( output, "[SP, 0x%02x]", u*4);
	else  output  += sprintf( output, "SP, 0x%02x", u*4);


	return 2;
}

int arcompact_handle18_00_dasm(DASM_OPS_16)
{
	return arcompact_handle18_0x_helper_dasm(DASM_PARAMS, "LD_S", 0,0);
}

int arcompact_handle18_01_dasm(DASM_OPS_16)
{
	return arcompact_handle18_0x_helper_dasm(DASM_PARAMS, "LDB_S", 0,0);
}

int arcompact_handle18_02_dasm(DASM_OPS_16)
{
	return arcompact_handle18_0x_helper_dasm(DASM_PARAMS, "ST_S", 1,0);
}

int arcompact_handle18_03_dasm(DASM_OPS_16)
{
	return arcompact_handle18_0x_helper_dasm(DASM_PARAMS, "STB_S", 1,0);
}

int arcompact_handle18_04_dasm(DASM_OPS_16)
{
	return arcompact_handle18_0x_helper_dasm(DASM_PARAMS, "ADD_S", 1,1); // check format
}

// op bits remaining for 0x18_05_xx subgroups 0x001f
int arcompact_handle18_05_00_dasm(DASM_OPS_16)
{
	int u;
	COMMON16_GET_u5;

	print("ADD_S SP, SP, 0x%02x", u*4);
	return 2;

}

int arcompact_handle18_05_01_dasm(DASM_OPS_16)
{
	int u;
	COMMON16_GET_u5;

	print("SUB_S SP, SP, 0x%02x", u*4);
	return 2;
}

// op bits remaining for 0x18_06_xx subgroups 0x0700
int arcompact_handle18_06_01_dasm(DASM_OPS_16)
{
	int breg;
	COMMON16_GET_breg
	REG_16BIT_RANGE(breg)

	print("POP_S %s", regnames[breg]);

	return 2;
}

int arcompact_handle18_06_11_dasm(DASM_OPS_16)
{
	int res = (op & 0x0700) >> 8;
	op &= ~0x0700; // all bits now used

	if (res)
		print("POP_S [BLINK] (Reserved Bits set %04x)", op);
	else
		print("POP_S [BLINK]");

	return 2;
}

// op bits remaining for 0x18_07_xx subgroups 0x0700
int arcompact_handle18_07_01_dasm(DASM_OPS_16)
{
	int breg;
	COMMON16_GET_breg
	REG_16BIT_RANGE(breg)

	print("PUSH_S %s", regnames[breg]);

	return 2;
}


int arcompact_handle18_07_11_dasm(DASM_OPS_16)
{
	int res = (op & 0x0700) >> 8;
	op &= ~0x0700; // all bits now used

	if (res)
		print("PUSH_S [BLINK] (Reserved Bits set %04x)", op);
	else
		print("PUSH_S [BLINK]");

	return 2;
}


int arcompact_handle19_0x_helper_dasm(DASM_OPS_16, const char* optext, int shift, int format)
{
	int s;

	COMMON16_GET_s9;
	// todo, signed
	s <<= shift;


	output  += sprintf( output, "%s %s, ", optext, regnames[0]);
	if (format == 0)
	{
		output  += sprintf( output, "[GP, %03x]", s);
	}
	else
	{
		output  += sprintf( output, "GP, %03x", s);
	}

	return 2;
}

int arcompact_handle19_00_dasm(DASM_OPS_16)  { return arcompact_handle19_0x_helper_dasm(DASM_PARAMS, "LD_S", 2, 0); }
int arcompact_handle19_01_dasm(DASM_OPS_16)  { return arcompact_handle19_0x_helper_dasm(DASM_PARAMS, "LDB_S", 0, 0); }
int arcompact_handle19_02_dasm(DASM_OPS_16)  { return arcompact_handle19_0x_helper_dasm(DASM_PARAMS, "LDW_S", 1, 0);  }
int arcompact_handle19_03_dasm(DASM_OPS_16)  { return arcompact_handle19_0x_helper_dasm(DASM_PARAMS, "ADD_S", 2, 1); }

int arcompact_handle1a_dasm(DASM_OPS_16)
{
	int breg, u;
	COMMON16_GET_breg;
	COMMON16_GET_u8;
	REG_16BIT_RANGE(breg);

	print("MOV_S %s, [PCL, %03x]", regnames[breg], u*4);

	return 2;
}

int arcompact_handle1b_dasm(DASM_OPS_16)
{
	int breg, u;
	COMMON16_GET_breg;
	COMMON16_GET_u8;
	REG_16BIT_RANGE(breg);

	print("MOV_S %s <- 0x%02x", regnames[breg], u);
	return 2;
}

int arcompact_handle1c_00_dasm(DASM_OPS_16)
{
	int breg, u;
	COMMON16_GET_breg;
	COMMON16_GET_u7;
	REG_16BIT_RANGE(breg);

	print("ADD_S %s <- %s, %02x", regnames[breg], regnames[breg], u);
	return 2;
}

int arcompact_handle1c_01_dasm(DASM_OPS_16)
{
	int breg, u;
	COMMON16_GET_breg;
	COMMON16_GET_u7;
	REG_16BIT_RANGE(breg);

	print("CMP_S %s, %02x", regnames[breg], u);
	return 2;
}

int arcompact_handle1d_helper_dasm(DASM_OPS_16, const char* optext)
{
	int breg;
	COMMON16_GET_breg;
	REG_16BIT_RANGE(breg);

	int s = (op & 0x007f) >> 0; op &= ~0x007f;
	if (s & 0x40) s = -0x40 + (s & 0x3f);

	print("%s %s, 0 to 0x%08x", optext, regnames[breg], PC_ALIGNED32 + s*2);
	return 2;
}


int arcompact_handle1d_00_dasm(DASM_OPS_16)  { return arcompact_handle1d_helper_dasm(DASM_PARAMS,"BREQ_S"); }
int arcompact_handle1d_01_dasm(DASM_OPS_16)  { return arcompact_handle1d_helper_dasm(DASM_PARAMS,"BRNE_S"); }


int arcompact_handle1e_0x_helper_dasm(DASM_OPS_16, const char* optext)
{
	int s = (op & 0x01ff) >> 0; op &= ~0x01ff;
	if (s & 0x100) s = -0x100 + (s & 0xff);

	print("%s %08x", optext, PC_ALIGNED32 + s*2);
	return 2;
}



int arcompact_handle1e_00_dasm(DASM_OPS_16)  { return arcompact_handle1e_0x_helper_dasm(DASM_PARAMS, "B_S");  }
int arcompact_handle1e_01_dasm(DASM_OPS_16)  { return arcompact_handle1e_0x_helper_dasm(DASM_PARAMS, "BEQ_S"); }
int arcompact_handle1e_02_dasm(DASM_OPS_16)  { return arcompact_handle1e_0x_helper_dasm(DASM_PARAMS, "BNE_S"); }

int arcompact_handle1e_03_0x_helper_dasm(DASM_OPS_16, const char* optext)
{
	int s = (op & 0x003f) >> 0; op &= ~0x003f;
	if (s & 0x020) s = -0x20 + (s & 0x1f);

	print("%s %08x", optext, PC_ALIGNED32 + s*2);
	return 2;
}

int arcompact_handle1e_03_00_dasm(DASM_OPS_16)  { return arcompact_handle1e_03_0x_helper_dasm(DASM_PARAMS, "BGT_S"); }
int arcompact_handle1e_03_01_dasm(DASM_OPS_16)  { return arcompact_handle1e_03_0x_helper_dasm(DASM_PARAMS, "BGE_S"); }
int arcompact_handle1e_03_02_dasm(DASM_OPS_16)  { return arcompact_handle1e_03_0x_helper_dasm(DASM_PARAMS, "BLT_S"); }
int arcompact_handle1e_03_03_dasm(DASM_OPS_16)  { return arcompact_handle1e_03_0x_helper_dasm(DASM_PARAMS, "BLE_S"); }
int arcompact_handle1e_03_04_dasm(DASM_OPS_16)  { return arcompact_handle1e_03_0x_helper_dasm(DASM_PARAMS, "BHI_S"); }
int arcompact_handle1e_03_05_dasm(DASM_OPS_16)  { return arcompact_handle1e_03_0x_helper_dasm(DASM_PARAMS, "BHS_S"); }
int arcompact_handle1e_03_06_dasm(DASM_OPS_16)  { return arcompact_handle1e_03_0x_helper_dasm(DASM_PARAMS, "BLO_S"); }
int arcompact_handle1e_03_07_dasm(DASM_OPS_16)  { return arcompact_handle1e_03_0x_helper_dasm(DASM_PARAMS, "BLS_S"); }

int arcompact_handle1f_dasm(DASM_OPS_16)
{
	int s = (op & 0x07ff) >> 0; op &= ~0x07ff;
	if (s & 0x400) s = -0x400 + (s & 0x3ff);

	print("BL_S %08x", PC_ALIGNED32 + (s*4));
	return 2;
}

/************************************************************************************************************************************
*                                                                                                                                   *
* illegal opcode handlers (disassembly)                                                                                             *
*                                                                                                                                   *
************************************************************************************************************************************/

int arcompact_handle01_01_00_06_dasm(DASM_OPS_32)  { print("<illegal 01_01_00_06> (%08x)", op); return 4; }
int arcompact_handle01_01_00_07_dasm(DASM_OPS_32)  { print("<illegal 01_01_00_07> (%08x)", op); return 4; }
int arcompact_handle01_01_00_08_dasm(DASM_OPS_32)  { print("<illegal 01_01_00_08> (%08x)", op); return 4; }
int arcompact_handle01_01_00_09_dasm(DASM_OPS_32)  { print("<illegal 01_01_00_09> (%08x)", op); return 4; }
int arcompact_handle01_01_00_0a_dasm(DASM_OPS_32)  { print("<illegal 01_01_00_0a> (%08x)", op); return 4; }
int arcompact_handle01_01_00_0b_dasm(DASM_OPS_32)  { print("<illegal 01_01_00_0b> (%08x)", op); return 4; }
int arcompact_handle01_01_00_0c_dasm(DASM_OPS_32)  { print("<illegal 01_01_00_0c> (%08x)", op); return 4; }
int arcompact_handle01_01_00_0d_dasm(DASM_OPS_32)  { print("<illegal 01_01_00_0d> (%08x)", op); return 4; }

int arcompact_handle01_01_01_06_dasm(DASM_OPS_32)  { print("<illegal 01_01_01_06> (%08x)", op); return 4; }
int arcompact_handle01_01_01_07_dasm(DASM_OPS_32)  { print("<illegal 01_01_01_07> (%08x)", op); return 4; }
int arcompact_handle01_01_01_08_dasm(DASM_OPS_32)  { print("<illegal 01_01_01_08> (%08x)", op); return 4; }
int arcompact_handle01_01_01_09_dasm(DASM_OPS_32)  { print("<illegal 01_01_01_09> (%08x)", op); return 4; }
int arcompact_handle01_01_01_0a_dasm(DASM_OPS_32)  { print("<illegal 01_01_01_0a> (%08x)", op); return 4; }
int arcompact_handle01_01_01_0b_dasm(DASM_OPS_32)  { print("<illegal 01_01_01_0b> (%08x)", op); return 4; }
int arcompact_handle01_01_01_0c_dasm(DASM_OPS_32)  { print("<illegal 01_01_01_0c> (%08x)", op); return 4; }
int arcompact_handle01_01_01_0d_dasm(DASM_OPS_32)  { print("<illegal 01_01_01_0d> (%08x)", op); return 4; }


int arcompact_handle04_1e_dasm(DASM_OPS_32)  { print("<illegal 0x04_1e> (%08x)", op); return 4;}
int arcompact_handle04_1f_dasm(DASM_OPS_32)  { print("<illegal 0x04_1f> (%08x)", op); return 4;}

int arcompact_handle04_24_dasm(DASM_OPS_32)  { print("<illegal 0x04_24> (%08x)", op); return 4;}
int arcompact_handle04_25_dasm(DASM_OPS_32)  { print("<illegal 0x04_25> (%08x)", op); return 4;}
int arcompact_handle04_26_dasm(DASM_OPS_32)  { print("<illegal 0x04_26> (%08x)", op); return 4;}
int arcompact_handle04_27_dasm(DASM_OPS_32)  { print("<illegal 0x04_27> (%08x)", op); return 4;}

int arcompact_handle04_2c_dasm(DASM_OPS_32)  { print("<illegal 0x04_2c> (%08x)", op); return 4;}
int arcompact_handle04_2d_dasm(DASM_OPS_32)  { print("<illegal 0x04_2d> (%08x)", op); return 4;}
int arcompact_handle04_2e_dasm(DASM_OPS_32)  { print("<illegal 0x04_2e> (%08x)", op); return 4;}

int arcompact_handle04_2f_0d_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_0d> (%08x)", op); return 4;}
int arcompact_handle04_2f_0e_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_0e> (%08x)", op); return 4;}
int arcompact_handle04_2f_0f_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_0f> (%08x)", op); return 4;}
int arcompact_handle04_2f_10_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_10> (%08x)", op); return 4;}
int arcompact_handle04_2f_11_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_11> (%08x)", op); return 4;}
int arcompact_handle04_2f_12_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_12> (%08x)", op); return 4;}
int arcompact_handle04_2f_13_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_13> (%08x)", op); return 4;}
int arcompact_handle04_2f_14_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_14> (%08x)", op); return 4;}
int arcompact_handle04_2f_15_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_15> (%08x)", op); return 4;}
int arcompact_handle04_2f_16_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_16> (%08x)", op); return 4;}
int arcompact_handle04_2f_17_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_17> (%08x)", op); return 4;}
int arcompact_handle04_2f_18_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_18> (%08x)", op); return 4;}
int arcompact_handle04_2f_19_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_19> (%08x)", op); return 4;}
int arcompact_handle04_2f_1a_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_1a> (%08x)", op); return 4;}
int arcompact_handle04_2f_1b_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_1b> (%08x)", op); return 4;}
int arcompact_handle04_2f_1c_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_1c> (%08x)", op); return 4;}
int arcompact_handle04_2f_1d_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_1d> (%08x)", op); return 4;}
int arcompact_handle04_2f_1e_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_1e> (%08x)", op); return 4;}
int arcompact_handle04_2f_1f_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_1f> (%08x)", op); return 4;}
int arcompact_handle04_2f_20_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_20> (%08x)", op); return 4;}
int arcompact_handle04_2f_21_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_21> (%08x)", op); return 4;}
int arcompact_handle04_2f_22_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_22> (%08x)", op); return 4;}
int arcompact_handle04_2f_23_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_23> (%08x)", op); return 4;}
int arcompact_handle04_2f_24_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_24> (%08x)", op); return 4;}
int arcompact_handle04_2f_25_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_25> (%08x)", op); return 4;}
int arcompact_handle04_2f_26_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_26> (%08x)", op); return 4;}
int arcompact_handle04_2f_27_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_27> (%08x)", op); return 4;}
int arcompact_handle04_2f_28_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_28> (%08x)", op); return 4;}
int arcompact_handle04_2f_29_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_29> (%08x)", op); return 4;}
int arcompact_handle04_2f_2a_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_2a> (%08x)", op); return 4;}
int arcompact_handle04_2f_2b_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_2b> (%08x)", op); return 4;}
int arcompact_handle04_2f_2c_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_2c> (%08x)", op); return 4;}
int arcompact_handle04_2f_2d_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_2d> (%08x)", op); return 4;}
int arcompact_handle04_2f_2e_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_2e> (%08x)", op); return 4;}
int arcompact_handle04_2f_2f_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_2f> (%08x)", op); return 4;}
int arcompact_handle04_2f_30_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_30> (%08x)", op); return 4;}
int arcompact_handle04_2f_31_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_31> (%08x)", op); return 4;}
int arcompact_handle04_2f_32_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_32> (%08x)", op); return 4;}
int arcompact_handle04_2f_33_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_33> (%08x)", op); return 4;}
int arcompact_handle04_2f_34_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_34> (%08x)", op); return 4;}
int arcompact_handle04_2f_35_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_35> (%08x)", op); return 4;}
int arcompact_handle04_2f_36_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_36> (%08x)", op); return 4;}
int arcompact_handle04_2f_37_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_37> (%08x)", op); return 4;}
int arcompact_handle04_2f_38_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_38> (%08x)", op); return 4;}
int arcompact_handle04_2f_39_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_39> (%08x)", op); return 4;}
int arcompact_handle04_2f_3a_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3a> (%08x)", op); return 4;}
int arcompact_handle04_2f_3b_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3b> (%08x)", op); return 4;}
int arcompact_handle04_2f_3c_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3c> (%08x)", op); return 4;}
int arcompact_handle04_2f_3d_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3d> (%08x)", op); return 4;}
int arcompact_handle04_2f_3e_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3e> (%08x)", op); return 4;}



int arcompact_handle05_2f_09_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_09> (%08x)", op); return 4;}
int arcompact_handle05_2f_0a_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_0a> (%08x)", op); return 4;}
int arcompact_handle05_2f_0b_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_0b> (%08x)", op); return 4;}
int arcompact_handle05_2f_0c_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_0c> (%08x)", op); return 4;}
int arcompact_handle05_2f_0d_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_0d> (%08x)", op); return 4;}
int arcompact_handle05_2f_0e_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_0e> (%08x)", op); return 4;}
int arcompact_handle05_2f_0f_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_0f> (%08x)", op); return 4;}
int arcompact_handle05_2f_10_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_10> (%08x)", op); return 4;}
int arcompact_handle05_2f_11_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_11> (%08x)", op); return 4;}
int arcompact_handle05_2f_12_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_12> (%08x)", op); return 4;}
int arcompact_handle05_2f_13_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_13> (%08x)", op); return 4;}
int arcompact_handle05_2f_14_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_14> (%08x)", op); return 4;}
int arcompact_handle05_2f_15_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_15> (%08x)", op); return 4;}
int arcompact_handle05_2f_16_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_16> (%08x)", op); return 4;}
int arcompact_handle05_2f_17_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_17> (%08x)", op); return 4;}
int arcompact_handle05_2f_18_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_18> (%08x)", op); return 4;}
int arcompact_handle05_2f_19_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_19> (%08x)", op); return 4;}
int arcompact_handle05_2f_1a_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_1a> (%08x)", op); return 4;}
int arcompact_handle05_2f_1b_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_1b> (%08x)", op); return 4;}
int arcompact_handle05_2f_1c_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_1c> (%08x)", op); return 4;}
int arcompact_handle05_2f_1d_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_1d> (%08x)", op); return 4;}
int arcompact_handle05_2f_1e_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_1e> (%08x)", op); return 4;}
int arcompact_handle05_2f_1f_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_1f> (%08x)", op); return 4;}
int arcompact_handle05_2f_20_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_20> (%08x)", op); return 4;}
int arcompact_handle05_2f_21_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_21> (%08x)", op); return 4;}
int arcompact_handle05_2f_22_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_22> (%08x)", op); return 4;}
int arcompact_handle05_2f_23_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_23> (%08x)", op); return 4;}
int arcompact_handle05_2f_24_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_24> (%08x)", op); return 4;}
int arcompact_handle05_2f_25_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_25> (%08x)", op); return 4;}
int arcompact_handle05_2f_26_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_26> (%08x)", op); return 4;}
int arcompact_handle05_2f_27_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_27> (%08x)", op); return 4;}
int arcompact_handle05_2f_28_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_28> (%08x)", op); return 4;}
int arcompact_handle05_2f_29_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_29> (%08x)", op); return 4;}
int arcompact_handle05_2f_2a_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_2a> (%08x)", op); return 4;}
int arcompact_handle05_2f_2b_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_2b> (%08x)", op); return 4;}
int arcompact_handle05_2f_2c_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_2c> (%08x)", op); return 4;}
int arcompact_handle05_2f_2d_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_2d> (%08x)", op); return 4;}
int arcompact_handle05_2f_2e_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_2e> (%08x)", op); return 4;}
int arcompact_handle05_2f_2f_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_2f> (%08x)", op); return 4;}
int arcompact_handle05_2f_30_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_30> (%08x)", op); return 4;}
int arcompact_handle05_2f_31_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_31> (%08x)", op); return 4;}
int arcompact_handle05_2f_32_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_32> (%08x)", op); return 4;}
int arcompact_handle05_2f_33_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_33> (%08x)", op); return 4;}
int arcompact_handle05_2f_34_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_34> (%08x)", op); return 4;}
int arcompact_handle05_2f_35_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_35> (%08x)", op); return 4;}
int arcompact_handle05_2f_36_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_36> (%08x)", op); return 4;}
int arcompact_handle05_2f_37_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_37> (%08x)", op); return 4;}
int arcompact_handle05_2f_38_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_38> (%08x)", op); return 4;}
int arcompact_handle05_2f_39_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_39> (%08x)", op); return 4;}
int arcompact_handle05_2f_3a_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3a> (%08x)", op); return 4;}
int arcompact_handle05_2f_3b_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3b> (%08x)", op); return 4;}
int arcompact_handle05_2f_3c_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3c> (%08x)", op); return 4;}
int arcompact_handle05_2f_3d_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3d> (%08x)", op); return 4;}
int arcompact_handle05_2f_3e_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3e> (%08x)", op); return 4;}


int arcompact_handle04_2f_3f_00_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_00> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_06_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_06> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_07_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_07> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_08_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_08> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_09_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_09> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_0a_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_0a> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_0b_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_0b> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_0c_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_0c> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_0d_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_0d> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_0e_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_0e> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_0f_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_0f> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_10_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_10> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_11_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_11> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_12_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_12> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_13_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_13> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_14_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_14> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_15_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_15> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_16_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_16> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_17_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_17> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_18_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_18> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_19_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_19> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_1a_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_1a> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_1b_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_1b> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_1c_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_1c> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_1d_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_1d> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_1e_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_1e> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_1f_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_1f> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_20_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_20> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_21_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_21> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_22_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_22> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_23_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_23> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_24_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_24> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_25_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_25> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_26_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_26> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_27_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_27> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_28_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_28> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_29_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_29> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_2a_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_2a> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_2b_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_2b> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_2c_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_2c> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_2d_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_2d> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_2e_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_2e> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_2f_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_2f> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_30_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_30> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_31_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_31> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_32_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_32> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_33_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_33> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_34_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_34> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_35_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_35> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_36_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_36> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_37_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_37> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_38_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_38> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_39_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_39> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_3a_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_3a> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_3b_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_3b> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_3c_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_3c> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_3d_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_3d> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_3e_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_3e> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_3f_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_3f> (%08x)", op); return 4;}

int arcompact_handle05_2f_3f_00_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_00> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_01_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_01> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_02_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_02> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_03_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_03> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_04_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_04> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_05_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_05> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_06_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_06> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_07_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_07> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_08_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_08> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_09_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_09> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_0a_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_0a> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_0b_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_0b> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_0c_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_0c> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_0d_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_0d> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_0e_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_0e> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_0f_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_0f> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_10_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_10> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_11_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_11> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_12_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_12> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_13_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_13> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_14_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_14> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_15_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_15> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_16_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_16> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_17_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_17> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_18_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_18> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_19_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_19> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_1a_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_1a> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_1b_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_1b> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_1c_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_1c> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_1d_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_1d> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_1e_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_1e> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_1f_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_1f> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_20_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_20> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_21_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_21> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_22_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_22> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_23_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_23> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_24_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_24> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_25_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_25> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_26_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_26> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_27_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_27> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_28_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_28> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_29_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_29> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_2a_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_2a> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_2b_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_2b> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_2c_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_2c> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_2d_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_2d> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_2e_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_2e> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_2f_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_2f> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_30_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_30> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_31_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_31> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_32_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_32> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_33_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_33> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_34_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_34> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_35_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_35> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_36_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_36> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_37_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_37> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_38_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_38> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_39_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_39> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_3a_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_3a> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_3b_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_3b> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_3c_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_3c> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_3d_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_3d> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_3e_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_3e> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_3f_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_3f> (%08x)", op); return 4;}




int arcompact_handle04_38_dasm(DASM_OPS_32)  { print("<illegal 0x04_38> (%08x)", op); return 4;}
int arcompact_handle04_39_dasm(DASM_OPS_32)  { print("<illegal 0x04_39> (%08x)", op); return 4;}
int arcompact_handle04_3a_dasm(DASM_OPS_32)  { print("<illegal 0x04_3a> (%08x)", op); return 4;}
int arcompact_handle04_3b_dasm(DASM_OPS_32)  { print("<illegal 0x04_3b> (%08x)", op); return 4;}
int arcompact_handle04_3c_dasm(DASM_OPS_32)  { print("<illegal 0x04_3c> (%08x)", op); return 4;}
int arcompact_handle04_3d_dasm(DASM_OPS_32)  { print("<illegal 0x04_3d> (%08x)", op); return 4;}
int arcompact_handle04_3e_dasm(DASM_OPS_32)  { print("<illegal 0x04_3e> (%08x)", op); return 4;}
int arcompact_handle04_3f_dasm(DASM_OPS_32)  { print("<illegal 0x04_3f> (%08x)", op); return 4;}


int arcompact_handle05_09_dasm(DASM_OPS_32)  { print("<illegal 0x05_09> (%08x)", op); return 4;}
int arcompact_handle05_0c_dasm(DASM_OPS_32)  { print("<illegal 0x05_0c> (%08x)", op); return 4;}
int arcompact_handle05_0d_dasm(DASM_OPS_32)  { print("<illegal 0x05_0d> (%08x)", op); return 4;}
int arcompact_handle05_0e_dasm(DASM_OPS_32)  { print("<illegal 0x05_0e> (%08x)", op); return 4;}
int arcompact_handle05_0f_dasm(DASM_OPS_32)  { print("<illegal 0x05_0f> (%08x)", op); return 4;}
int arcompact_handle05_10_dasm(DASM_OPS_32)  { print("<illegal 0x05_10> (%08x)", op); return 4;}
int arcompact_handle05_11_dasm(DASM_OPS_32)  { print("<illegal 0x05_11> (%08x)", op); return 4;}
int arcompact_handle05_12_dasm(DASM_OPS_32)  { print("<illegal 0x05_12> (%08x)", op); return 4;}
int arcompact_handle05_13_dasm(DASM_OPS_32)  { print("<illegal 0x05_13> (%08x)", op); return 4;}
int arcompact_handle05_14_dasm(DASM_OPS_32)  { print("<illegal 0x05_14> (%08x)", op); return 4;}
int arcompact_handle05_15_dasm(DASM_OPS_32)  { print("<illegal 0x05_15> (%08x)", op); return 4;}
int arcompact_handle05_16_dasm(DASM_OPS_32)  { print("<illegal 0x05_16> (%08x)", op); return 4;}
int arcompact_handle05_17_dasm(DASM_OPS_32)  { print("<illegal 0x05_17> (%08x)", op); return 4;}
int arcompact_handle05_18_dasm(DASM_OPS_32)  { print("<illegal 0x05_18> (%08x)", op); return 4;}
int arcompact_handle05_19_dasm(DASM_OPS_32)  { print("<illegal 0x05_19> (%08x)", op); return 4;}
int arcompact_handle05_1a_dasm(DASM_OPS_32)  { print("<illegal 0x05_1a> (%08x)", op); return 4;}
int arcompact_handle05_1b_dasm(DASM_OPS_32)  { print("<illegal 0x05_1b> (%08x)", op); return 4;}
int arcompact_handle05_1c_dasm(DASM_OPS_32)  { print("<illegal 0x05_1c> (%08x)", op); return 4;}
int arcompact_handle05_1d_dasm(DASM_OPS_32)  { print("<illegal 0x05_1d> (%08x)", op); return 4;}
int arcompact_handle05_1e_dasm(DASM_OPS_32)  { print("<illegal 0x05_1e> (%08x)", op); return 4;}
int arcompact_handle05_1f_dasm(DASM_OPS_32)  { print("<illegal 0x05_1f> (%08x)", op); return 4;}
int arcompact_handle05_20_dasm(DASM_OPS_32)  { print("<illegal 0x05_20> (%08x)", op); return 4;}
int arcompact_handle05_21_dasm(DASM_OPS_32)  { print("<illegal 0x05_21> (%08x)", op); return 4;}
int arcompact_handle05_22_dasm(DASM_OPS_32)  { print("<illegal 0x05_22> (%08x)", op); return 4;}
int arcompact_handle05_23_dasm(DASM_OPS_32)  { print("<illegal 0x05_23> (%08x)", op); return 4;}
int arcompact_handle05_24_dasm(DASM_OPS_32)  { print("<illegal 0x05_24> (%08x)", op); return 4;}
int arcompact_handle05_25_dasm(DASM_OPS_32)  { print("<illegal 0x05_25> (%08x)", op); return 4;}
int arcompact_handle05_26_dasm(DASM_OPS_32)  { print("<illegal 0x05_26> (%08x)", op); return 4;}
int arcompact_handle05_27_dasm(DASM_OPS_32)  { print("<illegal 0x05_27> (%08x)", op); return 4;}

int arcompact_handle05_2a_dasm(DASM_OPS_32)  { print("<illegal 0x05_2a> (%08x)", op); return 4;}
int arcompact_handle05_2b_dasm(DASM_OPS_32)  { print("<illegal 0x05_2b> (%08x)", op); return 4;}
int arcompact_handle05_2c_dasm(DASM_OPS_32)  { print("<illegal 0x05_2c> (%08x)", op); return 4;}
int arcompact_handle05_2d_dasm(DASM_OPS_32)  { print("<illegal 0x05_2d> (%08x)", op); return 4;}
int arcompact_handle05_2e_dasm(DASM_OPS_32)  { print("<illegal 0x05_2e> (%08x)", op); return 4;}

int arcompact_handle05_30_dasm(DASM_OPS_32)  { print("<illegal 0x05_30> (%08x)", op); return 4;}
int arcompact_handle05_31_dasm(DASM_OPS_32)  { print("<illegal 0x05_31> (%08x)", op); return 4;}
int arcompact_handle05_32_dasm(DASM_OPS_32)  { print("<illegal 0x05_32> (%08x)", op); return 4;}
int arcompact_handle05_33_dasm(DASM_OPS_32)  { print("<illegal 0x05_33> (%08x)", op); return 4;}
int arcompact_handle05_34_dasm(DASM_OPS_32)  { print("<illegal 0x05_34> (%08x)", op); return 4;}
int arcompact_handle05_35_dasm(DASM_OPS_32)  { print("<illegal 0x05_35> (%08x)", op); return 4;}
int arcompact_handle05_36_dasm(DASM_OPS_32)  { print("<illegal 0x05_36> (%08x)", op); return 4;}
int arcompact_handle05_37_dasm(DASM_OPS_32)  { print("<illegal 0x05_37> (%08x)", op); return 4;}
int arcompact_handle05_38_dasm(DASM_OPS_32)  { print("<illegal 0x05_38> (%08x)", op); return 4;}
int arcompact_handle05_39_dasm(DASM_OPS_32)  { print("<illegal 0x05_39> (%08x)", op); return 4;}
int arcompact_handle05_3a_dasm(DASM_OPS_32)  { print("<illegal 0x05_3a> (%08x)", op); return 4;}
int arcompact_handle05_3b_dasm(DASM_OPS_32)  { print("<illegal 0x05_3b> (%08x)", op); return 4;}
int arcompact_handle05_3c_dasm(DASM_OPS_32)  { print("<illegal 0x05_3c> (%08x)", op); return 4;}
int arcompact_handle05_3d_dasm(DASM_OPS_32)  { print("<illegal 0x05_3d> (%08x)", op); return 4;}
int arcompact_handle05_3e_dasm(DASM_OPS_32)  { print("<illegal 0x05_3e> (%08x)", op); return 4;}
int arcompact_handle05_3f_dasm(DASM_OPS_32)  { print("<illegal 0x05_3f> (%08x)", op); return 4;}

int arcompact_handle0f_00_04_dasm(DASM_OPS_16)  { print("<illegal 0x0f_00_00> (%08x)", op); return 2;}
int arcompact_handle0f_00_05_dasm(DASM_OPS_16)  { print("<illegal 0x0f_00_00> (%08x)", op); return 2;}
int arcompact_handle0f_00_07_02_dasm(DASM_OPS_16)  { print("<illegal 0x0f_00_07_02> (%08x)", op); return 2;}
int arcompact_handle0f_00_07_03_dasm(DASM_OPS_16)  { print("<illegal 0x0f_00_07_03> (%08x)", op); return 2;}
int arcompact_handle0f_01_dasm(DASM_OPS_16)  { print("<illegal 0x0f_01> (%08x)", op); return 2;}
int arcompact_handle0f_03_dasm(DASM_OPS_16)  { print("<illegal 0x0f_03> (%08x)", op); return 2;}
int arcompact_handle0f_08_dasm(DASM_OPS_16)  { print("<illegal 0x0f_08> (%08x)", op); return 2;}
int arcompact_handle0f_09_dasm(DASM_OPS_16)  { print("<illegal 0x0f_09> (%08x)", op); return 2;}
int arcompact_handle0f_0a_dasm(DASM_OPS_16)  { print("<illegal 0x0f_0a> (%08x)", op); return 2;}
int arcompact_handle0f_17_dasm(DASM_OPS_16)  { print("<illegal 0x0f_17> (%08x)", op); return 2;}

int arcompact_handle18_05_02_dasm(DASM_OPS_16)  { print("<illegal 0x18_05_02> (%04x)", op); return 2;}
int arcompact_handle18_05_03_dasm(DASM_OPS_16)  { print("<illegal 0x18_05_03> (%04x)", op); return 2;}
int arcompact_handle18_05_04_dasm(DASM_OPS_16)  { print("<illegal 0x18_05_04> (%04x)", op); return 2;}
int arcompact_handle18_05_05_dasm(DASM_OPS_16)  { print("<illegal 0x18_05_05> (%04x)", op); return 2;}
int arcompact_handle18_05_06_dasm(DASM_OPS_16)  { print("<illegal 0x18_05_06> (%04x)", op); return 2;}
int arcompact_handle18_05_07_dasm(DASM_OPS_16)  { print("<illegal 0x18_05_07> (%04x)", op); return 2;}
int arcompact_handle18_06_00_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_00> (%04x)",  op); return 2;}
int arcompact_handle18_06_02_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_02> (%04x)", op); return 2;}
int arcompact_handle18_06_03_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_03> (%04x)", op); return 2;}
int arcompact_handle18_06_04_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_04> (%04x)", op); return 2;}
int arcompact_handle18_06_05_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_05> (%04x)", op); return 2;}
int arcompact_handle18_06_06_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_06> (%04x)", op); return 2;}
int arcompact_handle18_06_07_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_07> (%04x)", op); return 2;}
int arcompact_handle18_06_08_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_08> (%04x)", op); return 2;}
int arcompact_handle18_06_09_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_09> (%04x)", op); return 2;}
int arcompact_handle18_06_0a_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_0a> (%04x)", op); return 2;}
int arcompact_handle18_06_0b_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_0b> (%04x)", op); return 2;}
int arcompact_handle18_06_0c_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_0c> (%04x)", op); return 2;}
int arcompact_handle18_06_0d_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_0d> (%04x)", op); return 2;}
int arcompact_handle18_06_0e_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_0e> (%04x)", op); return 2;}
int arcompact_handle18_06_0f_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_0f> (%04x)", op); return 2;}
int arcompact_handle18_06_10_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_10> (%04x)", op); return 2;}
int arcompact_handle18_06_12_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_12> (%04x)",  op); return 2;}
int arcompact_handle18_06_13_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_13> (%04x)",  op); return 2;}
int arcompact_handle18_06_14_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_14> (%04x)",  op); return 2;}
int arcompact_handle18_06_15_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_15> (%04x)",  op); return 2;}
int arcompact_handle18_06_16_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_16> (%04x)",  op); return 2;}
int arcompact_handle18_06_17_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_17> (%04x)",  op); return 2;}
int arcompact_handle18_06_18_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_18> (%04x)",  op); return 2;}
int arcompact_handle18_06_19_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_19> (%04x)",  op); return 2;}
int arcompact_handle18_06_1a_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_1a> (%04x)",  op); return 2;}
int arcompact_handle18_06_1b_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_1b> (%04x)",  op); return 2;}
int arcompact_handle18_06_1c_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_1c> (%04x)",  op); return 2;}
int arcompact_handle18_06_1d_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_1d> (%04x)",  op); return 2;}
int arcompact_handle18_06_1e_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_1e> (%04x)",  op); return 2;}
int arcompact_handle18_06_1f_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_1f> (%04x)",  op); return 2;}
int arcompact_handle18_07_00_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_00> (%04x)",  op); return 2;}
int arcompact_handle18_07_02_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_02> (%04x)", op); return 2;}
int arcompact_handle18_07_03_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_03> (%04x)", op); return 2;}
int arcompact_handle18_07_04_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_04> (%04x)", op); return 2;}
int arcompact_handle18_07_05_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_05> (%04x)", op); return 2;}
int arcompact_handle18_07_06_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_06> (%04x)", op); return 2;}
int arcompact_handle18_07_07_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_07> (%04x)", op); return 2;}
int arcompact_handle18_07_08_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_08> (%04x)", op); return 2;}
int arcompact_handle18_07_09_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_09> (%04x)", op); return 2;}
int arcompact_handle18_07_0a_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_0a> (%04x)", op); return 2;}
int arcompact_handle18_07_0b_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_0b> (%04x)", op); return 2;}
int arcompact_handle18_07_0c_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_0c> (%04x)", op); return 2;}
int arcompact_handle18_07_0d_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_0d> (%04x)", op); return 2;}
int arcompact_handle18_07_0e_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_0e> (%04x)", op); return 2;}
int arcompact_handle18_07_0f_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_0f> (%04x)", op); return 2;}
int arcompact_handle18_07_10_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_10> (%04x)", op); return 2;}
int arcompact_handle18_07_12_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_12> (%04x)",  op); return 2;}
int arcompact_handle18_07_13_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_13> (%04x)",  op); return 2;}
int arcompact_handle18_07_14_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_14> (%04x)",  op); return 2;}
int arcompact_handle18_07_15_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_15> (%04x)",  op); return 2;}
int arcompact_handle18_07_16_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_16> (%04x)",  op); return 2;}
int arcompact_handle18_07_17_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_17> (%04x)",  op); return 2;}
int arcompact_handle18_07_18_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_18> (%04x)",  op); return 2;}
int arcompact_handle18_07_19_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_19> (%04x)",  op); return 2;}
int arcompact_handle18_07_1a_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_1a> (%04x)",  op); return 2;}
int arcompact_handle18_07_1b_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_1b> (%04x)",  op); return 2;}
int arcompact_handle18_07_1c_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_1c> (%04x)",  op); return 2;}
int arcompact_handle18_07_1d_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_1d> (%04x)",  op); return 2;}
int arcompact_handle18_07_1e_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_1e> (%04x)",  op); return 2;}
int arcompact_handle18_07_1f_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_1f> (%04x)",  op); return 2;}
