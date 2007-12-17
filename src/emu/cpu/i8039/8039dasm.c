/****************************************************************************
 *
 *      mcs48 disassembler
 *
 * This file is Copyright 1996 Michael Cuddy, Fen's Ende Sofware.
 * Redistribution is allowed in source and binary form as long as both
 * forms are distributed together with the file 'README'.  This copyright
 * notice must also accompany the files.
 *
 * This software should be considered a small token to all of the
 * emulator authors for thier dilligence in preserving our Arcade and
 * Computer history.
 *
 * Michael Cuddy, Fen's Ende Software.
 * 11/25/1996
 *
 * Adapted by Andrea Mazzoleni for use with MAME
 *
 ***************************************************************************/

#include <ctype.h>

#include "cpuintrf.h"

#define mame_printf_debug printf

typedef unsigned char byte;

#define FMT(a,b) a, b
#define PTRS_PER_FORMAT 2

static const char *Formats[] = {
	FMT("00000011dddddddd", "add  a,#$%X"),
	FMT("01101rrr", "add  a,%R"),
	FMT("0110000r", "add  a,@%R"),
	FMT("00010011dddddddd", "adc  a,#$%X"),
	FMT("01111rrr", "adc  a,%R"),
	FMT("0111000r", "adc  a,@%R"),
	FMT("01010011dddddddd", "anl  a,#$%X"),
	FMT("01011rrr", "anl  a,%R"),
	FMT("0101000r", "anl  a,@%R"),
	FMT("10011000dddddddd", "anl  bus,#$%X"),
	FMT("10011001dddddddd", "anl  p1,#$%X"),
	FMT("10011010dddddddd", "anl  p2,#$%X"),
	FMT("100111pp", "anld %P,a"),
	FMT("aaa10100aaaaaaaa", "!call %A"),
	FMT("00100111", "clr  a"),
	FMT("10010111", "clr  c"),
	FMT("10100101", "clr  f1"),
	FMT("10000101", "clr  f0"),
	FMT("00110111", "cpl  a"),
	FMT("10100111", "cpl  c"),
	FMT("10010101", "cpl  f0"),
	FMT("10110101", "cpl  f1"),
	FMT("01010111", "da   a"),
	FMT("00000111", "dec  a"),
	FMT("11001rrr", "dec  %R"),
	FMT("00010101", "dis  i"),
	FMT("00110101", "dis  tcnti"),
	FMT("11101rrraaaaaaaa", "!djnz %R,%J"),
	FMT("00000101", "en   i"),
	FMT("00100101", "en   tcnti"),
	FMT("01110101", "ent0 clk"),
	FMT("00001001", "in   a,p1"),
	FMT("00001010", "in   a,p2"),
	FMT("00010111", "inc  a"),
	FMT("00011rrr", "inc  %R"),
	FMT("0001000r", "inc  @%R"),
	FMT("00001000", "ins  a,bus"),
	FMT("0001 0110aaaaaaaa", "jtf  %J"),
	FMT("0010 0110aaaaaaaa", "jnt0 %J"),
	FMT("0011 0110aaaaaaaa", "jt0  %J"),
	FMT("0100 0110aaaaaaaa", "jnt1 %J"),
	FMT("0101 0110aaaaaaaa", "jt1  %J"),
	FMT("0111 0110aaaaaaaa", "jf1  %J"),
	FMT("1000 0110aaaaaaaa", "jni  %J"),
	FMT("1001 0110aaaaaaaa", "jnz  %J"),
	FMT("1011 0110aaaaaaaa", "jf0  %J"),
	FMT("1100 0110aaaaaaaa", "jz   %J"),
	FMT("1110 0110aaaaaaaa", "jnc  %J"),
	FMT("1111 0110aaaaaaaa", "jc   %J"),
	FMT("bbb10010aaaaaaaa", "jb%B  %J"),
	FMT("aaa00100aaaaaaaa", "jmp  %A"),
	FMT("10110011", "jmpp @a"),
	FMT("00100011dddddddd", "mov  a,#$%X"),
	FMT("11111rrr", "mov  a,%R"),
	FMT("1111000r", "mov  a,@%R"),
	FMT("11000111", "mov  a,psw"),
	FMT("10111rrrdddddddd", "mov  %R,#$%X"),
	FMT("10101rrr", "mov  %R,a"),
	FMT("1010000r", "mov  @%R,a"),
	FMT("1011000rdddddddd", "mov  @%R,#$%X"),
	FMT("11010111", "mov  psw,a"),
	FMT("000011pp", "movd a,%P"),
	FMT("001111pp", "movd %P,a"),
	FMT("01000010", "mov  a,t"),
	FMT("01100010", "mov  t,a"),
	FMT("11100011", "movp3 a,@a"),
	FMT("10100011", "movp a,@a"),
	FMT("1000000r", "movx a,@%R"),
	FMT("1001000r", "movx @%R,a"),
	FMT("0100 1rrr", "orl  a,%R"),
	FMT("0100 000r", "orl  a,@%R"),
	FMT("0100 0011dddddddd", "orl  a,#$%X"),
	FMT("1000 1000dddddddd", "orl  bus,#$%X"),
	FMT("1000 1001dddddddd", "orl  p1,#$%X"),
	FMT("1000 1010dddddddd", "orl  p2,#$%X"),
	FMT("1000 11pp", "orld %P,a"),
	FMT("00000010", "outl bus,a"),
	FMT("001110pp", "outl %P,a"),
	FMT("10000011", "^ret"),
	FMT("10010011", "^retr"),
	FMT("11100111", "rl   a"),
	FMT("11110111", "rlc  a"),
	FMT("01110111", "rr   a"),
	FMT("01100111", "rrc  a"),
	FMT("11100101", "sel  mb0"),
	FMT("11110101", "sel  mb1"),
	FMT("11000101", "sel  rb0"),
	FMT("11010101", "sel  rb1"),
	FMT("01100101", "stop tcnt"),
	FMT("01000101", "strt cnt"),
	FMT("01010101", "strt t"),
	FMT("01000111", "swap a"),
	FMT("00101rrr", "xch  a,%R"),
	FMT("0010000r", "xch  a,@%R"),
	FMT("0011000r", "xchd a,@%R"),
	FMT("1101 0011dddddddd", "xrl  a,#$%X"),
	FMT("1101 1rrr", "xrl  a,%R"),
	FMT("1101 000r", "xrl  a,@%R"),
	FMT("00000000", "nop"),
	NULL
};

#define MAX_OPS (((sizeof(Formats) / sizeof(Formats[0])) - 1) / PTRS_PER_FORMAT)

typedef struct opcode {
	byte mask;	/* instruction mask */
	byte bits;	/* constant bits */
	char extcode;	/* value that gets extension code */
	const char *parse;	/* how to parse bits */
	const char *fmt;	/* instruction format */
	unsigned long flags;
} M48Opcode;

static M48Opcode Op[MAX_OPS+1];
static int OpInizialized = 0;

static void InitDasm8039(void)
{
	const char *p, **ops;
	byte mask, bits;
	int bit;
	int i;

	ops = Formats; i = 0;
	while (*ops) {
	unsigned long flags = 0;
	p = *ops;
	mask = 0; bits = 0; bit = 7;
	while (*p && bit >= 0) {
		switch (*p++) {
			case '1': mask |= 1<<bit; bits |= 1<<bit; bit--; break;
			case '0': mask |= 1<<bit; bit--; break;
			case ' ': break;
			case 'b':
			case 'a': case 'r': case 'd': case 'p':
				bit --;
				break;
			default:
				fatalerror("Invalid instruction encoding '%s %s'\n", ops[0],ops[1]);
				break;
		}
	}
	if (bit != -1 ) {
		fatalerror("not enough bits in encoding '%s %s' %d\n", ops[0],ops[1],bit);
		break;
	}
	while (isspace(*p)) p++;
	if (*p) Op[i].extcode = *p;
	Op[i].bits = bits;
	Op[i].mask = mask;
	Op[i].fmt = ops[1];
	Op[i].parse = ops[0];

	if (Op[i].fmt[0] == '!')
	{
		flags |= DASMFLAG_STEP_OVER;
		Op[i].fmt++;
	}
	if (Op[i].fmt[0] == '^')
	{
		flags |= DASMFLAG_STEP_OUT;
		Op[i].fmt++;
	}
	Op[i].flags = flags;

	ops += PTRS_PER_FORMAT;
	i++;
	}

	OpInizialized = 1;
}

offs_t i8039_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	int b, a, d, r, p;	/* these can all be filled in by parsing an instruction */
	int i;
	int op;
	int cnt = 1;
	int code, bit;
	const char *cp;

	if (!OpInizialized) InitDasm8039();

	code = oprom[0];
	op = -1;	/* no matching opcode */
	for ( i = 0; i < MAX_OPS; i++)
	{
		if( (code & Op[i].mask) == Op[i].bits )
		{
			if (op != -1)
			{
				fprintf(stderr, "Error: opcode %02X matches %d (%s) and %d (%s)\n",
					code,i,Op[i].fmt,op,Op[op].fmt);
			}
			op = i;
		}
	}

	if (op == -1)
	{
		sprintf(buffer,"db   %2.2x",code);
		return cnt | DASMFLAG_SUPPORTED;
	}

	if (Op[op].extcode)
	{
		cnt++;
		code <<= 8;
		code |= opram[1];
		bit = 15;
	}
	else
	{
		bit = 7;
	}

	/* shift out operands */
	cp = Op[op].parse;
	b = a = d = r = p = 0;

	while (bit >= 0)
	{
		/* mame_printf_debug("{%c/%d}",*cp,bit); */
		switch(*cp)
		{
			case 'a': a <<=1; a |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'b': b <<=1; b |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'd': d <<=1; d |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'r': r <<=1; r |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'p': p <<=1; p |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case ' ': break;
			case '1': case '0': bit--; break;
			case '\0': fatalerror("premature end of parse string, opcode %x, bit = %d\n",code,bit); break;
		}
		cp++;
	}

	/* now traverse format string */
	cp = Op[op].fmt;
	while (*cp)
	{
		if (*cp == '%')
		{
			char num[10], *q;
			cp++;
			switch (*cp++)
			{
				case 'A': sprintf(num,"$%04X",a); break;
				case 'J': sprintf(num,"$%04X",((pc+1) & 0xf00) | a); break;
				case 'B': sprintf(num,"%d",b); break;
				case 'D': sprintf(num,"%d",d); break;
				case 'X': sprintf(num,"%X",d); break;
				case 'R': sprintf(num,"r%d",r); break;
				case 'P': sprintf(num,"p%d",p); break;
				default:
					fatalerror("illegal escape character in format '%s'\n",Op[op].fmt);
					break;
			}
			q = num; while (*q) *buffer++ = *q++;
			*buffer = '\0';
		}
		else
		{
			*buffer++ = *cp++;
			*buffer = '\0';
		}
	}

	return cnt | Op[op].flags | DASMFLAG_SUPPORTED;
}
