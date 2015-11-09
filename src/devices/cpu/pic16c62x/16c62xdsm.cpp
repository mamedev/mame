// license:BSD-3-Clause
// copyright-holders:Tony La Porta
	/**************************************************************************\
	*                  Microchip PIC16C62X Emulator                            *
	*                                                                          *
	*                          Based On                                        *
	*                  Microchip PIC16C5X Emulator                             *
	*                    Copyright Tony La Porta                               *
	*                 Originally written for the MAME project.                 *
	*                                                                          *
	*                                                                          *
	*      Addressing architecture is based on the Harvard addressing scheme.  *
	*                                                                          *
	*         Many thanks to those involved in the i8039 Disassembler          *
	*                        as this was based on it.                          *
	*                                                                          *
	*                                                                          *
	*                                                                          *
	* A Address to jump to.                                                    *
	* B Bit address within an 8-bit file register.                             *
	* D Destination select (0 = store result in W (accumulator))               *
	*                      (1 = store result in file register)                 *
	* F Register file address (00-1F).                                         *
	* K Literal field, constant data.                                          *
	* X Not used                                                               *
	*                                                                          *
	\**************************************************************************/

#include "emu.h"
#include <ctype.h>

static const UINT8 *rombase;
static const UINT8 *rambase;
static offs_t pcbase;
#define READOP16(A)  (rombase[(A) - pcbase] | (rombase[(A) + 1 - pcbase] << 8))
#define READARG16(A) (rambase[(A) - pcbase] | (rambase[(A) + 1 - pcbase] << 8))



typedef unsigned char byte;
typedef unsigned short int word;

#define FMT(a,b) a, b
#define PTRS_PER_FORMAT 2

/* Registers bank 0/1 */
static const char *const regfile[32] = { "Reg$00 (INDF)",    "Reg$01 (TMR0/OPTION)",    "Reg$02 (PCL)",  "Reg$03 (STATUS)", "Reg$04 (FSR)", "Reg$05 (PORTA/TRISA)", "Reg$06 (PORTB/TRISB)", "Reg$07",
									"Reg$08", "Reg$09", "Reg$0A (PCLATH)", "Reg$0B (INTCON)", "Reg$0C (PIR1/PIE1)", "Reg$0D", "Reg$0E (none/PCON)", "Reg$0F",
									"Reg$10", "Reg$11", "Reg$12", "Reg$13", "Reg$14", "Reg$15", "Reg$16", "Reg$17",
									"Reg$18", "Reg$19", "Reg$1A", "Reg$1B", "Reg$1C", "Reg$1D", "Reg$1E", "Reg$1F (CMCON/VRCON)" };
/* Registers bank 1 */
/*static const char *const regfile1[32] = { "Reg$00 (INDF)",    "Reg$01 (OPTION)",    "Reg$02 (PCL)",  "Reg$03 (STATUS)", "Reg$04 (FSR)", "Reg$05 (TRISA)", "Reg$06 (TRISB)", "Reg$07",
                                 "Reg$08", "Reg$09", "Reg$0A (PCLATH)", "Reg$0B (INTCON)", "Reg$0C (PIE1)", "Reg$0D", "Reg$0E (PCON)", "Reg$0F",
                                 "Reg$10", "Reg$11", "Reg$12", "Reg$13", "Reg$14", "Reg$15", "Reg$16", "Reg$17",
                                 "Reg$18", "Reg$19", "Reg$1A", "Reg$1B", "Reg$1C", "Reg$1D", "Reg$1E", "Reg$1F (VRCON)" };
static const char **regfile[2] = { regfile0, regfile1 };*/

static const char *const dest[2] = { "W", "Reg" };

static const char *const PIC16C62xFormats[] = {
	FMT("0000000xx00000", "nop"),
	FMT("00000000001000", "return"),
	FMT("00000000001001", "retfie"),
	FMT("00000001100011", "sleep"),
	FMT("00000001100100", "clrwdt"),
	FMT("0000001fffffff", "movwf  %F"),
	FMT("00000100000011", "clrw"),
	FMT("0000011fffffff", "clrf   %F"),
	FMT("000010dfffffff", "subwf  %F,%D"),
	FMT("000011dfffffff", "decf   %F,%D"),
	FMT("000100dfffffff", "iorwf  %F,%D"),
	FMT("000101dfffffff", "andwf  %F,%D"),
	FMT("000110dfffffff", "xorwf  %F,%D"),
	FMT("000111dfffffff", "addwf  %F,%D"),
	FMT("001000dfffffff", "movf   %F,%D"),
	FMT("001001dfffffff", "comf   %F,%D"),
	FMT("001010dfffffff", "incf   %F,%D"),
	FMT("001011dfffffff", "decfsz %F,%D"),
	FMT("001100dfffffff", "rrf    %F,%D"),
	FMT("001101dfffffff", "rlf    %F,%D"),
	FMT("001110dfffffff", "swapf  %F,%D"),
	FMT("001111dfffffff", "incfsz %F,%D"),
	FMT("0100bbbfffffff", "bcf    %F,%B"),
	FMT("0101bbbfffffff", "bsf    %F,%B"),
	FMT("0110bbbfffffff", "btfsc  %F,%B"),
	FMT("0111bbbfffffff", "btfss  %F,%B"),
	FMT("1101xxkkkkkkkk", "retlw  %K"),
	FMT("100aaaaaaaaaaa", "call   %A"),
	FMT("101aaaaaaaaaaa", "goto   %A"),
	FMT("1100xxkkkkkkkk", "movlw  %K"),
	FMT("111000kkkkkkkk", "iorlw  %K"),
	FMT("111001kkkkkkkk", "andlw  %K"),
	FMT("111010kkkkkkkk", "xorlw  %K"),
	FMT("11110xkkkkkkkk", "sublw  %K"),
	FMT("11111xkkkkkkkk", "addlw  %K"),
	NULL
};

#define MAX_OPS ((ARRAY_LENGTH(PIC16C62xFormats) - 1) / PTRS_PER_FORMAT)

struct PIC16C62xOpcode  {
	word mask;          /* instruction mask */
	word bits;          /* constant bits */
	word extcode;       /* value that gets extension code */
	const char *parse;  /* how to parse bits */
	const char *fmt;    /* instruction format */
};

static PIC16C62xOpcode Op[MAX_OPS+1];
static int OpInizialized = 0;

static void InitDasm16C5x(void)
{
	const char *p;
	const char *const *ops;
	word mask, bits;
	int bit;
	int i;

	ops = PIC16C62xFormats; i = 0;
	while (*ops)
	{
		p = *ops;
		mask = 0; bits = 0; bit = 13;
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
				case 'f':
				case 'k':
				case 'x':
					bit --;
					break;
				default: fatalerror("Invalid instruction encoding '%s %s'\n",
					ops[0],ops[1]);
			}
		}
		if (bit != -1 )
		{
			fatalerror("not enough bits in encoding '%s %s' %d\n",
				ops[0],ops[1],bit);
		}
		while (isspace((UINT8)*p)) p++;
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

CPU_DISASSEMBLE( pic16c62x )
{
	int a, b, d, f, k;  /* these can all be filled in by parsing an instruction */
	int i;
	int op;
	int cnt = 1;
	int code;
	int bit;
	//char *buffertmp;
	const char *cp;             /* character pointer in OpFormats */
	UINT32 flags = 0;

	rombase = oprom;
	rambase = opram;
	pcbase = 2*pc;

	if (!OpInizialized) InitDasm16C5x();

	op = -1;                /* no matching opcode */
	code = READOP16(2*pc);
	for ( i = 0; i < MAX_OPS; i++)
	{
		if ((code & Op[i].mask) == Op[i].bits)
		{
			if (op != -1)
			{
				osd_printf_debug("Error: opcode %04Xh matches %d (%s) and %d (%s)\n",
					code,i,Op[i].fmt,op,Op[op].fmt);
			}
			op = i;
		}
	}
	if (op == -1)
	{
		sprintf(buffer,"???? dw %04Xh",code);
		return cnt;
	}
	//buffertmp = buffer;
	if (Op[op].extcode)     /* Actually, theres no double length opcodes */
	{
		bit = 29;
		code <<= 16;
		code |= READARG16(2*(pc+cnt));
		cnt++;
	}
	else
	{
		bit = 13;
	}

	/* shift out operands */
	cp = Op[op].parse;
	a = b = d = f = k = 0;

	while (bit >= 0)
	{
		/* osd_printf_debug("{%c/%d}",*cp,bit); */
		switch(*cp)
		{
			case 'a': a <<=1; a |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'b': b <<=1; b |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'd': d <<=1; d |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'f': f <<=1; f |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'k': k <<=1; k |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case ' ': break;
			case '1': case '0': case 'x':  bit--; break;
			case '\0': fatalerror("premature end of parse string, opcode %x, bit = %d\n",code,bit);
		}
		cp++;
	}

	/* now traverse format string */
	cp = Op[op].fmt;
	if (!strncmp(cp, "call", 4))
		flags = DASMFLAG_STEP_OVER;
	else if (!strncmp(cp, "ret", 3))
		flags = DASMFLAG_STEP_OUT;

	while (*cp)
	{
		if (*cp == '%')
		{
			char num[30], *q;
			cp++;
			switch (*cp++)
			{
				case 'A': sprintf(num,"$%03X",a); break;
				case 'B': sprintf(num,"%d",b); break;
				case 'D': sprintf(num,"%s",dest[d]); break;
				case 'F': if (f < 0x20) sprintf(num,"%s",regfile[f]); else sprintf(num,"Reg$%02X",f); break;
				case 'K': sprintf(num,"%02Xh",k); break;
				default:
					fatalerror("illegal escape character in format '%s'\n",Op[op].fmt);
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
	return cnt | flags | DASMFLAG_SUPPORTED;
}
