/***************************************************************************

    dis32031.c
    Disassembler for the portable TMS32C031 emulator.
    Written by Aaron Giles

***************************************************************************/

#include "tms32031.h"


/***************************************************************************
    MEMORY ACCESSORS
***************************************************************************/

#define INTEGER			0
#define FLOAT			1
#define NODEST			2
#define NOSOURCE		4
#define NOSOURCE1		NOSOURCE
#define NOSOURCE2		8
#define SWAPSRCDST		16
#define UNSIGNED		32


/***************************************************************************
    CODE CODE
***************************************************************************/

INLINE char *signed_16bit(INT16 val)
{
	static char temp[10];
	if (val < 0)
		sprintf(temp, "-$%x", -val & 0xffff);
	else
		sprintf(temp, "$%x", val);
	return temp;
}

static const char *regname[32] =
{
	"R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7",
	"AR0", "AR1", "AR2", "AR3", "AR4", "AR5", "AR6", "AR7",
	"DP", "IR0", "IR1", "BK", "SP", "ST", "IE", "IF",
	"IOF", "RS", "RE", "RC", "??", "??", "??", "??"
};

static const char *condition[32] =
{
	"U", "LO", "LS", "HI", "HS", "EQ", "NE", "LT",
	"LE", "GT", "GE", "??", "NV", "V", "NUF", "UF",
	"NLV", "LV", "NLUF", "LUF", "ZUF", "??", "??", "??",
	"??", "??", "??", "??", "??", "??", "??", "??"
};

//
// for instructions 0x000-0x3f
//  G = (op >> 21) & 3;
//  G == 0 -> register  -> 000ooooo o00ddddd 00000000 000sssss
//  G == 1 -> direct    -> 000ooooo o01ddddd DDDDDDDD DDDDDDDD
//  G == 2 -> indirect  -> 000ooooo o10ddddd mmmmmaaa DDDDDDDD
//  G == 3 -> immediate -> 000ooooo o11ddddd iiiiiiii iiiiiiii
//
// for instructions 0x040-0x7f
//  T = (op >> 21) & 3;                       (src1)   (src2)
//  T == 0 -> reg reg   -> 001ooooo o00ddddd 000sssss 000SSSSS
//  T == 1 -> ind reg   -> 001ooooo o01ddddd mmmmmaaa 000SSSSS
//  T == 2 -> reg ind   -> 001ooooo o10ddddd 000sssss MMMMMAAA
//  T == 3 -> ind ind   -> 001ooooo o11ddddd mmmmmaaa MMMMMAAA
//
// for instructions 0x100-0x1ff
//  10ooooPP dDsssSSS mmmmmaaa MMMMMAAA
//  10ooooPP dDsssSSS 111rrrrr 111RRRRR
//
// conditional branches
//  xxxxxxBa aaDccccc 00000000 000sssss
//  xxxxxxBa aaDccccc iiiiiiii iiiiiiii
//      D=0(standard) or 1(delayed)
//      B=0(register) or 1(immediate)
//
// status register
//  31-14 = 0
//  13 = GIE
//  12 = CC
//  11 = CE
//  10 = CF
//  9 = 0
//  8 = RM
//  7 = OVM
//  6 = LUF (latched FP underflow)
//  5 = LV (latched overflow)
//  4 = UF (FP underflow)
//  3 = N
//  2 = Z
//  1 = V
//  0 = C
//
// conditions:
//  0 = U (unconditional)
//  1 = LO (C)
//  2 = LS (C | Z)
//  3 = HI (~C & ~Z)
//  4 = HS (~C)
//  5 = EQ (Z)
//  6 = NE (~Z)
//  7 = LT (N)
//  8 = LE (N | Z)
//  9 = GT (~N & ~Z)
//  10 = GE (~N)
//  12 = NV (~V)
//  13 = V (V)
//  14 = NUF (~UF)
//  15 = UF (UF)
//  16 = NLV (~LV)
//  17 = LV (LV)
//  18 = NLUF (~LUF)
//  19 = LUF (LUF)
//  20 = ZUF (Z | UF)

static void append_indirect(UINT8 ma, INT8 disp, char *buffer)
{
	char *dst = &buffer[strlen(buffer)];
	char dispstr[20];
	int mode = (ma >> 3) & 0x1f;
	int ar = ma & 7;

	dispstr[0] = 0;
	if (disp < 0)
		sprintf(dispstr, "(-%X)", -disp & 0xffff);
	else if (disp > 0)
		sprintf(dispstr, "(%X)", disp);

	switch (mode)
	{
		case 0:		sprintf(dst, "*+AR%d%s", ar, dispstr);	break;
		case 1:		sprintf(dst, "*-AR%d%s", ar, dispstr);	break;
		case 2:		sprintf(dst, "*++AR%d%s", ar, dispstr);	break;
		case 3:		sprintf(dst, "*--AR%d%s", ar, dispstr);	break;
		case 4:		sprintf(dst, "*AR%d++%s", ar, dispstr);	break;
		case 5:		sprintf(dst, "*AR%d--%s", ar, dispstr);	break;
		case 6:		sprintf(dst, "*AR%d++%s%%", ar, dispstr);	break;
		case 7:		sprintf(dst, "*AR%d--%s%%", ar, dispstr);	break;

		case 8:		sprintf(dst, "*+AR%d(IR0)", ar);	break;
		case 9:		sprintf(dst, "*-AR%d(IR0)", ar);	break;
		case 10:	sprintf(dst, "*++AR%d(IR0)", ar);	break;
		case 11:	sprintf(dst, "*--AR%d(IR0)", ar);	break;
		case 12:	sprintf(dst, "*AR%d++(IR0)", ar);	break;
		case 13:	sprintf(dst, "*AR%d--(IR0)", ar);	break;
		case 14:	sprintf(dst, "*AR%d++(IR0)%%", ar);	break;
		case 15:	sprintf(dst, "*AR%d--(IR0)%%", ar);	break;

		case 16:	sprintf(dst, "*+AR%d(IR1)", ar);	break;
		case 17:	sprintf(dst, "*-AR%d(IR1)", ar);	break;
		case 18:	sprintf(dst, "*++AR%d(IR1)", ar);	break;
		case 19:	sprintf(dst, "*--AR%d(IR1)", ar);	break;
		case 20:	sprintf(dst, "*AR%d++(IR1)", ar);	break;
		case 21:	sprintf(dst, "*AR%d--(IR1)", ar);	break;
		case 22:	sprintf(dst, "*AR%d++(IR1)%%", ar);	break;
		case 23:	sprintf(dst, "*AR%d--(IR1)%%", ar);	break;

		case 24:	sprintf(dst, "*AR%d", ar);			break;
		case 25:	sprintf(dst, "*AR%d++(IR0)B", ar);	break;
		case 28:
		case 29:
		case 30:
		case 31:	strcpy(dst, regname[ma & 31]);		break;
		default:	sprintf(dst, "(unknown mode)");		break;
	}
}

static void append_immediate(UINT16 data, int is_float, int is_unsigned, char *buffer)
{
	char *dst = &buffer[strlen(buffer)];

	if (is_float)
	{
		int exp = ((INT16)data >> 12) + 127;
		UINT32 expanded_data;
		float float_val;

		expanded_data = ((data & 0x0800) << 20) + ((exp << 23) & 0x7f800000);
		if (data == 0x8000)
			*(float *)&expanded_data = 0;
		else if (!(data & 0x0800))
			expanded_data += ((data & 0x0fff) << 12);
		else
			expanded_data += ((-data & 0x0fff) << 12);
		float_val = *(float *)&expanded_data;
		sprintf(dst, "%8f", float_val);
	}
	else if (!is_unsigned && (INT16)data < 0)
		sprintf(dst, "-$%04X", -data & 0xffff);
	else
		sprintf(dst, "$%04X", data);
}

static void disasm_general(const char *opstring, UINT32 op, int flags, char *buffer)
{
	sprintf(buffer, "%-6s", opstring);

	if (flags & SWAPSRCDST)
	{
		strcat(buffer, regname[(op >> 16) & 31]);
		strcat(buffer, ",");
	}

	/* switch off of G */
	if (!(flags & NOSOURCE))
	{
		switch ((op >> 21) & 3)
		{
			case 0:
				strcat(buffer, regname[op & 31]);
				break;

			case 1:
				sprintf(&buffer[strlen(buffer)], "($%04X)", op & 0xffff);
				break;

			case 2:
				append_indirect((op >> 8) & 0xff, op, buffer);
				break;

			case 3:
				append_immediate(op & 0xffff, (flags & FLOAT), (flags & UNSIGNED), buffer);
				break;
		}
	}

	/* add destination op */
	if (!(flags & NODEST) && !(flags & SWAPSRCDST))
	{
		if (!(flags & NOSOURCE))
			strcat(buffer, ",");
		strcat(buffer, regname[(op >> 16) & 31]);
	}
}

static void disasm_3op(const char *opstring, UINT32 op, int flags, char *buffer)
{
	sprintf(buffer, "%-6s", opstring);

	/* switch off of T */
	if (!(flags & NOSOURCE1))
	{
		switch ((op >> 21) & 1)
		{
			case 0:
				strcat(buffer, regname[(op >> 8) & 31]);
				break;

			case 1:
				append_indirect(op >> 8, 1, buffer);
				break;
		}
	}

	/* switch off of T */
	if (!(flags & NOSOURCE2))
	{
		if (!(flags & NOSOURCE1))
			strcat(buffer, ",");
		switch ((op >> 22) & 1)
		{
			case 0:
				strcat(buffer, regname[op & 31]);
				break;

			case 1:
				append_indirect(op, 1, buffer);
				break;
		}
	}

	/* add destination op */
	if (!(flags & NODEST))
	{
		if (!(flags & (NOSOURCE1 | NOSOURCE2)))
			strcat(buffer, ",");
		strcat(buffer, regname[(op >> 16) & 31]);
	}
}

static void disasm_conditional(const char *opstring, UINT32 op, int flags, char *buffer)
{
	char temp[10];
	sprintf(temp, "%s%s", opstring, condition[(op >> 23) & 31]);
	disasm_general(temp, op, flags, buffer);
}


static void disasm_parallel_3op3op(const char *opstring1, const char *opstring2, UINT32 op, int flags, const UINT8 *srctable, char *buffer)
{
	const UINT8 *s = &srctable[((op >> 24) & 3) * 4];
	int d1 = (op >> 23) & 1;
	int d2 = 2 + ((op >> 22) & 1);
	char src[5][20];

	strcpy(src[1], regname[(op >> 19) & 7]);
	strcpy(src[2], regname[(op >> 16) & 7]);

	src[3][0] = 0;
	append_indirect(op >> 8, 1, src[3]);

	src[4][0] = 0;
	append_indirect(op, 1, src[4]);

	sprintf(buffer, "%s %s,%s,R%d || %s %s,%s,R%d",
			opstring1, src[s[0]], src[s[1]], d1,
			opstring2, src[s[2]], src[s[3]], d2);
}


static void disasm_parallel_3opstore(const char *opstring1, const char *opstring2, UINT32 op, int flags, char *buffer)
{
	int d1 = (op >> 22) & 7;
	int s1 = (op >> 19) & 7;
	int s3 = (op >> 16) & 7;
	char dst2[20], src2[20];

	dst2[0] = 0;
	append_indirect(op >> 8, 1, dst2);

	src2[0] = 0;
	append_indirect(op, 1, src2);

	if (!(flags & NOSOURCE1))
		sprintf(buffer, "%s R%d,%s,R%d || %s R%d,%s",
				opstring1, s1, src2, d1,
				opstring2, s3, dst2);
	else
		sprintf(buffer, "%s %s,R%d || %s R%d,%s",
				opstring1, src2, d1,
				opstring2, s3, dst2);
}


static void disasm_parallel_loadload(const char *opstring1, const char *opstring2, UINT32 op, int flags, char *buffer)
{
	int d2 = (op >> 22) & 7;
	int d1 = (op >> 19) & 7;
	char src1[20], src2[20];

	src1[0] = 0;
	append_indirect(op >> 8, 1, src1);

	src2[0] = 0;
	append_indirect(op, 1, src2);

	sprintf(buffer, "%s %s,R%d || %s %s,R%d",
			opstring1, src2, d2,
			opstring2, src1, d1);
}


static void disasm_parallel_storestore(const char *opstring1, const char *opstring2, UINT32 op, int flags, char *buffer)
{
	int s2 = (op >> 22) & 7;
	int s1 = (op >> 16) & 7;
	char dst1[20], dst2[20];

	dst1[0] = 0;
	append_indirect(op >> 8, 1, dst1);

	dst2[0] = 0;
	append_indirect(op, 1, dst2);

	sprintf(buffer, "%s R%d,%s || %s R%d,%s",
			opstring1, s2, dst2,
			opstring2, s1, dst1);
}



unsigned dasm_tms32031(char *buffer, unsigned pc, UINT32 op)
{
	UINT32 flags = 0;

	switch (op >> 23)
	{
		case 0x000:	disasm_general("ABSF", op, FLOAT, buffer);			break;
		case 0x001:	disasm_general("ABSI", op, INTEGER, buffer);		break;
		case 0x002:	disasm_general("ADDC", op, INTEGER, buffer);		break;
		case 0x003:	disasm_general("ADDF", op, FLOAT, buffer);			break;
		case 0x004:	disasm_general("ADDI", op, INTEGER, buffer);		break;
		case 0x005:	disasm_general("AND", op, INTEGER | UNSIGNED, buffer);	break;
		case 0x006:	disasm_general("ANDN", op, INTEGER | UNSIGNED, buffer);	break;
		case 0x007:	disasm_general("ASH", op, INTEGER, buffer);			break;

		case 0x008:	disasm_general("CMPF", op, FLOAT, buffer);			break;
		case 0x009:	disasm_general("CMPI", op, INTEGER, buffer);		break;
		case 0x00a:	disasm_general("FIX", op, FLOAT, buffer);			break;
		case 0x00b:	disasm_general("FLOAT", op, INTEGER, buffer);		break;
		case 0x00c:	disasm_general((op & 1) ? "IDLE2" : "IDLE", op, NOSOURCE | NODEST, buffer); break;
		case 0x00d:	disasm_general("LDE", op, FLOAT, buffer);			break;
		case 0x00e:	disasm_general("LDF", op, FLOAT, buffer);			break;
		case 0x00f:	disasm_general("LDFI", op, FLOAT, buffer);			break;

		case 0x010:	disasm_general("LDI", op, INTEGER, buffer);			break;
		case 0x011:	disasm_general("LDII", op, INTEGER, buffer);		break;
		case 0x012:	disasm_general("LDM", op, FLOAT, buffer);			break;
		case 0x013:	disasm_general("LSH", op, INTEGER, buffer);			break;
		case 0x014:	disasm_general("MPYF", op, FLOAT, buffer);			break;
		case 0x015:	disasm_general("MPYI", op, INTEGER, buffer);		break;
		case 0x016:	disasm_general("NEGB", op, INTEGER, buffer);		break;
		case 0x017:	disasm_general("NEGF", op, FLOAT, buffer);			break;

		case 0x018:	disasm_general("NEGI", op, INTEGER, buffer);		break;
		case 0x019:	disasm_general("NOP", op, NODEST, buffer); 			break;
		case 0x01a:	disasm_general("NORM", op, FLOAT, buffer);			break;
		case 0x01b:	disasm_general("NOT", op, INTEGER, buffer);			break;
		case 0x01c:	disasm_general("POP", op, NOSOURCE, buffer);	 	break;
		case 0x01d:	disasm_general("POPF", op, NOSOURCE, buffer); 		break;
		case 0x01e:	disasm_general("PUSH", op, NOSOURCE, buffer); 		break;
		case 0x01f:	disasm_general("PUSHF", op, NOSOURCE, buffer); 		break;

		case 0x020:	disasm_general("OR", op, INTEGER | UNSIGNED, buffer);	break;
		case 0x021:	disasm_general((op & 1) ? "LOPOWER" : "MAXSPEED", op, NOSOURCE | NODEST, buffer); break;
		case 0x022:	disasm_general("RND", op, FLOAT, buffer);			break;
		case 0x023:	disasm_general("ROL", op, INTEGER, buffer);			break;
		case 0x024:	disasm_general("ROLC", op, INTEGER, buffer);		break;
		case 0x025:	disasm_general("ROR", op, INTEGER, buffer);			break;
		case 0x026:	disasm_general("RORC", op, INTEGER, buffer);		break;
		case 0x027:	disasm_general("RTPS", op, INTEGER | NODEST, buffer); break;

		case 0x028:	disasm_general("STF", op, FLOAT | SWAPSRCDST, buffer);	break;
		case 0x029:	disasm_general("STFI", op, FLOAT | SWAPSRCDST, buffer);	break;
		case 0x02a:	disasm_general("STI", op, INTEGER | SWAPSRCDST, buffer); break;
		case 0x02b:	disasm_general("STII", op, INTEGER | SWAPSRCDST, buffer); break;
		case 0x02c:	disasm_general("SIGI", op, NOSOURCE | NODEST, buffer); break;
		case 0x02d:	disasm_general("SUBB", op, INTEGER, buffer);		break;
		case 0x02e:	disasm_general("SUBC", op, INTEGER, buffer);		break;
		case 0x02f:	disasm_general("SUBF", op, FLOAT, buffer);			break;

		case 0x030:	disasm_general("SUBI", op, INTEGER, buffer);		break;
		case 0x031:	disasm_general("SUBRB", op, INTEGER, buffer);		break;
		case 0x032:	disasm_general("SUBRF", op, FLOAT, buffer);			break;
		case 0x033:	disasm_general("SUBRI", op, INTEGER, buffer);		break;
		case 0x034:	disasm_general("TSTB", op, INTEGER, buffer);		break;
		case 0x035:	disasm_general("XOR", op, INTEGER | UNSIGNED, buffer);	break;
		case 0x036:	disasm_general("IACK", op, INTEGER | NODEST, buffer); break;

		case 0x040:	disasm_3op("ADDC3", op, INTEGER, buffer);			break;
		case 0x041:	disasm_3op("ADDF3", op, FLOAT, buffer);				break;
		case 0x042:	disasm_3op("ADDI3", op, INTEGER, buffer);			break;
		case 0x043:	disasm_3op("AND3", op, INTEGER, buffer);			break;
		case 0x044:	disasm_3op("ANDN3", op, INTEGER, buffer);			break;
		case 0x045:	disasm_3op("ASH3", op, INTEGER, buffer);			break;
		case 0x046:	disasm_3op("CMPF3", op, FLOAT | NODEST, buffer);	break;
		case 0x047:	disasm_3op("CMPI3", op, INTEGER | NODEST, buffer);	break;

		case 0x048:	disasm_3op("LSH3", op, INTEGER, buffer);			break;
		case 0x049:	disasm_3op("MPYF3", op, FLOAT, buffer);				break;
		case 0x04a:	disasm_3op("MPYI3", op, INTEGER, buffer);			break;
		case 0x04b:	disasm_3op("OR3", op, INTEGER, buffer);				break;
		case 0x04c:	disasm_3op("SUBB3", op, INTEGER, buffer);			break;
		case 0x04d:	disasm_3op("SUBF3", op, FLOAT, buffer);				break;
		case 0x04e:	disasm_3op("SUBI3", op, INTEGER, buffer);			break;
		case 0x04f:	disasm_3op("TSTB3", op, INTEGER, buffer);			break;

		case 0x050:	disasm_3op("XOR3", op, INTEGER, buffer);			break;

		case 0x080: case 0x081: case 0x082: case 0x083:
		case 0x084: case 0x085: case 0x086: case 0x087:
		case 0x088: case 0x089: case 0x08a: case 0x08b:
		case 0x08c: case 0x08d: case 0x08e: case 0x08f:
		case 0x090: case 0x091: case 0x092: case 0x093:
		case 0x094: case 0x095: case 0x096: case 0x097:
		case 0x098: case 0x099: case 0x09a: case 0x09b:
		case 0x09c: case 0x09d: case 0x09e: case 0x09f:
			disasm_conditional("LDF", op, FLOAT, buffer);
			break;

		case 0x0a0: case 0x0a1: case 0x0a2: case 0x0a3:
		case 0x0a4: case 0x0a5: case 0x0a6: case 0x0a7:
		case 0x0a8: case 0x0a9: case 0x0aa: case 0x0ab:
		case 0x0ac: case 0x0ad: case 0x0ae: case 0x0af:
		case 0x0b0: case 0x0b1: case 0x0b2: case 0x0b3:
		case 0x0b4: case 0x0b5: case 0x0b6: case 0x0b7:
		case 0x0b8: case 0x0b9: case 0x0ba: case 0x0bb:
		case 0x0bc: case 0x0bd: case 0x0be: case 0x0bf:
			disasm_conditional("LDI", op, INTEGER, buffer);
			break;


		case 0x0c0: case 0x0c1:
			sprintf(buffer, "BR    $%06X", op & 0xffffff);
			break;

		case 0x0c2: case 0x0c3:
			sprintf(buffer, "BRD   $%06X", op & 0xffffff);
			break;

		case 0x0c4: case 0x0c5:
			sprintf(buffer, "CALL  $%06X", op & 0xffffff);
			flags = DASMFLAG_STEP_OVER;
			break;


		case 0x0c8: case 0x0c9:
			sprintf(buffer, "RPTB  $%06X", op & 0xffffff);
			break;

		case 0x0cc: case 0x0cd: case 0x0ce: case 0x0cf:
			sprintf(buffer, "SWI");
			break;


		case 0x0d0:
		{
			char temp[10];
			sprintf(temp, "B%s%s", condition[(op >> 16) & 31], ((op >> 21) & 1) ? "D" : "");
			sprintf(buffer, "%-6s%s", temp, regname[op & 31]);
			break;
		}

		case 0x0d4:
		{
			char temp[10];
			sprintf(temp, "B%s%s", condition[(op >> 16) & 31], ((op >> 21) & 1) ? "D" : "");
			sprintf(buffer, "%-6s$%06X", temp, (pc + (((op >> 21) & 1) ? 3 : 1) + (INT16)op) & 0xffffff);
			break;
		}


		case 0x0d8: case 0x0d9: case 0x0da: case 0x0db:
		{
			char temp[10];
			sprintf(temp, "DB%s%s", condition[(op >> 16) & 31], ((op >> 21) & 1) ? "D" : "");
			sprintf(buffer, "%-6sAR%d,%s", temp, (op >> 22) & 7, regname[op & 31]);
			break;
		}

		case 0x0dc: case 0x0dd: case 0x0de: case 0x0df:
		{
			char temp[10];
			sprintf(temp, "DB%s%s", condition[(op >> 16) & 31], ((op >> 21) & 1) ? "D" : "");
			sprintf(buffer, "%-6sAR%d,$%06X", temp, (op >> 22) & 7, (pc + (((op >> 21) & 1) ? 3 : 1) + (INT16)op) & 0xffffff);
			break;
		}


		case 0x0e0:
		{
			char temp[10];
			sprintf(temp, "CALL%s", condition[(op >> 16) & 31]);
			sprintf(buffer, "%-6s%s", temp, regname[op & 31]);
			flags = DASMFLAG_STEP_OVER;
			break;
		}

		case 0x0e4:
		{
			char temp[10];
			sprintf(temp, "CALL%s", condition[(op >> 16) & 31]);
			sprintf(buffer, "%-6s$%06X", temp, (pc + 1 + (INT16)op) & 0xffffff);
			flags = DASMFLAG_STEP_OVER;
			break;
		}


		case 0x0e8: case 0x0e9: case 0x0ea: case 0x0eb:
		{
			char temp[10];
			sprintf(temp, "TRAP%s", condition[(op >> 16) & 31]);
			sprintf(buffer, "%-6s$%02X", temp, op & 31);
			flags = DASMFLAG_STEP_OVER;
			break;
		}


		case 0x0f0:
			sprintf(buffer, "RETI%s", condition[(op >> 16) & 31]);
			flags = DASMFLAG_STEP_OUT;
			break;

		case 0x0f1:
			sprintf(buffer, "RETS%s", condition[(op >> 16) & 31]);
			flags = DASMFLAG_STEP_OUT;
			break;


		case 0x100: case 0x101:	case 0x102: case 0x103:
		case 0x104: case 0x105:	case 0x106: case 0x107:	// MPYF3||ADDF3
		{
			static const UINT8 srctable[] = { 3,4,1,2, 3,1,4,2, 1,2,3,4, 3,1,2,4 };
			disasm_parallel_3op3op("MPYF3", "ADDF3", op, FLOAT, srctable, buffer);
			break;
		}


		case 0x108: case 0x109:	case 0x10a: case 0x10b:
		case 0x10c: case 0x10d:	case 0x10e: case 0x10f:	// MPYF3||SUBF3
		{
			static const UINT8 srctable[] = { 3,4,1,2, 3,1,4,2, 1,2,3,4, 3,1,2,4 };
			disasm_parallel_3op3op("MPYF3", "SUBF3", op, FLOAT, srctable, buffer);
			break;
		}


		case 0x110: case 0x111:	case 0x112: case 0x113:
		case 0x114: case 0x115:	case 0x116: case 0x117:	// MPYI3||ADDI3
		{
			static const UINT8 srctable[] = { 3,4,1,2, 3,1,4,2, 1,2,3,4, 3,1,2,4 };
			disasm_parallel_3op3op("MPYI3", "ADDI3", op, INTEGER, srctable, buffer);
			break;
		}


		case 0x118: case 0x119:	case 0x11a: case 0x11b:
		case 0x11c: case 0x11d:	case 0x11e: case 0x11f:	// MPYI3||SUBI3
		{
			static const UINT8 srctable[] = { 3,4,1,2, 3,1,4,2, 1,2,3,4, 3,1,2,4 };
			disasm_parallel_3op3op("MPYI3", "SUBI3", op, INTEGER, srctable, buffer);
			break;
		}


		case 0x180: case 0x181:	case 0x182: case 0x183:	// STF||STF
			disasm_parallel_storestore("STF", "STF", op, FLOAT, buffer);
			break;

		case 0x184: case 0x185:	case 0x186: case 0x187:	// STI||STI
			disasm_parallel_storestore("STI", "STI", op, INTEGER, buffer);
			break;


		case 0x188: case 0x189:	case 0x18a: case 0x18b:	// LDF||LDF
			disasm_parallel_loadload("LDF", "LDF", op, FLOAT, buffer);
			break;

		case 0x18c: case 0x18d:	case 0x18e: case 0x18f:	// LDI||LDI
			disasm_parallel_loadload("LDI", "LDI", op, INTEGER, buffer);
			break;


		case 0x190: case 0x191:	case 0x192: case 0x193:	// ABSF||STF
			disasm_parallel_3opstore("ABSF", "STF", op, FLOAT | NOSOURCE1, buffer);
			break;

		case 0x194: case 0x195:	case 0x196: case 0x197:	// ABSI||STI
			disasm_parallel_3opstore("ABSI", "STI", op, INTEGER | NOSOURCE1, buffer);
			break;


		case 0x198: case 0x199:	case 0x19a: case 0x19b:	// ADDF3||STF
			disasm_parallel_3opstore("ADDF3", "STF", op, FLOAT, buffer);
			break;

		case 0x19c: case 0x19d:	case 0x19e: case 0x19f:	// ADDI3||STI
			disasm_parallel_3opstore("ADDI3", "STI", op, INTEGER, buffer);
			break;


		case 0x1a0: case 0x1a1:	case 0x1a2: case 0x1a3:	// AND3||STI
			disasm_parallel_3opstore("AND3", "STI", op, INTEGER, buffer);
			break;

		case 0x1a4: case 0x1a5:	case 0x1a6: case 0x1a7:	// ASH3||STI
			disasm_parallel_3opstore("ASH3", "STI", op, INTEGER, buffer);
			break;


		case 0x1a8: case 0x1a9:	case 0x1aa: case 0x1ab:	// FIX||STI
			disasm_parallel_3opstore("FIX", "STF", op, FLOAT | NOSOURCE1, buffer);
			break;

		case 0x1ac: case 0x1ad:	case 0x1ae: case 0x1af:	// FLOAT||STF
			disasm_parallel_3opstore("FLOAT", "STF", op, FLOAT | NOSOURCE1, buffer);
			break;


		case 0x1b0: case 0x1b1:	case 0x1b2: case 0x1b3:	// LDF||STF
			disasm_parallel_3opstore("LDF", "STF", op, FLOAT | NOSOURCE1, buffer);
			break;

		case 0x1b4: case 0x1b5:	case 0x1b6: case 0x1b7:	// LDI||STI
			disasm_parallel_3opstore("LDI", "STI", op, INTEGER | NOSOURCE1, buffer);
			break;


		case 0x1b8: case 0x1b9:	case 0x1ba: case 0x1bb:	// LSH3||STI
			disasm_parallel_3opstore("LSH3", "STI", op, INTEGER, buffer);
			break;

		case 0x1bc: case 0x1bd:	case 0x1be: case 0x1bf:	// MPYF3||STF
			disasm_parallel_3opstore("MPYF3", "STF", op, FLOAT, buffer);
			break;


		case 0x1c0: case 0x1c1:	case 0x1c2: case 0x1c3:	// MPYI3||STI
			disasm_parallel_3opstore("MPYI3", "STI", op, INTEGER, buffer);
			break;

		case 0x1c4: case 0x1c5:	case 0x1c6: case 0x1c7:	// NEGF||STF
			disasm_parallel_3opstore("NEGF", "STF", op, FLOAT | NOSOURCE1, buffer);
			break;


		case 0x1c8: case 0x1c9:	case 0x1ca: case 0x1cb:	// NEGI||STI
			disasm_parallel_3opstore("NEGI", "STI", op, INTEGER | NOSOURCE1, buffer);
			break;

		case 0x1cc: case 0x1cd:	case 0x1ce: case 0x1cf:	// NOT||STI
			disasm_parallel_3opstore("NOT", "STI", op, INTEGER | NOSOURCE1, buffer);
			break;


		case 0x1d0: case 0x1d1:	case 0x1d2: case 0x1d3:	// OR3||STI
			disasm_parallel_3opstore("OR3", "STI", op, INTEGER, buffer);
			break;

		case 0x1d4: case 0x1d5:	case 0x1d6: case 0x1d7:	// SUBF3||STF
			disasm_parallel_3opstore("SUBF3", "STF", op, FLOAT, buffer);
			break;


		case 0x1d8: case 0x1d9:	case 0x1da: case 0x1db:	// SUBI3||STI
			disasm_parallel_3opstore("SUBI3", "STI", op, INTEGER, buffer);
			break;

		case 0x1dc: case 0x1dd:	case 0x1de: case 0x1df:	// XOR3||STI
			disasm_parallel_3opstore("XOR3", "STI", op, INTEGER, buffer);
			break;


		default:
			break;
	}

	return 1 | flags | DASMFLAG_SUPPORTED;
}
