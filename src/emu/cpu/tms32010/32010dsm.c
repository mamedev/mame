 /**************************************************************************\
 *                Texas Instruments TMS32010 DSP Disassembler               *
 *                                                                          *
 *                  Copyright (C) 1999-2002+ Tony La Porta                  *
 *               To be used with TMS32010 DSP Emulator engine.              *
 *      You are not allowed to distribute this software commercially.       *
 *                      Written for the MAME project.                       *
 *                                                                          *
 *         Many thanks to those involved in the i8039 Disassembler          *
 *                        as this was based on it.                          *
 *                                                                          *
 *                                                                          *
 *                                                                          *
 * A Memory address                                                         *
 * B Branch Address for Branch instructions (Requires next opcode read)     *
 * D Immediate byte load                                                    *
 * K Immediate bit  load                                                    *
 * W Immediate word load (Actually 13 bit)                                  *
 * M AR[x] register modification type (for indirect addressing)             *
 * N ARP register to change ARP pointer to (for indirect addressing)        *
 * P I/O port address number                                                *
 * R AR[R] register to use                                                  *
 * S Shift ALU left                                                         *
 *                                                                          *
 \**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef MAME_DEBUG					/* Compile interface to MAME */
#include "cpuintrf.h"
#include "tms32010.h"
#include "debugger.h"
#else								/* Compile interface for standalone */
extern unsigned char *Buffer;
#ifdef MSB_FIRST
#define READOP16(A)  ( ((Buffer[A+1]<<8) | Buffer[A]) )
#define READARG16(A) ( ((Buffer[A+1]<<8) | Buffer[A]) )
#else
#define READOP16(A)  ( ((Buffer[A]<<8) | Buffer[A+1]) )
#define READARG16(A) ( ((Buffer[A]<<8) | Buffer[A+1]) )
#endif
#endif



typedef unsigned char byte;
typedef unsigned short int word;

#define FMT(a,b) a, b
#define PTRS_PER_FORMAT 2

static const char *arith[4] = { "*" , "*-" , "*+" , "??" } ;
static const char *nextar[4] = { ",AR0" , ",AR1" , "" , "" } ;


static const char *TMS32010Formats[] = {
	FMT("0000ssss0aaaaaaa", "add  %A%S"),
	FMT("0000ssss10mmn00n", "add  %M%S%N"),
	FMT("0001ssss0aaaaaaa", "sub  %A%S"),
	FMT("0001ssss10mmn00n", "sub  %M%S%N"),
	FMT("0010ssss0aaaaaaa", "lac  %A%S"),
	FMT("0010ssss10mmn00n", "lac  %M%S%N"),
	FMT("0011000r0aaaaaaa", "sar  %R,%A"),
	FMT("0011000r10mmn00n", "sar  %R%M%N"),
	FMT("0011100r0aaaaaaa", "lar  %R,%A"),
	FMT("0011100r10mmn00n", "lar  %R%M%N"),
	FMT("01000ppp0aaaaaaa", "in   %A,%P"),
	FMT("01000ppp10mmn00n", "in   %M,%P%N"),
	FMT("01001ppp0aaaaaaa", "out  %A,%P"),
	FMT("01001ppp10mmn00n", "out  %M,%P%N"),
	FMT("01010sss0aaaaaaa", "sacl %A"),		/* This instruction has a shift but */
	FMT("01010sss10mmn00n", "sacl %M%N"),	/* is documented as not performed */
	FMT("01011sss0aaaaaaa", "sach %A%S"),
	FMT("01011sss10mmn00n", "sach %M%S%N"),
	FMT("011000000aaaaaaa", "addh %A"),
	FMT("0110000010mmn00n", "addh %M%N"),
	FMT("011000010aaaaaaa", "adds %A"),
	FMT("0110000110mmn00n", "adds %M%N"),
	FMT("011000100aaaaaaa", "subh %A"),
	FMT("0110001010mmn00n", "subh %M%N"),
	FMT("011000110aaaaaaa", "subs %A"),
	FMT("0110001110mmn00n", "subs %M%N"),
	FMT("011001000aaaaaaa", "subc %A"),
	FMT("0110010010mmn00n", "subc %M%N"),
	FMT("011001010aaaaaaa", "zalh %A"),
	FMT("0110010110mmn00n", "zalh %M%N"),
	FMT("011001100aaaaaaa", "zals %A"),
	FMT("0110011010mmn00n", "zals %M%N"),
	FMT("011001110aaaaaaa", "tblr %A"),
	FMT("0110011110mmn00n", "tblr %M%N"),
	FMT("011010001000000k", "larp %K"),
	FMT("011010000aaaaaaa", "mar  %A"),		/* Actually this is executed as a NOP */
/*  FMT("0110100010mmn00n", "mar  %M%N"),   */
/*  MAR indirect has been expanded out to all its variations because one of */
/*  its opcodes is the same as LARP (actually performs the same function) */

	FMT("0110100010001000", "mar  *"),
	FMT("0110100010001001", "mar  *"),
	FMT("0110100010010000", "mar  *-,AR0"),
	FMT("0110100010010001", "mar  *-,AR1"),
	FMT("0110100010011000", "mar  *-"),
	FMT("0110100010011001", "mar  *-"),
	FMT("0110100010100000", "mar  *+,AR0"),
	FMT("0110100010100001", "mar  *+,AR1"),
	FMT("0110100010101000", "mar  *+"),
	FMT("0110100010101001", "mar  *+"),
	FMT("0110100010110000", "mar  ??,AR0"),
	FMT("0110100010110001", "mar  ??,AR1"),
	FMT("0110100010111000", "mar  ??"),
	FMT("0110100010111001", "mar  ??"),

	FMT("011010010aaaaaaa", "dmov %A"),
	FMT("0110100110mmn00n", "dmov %M%N"),
	FMT("011010100aaaaaaa", "lt   %A"),
	FMT("0110101010mmn00n", "lt   %M%N"),
	FMT("011010110aaaaaaa", "ltd  %A"),
	FMT("0110101110mmn00n", "ltd  %M%N"),
	FMT("011011000aaaaaaa", "lta  %A"),
	FMT("0110110010mmn00n", "lta  %M%N"),
	FMT("011011010aaaaaaa", "mpy  %A"),
	FMT("0110110110mmn00n", "mpy  %M%N"),
	FMT("011011100000000k", "ldpk %K"),
	FMT("011011110aaaaaaa", "ldp  %A"),
	FMT("0110111110mmn00n", "ldp  %M%N"),
	FMT("0111000rdddddddd", "lark %R,%D"),
	FMT("011110000aaaaaaa", "xor  %A"),
	FMT("0111100010mmn00n", "xor  %M%N"),
	FMT("011110010aaaaaaa", "and  %A"),
	FMT("0111100110mmn00n", "and  %M%N"),
	FMT("011110100aaaaaaa", "or   %A"),
	FMT("0111101010mmn00n", "or   %M%N"),
	FMT("011110110aaaaaaa", "lst  %A"),
	FMT("0111101110mmn00n", "lst  %M%N"),
	FMT("011111000aaaaaaa", "sst  %A"),
	FMT("0111110010mmn00n", "sst  %M%N"),
	FMT("011111010aaaaaaa", "tblw %A"),
	FMT("0111110110mmn00n", "tblw %M%N"),
	FMT("01111110dddddddd", "lack %D"),
	FMT("0111111110000000", "nop"),			/* 7F80 */
	FMT("0111111110000001", "dint"),
	FMT("0111111110000010", "eint"),
	FMT("0111111110001000", "abs"),			/* 7F88 */
	FMT("0111111110001001", "zac"),
	FMT("0111111110001010", "rovm"),
	FMT("0111111110001011", "sovm"),
	FMT("0111111110001100", "cala"),
	FMT("0111111110001101", "ret"),
	FMT("0111111110001110", "pac"),
	FMT("0111111110001111", "apac"),
	FMT("0111111110010000", "spac"),
	FMT("0111111110011100", "push"),
	FMT("0111111110011101", "pop"),			/* 7F9D */
	FMT("100wwwwwwwwwwwww", "mpyk %W"),
	FMT("1111010000000000bbbbbbbbbbbbbbbb", "banz %B"),
	FMT("1111010100000000bbbbbbbbbbbbbbbb", "bv   %B"),
	FMT("1111011000000000bbbbbbbbbbbbbbbb", "bioz %B"),
	FMT("1111100000000000bbbbbbbbbbbbbbbb", "call %B"),
	FMT("1111100100000000bbbbbbbbbbbbbbbb", "b    %B"),
	FMT("1111101000000000bbbbbbbbbbbbbbbb", "blz  %B"),
	FMT("1111101100000000bbbbbbbbbbbbbbbb", "blez %B"),
	FMT("1111110000000000bbbbbbbbbbbbbbbb", "bgz  %B"),
	FMT("1111110100000000bbbbbbbbbbbbbbbb", "bgez %B"),
	FMT("1111111000000000bbbbbbbbbbbbbbbb", "bnz  %B"),
	FMT("1111111100000000bbbbbbbbbbbbbbbb", "bz   %B"),
	NULL
};

#define MAX_OPS (((sizeof(TMS32010Formats) / sizeof(TMS32010Formats[0])) - 1) / PTRS_PER_FORMAT)

typedef struct opcode {
	word mask;			/* instruction mask */
	word bits;			/* constant bits */
	word extcode;		/* value that gets extension code */
	const char *parse;		/* how to parse bits */
	const char *fmt;			/* instruction format */
} TMS32010Opcode;

static TMS32010Opcode Op[MAX_OPS+1];
static int OpInizialized = 0;

static void InitDasm32010(void)
{
	const char *p, **ops;
	word mask, bits;
	int bit;
	int i;

	ops = TMS32010Formats; i = 0;
	while (*ops)
	{
		p = *ops;
		mask = 0; bits = 0; bit = 15;
		while (*p && bit >= 0)
		{
			switch (*p++)
			{
				case '1': mask |= 1<<bit; bits |= 1<<bit; bit--; break;
				case '0': mask |= 1<<bit; bit--; break;
				case ' ': break;
				case 'a':
				case 'b':
				case 'd':
				case 'k':
				case 'm':
				case 'n':
				case 'p':
				case 'r':
				case 's':
				case 'w':
					bit --;
					break;
				default: fatalerror("Invalid instruction encoding '%s %s'",
					ops[0],ops[1]);
			}
		}
		if (bit != -1 )
		{
			fatalerror("not enough bits in encoding '%s %s' %d",
				ops[0],ops[1],bit);
		}
		while (isspace(*p)) p++;
		if (*p) Op[i].extcode = *p;
		Op[i].bits = bits;
		Op[i].mask = mask;
		Op[i].fmt = ops[1];
		Op[i].parse = ops[0];

		ops += PTRS_PER_FORMAT;
		i++;
	}

	OpInizialized = 1;
}

offs_t tms32010_dasm(char *str, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	UINT32 flags = 0;
	int a, b, d, k, m, n, p, r, s, w;	/* these can all be filled in by parsing an instruction */
	int i;
	int op;
	int cnt = 1;
	int code;
	int bit;
	char *strtmp;
	const char *cp;				/* character pointer in OpFormats */

	if (!OpInizialized) InitDasm32010();

	op = -1;				/* no matching opcode */
	code = (oprom[0] << 8) | oprom[1];
	for ( i = 0; i < MAX_OPS; i++)
	{
		if ((code & Op[i].mask) == Op[i].bits)
		{
			if (op != -1)
			{
				mame_printf_debug("Error: opcode %04Xh matches %d (%s) and %d (%s)\n",
					code,i,Op[i].fmt,op,Op[op].fmt);
			}
			op = i;
		}
	}
	if (op == -1)
	{
		sprintf(str,"???? dw %04Xh",code);
		return cnt | DASMFLAG_SUPPORTED;
	}
	strtmp = str;
	if (Op[op].extcode)
	{
		bit = 31;
		code <<= 16;
		code |= (opram[2] << 8) | opram[3];
		cnt++;
	}
	else
	{
		bit = 15;
	}

	/* shift out operands */
	cp = Op[op].parse;
	a = b = d = k = m = n = p = r = s = w = 0;

	while (bit >= 0)
	{
		/* mame_printf_debug("{%c/%d}",*cp,bit); */
		switch(*cp)
		{
			case 'a': a <<=1; a |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'b': b <<=1; b |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'd': d <<=1; d |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'k': k <<=1; k |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'm': m <<=1; m |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'n': n <<=1; n |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'p': p <<=1; p |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'r': r <<=1; r |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 's': s <<=1; s |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'w': w <<=1; w |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case ' ': break;
			case '1': case '0':  bit--; break;
			case '\0': fatalerror("premature end of parse string, opcode %x, bit = %d",code,bit);
		}
		cp++;
	}

	/* now traverse format string */
	cp = Op[op].fmt;

	if (!strncmp(cp, "cal", 3))
		flags = DASMFLAG_STEP_OVER;
	else if (!strncmp(cp, "ret", 3))
		flags = DASMFLAG_STEP_OUT;

	while (*cp)
	{
		if (*cp == '%')
		{
			char num[20], *q;
			cp++;
			switch (*cp++)
			{
				case 'A': sprintf(num,"$%02X",a); break;
				case 'B': sprintf(num,"$%04X",b); break;
				case 'D': sprintf(num,"%02Xh",d); break;
				case 'K': sprintf(num,"%d",k); break;
				case 'N': sprintf(num,"%s",nextar[n]); break;
				case 'M': sprintf(num,"%s",arith[m]); break;
				case 'P': sprintf(num,"PA%d",p); break;
				case 'R': sprintf(num,"AR%d",r); break;
				case 'S': sprintf(num,",%d",s); break;
				case 'W': sprintf(num,"%04Xh",w); break;
				default:
					fatalerror("illegal escape character in format '%s'",Op[op].fmt);
			}
			q = num; while (*q) *str++ = *q++;
			*str = '\0';
		}
		else
		{
			*str++ = *cp++;
			*str = '\0';
		}
	}
	return cnt | flags | DASMFLAG_SUPPORTED;
}
