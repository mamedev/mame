// license:BSD-3-Clause
// copyright-holders:Tatsuyuki Satoh
/****************************************************************************
                         Alpha 8201/8301 Disassembler

                      Copyright Tatsuyuki Satoh
                   Originally written for the MAME project.

****************************************************************************/

#include "emu.h"

#include <ctype.h>

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
00001aaa     LD A,(IX0+i) A=[IX0+i]     --
00010aaa     LD A,(IX1+i) A=[IX1+i]     --
00011aaa     LD (IX2+i),A [IX2+i]=A     --
00111aaa     BIT R0.n     ZF=R0 bit n   Z-
0100aaa0     LD A,Rn      A=Rn          Z- [1]
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

Notes:
[1] bug: the Z flag is not updated correctly after a LD A,Rn instruction. Fixed in 8302 (possibly 8301).


8302 CONFIRMED OPCODES:
----------------------
all of the 8201 ones, with stricter decoding for the following:

11010-00 imm JNC imm      branch if !C  --
11010-01 imm JZ  imm      branch if Z   --
11010-1- imm J   imm      branch        --

and these new opcodes:

opcode       mnemonic     function         flags
--------     ------------ ---------------  -----
11011000 imm LD A,(imm)   A=MB:[imm]       --
11011001 imm LD (imm),A   MB:[imm]=A       --
11011010 imm CMP A,imm    temp=A-imm       ZC
11011011 imm XOR A,imm    A^=imm           Z0
11011100 imm LD A,R(imm)  A=reg(imm)       --
11011101 imm LD R(imm),A  reg(imm)=A       --
11011110 imm JC imm       branch if C      --
11011111 imm CALL $xx     save PC, branch  --

11100000     EXG A,IX0    A<->IX0          --
11100001     EXG A,IX1    A<->IX1          --
11100010     EXG A,IX2    A<->IX2          --
11100011     EXG A,LP1    A<->LP1          --
11100100     EXG A,LP2    A<->LP2          --
11100101     EXG A,RXB    A<->RXB          --
11100110     EXG A,LP0    A<->LP0          --
11100111     EXG A,RB     A<->RB           --
11101000     LD IX0,A     IX0=A            --
11101001     LD IX1,A     IX1=A            --
11101010     LD IX2,A     IX2=A            --
11101011     LD LP1,A     LP1=A            --
11101100     LD LP2,A     LP2=A            --
11101101     LD RXB,A     RXB=A            --
11101110     LD LP0,A     LP0=A            --
11101111     LD RB,A      RB=A             --
11110000     EXG IX0,IX1  IX0<->IX1        --
11110001     EXG IX0,IX2  IX0<->IX2        --
11110010     REP LD (IX2),(RXB)  equivalent to LD (IX2),(RXB); INC RXB; DJNZ LP0
11110011     REP LD (RXB),(IX0)  equivalent to LD (RXB),(IX0); INC RXB; DJNZ LP0
11110100     SAVE ZC      save ZC          --
11110101     REST ZC      restore ZC       ZC
11110110     LD (RXB),A   reg(RXB)=A       --
11110111     LD A,(RXB)   A=reg(RXB)       --
11111000     CMP A,(RXB)  temp=A-reg(RXB)  ZC
11111001     XOR A,(RXB)  A^=reg(RXB)      Z0
11111010     ADD A,CF     if (C) A++       ZC
11111011     SUB A,!CF    if (!C) A--      ZC
11111100     TST A        A==0?            Z-
11111101     CLR A        A=0              --
11111110     LD A,(IX0+A) A=[IX0+A]        --
11111111     RET          restore PC       --


8303 CONFIRMED OPCODES:
----------------------
all of the 8302 ones, with stricter decoding for the following:

11010000 imm JNC imm      branch if !C  --
11010001 imm JZ  imm      branch if Z   --
1101001- imm J   imm      branch        --

additionally, this opcode is modified to support 11-bit instead of 10-bit
external addressing, this wasn't used in games however.

1011-0aa     LD MB,i      modified so that bit 3 is shifted to bit 2 before loading MB.

and these new opcodes are added:

110101--
11010100 imm LD A,(R77:$%02X)
11010101 imm LD (R77:$%02X),A
11010110 imm LD PC,(R77:$%02X)  [1]
11010111 imm LD (R77:$%02X),PC  [2]

Notes:
[1] appears to be LD PC,x in the disassembly, however it's LD LP0,x for kouyakyu
    which uses a 8304, so the opcode was probably changed again.
[2] appears to be LD x,PC in the disassembly, however it's LD x,LP0 for hvoltage
    which uses a 8304 (or 8404?), so the opcode was probably changed again.

****************************************************/


/****************************************************/

static const char *const Formats[] = {
	FMT("0000_0000", "NOP"),                // 00
	FMT("0000_0001", "RRCA"),               // 01
	FMT("0000_0010", "RLCA"),               // 02
	FMT("0000_0011", "INC  RXB"),           // 03 : shougi $360 to $377; splndrbt
	FMT("0000_0100", "DEC  RXB"),           // 04 : not found
	FMT("0000_0101", "INC  A"),             // 05
	FMT("0000_0110", "DEC  A"),             // 06
	FMT("0000_0111", "CPL  A"),             // 07
	FMT("0000_1aaa", "LD   A,(IX0+%X)"),    // 08-0F
	FMT("0001_0aaa", "LD   A,(IX1+%X)"),    // 10-17
	FMT("0001_1aaa", "LD   (IX2+%X),A"),    // 18-1F
	FMT("0010_0aaa", "LD   (RXB),(IX0+%X)"),// 20-27 : shougi $360 to $377
	FMT("0010_1aaa", "LD   (RXB),(IX1+%X)"),// 28-2f : not found
	FMT("0011_0aaa", "LD   (IX2+%X),(RXB)"),// 30-37 : not found
	FMT("0011_1aaa", "BIT  R0.%d"),         // 38-3F ZF = R0.a (bit test)
	FMT("0100_aaa0", "LD   A,R%X"),         // 40-4E
	FMT("0100_aaa1", "LD   R%X,A"),         // 41-4F
	FMT("0101_aaa0", "ADD  A,R%X"),         // 50-5E
	FMT("0101_aaa1", "SUB  A,R%X"),         // 51-5F
	FMT("0110_aaa0", "AND  A,R%X"),         // 60-6E
	FMT("0110_aaa1", "OR   A,R%X"),         // 61-6F
	FMT("0111_aaaa", "ADD  IX0,$%X"),       // 70-7f
	FMT("1000_aaaa", "ADD  IX1,$%X"),       // 80-8f
	FMT("1001_aaaa", "ADD  IX2,$%X"),       // 90-9f
	FMT("1010_aaaa", "LD   RB,%X"),         // A0-AF
	FMT("1011_x0aa", "LD   MB,%X"),         // B0-B3 (+ mirrors)
	FMT("1011_x1xx", "STOP"),               // B4 (+ mirrors)

	FMT("1100_0000 I", "LD   IX0,$%02X"),   // C0
	FMT("1100_0001 I", "LD   IX1,$%02X"),   // C1
	FMT("1100_0010 I", "LD   IX2,$%02X"),   // C2
	FMT("1100_0011 I", "LD   A,$%02X"),     // C3
	FMT("1100_0100 I", "LD   LP0,$%02X"),   // C4
	FMT("1100_0101 I", "LD   LP1,$%02X"),   // C5
	FMT("1100_0110 I", "LD   LP2,$%02X"),   // C6
	FMT("1100_0111 I", "LD   RXB,$%02X"),   // C7 : shougi, splndrbt, equites
	FMT("1100_1000 I", "ADD  A,$%02X"),     // C8
	FMT("1100_1001 I", "SUB  A,$%02X"),     // C9
	FMT("1100_1010 I", "AND  A,$%02X"),     // CA
	FMT("1100_1011 I", "OR   A,$%02X"),     // CB
	FMT("1100_1100 I", "DJNZ LP0,$%02X"),   // CC
	FMT("1100_1101 I", "DJNZ LP1,$%02X"),   // CD
	FMT("1100_1110 I", "DJNZ LP2,$%02X"),   // CE
	FMT("1100_1111 I", "JNZ  $%02X"),       // CF
	FMT("1101_0000 I", "JNC  $%02X"),       // D0
	FMT("1101_0001 I", "JZ   $%02X"),       // D1
	FMT("1101_001x I", "J    $%02X"),       // D2 (+ mirror)

	/* -------------- 830x only ------------- */

	FMT("1101_0100 I", "LD   A,(R77:$%02X)"),   // D4 : 8303+ only. exctscc2, bullfgtr
	FMT("1101_0101 I", "LD   (R77:$%02X),A"),   // D5 : 8303+ only. exctscc2, bullfgtr, kouyakyu
	FMT("1101_0110 I", "LD   LP0,(R77:$%02X)"), // D6 : 8303+ only. kouyakyu
	FMT("1101_0111 I", "LD   (R77:$%02X),LP0"), // D7 : 8303+ only. hvoltage
	FMT("1101_1000 I", "LD   A,($%02X)"),       // D8 : equites
	FMT("1101_1001 I", "LD   ($%02X),A"),       // D9 : equites
	FMT("1101_1010 I", "CMP  A,$%02X"),         // DA :
	FMT("1101_1011 I", "XOR  A,$%02X"),         // DB : equites splndrbt
	FMT("1101_1100 I", "LD   A,R($%02X)"),      // DC : not found
	FMT("1101_1101 I", "LD   R($%02X),A"),      // DD : equites, splndrbt
	FMT("1101_1110 I", "JC   $%02X"),           // DE : not found
	FMT("1101_1111 I", "CALL $%02X"),           // DF :

	FMT("1110_0000", "EXG  A,IX0"),         // E0 : exctsccr
	FMT("1110_0001", "EXG  A,IX1"),         // E1 : not found
	FMT("1110_0010", "EXG  A,IX2"),         // E2 : not found
	FMT("1110_0011", "EXG  A,LP1"),         // E3 : exctsccr in pair with EB
	FMT("1110_0100", "EXG  A,LP2"),         // E4 : not found
	FMT("1110_0101", "EXG  A,RXB"),         // E5 : splndrbt
	FMT("1110_0110", "EXG  A,LP0"),         // E6 : splndrbt, bullfgtr, kouyakyu. EXG, not LD: see splndrbt $3ba to $3d3
	FMT("1110_0111", "EXG  A,RB"),          // E7 : not found
	FMT("1110_1000", "LD   IX0,A"),         // E8 :
	FMT("1110_1001", "LD   IX1,A"),         // E9 : not found
	FMT("1110_1010", "LD   IX2,A"),         // EA :
	FMT("1110_1011", "LD   LP1,A"),         // EB : exctsccr in pair with E3
	FMT("1110_1100", "LP   LP2,A"),         // EC : not found
	FMT("1110_1101", "LD   RXB,A"),         // ED : splndrbt
	FMT("1110_1110", "LD   LP0,A"),         // EE : splndrbt, bullfgtr
	FMT("1110_1111", "LD   RB,A"),          // EF : not found
	FMT("1111_0000", "EXG  IX0,IX1"),       // F0 : not found
	FMT("1111_0001", "EXG  IX0,IX2"),       // F1 : splndrbt $2e to $38, equites $40 to $4a
	FMT("1111_0010", "REP  LD (IX2),(RXB)"),// F2 : splndrbt  LD (IX2),(RXB); INC RXB; DJNZ LP0
	FMT("1111_0011", "REP  LD (RXB),(IX0)"),// F3 : not found LD (RXB),(IX0); INC RXB; DJNZ LP0
	FMT("1111_0100", "SAVE ZC"),            // F4 : not found
	FMT("1111_0101", "REST ZC"),            // F5 : not found
	FMT("1111_0110", "LD   (RXB),A"),       // F6 : exctsccr
	FMT("1111_0111", "LD   A,(RXB)"),       // F7 : not found
	FMT("1111_1000", "CMP  A,(RXB)"),       // F8 : exctsccr
	FMT("1111_1001", "XOR  A,(RXB)"),       // F9 : exctsccr
	FMT("1111_1010", "ADD  A,CF"),          // FA :
	FMT("1111_1011", "SUB  A,!CF"),         // FB : not found
	FMT("1111_1100", "TST  A"),             // FC :
	FMT("1111_1101", "CLR  A"),             // FD :
	FMT("1111_1110", "LD   A,(IX0+A)"),     // FE :
	FMT("1111_1111", "RET"),                // FF :
	NULL
};

#define MAX_OPS ((ARRAY_LENGTH(Formats) - 1) / PTRS_PER_FORMAT)

struct AD8201Opcode {
	byte mask;
	byte bits;
	byte type;
	byte pmask;
	byte pdown;
	const char *fmt;
};

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
		while (isspace((UINT8)*p)) p++;
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

CPU_DISASSEMBLE( alpha8201 )
{
	offs_t dasmflags = 0;
	int i;
	int op;
	int cnt = 1;
	int code , disp;

	if (!OpInizialized) InitDasm8201();

	code = oprom[0];
	op = -1;    /* no matching opcode */
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
