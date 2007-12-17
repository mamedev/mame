/****************************************************************************
                         Alpha 8201/8301 Disassembler

                      Copyright (C) 2006 Tatsuyuki Satoh
                   Originally written for the MAME project.

****************************************************************************/

#include <ctype.h>

#include "cpuintrf.h"

typedef unsigned char byte;

#define FMT(a,b) a, b
#define PTRS_PER_FORMAT 2

static const char *Formats[] = {
	FMT("0000_0000", "NOP"),			// 00
	FMT("0000_0001", "RRCA"),			// 01
	FMT("0000_0010", "RLCA"),			// 02
	FMT("0000_0011", "INC  B"),			// 03 : shougi $360 to $377
	FMT("0000_0100", "DEC  B"),			// 04 : not found
	FMT("0000_0101", "INC  A"),			// 05
	FMT("0000_0110", "DEC  A"),			// 06
	FMT("0000_0111", "CPL  A"),			// 07
	FMT("0000_1aaa", "LD   A,(IX0+%X)"),// 08-0F
	FMT("0001_0aaa", "LD   A,(IX1+%X)"),// 10-17
	FMT("0001_1aaa", "LD   (IX2+%X),A"),// 18-1F
	FMT("0010_0xxx", "LD   [IX0+%X],B"), // 20-27 : shougi $360 to $377
	FMT("0010_1xxx", "ld   [ix1+%X],b ?"),// 28-2f : not found
	FMT("0011_0xxx", "ld   b,[ix2+%X] ?"),// 30-37 : not found
	FMT("0011_1aaa", "BIT  R0.%d"),		// 38-3f ZF = R0.a (bit test)
	FMT("0100_aaa0", "LD   A,R%X"),		// 40-4E
	FMT("0100_aaa1", "LD   R%X,A"),		// 41-4F
	FMT("0101_aaa0", "ADD  A,R%X"),		// 50-5E
	FMT("0101_aaa1", "SUB  A,R%X"),		// 51-5F
	FMT("0110_aaa0", "AND  A,R%X"),		// 60-6E
	FMT("0110_aaa1", "OR   A,R%X"),		// 61-6F
	FMT("0111_aaaa", "ADD  IX0,$%X"),	// 70-7f
	FMT("1000_aaaa", "ADD  IX1,$%X"),	// 80-8f
	FMT("1001_aaaa", "ADD  IX2,$%X"),	// 90-9f
	FMT("1010_0aaa", "LD   RB,%X"),		// A0-A7
//  FMT("1010_1xxx", "unknown"),        // A8-AF : not found
	FMT("1011_00aa", "LD   MB,%X"),		// B0-B3
	FMT("1011_0100", "HALT"),			// B4
//  FMT("1011_0101", "unknown"),        // B5 : not found
//  FMT("1011_0110", "unknown"),        // B6 : not found
//  FMT("1011_0111", "unknown"),        // B7 : not found
//  FMT("1011_1xxx", "unknown"),        // B8-BF : not found
	FMT("1100_0000 I", "LD   IX0,$%02X"),// C0
	FMT("1100_0001 I", "LD   IX1,$%02X"),// C1
	FMT("1100_0010 I", "LD   IX2,$%02X"),// C2
	FMT("1100_0011 I", "LD   A,$%02X"),	// C3
	FMT("1100_0100 I", "LD   LP0,$%02X"),// C4
	FMT("1100_0101 I", "LD   LP1,$%02X"),// C5
	FMT("1000_0110 I", "LD   LP2,$%02X"),// C6
	FMT("1100_0111 I", "LD   B,$%02X"),	// C7 : shougi.c : please check with real chip !
	FMT("1100_1000 I", "ADD  A,$%02X"),	// C8
	FMT("1100_1001 I", "SUB  A,$%02X"),	// C9
	FMT("1100_1010 I", "AND  A,$%02X"),	// CA
	FMT("1100_1011 I", "OR   A,$%02X"),	// CB
	FMT("1100_1100 I", "DJNZ LP0,$%02X"),// CC
	FMT("1100_1101 I", "DJNZ LP1,$%02X"),// CD
	FMT("1100_1110 I", "DJNZ LP2,$%02X"),// CE
	FMT("1100_1111 I", "JNZ  $%02X"),	// CF
	FMT("1101_0000 I", "JC   $%02X"),	// D0
	FMT("1101_0001 I", "JZ   $%02X"),	// D1
	FMT("1101_0010 I", "J    $%02X"),	// D2
	FMT("1101_0011 I", "unk  $%02X"),	// D3
	FMT("1101_0100 I", "alu  a,$%02X ?"), // D4 : 8031 , exctscc2 alu a,n ?
	FMT("1101_0101 I", "alu  a,$%02X ?"), // D5 : 8031 , exctscc2 alu a,n ?
	FMT("1101_0110 I", "unk  $%02X"),	// D6
	FMT("1101_0111 I", "unk  $%02X"),	// D7
	FMT("1101_1000 I", "unk  $%02X"),	// D8
	FMT("1101_1001 I", "unk  $%02X"),	// D9
	FMT("1101_1010 I", "CMP  A,$%02X"),	// DA : 8301
	FMT("1101_1011 I", "unk  $%02X"),	// DB
	FMT("1101_1100 I", "unk  $%02X"),	// DC
	FMT("1101_1101 I", "unk  $%02X"),	// DD
	FMT("1101_1110 I", "unk  $%02X"),	// DE
	FMT("1101_1111 I", "CALL $%02X"),	// DF : 8301
//
	FMT("1110_0000", "DEC  IX0"),		// E0 : 8301
//  FMT("1110_0001", "unknown"),        // E1
//  FMT("1110_0010", "unknown"),        // E2
	FMT("1110_0011", "ld   a,b ?"),		// E3 : 8301
//  FMT("1110_0100", "unknown"),        // E4
//  FMT("1110_0101", "unknown"),        // E5
//  FMT("1110_0110", "unknown"),        // E6 : 8301
//  FMT("1110_0110", "unknown"),        // E7
	FMT("1110_1000", "LD   IX0,A"),		// E8 : 8301
	FMT("1110_1001", "LD   IX1,A ?"),	// E9 : not found
	FMT("1110_1010", "LD   IX2,A"),		// EA : 8301
	FMT("1110_1011", "ld   b,a ?"),		// EB : 8301
//  FMT("1110_1100", "unknown"),        // EC
//  FMT("1110_1100", "unknown"),        // ED
//  FMT("1110_1110", "unknown"),        // EE : 8301
//  FMT("1110_1111", "unknown"),        // EF
//  FMT("1111_0000", "unknown"),        // F0
	FMT("1111_0001", "LD   IX2,IX0"),	// F1 : 8301
//  FMT("1111_0010", "unknown"),        // F2 : 8301 , alu a,[IXR] ?
//  FMT("1111_0011", "unknown"),        // F3
//  FMT("1111_0100", "unknown"),        // F4
//  FMT("1111_0101", "unknown"),        // F5
	FMT("1111_0110", "alu  ?,a"),		// F6 : 8301 , exctsccr, alu ?,a
//  FMT("1111_0110", "unknown"),        // F7
	FMT("1111_1000", "alu  a,?"),		// F8 : 8301 , exctsccr, alu b,a ? : ZF check
	FMT("1111_1001", "alu  a,?"),		// F9 : 8301 , exctsccr, alu a,?
	FMT("1111_1010", "ADD  A,CF"),		// FA : 8301
//  FMT("1111_1011", "unknown"),        // FB
	FMT("1111_1100", "tst  a ?"),		// FC : 8301 , ZF = (A==0) ?
	FMT("1111_1101", "CLR  A"),			// FD : 8301
	FMT("1111_1110", "LD   A,(IX0+A)"),	// FE : 8301
	FMT("1111_1111", "RET"),			// FF : 8301
	NULL
};

#define MAX_OPS (((sizeof(Formats) / sizeof(Formats[0])) - 1) / PTRS_PER_FORMAT)

typedef struct opcode {
	byte mask;
	byte bits;
	byte type;
	byte pmask;
	byte pdown;
	const char *fmt;
} AD8201Opcode;

static AD8201Opcode Op[MAX_OPS+1];
static int OpInizialized = 0;

static void InitDasm8201(void)
{
	const char *p;
	byte mask, bits;
	int bit;
	int i;
	char chr , type;
	int pmask , pdown;

	for(i=0;(p=Formats[i*2])!=NULL;i++)
	{
		mask = 0;
		bits = 0;
		pmask = 0;
		pdown = 0;
		type = 0;
		bit = 7;
		while ( (chr=*p) && bit >= 0) {
			p++;
			switch (chr) {
				case '1': bits |= 1<<bit;
				case '0': mask |= 1<<bit; bit--; break;
/*
                case 'b':
                    type |= 0x80;
*/
				case 'a':
					pmask |= 1<<bit;
					pdown  = bit;
				case 'x':
					bit --;
					break;
				case '_':
					continue;
				default:
					fatalerror("Invalid instruction encoding '%s %s'\n", Formats[i*2],Formats[i*2+1]);
			}
		}
		if (bit != -1 ) {
			fatalerror("not enough bits in encoding '%s %s' %d\n", Formats[i*2],Formats[i*2+1],bit);
		}

		Op[i].mask  = mask;
		Op[i].bits  = bits;
		Op[i].pmask = pmask;
		Op[i].pdown = pdown;
		Op[i].fmt   = Formats[i*2+1];
		Op[i].type  = type;

		/* 2 byte code ? */
		while (isspace(*p)) p++;
		if( (*p) )
			Op[i].type |= 0x10;
		/* number of param */
		if( (p=strchr(Op[i].fmt,'%'))!=NULL )
		{
			Op[i].type |= 0x01;     /* single param */
			if(strchr(p+1,'%') )
				Op[i].type |= 0x02; /* double param */
		}
	}

	OpInizialized = 1;
}

offs_t ALPHA8201_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	offs_t dasmflags = 0;
	int i;
	int op;
	int cnt = 1;
	int code , disp;

	if (!OpInizialized) InitDasm8201();

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
		return cnt;
	}

	if (Op[op].type & 0x10)
	{
		disp = opram[1];
		cnt++;
	}
	else
	{
		disp = (code & Op[op].pmask) >> Op[op].pdown;
	}

	if (Op[op].type & 0x02)
		sprintf(buffer, Op[op].fmt,disp,disp);
	else if (Op[op].type & 0x01)
		sprintf(buffer, Op[op].fmt,disp);
	else
		sprintf(buffer, "%s",Op[op].fmt);

	switch (code)
	{
		case 0xcc:
		case 0xcd:
		case 0xce:
		case 0xdf:
			dasmflags = DASMFLAG_STEP_OVER;
			break;

		case 0xff:
			dasmflags = DASMFLAG_STEP_OUT;
			break;
	}

	return cnt | dasmflags | DASMFLAG_SUPPORTED;
}
