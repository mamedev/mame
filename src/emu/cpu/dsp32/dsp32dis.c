/***************************************************************************

    dsp32dis.c
    Disassembler for the portable AT&T/Lucent DSP32C emulator.
    Written by Aaron Giles

***************************************************************************/

#include "dsp32.h"


/***************************************************************************
    MEMORY ACCESSORS
***************************************************************************/

#define ROPCODE(pc)		cpu_readop32(pc)

#define ABS(x) (((x) >= 0) ? (x) : -(x))


/***************************************************************************
    CODE CODE
***************************************************************************/

static const char *sizesuffix[] = { "", "e" };
static const char *unarysign[] = { "", "-" };
static const char *sign[] = { "+", "-" };
static const char *aMvals[] = { "a0", "a1", "a2", "a3", "0.0", "1.0", "Format 4", "Reserved" };
static const char *memsuffix[] = { "h", "l", "", "e" };
static const char *functable[] =
{
	"ic", "oc", "float", "int", "round", "ifalt", "ifaeq", "ifagt",
	"reserved8", "reserved9", "float24", "int24", "ieee", "dsp", "seed", "reservedf"
};
static const char *condtable[] =
{
	"false", "true",
	"pl", "mi",
	"ne", "eq",
	"vc", "vs",
	"cc", "cs",
	"ge", "lt",
	"gt", "le",
	"hi", "ls",
	"auc", "aus",
	"age", "alt",
	"ane", "aeq",
	"avc", "avs",
	"agt", "ale",
	"!resd", "resd",
	"!rese", "rese",
	"!resf", "resf",
	"ibe", "ibf",
	"obf", "obe",
	"pde", "pdf",
	"pie", "pif",
	"syc", "sys",
	"fbc", "fbs",
	"ireq1_lo", "ireq1_hi",
	"ireq2_lo", "ireq2_hi",
	"!?18", "?18",
	"!?19", "?19",
	"!?1a", "?1a",
	"!?1b", "?1b",
	"!?1c", "?1c",
	"!?1d", "?1d",
	"!?1e", "?1e",
	"!?1f", "?1f"
};
static const char *regname[] =
{
	"0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
	"r8", "r9", "r10", "r11", "r12", "r13", "r14", "pc",
	"0", "r15", "r16", "r17", "r18", "r19", "-1", "1",
	"r20", "r21", "dauc", "ioc", "res1c", "r22", "pcsh", "res1f"
};
static const char *regnamee[] =
{
	"0", "r1e", "r2e", "r3e", "r4e", "r5e", "r6e", "r7e",
	"r8e", "r9e", "r10e", "r11e", "r12e", "r13e", "r14e", "pce",
	"0", "r15e", "r16e", "r17e", "r18e", "r19e", "--", "++",
	"r20e", "r21e", "dauce", "ioce", "res1ce", "r22e", "pcshe", "res1fe"
};

static char tempbuf[10][40];

INLINE char *signed_16bit_unary(INT16 val)
{
	static char temp[10];
	if (val < 0)
		sprintf(temp, "-$%x", -val);
	else
		sprintf(temp, "$%x", val);
	return temp;
}

INLINE char *signed_16bit_sep(INT16 val)
{
	static char temp[10];
	if (val < 0)
		sprintf(temp, " - $%x", -val);
	else
		sprintf(temp, " + $%x", val);
	return temp;
}

INLINE char *signed_16bit_sep_nospace(INT16 val)
{
	static char temp[10];
	if (val < 0)
		sprintf(temp, "-$%x", -val);
	else
		sprintf(temp, "+$%x", val);
	return temp;
}

INLINE char *unsigned_16bit_size(INT16 val, UINT8 size)
{
	static char temp[10];
	if (size)
		sprintf(temp, "$%06x", (INT32)val & 0xffffff);
	else
		sprintf(temp, "$%04x", val & 0xffff);
	return temp;
}

static UINT8 lastp;
static const char *dasm_XYZ(UINT8 bits, char *buffer)
{
	UINT8 p = bits >> 3;
	UINT8 i = bits & 7;

	if (p)
	{
		if (p == 15) p = lastp;		/* P=15 means Z inherits from Y, Y inherits from X */
		lastp = p;
		switch (i)
		{
			case 0:		sprintf(buffer, "*r%d", p);	break;
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:		sprintf(buffer, "*r%d++r%d", p, i + 14); break;
			case 6:		sprintf(buffer, "*r%d--", p); break;
			case 7:		sprintf(buffer, "*r%d++", p); break;
		}
	}
	else
	{
		switch (i)
		{
			case 0:
			case 1:
			case 2:
			case 3:		sprintf(buffer, "a%d", i); break;
			case 4:		sprintf(buffer, "ibuf"); break;
			case 5:		sprintf(buffer, "obuf"); break;
			case 6:		sprintf(buffer, "pdr"); break;
			case 7:		buffer[0] = 0; break;
		}
	}
	return buffer;
}


static const char *dasm_PI(UINT16 bits, char *buffer)
{
	UINT8 p = bits >> 5;
	UINT8 i = bits & 0x1f;

	if (p)
	{
		switch (i)
		{
			case 0:
			case 16:	sprintf(buffer, "*%s", regname[p]);	break;
			case 22:	sprintf(buffer, "*%s--", regname[p]); break;
			case 23:	sprintf(buffer, "*%s++", regname[p]); break;
			default:	sprintf(buffer, "*%s++%s", regname[p], regname[i]); break;
		}
	}
	else
	{
		switch (i)
		{
			case 4:		sprintf(buffer, "ibuf"); break;
			case 5:		sprintf(buffer, "obuf"); break;
			case 6:		sprintf(buffer, "pdr"); break;
			case 14:	sprintf(buffer, "piop"); break;
			case 20:	sprintf(buffer, "pdr2"); break;
			case 22:	sprintf(buffer, "pir"); break;
			case 30:	sprintf(buffer, "pcw"); break;
			default:	sprintf(buffer, "????"); break;
		}
	}
	return buffer;
}


unsigned dasm_dsp32(char *buffer, unsigned pc, UINT32 op)
{
	UINT32 flags = 0;

	switch (op >> 25)
	{
		/* DA format 1 */
		case 0x10:	case 0x11:	case 0x12:	case 0x13:
		case 0x14:	case 0x15:	case 0x16:	case 0x17:
		case 0x18:	case 0x19:	case 0x1a:	case 0x1b:
		{
			const char *X = dasm_XYZ((op >> 14) & 0x7f, tempbuf[0]);
			const char *Y = dasm_XYZ((op >> 7) & 0x7f, tempbuf[1]);
			const char *Z = dasm_XYZ((op >> 0) & 0x7f, tempbuf[2]);
			const char *aM = aMvals[(op >> 26) & 7];
			UINT8 aN = (op >> 21) & 3;
			if ((op & 0x7f) == 7)
			{
				if (aM[0] == '0')
					sprintf(buffer, "a%d = %s%s", aN, unarysign[(op >> 24) & 1], Y);
				else if (aM[0] == '1')
					sprintf(buffer, "a%d = %s%s %s %s", aN, unarysign[(op >> 24) & 1], Y, sign[(op >> 23) & 1], X);
				else
					sprintf(buffer, "a%d = %s%s %s %s * %s", aN, unarysign[(op >> 24) & 1], Y, sign[(op >> 23) & 1], aM, X);
			}
			else
			{
				if (aM[0] == '0')
					sprintf(buffer, "%s = a%d = %s%s", Z, aN, unarysign[(op >> 24) & 1], Y);
				else if (aM[0] == '1')
					sprintf(buffer, "%s = a%d = %s%s %s %s", Z, aN, unarysign[(op >> 24) & 1], Y, sign[(op >> 23) & 1], X);
				else
					sprintf(buffer, "%s = a%d = %s%s %s %s * %s", Z, aN, unarysign[(op >> 24) & 1], Y, sign[(op >> 23) & 1], aM, X);
			}
			break;
		}

		/* DA format 2 */
		case 0x20:	case 0x21:	case 0x22:	case 0x23:
		case 0x24:	case 0x25:	case 0x26:	case 0x27:
		case 0x28:	case 0x29:	case 0x2a:	case 0x2b:
		{
			const char *X = dasm_XYZ((op >> 14) & 0x7f, tempbuf[0]);
			const char *Y = dasm_XYZ((op >> 7) & 0x7f, tempbuf[1]);
			const char *Z = dasm_XYZ((op >> 0) & 0x7f, tempbuf[2]);
			const char *aM = aMvals[(op >> 26) & 7];
			UINT8 aN = (op >> 21) & 3;

			if ((op & 0x7f) == 7)
			{
				if (aM[0] == '0')
					sprintf(buffer, "a%d = %s%s * %s", aN, unarysign[(op >> 23) & 1], Y, X);
				else
					sprintf(buffer, "a%d = %s%s %s %s * %s", aN, unarysign[(op >> 24) & 1], aM, sign[(op >> 23) & 1], Y, X);
			}
			else
			{
				if (aM[0] == '0')
					sprintf(buffer, "a%d = %s(%s=%s) * %s", aN, unarysign[(op >> 23) & 1], Z, Y, X);
				else
					sprintf(buffer, "a%d = %s%s %s (%s=%s) * %s", aN, unarysign[(op >> 24) & 1], aM, sign[(op >> 23) & 1], Z, Y, X);
			}
			break;
		}

		/* DA format 3 */
		case 0x30:	case 0x31:	case 0x32:	case 0x33:
		case 0x34:	case 0x35:	case 0x36:	case 0x37:
		case 0x38:	case 0x39:	case 0x3a:	case 0x3b:
		{
			const char *X = dasm_XYZ((op >> 14) & 0x7f, tempbuf[0]);
			const char *Y = dasm_XYZ((op >> 7) & 0x7f, tempbuf[1]);
			const char *Z = dasm_XYZ((op >> 0) & 0x7f, tempbuf[2]);
			const char *aM = aMvals[(op >> 26) & 7];
			UINT8 aN = (op >> 21) & 3;

			if ((op & 0x7f) == 7)
			{
				if (aM[0] == '0')
					sprintf(buffer, "a%d = %s%s * %s", aN, unarysign[(op >> 23) & 1], Y, X);
				else
					sprintf(buffer, "a%d = %s%s %s %s * %s", aN, unarysign[(op >> 24) & 1], aM, sign[(op >> 23) & 1], Y, X);
			}
			else
			{
				if (aM[0] == '0')
					sprintf(buffer, "%s = a%d = %s%s * %s", Z, aN, unarysign[(op >> 23) & 1], Y, X);
				else
					sprintf(buffer, "%s = a%d = %s%s %s %s * %s", Z, aN, unarysign[(op >> 24) & 1], aM, sign[(op >> 23) & 1], Y, X);
			}
			break;
		}

		/* DA format 4 */
		case 0x1c:	case 0x1d:
		{
			const char *X = dasm_XYZ((op >> 14) & 0x7f, tempbuf[0]);
			const char *Y = dasm_XYZ((op >> 7) & 0x7f, tempbuf[1]);
			const char *Z = dasm_XYZ((op >> 0) & 0x7f, tempbuf[2]);
			UINT8 aN = (op >> 21) & 3;

			if ((op & 0x7f) == 7)
				sprintf(buffer, "a%d = %s%s %s %s", aN, unarysign[(op >> 24) & 1], Y, sign[(op >> 23) & 1], X);
			else
				sprintf(buffer, "a%d = %s(%s=%s) %s %s", aN, unarysign[(op >> 24) & 1], Z, Y, sign[(op >> 23) & 1], X);
			break;
		}

		/* DA format 5 */
		case 0x3c:	case 0x3d:	case 0x3e:	case 0x3f:
			if ((op & 0x7f) == 7)
				sprintf(buffer, "a%d = %s(%s)",
						(op >> 21) & 3, 							// aN
						functable[(op >> 23) & 15],					// G
						dasm_XYZ((op >> 7) & 0x7f, tempbuf[0]));	// Y
			else
				sprintf(buffer, "%s = a%d = %s(%s)",
						dasm_XYZ((op >> 0) & 0x7f, tempbuf[2]),		// Z
						(op >> 21) & 3, 							// aN
						functable[(op >> 23) & 15],					// G
						dasm_XYZ((op >> 7) & 0x7f, tempbuf[0]));	// Y
			break;

		/* CA formats 0/1 */
		case 0x00:	case 0x01:	case 0x02:	case 0x03:
		{
			const char *rH = regname[(op >> 16) & 0x1f];
			UINT8 C = (op >> 21) & 0x3f;
			INT16 N = (INT16)op;

			if (op == 0)
				sprintf(buffer, "nop");
			else if (C == 1 && N == 0 && ((op >> 16) & 0x1f) == 0x1e)
				sprintf(buffer, "ireturn");
			else if (C == 1)
			{
				if (((op >> 16) & 0x1f) == 15)
					sprintf(buffer, "goto %s%s [%x]", rH, signed_16bit_sep_nospace(N), (pc + 8 + N) & 0xffffff);
				else if (N && rH[0] != '0')
					sprintf(buffer, "goto %s%s", rH, signed_16bit_sep_nospace(N));
				else if (N)
					sprintf(buffer, "goto $%x", ((INT32)N & 0xffffff));
				else
				{
					if (((op >> 16) & 0x1f) == 20)
						flags = DASMFLAG_STEP_OUT;
					sprintf(buffer, "goto %s", rH);
				}
			}
			else
			{
				if (((op >> 16) & 0x1f) == 15)
					sprintf(buffer, "if (%s) goto %s%s [%x]", condtable[C], rH, signed_16bit_sep_nospace(N), (pc + 8 + N) & 0xffffff);
				else if (N && rH[0] != '0')
					sprintf(buffer, "if (%s) goto %s%s", condtable[C], rH, signed_16bit_sep_nospace(N));
				else if (N)
					sprintf(buffer, "if (%s) goto $%x", condtable[C], ((INT32)N & 0xffffff));
				else
				{
					if (((op >> 16) & 0x1f) == 20)
						flags = DASMFLAG_STEP_OUT;
					sprintf(buffer, "if (%s) goto %s", condtable[C], rH);
				}
			}
			break;
		}

		/* CA format 3a */
		case 0x06:	case 0x07:
		{
			const char *rH = regname[(op >> 16) & 0x1f];
			const char *rM = regname[(op >> 21) & 0x1f];
			INT16 N = (INT16)op;

			if (((op >> 16) & 0x1f) == 15)
			{
				sprintf(buffer, "if (%s-- >= 0) goto %s%s [%x]", rM, rH, signed_16bit_sep_nospace(N), (pc + 8 + N) & 0xffffff);
				if (((pc + 8 + N) & 0xffffff) < pc)
					flags = DASMFLAG_STEP_OVER;
			}
			else if (N && rH[0] != '0')
				sprintf(buffer, "if (%s-- >= 0) goto %s%s", rM, rH, signed_16bit_sep_nospace(N));
			else if (N)
			{
				sprintf(buffer, "if (%s-- >= 0) goto $%x", rM, ((INT32)N & 0xffffff));
				if (((INT32)N & 0xffffff) < pc)
					flags = DASMFLAG_STEP_OVER;
			}
			else
			{
				sprintf(buffer, "if (%s-- >= 0) goto %s", rM, rH);
				if (((op >> 16) & 0x1f) == 20)
					flags = DASMFLAG_STEP_OUT;
			}
			break;
		}

		/* CA format 3b/3c */
		case 0x46:
			if (((op >> 21) & 0x1f) == 0)
				sprintf(buffer, "do %d,%d", (op >> 16) & 0x1f, op & 0x7ff);
			else if (((op >> 21) & 0x1f) == 1)
				sprintf(buffer, "do %d,%s", (op >> 16) & 0x1f, regname[op & 0x1f]);
			break;

		/* CA format 4 */
		case 0x08:	case 0x09:
		{
			const char *rH = regname[(op >> 16) & 0x1f];
			const char *rM = regname[(op >> 21) & 0x1f];
			INT16 N = (INT16)op;

			if (((op >> 16) & 0x1f) == 15)
				sprintf(buffer, "call %s%s (%s) [%x]", rH, signed_16bit_sep_nospace(N), rM, (pc + 8 + N) & 0xffffff);
			else if (N && rH[0] != '0')
				sprintf(buffer, "call %s%s (%s)", rH, signed_16bit_sep_nospace(N), rM);
			else if (N)
				sprintf(buffer, "call $%x (%s)", ((INT32)N & 0xffffff), rM);
			else
				sprintf(buffer, "call %s (%s)", rH, rM);
			flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1);
			break;
		}

		/* CA format 5a/5b */
		case 0x0a:	case 0x0b:
		case 0x4a:	case 0x4b:
		{
			const char *rD = regname[(op >> 21) & 0x1f];
			const char *rH = regname[(op >> 16) & 0x1f];
			const char *s = sizesuffix[(op >> 31) & 1];
			INT16 N = (INT16)op;
			if (N == 0)
				sprintf(buffer, "%s%s = %s%s", rD, s, rH, s);
			else if (rH[0] == '0')
				sprintf(buffer, "%s%s = %s", rD, s, signed_16bit_unary(N));
			else
				sprintf(buffer, "%s%s = %s%s%s", rD, s, rH, s, signed_16bit_sep((INT16)op));
			break;
		}

		/* CA format 6a/6b */
		case 0x0c:	case 0x4c:
		{
			const char *rD = regname[(op >> 16) & 0x1f];
			const char *rS1 = regname[(op >> 5) & 0x1f];
			const char *rS2 = regname[(op >> 0) & 0x1f];
			const char *s = sizesuffix[(op >> 31) & 1];
			UINT8 threeop = (op >> 11) & 1;
			char condbuf[40] = { 0 };

			if ((op >> 10) & 1)
				sprintf(condbuf, "if (%s) ", condtable[(op >> 12) & 15]);

			switch ((op >> 21) & 15)
			{
				/* add */
				case 0:
					if (threeop)
					{
						if (rS1[0] == '0' && rS2[0] == '0')
							sprintf(buffer, "%s%s%s = 0", condbuf, rD, s);
						else if (rS1[0] == '0')
							sprintf(buffer, "%s%s%s = %s%s", condbuf, rD, s, rS2, s);
						else if (rS2[0] == '0')
							sprintf(buffer, "%s%s%s = %s%s", condbuf, rD, s, rS1, s);
						else
							sprintf(buffer, "%s%s%s = %s%s + %s%s", condbuf, rD, s, rS2, s, rS1, s);
					}
					else
					{
						if (rS1[0] == '0')
							sprintf(buffer, "%s%s%s = %s%s", condbuf, rD, s, rD, s);
						else
							sprintf(buffer, "%s%s%s = %s%s + %s%s", condbuf, rD, s, rD, s, rS1, s);
					}
					break;

				case 1:
					sprintf(buffer, "%s%s%s = %s%s * 2", condbuf, rD, s, rS1, s);
					break;

				case 2:
					if (threeop)
						sprintf(buffer, "%s%s%s = %s%s - %s%s", condbuf, rD, s, rS1, s, rS2, s);
					else
						sprintf(buffer, "%s%s%s = %s%s - %s%s", condbuf, rD, s, rS1, s, rD, s);
					break;

				case 3:
					if (threeop)
						sprintf(buffer, "%s%s%s = %s%s # %s%s", condbuf, rD, s, rS2, s, rS1, s);
					else
						sprintf(buffer, "%s%s%s = %s%s # %s%s", condbuf, rD, s, rD, s, rS1, s);
					break;

				case 4:
					if (threeop)
						sprintf(buffer, "%s%s%s = %s%s - %s%s", condbuf, rD, s, rS2, s, rS1, s);
					else
						sprintf(buffer, "%s%s%s = %s%s - %s%s", condbuf, rD, s, rD, s, rS1, s);
					break;

				case 5:
					sprintf(buffer, "%s%s%s = -%s%s", condbuf, rD, s, rS1, s);
					break;

				case 6:
					if (threeop)
						sprintf(buffer, "%s%s%s = %s%s &~ %s%s", condbuf, rD, s, rS2, s, rS1, s);
					else
						sprintf(buffer, "%s%s%s = %s%s &~ %s%s", condbuf, rD, s, rD, s, rS1, s);
					break;

				case 7:
//                  if (threeop)
//                      sprintf(buffer, "%s%s%s - %s%s", condbuf, rS2, s, rS1, s);
//                  else
						sprintf(buffer, "%s%s%s - %s%s", condbuf, rD, s, rS1, s);
					break;

				case 8:
					if (threeop)
						sprintf(buffer, "%s%s%s = %s%s ^ %s%s", condbuf, rD, s, rS2, s, rS1, s);
					else
						sprintf(buffer, "%s%s%s = %s%s ^ %s%s", condbuf, rD, s, rD, s, rS1, s);
					break;

				case 9:
					sprintf(buffer, "%s%s%s = %s%s >>> 1", condbuf, rD, s, rS1, s);
					break;

				case 10:
					if (threeop)
						sprintf(buffer, "%s%s%s = %s%s | %s%s", condbuf, rD, s, rS2, s, rS1, s);
					else
						sprintf(buffer, "%s%s%s = %s%s | %s%s", condbuf, rD, s, rD, s, rS1, s);
					break;

				case 11:
					sprintf(buffer, "%s%s%s = %s%s <<< 1", condbuf, rD, s, rS1, s);
					break;

				case 12:
					sprintf(buffer, "%s%s%s = %s%s >> 1", condbuf, rD, s, rS1, s);
					break;

				case 13:
					sprintf(buffer, "%s%s%s = %s%s / 2", condbuf, rD, s, rS1, s);
					break;

				case 14:
					if (threeop)
						sprintf(buffer, "%s%s%s = %s%s & %s%s", condbuf, rD, s, rS2, s, rS1, s);
					else
						sprintf(buffer, "%s%s%s = %s%s & %s%s", condbuf, rD, s, rD, s, rS1, s);
					break;

				case 15:
//                  if (threeop)
//                      sprintf(buffer, "%s%s%s & %s%s", condbuf, rS1, s, rS2, s);
//                  else
						sprintf(buffer, "%s%s%s & %s%s", condbuf, rD, s, rS1, s);
					break;
			}
			break;
		}

		/* CA format 6c/6d */
		case 0x0d:	case 0x4d:
		{
			const char *rD = regname[(op >> 16) & 0x1f];
			const char *s = sizesuffix[(op >> 31) & 1];
			INT16 N = (INT16)op;

			switch ((op >> 21) & 15)
			{
				case 0:
				case 1:
				case 5:
				case 9:
				case 11:
				case 12:
				case 13:
					sprintf(buffer, "Unexpected: %08X", op);
					break;

				case 2:
					sprintf(buffer, "%s%s = %s - %s%s", rD, s, signed_16bit_unary(N), rD, s);
					break;

				case 3:
					sprintf(buffer, "%s%s = %s%s # %s", rD, s, rD, s, signed_16bit_unary(N));
					break;

				case 4:
					sprintf(buffer, "%s%s = %s%s - %s", rD, s, rD, s, signed_16bit_unary(N));
					break;

				case 6:
					sprintf(buffer, "%s%s = %s%s &~ %s", rD, s, rD, s, unsigned_16bit_size(N, (op >> 31) & 1));
					break;

				case 7:
					sprintf(buffer, "%s%s - %s", rD, s, signed_16bit_unary(N));
					break;

				case 8:
					sprintf(buffer, "%s%s = %s%s ^ %s", rD, s, rD, s, unsigned_16bit_size(N, (op >> 31) & 1));
					break;

				case 10:
					sprintf(buffer, "%s%s = %s%s | %s", rD, s, rD, s, unsigned_16bit_size(N, (op >> 31) & 1));
					break;

				case 14:
					sprintf(buffer, "%s%s = %s%s & %s", rD, s, rD, s, unsigned_16bit_size(N, (op >> 31) & 1));
					break;

				case 15:
					sprintf(buffer, "%s%s & %s", rD, s, unsigned_16bit_size(N, (op >> 31) & 1));
					break;
			}
			break;
		}

		/* CA format 7a */
		case 0x0e:
			if ((op >> 24) & 1)
				sprintf(buffer, "*%08X = %s%s", (INT16)op, regname[(op >> 16) & 0x1f], memsuffix[(op >> 22) & 3]);
			else
				sprintf(buffer, "%s%s = *%08X", regname[(op >> 16) & 0x1f], memsuffix[(op >> 22) & 3], (INT16)op);
			break;

		/* CA format 7b */
		case 0x0f:
			if ((op >> 24) & 1)
				sprintf(buffer, "%s = %s%s", dasm_PI(op & 0x3ff, tempbuf[0]), regname[(op >> 16) & 0x1f], memsuffix[(op >> 22) & 3]);
			else
				sprintf(buffer, "%s%s = %s", regname[(op >> 16) & 0x1f], memsuffix[(op >> 22) & 3], dasm_PI(op & 0x3ff, tempbuf[0]));
			break;

		/* CA format 8a */
		case 0x50:	case 0x51:	case 0x52:	case 0x53:
		case 0x54:	case 0x55:	case 0x56:	case 0x57:
		case 0x58:	case 0x59:	case 0x5a:	case 0x5b:
		case 0x5c:	case 0x5d:	case 0x5e:	case 0x5f:
		{
			INT32 N = (op & 0xffff) | ((INT32)((op & 0x1fe00000) << 3) >> 8);
			const char *rH = regname[(op >> 16) & 0x1f];

			if (((op >> 16) & 0x1f) == 15)
				sprintf(buffer, "goto %s%s [%x]", rH, signed_16bit_sep_nospace(N), (pc + 8 + N) & 0xffffff);
			else if (N && rH[0] != '0')
				sprintf(buffer, "goto %s%s", rH, signed_16bit_sep_nospace(N));
			else if (N)
				sprintf(buffer, "goto $%x", ((INT32)N & 0xffffff));
			else
			{
				if (((op >> 16) & 0x1f) == 20)
					flags = DASMFLAG_STEP_OUT;
				sprintf(buffer, "goto %s", rH);
			}
			break;
		}

		/* CA format 8b */
		case 0x60:	case 0x61:	case 0x62:	case 0x63:
		case 0x64:	case 0x65:	case 0x66:	case 0x67:
		case 0x68:	case 0x69:	case 0x6a:	case 0x6b:
		case 0x6c:	case 0x6d:	case 0x6e:	case 0x6f:
		{
			INT32 immed = (op & 0xffff) | ((INT32)((op & 0x1fe00000) << 3) >> 8);
			sprintf(buffer, "%s = $%x", regnamee[(op >> 16) & 0x1f], immed & 0xffffff);
			break;
		}

		/* CA format 8c */
		case 0x70:	case 0x71:	case 0x72:	case 0x73:
		case 0x74:	case 0x75:	case 0x76:	case 0x77:
		case 0x78:	case 0x79:	case 0x7a:	case 0x7b:
		case 0x7c:	case 0x7d:	case 0x7e:	case 0x7f:
		{
			INT32 N = (op & 0xffff) | ((INT32)((op & 0x1fe00000) << 3) >> 8);
			const char *rM = regname[(op >> 16) & 0x1f];
			sprintf(buffer, "call $%x (%s)", N & 0xffffff, rM);
			flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1);
			break;
		}
	}

	return 4 | flags | DASMFLAG_SUPPORTED;
}
