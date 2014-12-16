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


// condition codes (basic ones are the same as arc 
static const char *conditions[0x20] =
{
	/* 00 */ "AL", // (aka RA         - Always)
	/* 01 */ "EQ", // (aka Z          - Zero
	/* 02 */ "NE", // (aka NZ         - Non-Zero)
	/* 03 */ "PL", // (aka P          - Positive)
	/* 04 */ "MI", // (aka N          - Negative)
	/* 05 */ "CS", // (aka C,  LO     - Carry set / Lower than) (unsigned)
	/* 06 */ "CC", // (aka CC, NC, HS - Carry Clear / Higher or Same) (unsigned) 
	/* 07 */ "VS", // (aka V          - Overflow set)
	/* 08 */ "VC", // (aka NV         - Overflow clear)
	/* 09 */ "GT", // (               - Greater than) (signed)
	/* 0a */ "GE", // (               - Greater than or Equal) (signed)
	/* 0b */ "LT", // (               - Less than) (signed)
	/* 0c */ "LE", // (               - Less than or Equal) (signed)
	/* 0d */ "HI", // (               - Higher than) (unsigned)
	/* 0e */ "LS", // (               - Lower or Same) (unsigned)
	/* 0f */ "PNZ",// (               - Positive non-0 value)
	/* 10 */ "0x10 Reserved", // possible CPU implementation specifics
	/* 11 */ "0x11 Reserved",
	/* 12 */ "0x12 Reserved",
	/* 13 */ "0x13 Reserved",
	/* 14 */ "0x14 Reserved",
	/* 15 */ "0x15 Reserved",
	/* 16 */ "0x16 Reserved",
	/* 17 */ "0x17 Reserved",
	/* 18 */ "0x18 Reserved",
	/* 19 */ "0x19 Reserved",
	/* 1a */ "0x1a Reserved",
	/* 1b */ "0x1b Reserved",
	/* 1c */ "0x1c Reserved",
	/* 1d */ "0x1d Reserved",
	/* 1e */ "0x1e Reserved",
	/* 1f */ "0x1f Reserved"
};


//#define EXPLICIT_EXTENSIONS

static const char *datasize[0x4] =
{
#ifdef EXPLICIT_EXTENSIONS
	/* 00 */ ".L", // Dword (default) (can use no extension, using .L to be explicit)
#else
	/* 00 */ "",// Dword (default)
#endif
	/* 01 */ ".B", // Byte
	/* 02 */ ".W", // Word
	/* 03 */ ".<illegal data size>"
};

static const char *dataextend[0x2] =
{
#ifdef EXPLICIT_EXTENSIONS
	/* 00 */ ".ZX", // Zero Extend (can use no extension, using .ZX to be explicit)
else
	/* 00 */ "", // Zero Extend
#endif
	/* 01 */ ".X" // Sign Extend
};

static const char *addressmode[0x4] =
{
#ifdef EXPLICIT_EXTENSIONS
	/* 00 */ ".AN", // No Writeback (can use no extension, using .AN to be explicit)
#else
	/* 00 */ "", // No Writeback
#endif
	/* 01 */ ".AW", // Writeback pre memory access
	/* 02 */ ".AB", // Writeback post memory access
	/* 03 */ ".AS"  // scaled
};

static const char *cachebit[0x2] =
{
#ifdef EXPLICIT_EXTENSIONS
	/* 00 */ ".EN", // Data Cache Enabled (can use no extension, using .EN to be explicit)
#else
	/* 00 */ "", // Data Cache Enabled
#endif
	/* 01 */ ".DI" // Direct to Memory (Cache Bypass)
};

static const char *flagbit[0x2] =
{
#ifdef EXPLICIT_EXTENSIONS
	/* 00 */ ".NF", // Don't Set Flags (can use no extension, using .NF to be explicit)
#else
	/* 00 */ "", // Don't Set Flags
#endif
	/* 01 */ ".F" // Set Flags
};

static const char *delaybit[0x2] =
{
	/* 00 */ ".ND", // Don't execute opcode in delay slot
	/* 01 */ ".D"   // Execute Opcode in delay slot
};


static const char *regnames[0x40] =
{
	/* 00 */ "r0",
	/* 01 */ "r1",
	/* 02 */ "r2",
	/* 03 */ "r3",
	/* 04 */ "r4",
	/* 05 */ "r5",
	/* 06 */ "r6",
	/* 07 */ "r7",
	/* 08 */ "r8",
	/* 09 */ "r9",
	/* 0a */ "r10",
	/* 0b */ "r11",
	/* 0c */ "r12",
	/* 0d */ "r13",
	/* 0e */ "r14",
	/* 0f */ "r15",

	/* 10 */ "r16",
	/* 11 */ "r17",
	/* 12 */ "r18",
	/* 13 */ "r19",
	/* 14 */ "r20",
	/* 15 */ "r21",
	/* 16 */ "r22",
	/* 17 */ "r23",
	/* 18 */ "r24",
	/* 19 */ "r25",
	/* 1a */ "r26(GP)",
	/* 1b */ "r27(FP)",
	/* 1c */ "r28(SP)",
	/* 1d */ "r29(ILINK1)",
	/* 1e */ "r30(ILINK2)",
	/* 1f */ "r31(BLINK)",

	/* 20 */ "r32(ext)",
	/* 21 */ "r33(ext)",
	/* 22 */ "r34(ext)",
	/* 23 */ "r35(ext)",
	/* 24 */ "r36(ext)",
	/* 25 */ "r37(ext)",
	/* 26 */ "r38(ext)",
	/* 27 */ "r39(ext)",
	/* 28 */ "r40(ext)",
	/* 29 */ "r41(ext)",
	/* 2a */ "r42(ext)",
	/* 2b */ "r43(ext)",
	/* 2c */ "r44(ext)",
	/* 2d */ "r45(ext)",
	/* 2e */ "r46(ext)",
	/* 2f */ "r47(ext)",

	/* 30 */ "r48(ext)",
	/* 31 */ "r49(ext)",
	/* 32 */ "r50(ext)",
	/* 33 */ "r51(ext)",
	/* 34 */ "r52(ext)",
	/* 35 */ "r53(ext)",
	/* 36 */ "r54(ext)",
	/* 37 */ "r55(ext)",
	/* 38 */ "r56(ext)",
	/* 39 */ "r57(ext)", // MLO  (result registers for optional multply functions)
	/* 3a */ "r58(ext)", // MMID 
	/* 3b */ "r59(ext)", // MHI 
	/* 3c */ "r60(LP_COUNT)",
	/* 3d */ "r61(reserved)",
	/* 3e */ "r62(LIMM)", // use Long Immediate Data instead of register
	/* 3f */ "r63(PCL)"
};



#define GET_01_01_01_BRANCH_ADDR \
	INT32 address = (op & 0x00fe0000) >> 17; \
	address |= ((op & 0x00008000) >> 15) << 7; \
	if (address & 0x80) address = -0x80 + (address & 0x7f); \
	op &= ~ 0x00fe800f;


#define GROUP_0e_GET_h \
	h =  ((op & 0x0007) << 3); \
    h |= ((op & 0x00e0) >> 5); \
	op &= ~0x00e7; \

#define COMMON16_GET_breg \
	breg =  ((op & 0x0700) >>8); \
	op &= ~0x0700; \

#define COMMON16_GET_creg \
	creg =  ((op & 0x00e0) >>5); \
	op &= ~0x00e0; \

#define COMMON16_GET_areg \
	areg =  ((op & 0x0007) >>0); \
	op &= ~0x0007; \

#define COMMON16_GET_u3 \
	u =  ((op & 0x0007) >>0); \
	op &= ~0x0007; \

#define COMMON16_GET_u5 \
	u =  ((op & 0x001f) >>0); \
	op &= ~0x001f; \

#define COMMON16_GET_u8 \
	u =  ((op & 0x00ff) >>0); \
	op &= ~0x00ff; \

#define COMMON16_GET_u7 \
	u =  ((op & 0x007f) >>0); \
	op &= ~0x007f; \

// registers used in 16-bit opcodes hae a limited range
// and can only address registers r0-r3 and r12-r15

#define REG_16BIT_RANGE(_reg_) \
	if (_reg_>3) _reg_+= 8; \


// this is as messed up as the rest of the 16-bit alignment in LE mode...

#define GET_LIMM \
	limm = oprom[4] | (oprom[5] << 8); \
	limm |= (oprom[2] << 16) | (oprom[3] << 24); \

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
	if (address & 0x800000) address = -0x800000 + (address & 0x7fffff);
	int n = (op & 0x00000020) >> 5; op &= ~0x00000020;
	UINT8 condition = op & 0x0000001f;

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

	UINT8 condition = op & 0x0000001f;

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


	int c = (op & 0x00000fc0) >> 6;
	int b = (op & 0x07000000) >> 24;
	b |= ((op & 0x00007000) >> 12) << 3;
	int n = (op & 0x00000020) >> 5; op &= ~0x00000020;

	op &= ~0x07007fe0;

	if ((b != LIMM_REG) && (c != LIMM_REG))
	{
		print("%s%s %s, %s %08x (%08x)", optext, delaybit[n], regnames[b], regnames[c], PC_ALIGNED32 + (address * 2), op & ~0xf8fe800f);
	}
	else
	{
		UINT32 limm;
		GET_LIMM_32;
		size = 8;

		if ((b == LIMM_REG) && (c != LIMM_REG))
		{
			print("%s%s (%08x) %s %08x (%08x)", optext, delaybit[n], limm, regnames[b], PC_ALIGNED32 + (address * 2), op & ~0xf8fe800f);
		}
		else if ((c == LIMM_REG) && (b != LIMM_REG))
		{
			print("%s%s %s, (%08x) %08x (%08x)", optext, delaybit[n], regnames[b], limm, PC_ALIGNED32 + (address * 2), op & ~0xf8fe800f);
		}
		else
		{
			// b and c are LIMM? invalid??
			print("%s%s (%08x), (%08x) (illegal?) %08x (%08x)", optext, delaybit[n], limm, limm, PC_ALIGNED32 + (address * 2), op & ~0xf8fe800f);

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

	int u = (op & 0x00000fc0) >> 6;
	int b = (op & 0x07000000) >> 24;
	b |= ((op & 0x00007000) >> 12) << 3;
	int n = (op & 0x00000020) >> 5; op &= ~0x00000020;

	op &= ~0x07007fe0;

	print("%s%s %s, 0x%02x %08x (%08x)", optext, delaybit[n], regnames[b], u, PC_ALIGNED32 + (address * 2), op & ~0xf8fe800f);
	
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

	int A = (op & 0x0000003f) >> 0;  //op &= ~0x0000003f;
	int X = (op & 0x00000040) >> 6;  //op &= ~0x00000040;
	int Z = (op & 0x00000180) >> 7;  //op &= ~0x00000180;
	int a = (op & 0x00000600) >> 9;  //op &= ~0x00000600;
	int D = (op & 0x00000800) >> 11;// op &= ~0x00000800;
	int B = (op & 0x00007000) >> 12;// op &= ~0x00007000;
	int S = (op & 0x00008000) >> 15;// op &= ~0x00008000;
	int s = (op & 0x00ff0000) >> 16;// op &= ~0x00ff0000;
	int b = (op & 0x07000000) >> 24;// op &= ~0x07000000;

	int breg = b | (B << 3);
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
	output  += sprintf( output, "%s <- ", regnames[A]);
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
	// bitpos
	// 1111 1111 1111 1111 0000 0000 0000 0000
	// fedc ba98 7654 3210 fedc ba98 7654 3210
	// fields
	// 0001 1bbb ssss ssss SBBB CCCC CCDa aZZR
	int B = (op & 0x00007000) >> 12;// op &= ~0x00007000;
	int S = (op & 0x00008000) >> 15;// op &= ~0x00008000;
	int s = (op & 0x00ff0000) >> 16;// op &= ~0x00ff0000;
	int b = (op & 0x07000000) >> 24;// op &= ~0x07000000;

	int breg = b | (B << 3);
	int sdat = s | (S << 8); // todo - signed

	int R = (op & 0x00000001) >> 0; op &= ~0x00000001;
	int Z = (op & 0x00000006) >> 1; op &= ~0x00000006;
	int a = (op & 0x00000018) >> 3; op &= ~0x00000018;
	int D = (op & 0x00000020) >> 5; op &= ~0x00000020;
	int C = (op & 0x00000fc0) >> 6; op &= ~0x00000fc0;
	
	UINT32 limm = 0;
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
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

	output  += sprintf( output, "%s", regnames[C]);

	if (R) output  += sprintf( output, "(reserved bit set)");


	return size;
}


int arcompact_handle04_helper_dasm(DASM_OPS_32, const char* optext, int ignore_dst, int b_reserved)
{
	//           PP
	// 0010 0bbb 00ii iiii FBBB CCCC CCAA AAAA
	int size = 4;

	int p = (op & 0x00c00000) >> 22; op &= ~0x00c00000;
	int b = (op & 0x07000000) >> 24; op &= ~0x07000000;
	int B = (op & 0x00007000) >> 12; op &= ~0x00007000;
	int breg = b | (B << 3);
	int F = (op & 0x00008000) >> 15;op &= ~0x00008000;

	output  += sprintf( output, "%s", optext);
	output  += sprintf( output, "%s", flagbit[F]);
//	output  += sprintf( output, " p(%d)", p);
	
	
	if (!b_reserved)
	{
		output += sprintf(output, " %s, ", regnames[breg]);
	}
	else
	{
		if (breg) output += sprintf(output, "reserved(%s), ", regnames[breg]);
	}


	if (p == 0)
	{
		// 0010 0bbb 00ii iiii FBBB CCCC CCAA AAAA

		int C = (op & 0x00000fc0) >> 6; op &= ~0x00000fc0;
		int A = (op & 0x0000003f) >> 0; op &= ~0x0000003f;

		if (C == LIMM_REG)
		{
			UINT32 limm;
			GET_LIMM_32;
			size = 8;	
			output  += sprintf( output, "(%08x) ", limm );
			if (!ignore_dst) output  += sprintf( output, "DST(%s)", regnames[A]);
			else
			{
				if (A) output += sprintf(output, "unused(%s)", regnames[A]);
			}
		}
		else
		{
			output  += sprintf( output, "C(%s) ", regnames[C]);
			if (!ignore_dst) output  += sprintf( output, "DST(%s)", regnames[A]);
			else
			{
				if (A) output += sprintf(output, "unused(%s)", regnames[A]);
			}

		}
	}
	else if (p == 1)
	{
		// 0010 0bbb 00ii iiii FBBB UUUU UUAA AAAA
		int U = (op & 0x00000fc0) >> 6; op &= ~0x00000fc0;
		int A = (op & 0x0000003f) >> 0; op &= ~0x0000003f;

		output  += sprintf( output, "U(%02x) ", U );
		if (!ignore_dst) output  += sprintf( output, "DST(%s)", regnames[A]);			
		else
		{
			if (A) output += sprintf(output, "unused(%s)", regnames[A]);
		}
	}
	else if (p == 2)
	{
		int S = (op & 0x00000fff) >> 0; op &= ~0x00000fff;
		output  += sprintf( output, "S(%02x)", S);

	}
	else if (p == 3)
	{
		int M = (op & 0x00000020) >> 5; op &= ~0x00000020;
		int Q = (op & 0x0000001f) >> 0; op &= ~0x0000001f;
	
		output  += sprintf( output, " M(%d)", M);
		output  += sprintf( output, " Cond<%s> ", conditions[Q]);

		if (M == 0)
		{
			int C = (op & 0x00000fc0) >> 6; op &= ~0x00000fc0;
			output  += sprintf( output, "C(%s)", regnames[C]);

		}
		else if (M == 1)
		{
			int U = (op & 0x00000fc0) >> 6; op &= ~0x00000fc0;
			output  += sprintf( output, "U(%02x)", U);

		}

	}

	return size;
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




int arcompact_handle04_28_dasm(DASM_OPS_32)  { print("LPcc (%08x)", op); return 4;}
int arcompact_handle04_29_dasm(DASM_OPS_32)  { print("FLAG (%08x)", op); return 4;}
int arcompact_handle04_2a_dasm(DASM_OPS_32)  { print("LR (%08x)", op); return 4;}
int arcompact_handle04_2b_dasm(DASM_OPS_32)  { print("SR (%08x)", op); return 4;}


int arcompact_handle04_2f_helper_dasm(DASM_OPS_32, const char* optext)
{
	//           
	// 0010 0bbb pp10 1111 FBBB CCCC CCII IIII
	int size = 4;

	int p = (op & 0x00c00000) >> 22; op &= ~0x00c00000;
	int b = (op & 0x07000000) >> 24; op &= ~0x07000000;
	int B = (op & 0x00007000) >> 12; op &= ~0x00007000;
	int breg = b | (B << 3);
	int F = (op & 0x00008000) >> 15;op &= ~0x00008000;

	output  += sprintf( output, "%s", optext);
	output  += sprintf( output, "%s", flagbit[F]);
//	output  += sprintf( output, " p(%d)", p);
	
	
	output += sprintf(output, " %s, ", regnames[breg]);


	if (p == 0)
	{
		int C = (op & 0x00000fc0) >> 6; op &= ~0x00000fc0;

		if (C == LIMM_REG)
		{
			UINT32 limm;
			GET_LIMM_32;
			size = 8;	
			output  += sprintf( output, "(%08x) ", limm );

		}
		else
		{
			output  += sprintf( output, "C(%s) ", regnames[C]);
		}
	}
	else if (p == 1)
	{
		int U = (op & 0x00000fc0) >> 6; op &= ~0x00000fc0;

		output  += sprintf( output, "U(0x%02x) ", U);
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
	output += sprintf(output, "LD");
	output += sprintf(output, "%s", datasize[dsize]);
	output += sprintf(output, "%s", dataextend[extend]);

	int mode = (op & 0x00c00000) >> 22; op &= ~0x00c00000;
	int b = (op & 0x07000000) >> 24; op &= ~0x07000000;
	int B = (op & 0x00007000) >> 12; op &= ~0x00007000;
	int breg = b | (B << 3);
	int D = (op & 0x00008000) >> 15;op &= ~0x00008000;
	int C = (op & 0x00000fc0) >> 6; op &= ~0x00000fc0;
	int A = (op & 0x0000003f) >> 0; op &= ~0x0000003f;

	output += sprintf(output, "%s", addressmode[mode]);
	output += sprintf(output, "%s", cachebit[D]);

	output  += sprintf( output, "DST(%s)", regnames[A]);
	output  += sprintf( output, "SRC1(%s)", regnames[breg]);
	output  += sprintf( output, "SRC2(%s)", regnames[C]);

	


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
int arcompact_handle05_04_dasm(DASM_OPS_32)  { return arcompact_handle04_helper_dasm(DASM_PARAMS, "MUL64", 0,0); } // special
int arcompact_handle05_05_dasm(DASM_OPS_32)  { return arcompact_handle04_helper_dasm(DASM_PARAMS, "MULU64", 0,0);} // special
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

	int p = (op & 0x00c00000) >> 22; op &= ~0x00c00000;
	int b = (op & 0x07000000) >> 24; op &= ~0x07000000;
	int B = (op & 0x00007000) >> 12; op &= ~0x00007000;
	int breg = b | (B << 3);
	int F = (op & 0x00008000) >> 15;op &= ~0x00008000;

	output  += sprintf( output, "%s", optext);
	output  += sprintf( output, "%s", flagbit[F]);
//	output  += sprintf( output, " p(%d)", p);
	
	
	output += sprintf(output, " %s, ", regnames[breg]);

	if (p == 0)
	{
		int C = (op & 0x00000fc0) >> 6; op &= ~0x00000fc0;

		if (C == LIMM_REG)
		{
			UINT32 limm;
			GET_LIMM_32;
			size = 8;	
			output  += sprintf( output, "(%08x) ", limm );

		}
		else
		{
			output  += sprintf( output, "C(%s) ", regnames[C]);
		}
	}
	else if (p == 1)
	{
		int U = (op & 0x00000fc0) >> 6; op &= ~0x00000fc0;

		output  += sprintf( output, "U(0x%02x) ", U);
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



int arcompact_handle0c_helper_dasm(DASM_OPS_16, const char* optext)
{
	int areg, breg, creg;

	COMMON16_GET_areg;
	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(areg);
	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);


	print("%s %s <- [%s, %s]", optext, regnames[areg], regnames[breg], regnames[creg]);
	return 2;
}


int arcompact_handle0c_00_dasm(DASM_OPS_16)
{
	return arcompact_handle0c_helper_dasm(DASM_PARAMS, "LD_S");
}

int arcompact_handle0c_01_dasm(DASM_OPS_16)
{
	return arcompact_handle0c_helper_dasm(DASM_PARAMS, "LDB_S");
}

int arcompact_handle0c_02_dasm(DASM_OPS_16)
{
	return arcompact_handle0c_helper_dasm(DASM_PARAMS, "LDW_S");
}

int arcompact_handle0c_03_dasm(DASM_OPS_16)
{
	return arcompact_handle0c_helper_dasm(DASM_PARAMS, "ADD_S");
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
		if (!revop) print("%s %s, (%08x) (%04x)", optext, regnames[breg], limm, op);
		else print("%s (%08x), %s (%04x)", optext, limm, regnames[breg], op);
	}
	else
	{
		if (!revop) print("%s %s, %s (%04x)", optext, regnames[breg], regnames[h], op);
		else print("%s %s, %s (%04x)", optext, regnames[h], regnames[breg], op);

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





int arcompact_handle0f_0x_helper_dasm(DASM_OPS_16, const char* optext)
{
	int breg, creg;

	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);


	print("%s %s <- %s", optext, regnames[breg], regnames[creg]);
	return 2;
}

int arcompact_handle0f_02_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "SUB_S");}
int arcompact_handle0f_04_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "AND_S");  }
int arcompact_handle0f_05_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "OR_S");   }
int arcompact_handle0f_06_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "BIC_S");  }
int arcompact_handle0f_07_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "XOR_S");  }
int arcompact_handle0f_0b_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "TST_S");  }
int arcompact_handle0f_0d_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "SEXB_S"); }
int arcompact_handle0f_0e_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "SEXW_S"); }
int arcompact_handle0f_0f_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "EXTB_S"); }
int arcompact_handle0f_10_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "EXTW_S"); }
int arcompact_handle0f_11_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "ABS_S");  }
int arcompact_handle0f_12_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "NOT_S");  }
int arcompact_handle0f_13_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "NEG_S");  }
int arcompact_handle0f_14_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "ADD1_S"); }
int arcompact_handle0f_15_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "ADD2_S"); }
int arcompact_handle0f_16_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "ADD3_S"); }
int arcompact_handle0f_18_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "ASL_S");  }
int arcompact_handle0f_19_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "LSR_S");  }
int arcompact_handle0f_1a_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "ASR_S");  }
int arcompact_handle0f_1b_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "ASL1_S"); }
int arcompact_handle0f_1c_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "ASR1_S"); }
int arcompact_handle0f_1d_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "LSR1_S"); }

int arcompact_handle0f_0c_dasm(DASM_OPS_16)  { print("MUL64_S mulres <- b * c  (%08x)", op); return 2;} // special
int arcompact_handle0f_1e_dasm(DASM_OPS_16)  { print("TRAP_S (%08x)", op); return 2;} // special
int arcompact_handle0f_1f_dasm(DASM_OPS_16)  { print("BRK_S (%08x)", op); return 2;} // special


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

int arcompact_handle18_00_dasm(DASM_OPS_16) 
{
	print("LD_S (SP) (%04x)",  op);
	return 2;
}

int arcompact_handle18_01_dasm(DASM_OPS_16) 
{
	print("LDB_S (SP) (%04x)",  op);
	return 2;
}

int arcompact_handle18_02_dasm(DASM_OPS_16) 
{
	print("ST_S (SP) (%04x)",  op);
	return 2;
}

int arcompact_handle18_03_dasm(DASM_OPS_16) 
{
	print("STB_S (SP) (%04x)",  op);
	return 2;
}

int arcompact_handle18_04_dasm(DASM_OPS_16) 
{
	print("ADD_S (SP) (%04x)",  op);
	return 2;
}

// op bits remaining for 0x18_05_xx subgroups 0x001f
int arcompact_handle18_05_00_dasm(DASM_OPS_16)
{
	int u = op & 0x001f;
	op &= ~0x001f; // all bits now used

	print("ADD_S %02x (SP)", u);
	return 2;

}

int arcompact_handle18_05_01_dasm(DASM_OPS_16)
{
	int u = op & 0x001f;
	op &= ~0x001f; // all bits now used

	print("SUB_S %02x (SP)", u);
	return 2;
}

// op bits remaining for 0x18_06_xx subgroups 0x0700 
int arcompact_handle18_06_01_dasm(DASM_OPS_16) 
{
	int breg = (op & 0x0700) >> 8;
	op &= ~0x0700; // all bits now used
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
	int breg = (op & 0x0700) >> 8;
	op &= ~0x0700; // all bits now used
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

int arcompact_handle19_00_dasm(DASM_OPS_16)  { print("LD_S r0 <- m[GP + s11].long (%04x)",  op); return 2;}
int arcompact_handle19_01_dasm(DASM_OPS_16)  { print("LDB_S r0 <- m[GP + s9].byte (%04x)",  op); return 2;}
int arcompact_handle19_02_dasm(DASM_OPS_16)  { print("LDW_S r0 <- m[GP + s10].word (%04x)",  op); return 2;}
int arcompact_handle19_03_dasm(DASM_OPS_16)  { print("ADD_S r0 <- GP + s11 (%04x)",  op); return 2;}

int arcompact_handle1a_dasm(DASM_OPS_16)
{
	print("PCL Instr (%04x)", op);
	return 2;
}

int arcompact_handle1b_dasm(DASM_OPS_16)
{
	int breg, u;
	COMMON16_GET_breg;
	COMMON16_GET_u8;
	REG_16BIT_RANGE(breg);

	print("MOV_S %s, %02x", regnames[breg], u);
	return 2;
}

int arcompact_handle1c_00_dasm(DASM_OPS_16)
{
	int breg, u;
	COMMON16_GET_breg;
	COMMON16_GET_u7;
	REG_16BIT_RANGE(breg);

	print("ADD_S %s, %02x", regnames[breg], u);
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

	int s = (op & 0x007f) >> 0;	op &= ~0x007f;
	if (s & 0x40) s = -0x40 + (s & 0x3f);

	print("%s %s %08x", optext, regnames[breg], PC_ALIGNED32 + s*2);
	return 2;
}


int arcompact_handle1d_00_dasm(DASM_OPS_16)  { return arcompact_handle1d_helper_dasm(DASM_PARAMS,"BREQ_S"); }
int arcompact_handle1d_01_dasm(DASM_OPS_16)  { return arcompact_handle1d_helper_dasm(DASM_PARAMS,"BRNE_S"); }


int arcompact_handle1e_0x_helper_dasm(DASM_OPS_16, const char* optext)
{
	int s = (op & 0x01ff) >> 0;	op &= ~0x01ff;
	if (s & 0x100) s = -0x100 + (s & 0xff);

	print("%s %08x", optext, PC_ALIGNED32 + s*2);
	return 2;
}



int arcompact_handle1e_00_dasm(DASM_OPS_16)  { return arcompact_handle1e_0x_helper_dasm(DASM_PARAMS, "BL_S");  }
int arcompact_handle1e_01_dasm(DASM_OPS_16)  { return arcompact_handle1e_0x_helper_dasm(DASM_PARAMS, "BEQ_S"); }
int arcompact_handle1e_02_dasm(DASM_OPS_16)  { return arcompact_handle1e_0x_helper_dasm(DASM_PARAMS, "BNE_S"); }

int arcompact_handle1e_03_0x_helper_dasm(DASM_OPS_16, const char* optext)
{
	int s = (op & 0x003f) >> 0;	op &= ~0x003f;
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
	int s = (op & 0x07ff) >> 0;	op &= ~0x07ff;
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

