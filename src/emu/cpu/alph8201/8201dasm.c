/****************************************************************************
                         Alpha 8201/8301 Disassembler

                      Copyright Tatsuyuki Satoh
                   Originally written for the MAME project.

****************************************************************************/

#include <ctype.h>

#include "cpuintrf.h"

typedef unsigned char byte;

#define FMT(a,b) a, b
#define PTRS_PER_FORMAT 2

/****************************************************
8201 CONFIRMED OPCODES:

opcode       mnemonic     function      flags
--------     ------------ ------------- -----
00000000     NOP          -             --
00000001     RORA         ror A         -C
00000010     ROLA         rol A         -C
00000011     INC RXB      RXB+=2        ZC
00000100     DEC RXB      RXB-=2        ZC (C=1 means No Borrow: RXB>=2)
00000101     INC A        A++           ZC
00000110     DEC A        A--           ZC (C=1 means No Borrow: A>=1)
00000110     CPL A        A^=$FF        --
00001aaa     LD A,(IX0+i) [IX0+i]->A    --
00010aaa     LD A,(IX1+i) [IX1+i]->A    --
00011aaa     LD (IX2+i),A A->[IX2+i]    --
00111aaa     BIT R0.n     ZF=R0 bit n   Z-
0100aaa0     LD A,Rn      A=Rn          Z-
0100aaa1     LD Rn,A      Rn=A          --
0101aaa0     ADD A,Rn     A+=Rn         ZC
0101aaa1     SUB A,Rn     A-=Rn         ZC (C=1 means No Borrow: A>=Rn)
0110aaa0     AND A,Rn     A&=Rn         Z-
0110aaa1     OR A,Rn      A|=Rn         Z-
0111aaaa     ADD IX0,i    IX0+=i        --
1000aaaa     ADD IX1,i    IX1+=i        --
1001aaaa     ADD IX2,i    IX2+=i        --
1010aaaa     LD RB,i      RB=i          -- Note: no bounds checking. Can set bank up to F.
1011-0aa     LD MB,i      set after-jump page
1011-1--     STOP
11000000 imm LD IX0,imm   IX0=imm       --
11000001 imm LD IX1,imm   IX1=imm       --
11000010 imm LD IX2,imm   IX2=imm       --
11000011 imm LD A,imm     A=imm         --
11000100 imm LD LP0,imm   LP0=imm       --
11000101 imm LD LP1,imm   LP1=imm       --
11000110 imm LD LP2,imm   LP2=imm       --
11000111 imm LD RXB,imm   RXB=imm       --
11001000 imm ADD A,imm    A+=imm        ZC
11001001 imm SUB A,imm    A-=imm        ZC (C=1 means No Borrow: A>=imm)
11001010 imm AND A,imm    A&=imm        Z-
11001011 imm OR  A,imm    A|=imm        Z-
11001100 imm DJNZ LP0,imm LP0--,branch  --
11001101 imm DJNZ LP1,imm LP1--,branch  --
11001110 imm DJNZ LP2,imm LP2--,branch  --
11001111 imm JNZ imm      branch if !Z  --
1101--00 imm JNC imm      branch if !C  --
1101--01 imm JZ  imm      branch if Z   --
1101--1- imm J   imm      branch        --
1110--xx mirror for the above
1111--xx mirror for the above

****************************************************/


/****************************************************/

static const char *const Formats[] = {
	FMT("0000_0000", "NOP"),				// 00
	FMT("0000_0001", "RRCA"),				// 01
	FMT("0000_0010", "RLCA"),				// 02
	FMT("0000_0011", "INC  RXB"),			// 03 : shougi $360 to $377; splndrbt
	FMT("0000_0100", "DEC  RXB"),			// 04 : not found
	FMT("0000_0101", "INC  A"),				// 05
	FMT("0000_0110", "DEC  A"),				// 06
	FMT("0000_0111", "CPL  A"),				// 07
	FMT("0000_1aaa", "LD   A,(IX0+%X)"),	// 08-0F
	FMT("0001_0aaa", "LD   A,(IX1+%X)"),	// 10-17
	FMT("0001_1aaa", "LD   (IX2+%X),A"),	// 18-1F
	FMT("0010_0aaa", "LD   (RXB),(IX0+%X)"),// 20-27 : shougi $360 to $377
	FMT("0010_1aaa", "LD   (RXB),(IX1+%X)"),// 28-2f : not found
	FMT("0011_0aaa", "LD   (IX2+%X),(RXB)"),// 30-37 : not found
	FMT("0011_1aaa", "BIT  R0.%d"),			// 38-3F ZF = R0.a (bit test)
	FMT("0100_aaa0", "LD   A,R%X"),			// 40-4E
	FMT("0100_aaa1", "LD   R%X,A"),			// 41-4F
	FMT("0101_aaa0", "ADD  A,R%X"),			// 50-5E
	FMT("0101_aaa1", "SUB  A,R%X"),			// 51-5F
	FMT("0110_aaa0", "AND  A,R%X"),			// 60-6E
	FMT("0110_aaa1", "OR   A,R%X"),			// 61-6F
	FMT("0111_aaaa", "ADD  IX0,$%X"),		// 70-7f
	FMT("1000_aaaa", "ADD  IX1,$%X"),		// 80-8f
	FMT("1001_aaaa", "ADD  IX2,$%X"),		// 90-9f
	FMT("1010_aaaa", "LD   RB,%X"),			// A0-AF
	FMT("1011_x0aa", "LD   MB,%X"),			// B0-B3 (+ mirrors)
	FMT("1011_x1xx", "STOP"),				// B4 (+ mirrors)

	FMT("1100_0000 I", "LD   IX0,$%02X"),	// C0
	FMT("1100_0001 I", "LD   IX1,$%02X"),	// C1
	FMT("1100_0010 I", "LD   IX2,$%02X"),	// C2
	FMT("1100_0011 I", "LD   A,$%02X"),		// C3
	FMT("1100_0100 I", "LD   LP0,$%02X"),	// C4
	FMT("1100_0101 I", "LD   LP1,$%02X"),	// C5
	FMT("1100_0110 I", "LD   LP2,$%02X"),	// C6
	FMT("1100_0111 I", "LD   RXB,$%02X"),	// C7 : shougi, splndrbt, equites
	FMT("1100_1000 I", "ADD  A,$%02X"),		// C8
	FMT("1100_1001 I", "SUB  A,$%02X"),		// C9
	FMT("1100_1010 I", "AND  A,$%02X"),		// CA
	FMT("1100_1011 I", "OR   A,$%02X"),		// CB
	FMT("1100_1100 I", "DJNZ LP0,$%02X"),	// CC
	FMT("1100_1101 I", "DJNZ LP1,$%02X"),	// CD
	FMT("1100_1110 I", "DJNZ LP2,$%02X"),	// CE
	FMT("1100_1111 I", "JNZ  $%02X"),		// CF
	FMT("1101_0000 I", "JNC  $%02X"),		// D0
	FMT("1101_0001 I", "JZ   $%02X"),		// D1
	FMT("1101_001x I", "J    $%02X"),		// D2 (+ mirror)

	/* -------------- 830x only ------------- */

	FMT("1101_0100 I", "LD   A,(R7:$%02X)"),	// D4 : exctscc2, bullfgtr; not sure if R7 or R77
	FMT("1101_0101 I", "LD   (R7:$%02X),A"),	// D5 : exctscc2, bullfgtr, kouyakyu; not sure if R7 or R77
	FMT("1101_0110 I", "LD   LP0,(R7:$%02X)"),	// D6 : kouyakyu; not sure if R7 or R77
	FMT("1101_0111 I", "LD   (R7:$%02X),LP0"),	// D7 : hvoltage; not sure if R7 or R77
	FMT("1101_1000 I", "LD   A,($%02X)"),		// D8 : equites
	FMT("1101_1001 I", "LD   ($%02X),A"),		// D9 : equites
	FMT("1101_1010 I", "CMP  A,$%02X"),			// DA :
	FMT("1101_1011 I", "XOR  A,$%02X"),			// DB : equites splndrbt
	FMT("1101_1100 I", "unk  $%02X"),			// DC : not found (LD   A,R($%02X) ?)
	FMT("1101_1101 I", "LD   R($%02X),A"),		// DD : equites, splndrbt
	FMT("1101_1110 I", "unk  $%02X"),			// DE : not found
	FMT("1101_1111 I", "CALL $%02X"),			// DF :

	FMT("1110_0000", "DEC  IX0"),			// E0 :
	FMT("1110_0001", "unknown"),			// E1 : not found (DEC IX1?)
	FMT("1110_0010", "unknown"),			// E2 : not found (DEC IX2?)
	FMT("1110_0011", "ld   a,unk ?"),		// E3 : exctsccr in pair with EB
	FMT("1110_0100", "unknown"),			// E4 : not found
	FMT("1110_0101", "LD   A,B"),			// E5 : splndrbt
	FMT("1110_0110", "EXG  A,LP0"),			// E6 : splndrbt, bullfgtr, kouyakyu. EXG, not LD: see splndrbt $3ba to $3d3
	FMT("1110_0111", "unknown"),			// E7 : not found
	FMT("1110_1000", "LD   IX0,A"),			// E8 :
	FMT("1110_1001", "ld   ix1,a ?"),		// E9 : not found
	FMT("1110_1010", "LD   IX2,A"),			// EA :
	FMT("1110_1011", "ld   unk,a ?"),		// EB : exctsccr in pair with E3. unk is not LP0, IX0, IX2
	FMT("1110_1100", "unknown"),			// EC : not found
	FMT("1110_1101", "LD   B,A"),			// ED : splndrbt
	FMT("1110_1110", "LD   LP0,A"),			// EE : splndrbt, bullfgtr
	FMT("1110_1111", "unknown"),			// EF : not found
	FMT("1111_0000", "unknown"),			// F0 : not found
	FMT("1111_0001", "EXG  IX0,IX2"),		// F1 : should be EXG, see splndrbt $2e to $38, equites $40 to $4a
	FMT("1111_0010", "LDIR"),				// F2 : splndrbt  LD (IX2+%X),(RXB); INC B; DJNZ LP0
	FMT("1111_0011", "unknown"),			// F3 : not found
	FMT("1111_0100", "unknown"),			// F4 : not found
	FMT("1111_0101", "unknown"),			// F5 : not found
	FMT("1111_0110", "LD   (RXB),A"),		// F6 : exctsccr
	FMT("1111_0111", "unknown"),			// F7 : not found
	FMT("1111_1000", "sub/cmp  a,(rxb) ?"),	// F8 : exctsccr : ZF check  could be CMP instead? see DA/DB
	FMT("1111_1001", "XOR  A,(RXB)"),		// F9 : exctsccr
	FMT("1111_1010", "ADD  A,CF"),			// FA :
	FMT("1111_1011", "unknown"),			// FB : not found (SUB  A,CF ?)
	FMT("1111_1100", "TST  A"),				// FC : ZF = (A==0) ?
	FMT("1111_1101", "CLR  A"),				// FD :
	FMT("1111_1110", "LD   A,(IX0+A)"),		// FE :
	FMT("1111_1111", "RET"),				// FF :
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
		while (*p && bit >= 0) {
			chr = *p++;
			switch (chr) {
				case '1': bits |= 1<<bit;
				case '0': mask |= 1<<bit; bit--; break;
#if 0
				case 'b':
					type |= 0x80;
#endif
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
