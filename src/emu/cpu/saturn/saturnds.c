/*****************************************************************************
 *
 *   saturn.c
 *   portable saturn emulator interface
 *   (hp calculators)
 *
 *   Copyright (c) 2000 Peter Trauner, all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     peter.trauner@jk.uni-linz.ac.at
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *****************************************************************************/

#include "cpuintrf.h"
#include "debugger.h"

#include "saturn.h"
#include "sat.h"

#if defined SATURN_HP_MNEMONICS
// class/hp mnemonics
static int set=0;
#else
// readable/normal mnemonics
static int set=1;
#endif

#define P "P"
#define WP "WP"
#define XS "XS"
#define X "X"
#define S "S"
#define M "M"
#define B "B"
#define W "W"
#define A "A"

static const char *adr_b[]=
{ P, WP, XS, X, S, M, B, W };

static const char *adr_af[]=
{ P, WP, XS, X, S, M, B, W, 0, 0, 0, 0, 0, 0, 0, A };

static const char *adr_a[]=
{ P, WP, XS, X, S, M, B, W };

static const char number_2_hex[]=
{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

#define	SATURN_PEEKOP_DIS8(v)	v = (INT8)( oprom[pos] | ( oprom[pos+1] << 4 ) ); pos+= 2;

#define SATURN_PEEKOP_DIS12(v)	v = oprom[pos] | ( oprom[pos+1] << 4 ) | ( oprom[pos+2] << 8 );	\
								pos += 3;														\
								if ( v & 0x0800 )	v = -0x1000 + v;

#define SATURN_PEEKOP_DIS16(v)	v = (INT16)( oprom[pos] | ( oprom[pos+1] << 4 ) | ( oprom[pos+2] << 8 ) | ( oprom[pos+3] << 12 ) ); pos += 4;

#define SATURN_PEEKOP_ADR(v)	v = oprom[pos] | ( oprom[pos+1] << 4 ) | ( oprom[pos+2] << 8 ) | ( oprom[pos+3] << 12 ) | ( oprom[pos+4] << 16 ); pos += 5;


// don't split branch and return, source relies on this ordering
typedef enum {
	Return, ReturnSetXM, ReturnSetCarry, ReturnClearCarry, ReturnFromInterrupt,
	jump3,jump4,jump,
	call3,call4,call,
	branchCarrySet, returnCarrySet,
	branchCarryClear, returnCarryClear,

	outCS, outC, inA, inC,
	unconfig, config, Cid, shutdown, cp1, reset, buscc,
	CcopyP, PcopyC, sreq, CswapP,

	inton, AloadImm, buscb,
	clearAbit, setAbit, Abitclear, Abitset,
	clearCbit, setCbit, Cbitclear, Cbitset,
	PCloadA, buscd, PCloadC, intoff, rsi,

	jumpA, jumpC, PCcopyA, PCcopyC, AcopyPC, CcopyPC,

	clearHST,
	branchHSTclear, returnHSTclear,

	clearBitST, setBitST,
	branchSTclear, returnSTclear,
	branchSTset, returnSTset,


	branchPdiffers, returnPdiffers,
	branchPequals, returnPequals,

	branchAequalsB, returnAequalsB,
	branchBequalsC, returnBequalsC,
	branchAequalsC, returnAequalsC,
	branchCequalsD, returnCequalsD,
	branchAdiffersB, returnAdiffersB,
	branchBdiffersC, returnBdiffersC,
	branchAdiffersC, returnAdiffersC,
	branchCdiffersD, returnCdiffersD,
	branchAzero, returnAzero,
	branchBzero, returnBzero,
	branchCzero, returnCzero,
	branchDzero, returnDzero,
	branchAnotzero, returnAnotzero,
	branchBnotzero, returnBnotzero,
	branchCnotzero, returnCnotzero,
	branchDnotzero, returnDnotzero,

	branchAgreaterB, returnAgreaterB,
	branchBgreaterC, returnBgreaterC,
	branchCgreaterA, returnCgreaterA,
	branchDgreaterC, returnDgreaterC,
	branchAlowerB, returnAlowerB,
	branchBlowerC, returnBlowerC,
	branchClowerA, returnClowerA,
	branchDlowerC, returnDlowerC,
	branchAnotlowerB, returnAnotlowerB,
	branchBnotlowerC, returnBnotlowerC,
	branchCnotlowerA, returnCnotlowerA,
	branchDnotlowerC, returnDnotlowerC,
	branchAnotgreaterB, returnAnotgreaterB,
	branchBnotgreaterC, returnBnotgreaterC,
	branchCnotgreaterA, returnCnotgreaterA,
	branchDnotgreaterC, returnDnotgreaterC,

	SetHexMode, SetDecMode,
	PopC, PushC,

	D0loadImm2, D0loadImm4, D0loadImm5,
	D1loadImm2, D1loadImm4, D1loadImm5,
	PloadImm, CloadImm,

	clearST,
	CcopyST, STcopyC,
	swapCST,

	incP, decP,

	R0copyA, R1copyA, R2copyA, R3copyA, R4copyA,
	R0copyC, R1copyC, R2copyC, R3copyC, R4copyC,

	AcopyR0, AcopyR1, AcopyR2, AcopyR3, AcopyR4,
	CcopyR0, CcopyR1, CcopyR2, CcopyR3, CcopyR4,

	D0copyA, D1copyA, D0copyC, D1copyC,
	D0copyAShort, D1copyAShort, D0copyCShort, D1copyCShort, // other class mnemonic

	SwapAR0, SwapAR1, SwapAR2, SwapAR3, SwapAR4,
	SwapCR0, SwapCR1, SwapCR2, SwapCR3, SwapCR4,

	SwapAD0, SwapAD1, SwapCD0, SwapCD1,
	SwapAD0Short, SwapAD1Short, SwapCD0Short, SwapCD1Short, // other class mnemonic

	D0storeA, D1storeA, D0storeC, D1storeC,
	AloadD0, AloadD1, CloadD0, CloadD1,

	D0addImm, D1addImm, D0subImm, D1subImm,
	AaddImm, BaddImm, CaddImm, DaddImm,
	AsubImm, BsubImm, CsubImm, DsubImm,

	AandB, BandC, CandA, DandC, BandA, CandB, AandC, CandD,
	AorB, BorC, CorA, DorC, BorA, CorB, AorC, CorD,

	Ashiftrightbit, Bshiftrightbit, Cshiftrightbit, Dshiftrightbit,

	AshiftleftCarry, BshiftleftCarry, CshiftleftCarry, DshiftleftCarry,
	AshiftrightCarry, BshiftrightCarry, CshiftrightCarry, DshiftrightCarry,

	AaddB, BaddC, CaddA, DaddC, AaddA, BaddB, CaddC, DaddD,
	BaddA, CaddB, AaddC, CaddD, decA, decB, decC, decD,

	AsubB, BsubC, CsubA, DsubC, incA, incB, incC, incD,
	BsubA, CsubB, AsubC, CsubD, AsubnB, BsubnC, CsubnA, DsubnC,

	clearA, clearB, clearC, clearD,
	AcopyB, BcopyC, CcopyA, DcopyC, BcopyA, CcopyB, AcopyC, CcopyD,
	AswapB, BswapC, CswapA, DswapC,

	Ashiftleft, Bshiftleft, Cshiftleft, Dshiftleft,
	Ashiftright, Bshiftright, Cshiftright, Dshiftright,
	negateA, negateB, negateC, negateD,
	notA, notB, notC, notD

} MNEMONICS;

static struct {
	const char *name[2];
} mnemonics[]={
	{ { "rtn",					"RET" } },
	{ { "rtnsXM",				"RETSETXM" } },
	{ { "rtnsC",				"RETSETC" } },
	{ { "rtncC",				"RETCLRC" } },
	{ { "rti",					"RETI" } },
	{ { "goto    %05x",			"JUMP.3   %05x" } },
	{ { "goto    %05x",			"JUMP.4   %05x" } },
	{ { "goto    %05x",			"JUMP     %05x" } },
	{ { "gosub   %05x",			"CALL.3   %05x" } },
	{ { "gosub   %05x",			"CALL.4   %05x" } },
	{ { "gosub   %05x",			"CALL     %05x" } },
	{ { "goC     %05x",			"BRCS     %05x" } },
	{ { "rtnC",					"RETCS" } },
	{ { "gonC    %05x",			"BRCC     %05x" } },
	{ { "rtnnC",				"RETCC" } },

	{ { "OUT=CS",				"OUT.S    C" } },
	{ { "OUT=C",				"OUT.X    C" } },
	{ { "A=IN",					"IN.4     A" } },
	{ { "C=IN",					"IN.4     C" } },
	{ { "uncnfg",				"UNCNFG" } },
	{ { "config",				"CONFIG" } },
	{ { "C=id",					"MOVE.A   ID,C" } },
	{ { "!shutdn",				"!SHUTDN" } },
	{ { "C+P+1",				"ADD.A    P+1,C" } },
	{ { "reset",				"RESET" } },
	{ { "!buscc",				"!BUSCC" } },
	{ { "C=P     %x",			"MOVE.1   P,C,%x" } },
	{ { "P=C     %x",			"MOVE.1   C,%x,P" } },
	{ { "!sreq?",				"!SREQ" } },
	{ { "CPex    %x",			"SWAP.1   P,C,%x" } },

	{ { "!inton",				"!INTON" } },
	{ { "LA %-2x   %s",			"MOVE.P%-2x %s,A" } },
	{ { "!buscb",				"!BUSCB" } },
	{ { "Abit=0  %x",			"CLRB     %x,A" } },
	{ { "Abit=1  %x",			"SETB     %x,A" } },
	{ { "?Abit=0 %x,%05x",		"BRBC     %x,A,%05x" } },
	{ { "?Abit=1 %x,%05x",		"BRBS     %x,A,%05x" } },
	{ { "Cbit=0  %x",			"CLRB     %x,C" } },
	{ { "Cbit=1  %x",			"SETB     %x,C" } },
	{ { "?Cbit=0 %x,%05x",		"BRBC     %x,C,%05x" } },
	{ { "?Cbit=1 %x,%05x",		"BRBS     %x,C,%05x" } },
	{ { "PC=(A)",               "JUMP.A   @A" } },
	{ { "!buscd",				"!BUSCD" } },
	{ { "PC=(C)",				"JUMP.A   @C" } },
	{ { "!intoff",				"!INTOFF" } },
	{ { "!rsi",					"!RSI" } },

	{ { "PC=A",					"JUMP.A   A" } },
	{ { "PC=C",					"JUMP.A   C" } },
	{ { "A=PC",					"MOVE.A   PC,A" } },
	{ { "C=PC",					"MOVE.A   PC,C" } },
	{ { "APCex",				"SWAP.A   A,PC" } },
	{ { "CPCex",				"SWAP.A   C,PC" } },

	{ { "HST=0   %x",			"CLRHST   %x" } },
	{ { "?HST=0  %x,%05x",		"BRBCHST  %x,%05x" } },
	{ { "?HST=0  %x",			"RETBCHST %x" } },
	{ { "ST=0    %x",			"CLRB     %x,ST" } },
	{ { "ST=1    %x",			"SETB     %x,ST" } },
	{ { "?ST=0   %x,%05x",		"BRBC     ST,%x,%05x" } },
	{ { "?ST=0   %x",			"RETBC    ST,%x" } },
	{ { "?ST=1   %x,%05x",		"BRBS     ST,%x,%05x" } },
	{ { "?ST=1   %x",			"RETBS    ST,%x" } },
	{ { "?P#     %x,%05x",		"BRNE     P,%x,%05x" } },
	{ { "?P#     %x",			"RETNE    P,%x" } },
	{ { "?P=     %x,%05x",		"BREQ     P,%x,%05x" } },
	{ { "?P=     %x",			"RETEQ    P,%x" } },

	{ { "?A=B    %x,%05x",		"BREQ.%-2s  A,B,%05x" } },
	{ { "?A=B    %x",			"RETEQ.%-2s A,B" } },
	{ { "?B=C    %x,%05x",		"BREQ.%-2s  B,C,%05x" } },
	{ { "?B=C    %x",			"RETEQ.%-2s B,C" } },
	{ { "?A=C    %x,%05x",		"BREQ.%-2s  A,C,%05x" } },
	{ { "?A=C    %x",			"RETEQ.%-2s A,C" } },
	{ { "?C=D    %x,%05x",		"BREQ.%-2s  C,D,%05x" } },
	{ { "?C=D    %x",			"RETEQ.%-2s C,D" } },
	{ { "?A#B    %x,%05x",		"BRNE.%-2s  A,B,%05x" } },
	{ { "?A#B    %x",			"RETNE.%-2s A,B" } },
	{ { "?B#C    %x,%05x",		"BRNE.%-2s  B,C,%05x" } },
	{ { "?B#C    %x",			"RETNE.%-2s B,C" } },
	{ { "?A#C    %x,%05x",		"BRNE.%-2s  A,C,%05x" } },
	{ { "?A#C    %x",			"RETNE.%-2s A,C" } },
	{ { "?C#D    %x,%05x",		"BRNE.%-2s  C,D,%05x" } },
	{ { "?C#D    %x",			"RETNE.%-2s C,D" } },
	{ { "?A=0    %x,%05x",		"BRZ.%-2s   A,%05x" } },
	{ { "?A=0    %x",			"RETZ.%-2s  A" } },
	{ { "?B=0    %x,%05x",		"BRZ.%-2s   B,%05x" } },
	{ { "?B=0    %x",			"RETZ.%-2s  B" } },
	{ { "?C=0    %x,%05x",		"BRZ.%-2s   C,%05x" } },
	{ { "?C=0    %x",			"RETZ.%-2s  C" } },
	{ { "?D=0    %x,%05x",		"BRZ.%-2s   D,%05x" } },
	{ { "?D=0    %x",			"RETZ.%-2s  D" } },
	{ { "?A#0    %x,%05x",		"BRNZ.%-2s  A,%05x" } },
	{ { "?A#0    %x",			"RETNZ.%-2s A" } },
	{ { "?B#0    %x,%05x",		"BRNZ.%-2s  B,%05x" } },
	{ { "?B#0    %x",			"RETNZ.%-2s B" } },
	{ { "?C#0    %x,%05x",		"BRNZ.%-2s  C,%05x" } },
	{ { "?C#0    %x",			"RETNZ.%-2s C" } },
	{ { "?D#0    %x,%05x",		"BRNZ.%-2s  D,%05x" } },
	{ { "?D#0    %x",			"RETNZ.%-2s D" } },

	{ { "?A>B    %x,%05x",		"BRGT.%-2s  A,B,%05x" } },
	{ { "?A>B    %x",			"RETGT.%-2s A,B" } },
	{ { "?B>C    %x,%05x",		"BRGT.%-2s  B,C,%05x" } },
	{ { "?B>C    %x",			"RETGT.%-2s B,C" } },
	{ { "?C>A    %x,%05x",		"BRGT.%-2s  C,A,%05x" } },
	{ { "?C>A    %x",			"RETGT.%-2s C,A" } },
	{ { "?D>C    %x,%05x",		"BRGT.%-2s  D,C,%05x" } },
	{ { "?D>C    %x",			"RETGT.%-2s D,C" } },
	{ { "?A<B    %x,%05x",		"BRLT.%-2s  A,B,%05x" } },
	{ { "?A<B    %x",			"RETLT.%-2s A,B" } },
	{ { "?B<C    %x,%05x",		"BRLT.%-2s  B,C,%05x" } },
	{ { "?B<C    %x",			"RETLT.%-2s B,C" } },
	{ { "?C<A    %x,%05x",		"BRLT.%-2s  C,A,%05x" } },
	{ { "?C<A    %x",			"RETLT.%-2s C,A" } },
	{ { "?D<C    %x,%05x",		"BRLT.%-2s  D,C,%05x" } },
	{ { "?D<C    %x",			"RETLT.%-2s D,C" } },
	{ { "?A>=B   %x,%05x",		"BRGE.%-2s  A,B,%05x" } },
	{ { "?A>=B   %x",			"RETGE.%-2s A,B" } },
	{ { "?B>=C   %x,%05x",		"BRGE.%-2s  B,C,%05x" } },
	{ { "?B>=C   %x",			"RETGE.%-2s B,C" } },
	{ { "?C>=A   %x,%05x",		"BRGE.%-2s  C,A,%05x" } },
	{ { "?C>=A   %x",			"RETGE.%-2s C,A" } },
	{ { "?D>=C   %x,%05x",		"BRGE.%-2s  D,C,%05x" } },
	{ { "?D>=C   %x",			"RETGE.%-2s D,C" } },
	{ { "?A<=B   %x,%05x",		"BRLE.%-2s  A,B,%05x" } },
	{ { "?A<=B   %x",			"RETLE.%-2s A,B" } },
	{ { "?B<=C   %x,%05x",		"BRLE.%-2s  B,C,%05x" } },
	{ { "?B<=C   %x",			"RETLE.%-2s B,C" } },
	{ { "?C<=A   %x,%05x",		"BRLE.%-2s  C,A,%05x" } },
	{ { "?C<=A   %x",			"RETLE.%-2s C,A" } },
	{ { "?D<=C   %x,%05x",		"BRLE.%-2s  D,C,%05x" } },
	{ { "?D<=C   %x",			"RETLE.%-2s D,C" } },

	{ { "sethex",				"SETHEX" } },
	{ { "setdec",				"SETDEC" } },
	{ { "RSTK=C",				"PUSH.A   C" } },
	{ { "C=RSTK",				"POP.A    C" } },

	// load immediate
	{ { "D0=     %02x",			"MOVE.2   %02x,D0" } },
	{ { "D0=     %04x",			"MOVE.4   %04x,D0" } },
	{ { "D0=     %05x",			"MOVE.5   %05x,D0" } },

	{ { "D1=     %02x",			"MOVE.2   %02x,D1" } },
	{ { "D1=     %04x",			"MOVE.4   %04x,D1" } },
	{ { "D1=     %05x",			"MOVE.5   %05x,D1" } },

	{ { "P=      %x",			"MOVE     %x,P" } },
	{ { "lC %-2x   %s",			"MOVE.P%-2x %s,C" } },

	{ { "clrST",				"CLR.X    ST" } },
	{ { "C=ST",					"MOVE.X   ST,C" } },
	{ { "ST=C",					"MOVE.X   C,ST" } },
	{ { "CSTex",				"SWAP.X   C,ST" } },

	{ { "P=P+1",				"INC      P" } },
	{ { "P=P-1",				"DEC      P" } },

	// copy
	{ { "R0=A    %s",			"MOVE.%-2s  A,R0" } },
	{ { "R1=A    %s",			"MOVE.%-2s  A,R1" } },
	{ { "R2=A    %s",			"MOVE.%-2s  A,R2" } },
	{ { "R3=A    %s",			"MOVE.%-2s  A,R3" } },
	{ { "R4=A    %s",			"MOVE.%-2s  A,R4" } },

	{ { "R0=C    %s",			"MOVE.%-2s  C,R0" } },
	{ { "R1=C    %s",			"MOVE.%-2s  C,R1" } },
	{ { "R2=C    %s",			"MOVE.%-2s  C,R2" } },
	{ { "R3=C    %s",			"MOVE.%-2s  C,R3" } },
	{ { "R4=C    %s",			"MOVE.%-2s  C,R4" } },

	{ { "A=R0    %s",			"MOVE.%-2s  R0,A" } },
	{ { "A=R1    %s",			"MOVE.%-2s  R1,A" } },
	{ { "A=R2    %s",			"MOVE.%-2s  R2,A" } },
	{ { "A=R3    %s",			"MOVE.%-2s  R3,A" } },
	{ { "A=R4    %s",			"MOVE.%-2s  R4,A" } },

	{ { "C=R0    %s",			"MOVE.%-2s  R0,C" } },
	{ { "C=R1    %s",			"MOVE.%-2s  R1,C" } },
	{ { "C=R2    %s",			"MOVE.%-2s  R2,C" } },
	{ { "C=R3    %s",			"MOVE.%-2s  R3,C" } },
	{ { "C=R4    %s",			"MOVE.%-2s  R4,C" } },

	{ { "D0=A",					"MOVE.A   A,D0" } },
	{ { "D1=A",					"MOVE.A   A,D1" } },
	{ { "D0=C",					"MOVE.A   C,D0" } },
	{ { "D1=C",					"MOVE.A   C,D1" } },
	{ { "D0=As",				"MOVE.S   A,D0" } },
	{ { "D1=As",				"MOVE.S   A,D1" } },
	{ { "D0=Cs",				"MOVE.S   C,D0" } },
	{ { "D1=Cs",				"MOVE.S   C,D1" } },

	// swap operations
	{ { "AR0ex   %s",			"SWAP.%-2s  A,R0" } },
	{ { "AR1ex   %s",			"SWAP.%-2s  A,R1" } },
	{ { "AR2ex   %s",			"SWAP.%-2s  A,R2" } },
	{ { "AR3ex   %s",			"SWAP.%-2s  A,R3" } },
	{ { "AR4ex   %s",			"SWAP.%-2s  A,R4" } },

	{ { "CR0ex   %s",			"SWAP.%-2s  C,R0" } },
	{ { "CR1ex   %s",			"SWAP.%-2s  C,R1" } },
	{ { "CR2ex   %s",			"SWAP.%-2s  C,R2" } },
	{ { "CR3ex   %s",			"SWAP.%-2s  C,R3" } },
	{ { "CR4ex   %s",			"SWAP.%-2s  C,R4" } },

	{ { "AD0ex",				"SWAP.A   A,D0" } },
	{ { "AD1ex",				"SWAP.A   A,D1" } },
	{ { "CD0ex",				"SWAP.A   C,D0" } },
	{ { "CD1ex",				"SWAP.A   C,D1" } },
	{ { "AD0xs",				"SWAP.S   A,D0" } },
	{ { "AD1xs",				"SWAP.S   A,D1" } },
	{ { "CD0xs",				"SWAP.S   C,D0" } },
	{ { "CD1xs",				"SWAP.S   C,D1" } },

	// store
	{ { "Dat0=A  %s",			"MOVE.%-2s  A,@D0" } },
	{ { "Dat1=A  %s",			"MOVE.%-2s  A,@D0" } },
	{ { "Dat0=C  %s",			"MOVE.%-2s  C,@D0" } },
	{ { "Dat1=C  %s",			"MOVE.%-2s  C,@D0" } },

	// load
	{ { "A=Dat0  %s",			"MOVE.%-2s  @D0,A" } },
	{ { "A=Dat1  %s",			"MOVE.%-2s  @D0,A" } },
	{ { "C=Dat0  %s",			"MOVE.%-2s  @D0,C" } },
	{ { "C=Dat1  %s",			"MOVE.%-2s  @D0,C" } },

	// add/sub immediate
	{ { "D0=D0+  %x",			"ADD.A    %x,D0" } },
	{ { "D1=D1+  %x",			"ADD.A    %x,D1" } },
	{ { "D0=D0-  %x",			"SUB.A    %x,D0" } },
	{ { "D1=D1-  %x",			"SUB.A    %x,D1" } },

	{ { "A=A+    %s,%x",		"ADD.%-2s   %x,A" } },
	{ { "B=B+    %s,%x",		"ADD.%-2s   %x,B" } },
	{ { "C=C+    %s,%x",		"ADD.%-2s   %x,C" } },
	{ { "D=D+    %s,%x",		"ADD.%-2s   %x,D" } },
	{ { "A=A-    %s,%x",		"SUB.%-2s   %x,A" } },
	{ { "B=B-    %s,%x",		"SUB.%-2s   %x,B" } },
	{ { "C=C-    %s,%x",		"SUB.%-2s   %x,C" } },
	{ { "D=D-    %s,%x",		"SUB.%-2s   %x,D" } },

	{ { "A=A&B   %s",			"AND.%-2s   B,A" } },
	{ { "B=B&C   %s",			"AND.%-2s   C,B" } },
	{ { "C=C&A   %s",			"AND.%-2s   A,C" } },
	{ { "D=D&C   %s",			"AND.%-2s   C,D" } },
	{ { "B=B&A   %s",			"AND.%-2s   A,B" } },
	{ { "C=C&B   %s",			"AND.%-2s   B,C" } },
	{ { "A=A&C   %s",			"AND.%-2s   C,A" } },
	{ { "C=C&D   %s",			"AND.%-2s   D,C" } },

	{ { "A=A!B   %s",			"OR.%-2s    B,A" } },
	{ { "B=B!C   %s",			"OR.%-2s    C,B" } },
	{ { "C=C!A   %s",			"OR.%-2s    A,C" } },
	{ { "D=D!C   %s",			"OR.%-2s    C,D" } },
	{ { "B=B!A   %s",			"OR.%-2s    A,B" } },
	{ { "C=C!B   %s",			"OR.%-2s    B,C" } },
	{ { "A=A!C   %s",			"OR.%-2s    C,A" } },
	{ { "C=C!D   %s",			"OR.%-2s    D,C" } },

	{ { "Asrb    %x",			"SRB.%-2s   A" } },
	{ { "Bsrb    %x",			"SRB.%-2s   B" } },
	{ { "Csrb    %x",			"SRB.%-2s   C" } },
	{ { "Dsrb    %x",			"SRB.%-2s   D" } },

	{ { "Aslc    %s",			"RLN.%-2s   A" } },
	{ { "Bslc    %s",			"RLN.%-2s   B" } },
	{ { "Cslc    %s",			"RLN.%-2s   C" } },
	{ { "Dslc    %s",			"RLN.%-2s   D" } },
	{ { "Aslc    %s",			"RRN.%-2s   A" } },
	{ { "Bslc    %s",			"RRN.%-2s   B" } },
	{ { "Cslc    %s",			"RRN.%-2s   C" } },
	{ { "Dslc    %s",			"RRN.%-2s   D" } },

	{ { "A=A+B   %s",			"ADD.%-2s   B,A" } },
	{ { "B=B+C   %s",			"ADD.%-2s   C,B" } },
	{ { "C=C+A   %s",			"ADD.%-2s   A,C" } },
	{ { "D=D+C   %s",			"ADD.%-2s   C,D" } },
	{ { "A=A+A   %s",			"ADD.%-2s   A,A" } },
	{ { "B=B+B   %s",			"ADD.%-2s   B,B" } },
	{ { "C=C+C   %s",			"ADD.%-2s   C,C" } },
	{ { "D=D+C   %s",			"ADD.%-2s   D,D" } },
	{ { "B=B+A   %s",			"ADD.%-2s   A,B" } },
	{ { "C=C+B   %s",			"ADD.%-2s   B,C" } },
	{ { "A=A+C   %s",			"ADD.%-2s   C,A" } },
	{ { "C=C+D   %s",			"ADD.%-2s   D,C" } },
	{ { "A=A-1   %s",			"DEC.%-2s   A" } },
	{ { "B=B-1   %s",			"DEC.%-2s   B" } },
	{ { "C=C-1   %s",			"DEC.%-2s   C" } },
	{ { "D=D-1   %s",			"DEC.%-2s   D" } },

	{ { "A=A-B   %s",			"ADD.%-2s   B,A" } },
	{ { "B=B-C   %s",			"ADD.%-2s   C,B" } },
	{ { "C=C-A   %s",			"ADD.%-2s   A,C" } },
	{ { "D=D-C   %s",			"ADD.%-2s   C,D" } },
	{ { "A=A+1   %s",			"INC.%-2s   A" } },
	{ { "B=B+1   %s",			"INC.%-2s   B" } },
	{ { "C=C+1   %s",			"INC.%-2s   C" } },
	{ { "D=D+1   %s",			"INC.%-2s   D" } },
	{ { "B=B-A   %s",			"SUB.%-2s   A,B" } },
	{ { "C=C-B   %s",			"SUB.%-2s   B,C" } },
	{ { "A=A-C   %s",			"SUB.%-2s   C,A" } },
	{ { "C=C-D   %s",			"SUB.%-2s   D,C" } },
	{ { "A=B-A   %s",			"SUBN.%-2s  B,A" } },
	{ { "B=C-B   %s",			"SUBN.%-2s  C,B" } },
	{ { "C=A-C   %s",			"SUBN.%-2s  A,C" } },
	{ { "D=C-D   %s",			"SUBN.%-2s  C,D" } },

	{ { "A=0     %s",			"CLR.%-2s   A" } },
	{ { "B=0     %s",			"CLR.%-2s   B" } },
	{ { "C=0     %s",			"CLR.%-2s   C" } },
	{ { "D=0     %s",			"CLR.%-2s   D" } },
	{ { "A=B     %s",			"MOVE.%-2s  B,A" } },
	{ { "B=C     %s",			"MOVE.%-2s  C,B" } },
	{ { "C=A     %s",			"MOVE.%-2s  A,C" } },
	{ { "D=C     %s",			"MOVE.%-2s  C,D" } },
	{ { "B=A     %s",			"MOVE.%-2s  A,B" } },
	{ { "C=B     %s",			"MOVE.%-2s  B,C" } },
	{ { "A=C     %s",			"MOVE.%-2s  C,A" } },
	{ { "C=D     %s",			"MOVE.%-2s  D,C" } },
	{ { "ABex    %s",			"SWAP.%-2s  A,B" } },
	{ { "BCex    %s",			"SWAP.%-2s  B,C" } },
	{ { "ACex    %s",			"SWAP.%-2s  A,C" } },
	{ { "CDex    %s",			"SWAP.%-2s  C,D" } },

	{ { "Asl     %s",			"SLN.%-2s   A" } },
	{ { "Bsl     %s",			"SLN.%-2s   B" } },
	{ { "Csl     %s",			"SLN.%-2s   C" } },
	{ { "Dsl     %s",			"SLN.%-2s   D" } },
	{ { "Asr     %s",			"SRN.%-2s   A" } },
	{ { "Bsr     %s",			"SRN.%-2s   B" } },
	{ { "Csr     %s",			"SRN.%-2s   C" } },
	{ { "Dsr     %s",			"SRN.%-2s   D" } },
	{ { "A=-A    %s",			"NEG.%-2s   A" } },
	{ { "B=-B    %s",			"NEG.%-2s   B" } },
	{ { "C=-C    %s",			"NEG.%-2s   C" } },
	{ { "D=-D    %s",			"NEG.%-2s   D" } },
	{ { "A=-A-1  %s",			"NOT.%-2s   A" } },
	{ { "B=-B-1  %s",			"NOT.%-2s   B" } },
	{ { "C=-C-1  %s",			"NOT.%-2s   C" } },
	{ { "D=-D-1  %s",			"NOT.%-2s   D" } }

};

typedef struct {
	enum {
		Complete=-1,
		Illegal,
		Opcode0, Opcode0E, Opcode0Ea,
		Opcode1, Opcode10, Opcode11, Opcode12, Opcode13, Opcode14, Opcode15,
		Opcode8, Opcode80, Opcode808, Opcode8081,
		Opcode81, Opcode818, Opcode818a, Opcode819, Opcode819a,
		Opcode81A, Opcode81Aa, Opcode81Aa0,Opcode81Aa1, Opcode81Aa2, Opcode81B,
		Opcode8A, Opcode8B,
		Opcode9, Opcode9a, Opcode9b,
		OpcodeA, OpcodeAa, OpcodeAb,
		OpcodeB, OpcodeBa, OpcodeBb,
		OpcodeC,
		OpcodeD,
		OpcodeE,
		OpcodeF
	} sel;
	enum {
		AdrNone,
		AdrAF, AdrA, AdrB, AdrCount,
		BranchReturn, TestBranchRet, ImmBranch,
		ABranchReturn, // address field A
		xBranchReturn, // address field specified in previous opcode entry
		Imm, ImmCount, ImmCload, Imm2, Imm4, Imm5,
		Dis3, Dis3Call, Dis4, Dis4Call, Abs,
		FieldP, FieldWP, FieldXS, FieldX, FieldS, FieldM, FieldB, FieldW, FieldA
	} adr;
	MNEMONICS mnemonic;
} OPCODE;

static const char *field_2_string(int adr_enum)
{
	switch (adr_enum) {
	case FieldP: return P;
	case FieldWP: return WP;
	case FieldXS: return XS;
	case FieldX: return X;
	case FieldS: return S;
	case FieldM: return M;
	case FieldB: return B;
	case FieldW: return W;
	case FieldA: return A;
	}
	return 0;
}

static OPCODE opcodes[][0x10]= {
	{
		// first digit
		{ Opcode0 },
		{ Opcode1 },
		{ Complete,		Imm,			PloadImm },
		{ Complete,		ImmCload,		CloadImm },
		{ Complete,		BranchReturn,	branchCarrySet},
		{ Complete,		BranchReturn,	branchCarryClear },
		{ Complete,		Dis3,			jump3 },
		{ Complete,		Dis3Call,		call3 },
		{ Opcode8 },
		{ Opcode9 },
		{ OpcodeA },
		{ OpcodeB },
		{ OpcodeC },
		{ OpcodeD },
		{ OpcodeE },
		{ OpcodeF }
	}, { // 0
		{ Complete,		AdrNone,		ReturnSetXM },
		{ Complete,		AdrNone,		Return },
		{ Complete,		AdrNone,		ReturnSetCarry },
		{ Complete,		AdrNone,		ReturnClearCarry },
		{ Complete,		AdrNone,		SetHexMode },
		{ Complete,		AdrNone,		SetDecMode },
		{ Complete,		AdrNone,		PopC },
		{ Complete,		AdrNone,		PushC },
		{ Complete,		AdrNone,		clearST },
		{ Complete,		AdrNone,		CcopyST },
		{ Complete,		AdrNone,		STcopyC },
		{ Complete,		AdrNone,		swapCST },
		{ Complete,		AdrNone,		incP },
		{ Complete,		AdrNone,		decP },
		{ Opcode0E },
		{ Complete,		AdrNone,		ReturnFromInterrupt }
	}, { //0E
		{ Opcode0Ea,	AdrAF },
		{ Opcode0Ea,	AdrAF },
		{ Opcode0Ea,	AdrAF },
		{ Opcode0Ea,	AdrAF },
		{ Opcode0Ea,	AdrAF },
		{ Opcode0Ea,	AdrAF },
		{ Opcode0Ea,	AdrAF },
		{ Opcode0Ea,	AdrAF },
		{ Illegal },
		{ Illegal },
		{ Illegal },
		{ Illegal },
		{ Illegal },
		{ Illegal },
		{ Illegal },
		{ Opcode0Ea,	AdrAF }
	}, { //0Ea
		{ Complete,		AdrNone,		AandB },
		{ Complete,		AdrNone,		BandC },
		{ Complete,		AdrNone,		CandA },
		{ Complete,		AdrNone,		DandC },
		{ Complete,		AdrNone,		BandA },
		{ Complete,		AdrNone,		CandB },
		{ Complete,		AdrNone,		AandC },
		{ Complete,		AdrNone,		CandD },
		{ Complete,		AdrNone,		AorB },
		{ Complete,		AdrNone,		BorC },
		{ Complete,		AdrNone,		CorA },
		{ Complete,		AdrNone,		DorC },
		{ Complete,		AdrNone,		BorA },
		{ Complete,		AdrNone,		CorB },
		{ Complete,		AdrNone,		AorC },
		{ Complete,		AdrNone,		CorD }
	}, { //1
		{ Opcode10 },
		{ Opcode11 },
		{ Opcode12 },
		{ Opcode13 },
		{ Opcode14 },
		{ Opcode15 },
		{ Complete,		ImmCount,		D0addImm },
		{ Complete,		ImmCount,		D1addImm },
		{ Complete,		ImmCount,		D0subImm },
		{ Complete,		Imm2,			D0loadImm2 },
		{ Complete,		Imm4,			D0loadImm4 },
		{ Complete,		Imm5,			D0loadImm5 },
		{ Complete,		ImmCount,		D1subImm },
		{ Complete,		Imm2,			D1loadImm2 },
		{ Complete,		Imm4,			D1loadImm4 },
		{ Complete,		Imm5,			D1loadImm5 }
	}, { //10
		{ Complete,		FieldW,			R0copyA },
		{ Complete,		FieldW,			R1copyA },
		{ Complete,		FieldW,			R2copyA },
		{ Complete,		FieldW,			R3copyA },
		{ Complete,		FieldW,			R4copyA },
		{ Illegal },
		{ Illegal },
		{ Illegal },
		{ Complete,		FieldW,			R0copyC },
		{ Complete,		FieldW,			R1copyC },
		{ Complete,		FieldW,			R2copyC },
		{ Complete,		FieldW,			R3copyC },
		{ Complete,		FieldW,			R4copyC },
		{ Illegal },
		{ Illegal },
		{ Illegal }
	}, { //11
		{ Complete,		FieldW,			AcopyR0 },
		{ Complete,		FieldW,			AcopyR1 },
		{ Complete,		FieldW,			AcopyR2 },
		{ Complete,		FieldW,			AcopyR3 },
		{ Complete,		FieldW,			AcopyR4 },
		{ Illegal },
		{ Illegal },
		{ Illegal },
		{ Complete,		FieldW,			CcopyR0 },
		{ Complete,		FieldW,			CcopyR1 },
		{ Complete,		FieldW,			CcopyR2 },
		{ Complete,		FieldW,			CcopyR3 },
		{ Complete,		FieldW,			CcopyR4 },
		{ Illegal },
		{ Illegal },
		{ Illegal }
	}, { //12
		{ Complete,		FieldW,			SwapAR0 },
		{ Complete,		FieldW,			SwapAR1 },
		{ Complete,		FieldW,			SwapAR2 },
		{ Complete,		FieldW,			SwapAR3 },
		{ Complete,		FieldW,			SwapAR4 },
		{ Illegal },
		{ Illegal },
		{ Illegal },
		{ Complete,		FieldW,			SwapCR0 },
		{ Complete,		FieldW,			SwapCR1 },
		{ Complete,		FieldW,			SwapCR2 },
		{ Complete,		FieldW,			SwapCR3 },
		{ Complete,		FieldW,			SwapCR4 },
		{ Illegal },
		{ Illegal },
		{ Illegal }
	}, { //13
		{ Complete,		FieldA,			D0copyA },
		{ Complete,		FieldA,			D1copyA },
		{ Complete,		FieldA,			SwapAD0 },
		{ Complete,		FieldA,			SwapAD1 },
		{ Complete,		FieldA,			D0copyC },
		{ Complete,		FieldA,			D1copyC },
		{ Complete,		FieldA,			SwapCD0 },
		{ Complete,		FieldA,			SwapCD1 },
		{ Complete,		FieldS,			D0copyAShort },
		{ Complete,		FieldS,			D1copyAShort },
		{ Complete,		FieldS,			SwapAD0Short },
		{ Complete,		FieldS,			SwapAD1Short },
		{ Complete,		FieldS,			D0copyCShort },
		{ Complete,		FieldS,			D1copyCShort },
		{ Complete,		FieldS,			SwapCD0Short },
		{ Complete,		FieldS,			SwapCD1Short }
	}, { //14
		{ Complete,		FieldA,			D0storeA },
		{ Complete,		FieldA,			D1storeA },
		{ Complete,		FieldA,			AloadD0 },
		{ Complete,		FieldA,			AloadD1 },
		{ Complete,		FieldA,			D0storeC },
		{ Complete,		FieldA,			D1storeC },
		{ Complete,		FieldA,			CloadD0 },
		{ Complete,		FieldA,			CloadD1 },
		{ Complete,		FieldB,			D0storeA },
		{ Complete,		FieldB,			D1storeA },
		{ Complete,		FieldB,			AloadD0 },
		{ Complete,		FieldB,			AloadD1 },
		{ Complete,		FieldB,			D0storeC },
		{ Complete,		FieldB,			D1storeC },
		{ Complete,		FieldB,			CloadD0 },
		{ Complete,		FieldB,			CloadD1 }
	}, { //15
		{ Complete,		AdrA,			D0storeA },
		{ Complete,		AdrA,			D1storeA },
		{ Complete,		AdrA,			AloadD0 },
		{ Complete,		AdrA,			AloadD1 },
		{ Complete,		AdrA,			D0storeC },
		{ Complete,		AdrA,			D1storeC },
		{ Complete,		AdrA,			CloadD0 },
		{ Complete,		AdrA,			CloadD1 },
		{ Complete,		AdrCount,		D0storeA },
		{ Complete,		AdrCount,		D1storeA },
		{ Complete,		AdrCount,		AloadD0 },
		{ Complete,		AdrCount,		AloadD1 },
		{ Complete,		AdrCount,		D0storeC },
		{ Complete,		AdrCount,		D1storeC },
		{ Complete,		AdrCount,		CloadD0 },
		{ Complete,		AdrCount,		CloadD1 },
	}, { //8
		{ Opcode80 },
		{ Opcode81 },
		{ Complete,		Imm,			clearHST },
		{ Complete,		TestBranchRet,	branchHSTclear },
		{ Complete,		Imm,			clearBitST },
		{ Complete,		Imm,			setBitST },
		{ Complete,		TestBranchRet,	branchSTclear },
		{ Complete,		TestBranchRet,	branchSTset },
		{ Complete,		TestBranchRet,  branchPdiffers},
		{ Complete,		TestBranchRet,  branchPequals},
		{ Opcode8A },
		{ Opcode8B },
		{ Complete,		Dis4,			jump4 },
		{ Complete,		Abs,			jump },
		{ Complete,     Dis4Call,		call4 },
		{ Complete,		Abs,			call }
	}, { //80
		{ Complete,		AdrNone,		outCS },
		{ Complete,		AdrNone,		outC },
		{ Complete,		AdrNone,		inA },
		{ Complete,		AdrNone,		inC },
		{ Complete,		AdrNone,		unconfig },
		{ Complete,		AdrNone,		config },
		{ Complete,		AdrNone,		Cid },
		{ Complete,		AdrNone,		shutdown },
		{ Opcode808 },
		{ Complete,		AdrNone,		cp1 },
		{ Complete,		AdrNone,		reset },
		{ Complete,		AdrNone,		buscc },
		{ Complete,		Imm,			CcopyP },
		{ Complete,		Imm,			PcopyC },
		{ Complete,		AdrNone,		sreq },
		{ Complete,		Imm,			CswapP }
	}, { //808
		{ Complete,		AdrNone,		inton },
		{ Opcode8081 },
		{ Complete,		ImmCload,		AloadImm },
		{ Complete,		AdrNone,		buscb },
		{ Complete,		Imm,			clearAbit },
		{ Complete,		Imm,			setAbit },
		{ Complete,		ImmBranch,		Abitclear },
		{ Complete,		ImmBranch,		Abitset },
		{ Complete,		Imm,			clearCbit },
		{ Complete,		Imm,			setCbit },
		{ Complete,		ImmBranch,		Cbitclear },
		{ Complete,		ImmBranch,		Cbitset },
		{ Complete,		AdrNone,		PCloadA },
		{ Complete,		AdrNone,		buscd },
		{ Complete,		AdrNone,		PCloadC },
		{ Complete,		AdrNone,		intoff }
	}, { //8081
		{ Complete,		AdrNone,		rsi },
		//! rest illegal
	}, { //81
		{ Complete,		FieldW,			AshiftleftCarry },
		{ Complete,		FieldW,			BshiftleftCarry },
		{ Complete,		FieldW,			DshiftleftCarry },
		{ Complete,		FieldW,			DshiftleftCarry },
		{ Complete,		FieldW,			AshiftrightCarry },
		{ Complete,		FieldW,			AshiftrightCarry },
		{ Complete,		FieldW,			AshiftrightCarry },
		{ Complete,		FieldW,			AshiftrightCarry },
		{ Opcode818 },
		{ Opcode819 },
		{ Opcode81A },
		{ Opcode81B },
		{ Complete,		FieldW,			Ashiftrightbit },
		{ Complete,		FieldW,			Bshiftrightbit },
		{ Complete,		FieldW,			Cshiftrightbit },
		{ Complete,		FieldW,			Dshiftrightbit }
	}, { //818
		{ Opcode818a,	AdrAF },
		{ Opcode818a,	AdrAF },
		{ Opcode818a,	AdrAF },
		{ Opcode818a,	AdrAF },
		{ Opcode818a,	AdrAF },
		{ Opcode818a,	AdrAF },
		{ Opcode818a,	AdrAF },
		{ Opcode818a,	AdrAF },
		{ Illegal },
		{ Illegal },
		{ Illegal },
		{ Illegal },
		{ Illegal },
		{ Illegal },
		{ Illegal },
		{ Opcode818a,	AdrAF },
	}, { //818a
		{ Complete,		AdrNone,		AaddImm },
		{ Complete,		AdrNone,		BaddImm },
		{ Complete,		AdrNone,		CaddImm },
		{ Complete,		AdrNone,		DaddImm },
		{ Complete,		AdrNone,		AsubImm },
		{ Complete,		AdrNone,		BsubImm },
		{ Complete,		AdrNone,		CsubImm },
		{ Complete,		AdrNone,		DsubImm }
		//! rest illegal
	}, { //819
		{ Opcode819a,	AdrAF },
		{ Opcode819a,	AdrAF },
		{ Opcode819a,	AdrAF },
		{ Opcode819a,	AdrAF },
		{ Opcode819a,	AdrAF },
		{ Opcode819a,	AdrAF },
		{ Opcode819a,	AdrAF },
		{ Opcode819a,	AdrAF }, //?
		{ Illegal },
		{ Illegal },
		{ Illegal },
		{ Illegal },
		{ Illegal },
		{ Illegal },
		{ Illegal },
		{ Opcode819a,	AdrAF },
	}, { //819a }
		{ Complete,		AdrNone,		Ashiftright },
		{ Complete,		AdrNone,		Bshiftright },
		{ Complete,		AdrNone,		Cshiftright },
		{ Complete,		AdrNone,		Dshiftright },
		//! rest illegal
	}, { //81A
		{ Opcode81Aa,	AdrAF },
		{ Opcode81Aa,	AdrAF },
		{ Opcode81Aa,	AdrAF },
		{ Opcode81Aa,	AdrAF },
		{ Opcode81Aa,	AdrAF },
		{ Opcode81Aa,	AdrAF },
		{ Opcode81Aa,	AdrAF },
		{ Opcode81Aa,	AdrAF },
		{ Illegal },
		{ Illegal },
		{ Illegal },
		{ Illegal },
		{ Illegal },
		{ Illegal },
		{ Illegal },
		{ Opcode81Aa,	AdrAF },
	}, { //81Aa
		{ Opcode81Aa0 },
		{ Opcode81Aa1 },
		{ Opcode81Aa2 },
		//! rest illegal
	}, { //81Aa0
		{ Complete,		AdrNone,		R0copyA },
		{ Complete,		AdrNone,		R1copyA },
		{ Complete,		AdrNone,		R2copyA },
		{ Complete,		AdrNone,		R3copyA },
		{ Complete,		AdrNone,		R4copyA },
		{ Illegal },
		{ Illegal },
		{ Illegal },
		{ Complete,		AdrNone,		R0copyC },
		{ Complete,		AdrNone,		R1copyC },
		{ Complete,		AdrNone,		R2copyC },
		{ Complete,		AdrNone,		R3copyC },
		{ Complete,		AdrNone,		R4copyC },
		{ Illegal },
		{ Illegal },
		{ Illegal }
	}, { //81Aa1
		{ Complete,		AdrNone,		AcopyR0 },
		{ Complete,		AdrNone,		AcopyR1 },
		{ Complete,		AdrNone,		AcopyR2 },
		{ Complete,		AdrNone,		AcopyR3 },
		{ Complete,		AdrNone,		AcopyR4 },
		{ Illegal },
		{ Illegal },
		{ Illegal },
		{ Complete,		AdrNone,		CcopyR0 },
		{ Complete,		AdrNone,		CcopyR1 },
		{ Complete,		AdrNone,		CcopyR2 },
		{ Complete,		AdrNone,		CcopyR3 },
		{ Complete,		AdrNone,		CcopyR4 },
		{ Illegal },
		{ Illegal },
		{ Illegal }
	}, { //81Aa2
		{ Complete,		AdrNone,		SwapAR0 },
		{ Complete,		AdrNone,		SwapAR1 },
		{ Complete,		AdrNone,		SwapAR2 },
		{ Complete,		AdrNone,		SwapAR3 },
		{ Complete,		AdrNone,		SwapAR4 },
		{ Illegal },
		{ Illegal },
		{ Illegal },
		{ Complete,		AdrNone,		SwapCR0 },
		{ Complete,		AdrNone,		SwapCR1 },
		{ Complete,		AdrNone,		SwapCR2 },
		{ Complete,		AdrNone,		SwapCR3 },
		{ Complete,		AdrNone,		SwapCR4 },
		{ Illegal },
		{ Illegal },
		{ Illegal }
	}, { //81B
		{ Illegal },
		{ Illegal },
		{ Complete,		AdrNone,		jumpA },
		{ Complete,		AdrNone,		jumpC },
		{ Complete,		AdrNone,		PCcopyA },
		{ Complete,		AdrNone,		PCcopyC },
		{ Complete,		AdrNone,		AcopyPC },
		{ Complete,		AdrNone,		CcopyPC },
		{ Illegal },
		{ Illegal },
		{ Illegal },
		{ Illegal },
		{ Illegal },
		{ Illegal },
		{ Illegal },
		{ Illegal }
	}, { //8A
		{ Complete,		ABranchReturn,	branchAequalsB },
		{ Complete,		ABranchReturn,	branchBequalsC },
		{ Complete,		ABranchReturn,	branchAequalsC },
		{ Complete,		ABranchReturn,	branchCequalsD },
		{ Complete,		ABranchReturn,	branchAdiffersB },
		{ Complete,		ABranchReturn,	branchBdiffersC },
		{ Complete,		ABranchReturn,	branchAdiffersC },
		{ Complete,		ABranchReturn,	branchCdiffersD },
		{ Complete,		ABranchReturn,	branchAzero },
		{ Complete,		ABranchReturn,	branchBzero },
		{ Complete,		ABranchReturn,	branchCzero },
		{ Complete,		ABranchReturn,	branchDzero },
		{ Complete,		ABranchReturn,	branchAnotzero },
		{ Complete,		ABranchReturn,	branchBnotzero },
		{ Complete,		ABranchReturn,	branchCnotzero },
		{ Complete,		ABranchReturn,	branchDnotzero }
	}, { //8B
		{ Complete,		ABranchReturn,	branchAgreaterB },
		{ Complete,		ABranchReturn,	branchBgreaterC },
		{ Complete,		ABranchReturn,	branchCgreaterA },
		{ Complete,		ABranchReturn,	branchDgreaterC },
		{ Complete,		ABranchReturn,	branchAlowerB },
		{ Complete,		ABranchReturn,	branchBlowerC },
		{ Complete,		ABranchReturn,	branchClowerA },
		{ Complete,		ABranchReturn,	branchDlowerC },
		{ Complete,		ABranchReturn,	branchAnotlowerB },
		{ Complete,		ABranchReturn,	branchBnotlowerC },
		{ Complete,		ABranchReturn,	branchCnotlowerA },
		{ Complete,		ABranchReturn,	branchDnotlowerC },
		{ Complete,		ABranchReturn,	branchAnotgreaterB },
		{ Complete,		ABranchReturn,	branchBnotgreaterC },
		{ Complete,		ABranchReturn,	branchCnotgreaterA },
		{ Complete,		ABranchReturn,	branchDnotgreaterC }
	}, { //9
		{ Opcode9a,		AdrA },
		{ Opcode9a,		AdrA },
		{ Opcode9a,		AdrA },
		{ Opcode9a,		AdrA },
		{ Opcode9a,		AdrA },
		{ Opcode9a,		AdrA },
		{ Opcode9a,		AdrA },
		{ Opcode9a,		AdrA },
		{ Opcode9b,		AdrB },
		{ Opcode9b,		AdrB },
		{ Opcode9b,		AdrB },
		{ Opcode9b,		AdrB },
		{ Opcode9b,		AdrB },
		{ Opcode9b,		AdrB },
		{ Opcode9b,		AdrB },
		{ Opcode9b,		AdrB },
	}, { //9a
		{ Complete,		xBranchReturn,	branchAequalsB },
		{ Complete,		xBranchReturn,	branchBequalsC },
		{ Complete,		xBranchReturn,	branchAequalsC },
		{ Complete,		xBranchReturn,	branchCequalsD },
		{ Complete,		xBranchReturn,	branchAdiffersB },
		{ Complete,		xBranchReturn,	branchBdiffersC },
		{ Complete,		xBranchReturn,	branchAdiffersC },
		{ Complete,		xBranchReturn,	branchCdiffersD },
		{ Complete,		xBranchReturn,	branchAzero },
		{ Complete,		xBranchReturn,	branchBzero },
		{ Complete,		xBranchReturn,	branchCzero },
		{ Complete,		xBranchReturn,	branchDzero },
		{ Complete,		xBranchReturn,	branchAnotzero },
		{ Complete,		xBranchReturn,	branchBnotzero },
		{ Complete,		xBranchReturn,	branchCnotzero },
		{ Complete,	    xBranchReturn,	branchDnotzero }
	}, { //9b
		{ Complete,		xBranchReturn,	branchAgreaterB },
		{ Complete,		xBranchReturn,	branchBgreaterC },
		{ Complete,		xBranchReturn,	branchCgreaterA },
		{ Complete,		xBranchReturn,	branchDgreaterC },
		{ Complete,		xBranchReturn,	branchAlowerB },
		{ Complete,		xBranchReturn,	branchBlowerC },
		{ Complete,		xBranchReturn,	branchClowerA },
		{ Complete,		xBranchReturn,	branchDlowerC },
		{ Complete,		xBranchReturn,	branchAnotlowerB },
		{ Complete,		xBranchReturn,	branchBnotlowerC },
		{ Complete,		xBranchReturn,	branchCnotlowerA },
		{ Complete,		xBranchReturn,	branchDnotlowerC },
		{ Complete,		xBranchReturn,	branchAnotgreaterB },
		{ Complete,		xBranchReturn,	branchBnotgreaterC },
		{ Complete,		xBranchReturn,	branchCnotgreaterA },
		{ Complete,		xBranchReturn,	branchDnotgreaterC }
	}, { //A
		{ OpcodeAa,		AdrA },
		{ OpcodeAa,		AdrA },
		{ OpcodeAa,		AdrA },
		{ OpcodeAa,		AdrA },
		{ OpcodeAa,		AdrA },
		{ OpcodeAa,		AdrA },
		{ OpcodeAa,		AdrA },
		{ OpcodeAa,		AdrA },
		{ OpcodeAb,		AdrB },
		{ OpcodeAb,		AdrB },
		{ OpcodeAb,		AdrB },
		{ OpcodeAb,		AdrB },
		{ OpcodeAb,		AdrB },
		{ OpcodeAb,		AdrB },
		{ OpcodeAb,		AdrB },
		{ OpcodeAb,		AdrB },
	}, { //Aa
		{ Complete,		AdrNone,		AaddB },
		{ Complete,		AdrNone,		BaddC },
		{ Complete,		AdrNone,		CaddA },
		{ Complete,		AdrNone,		DaddC },
		{ Complete,		AdrNone,		AaddA },
		{ Complete,		AdrNone,		BaddB },
		{ Complete,		AdrNone,		CaddC },
		{ Complete,		AdrNone,		DaddD },
		{ Complete,		AdrNone,		BaddA },
		{ Complete,		AdrNone,		CaddB },
		{ Complete,		AdrNone,		AaddC },
		{ Complete,		AdrNone,		CaddD },
		{ Complete,		AdrNone,		decA },
		{ Complete,		AdrNone,		decB },
		{ Complete,		AdrNone,		decC },
		{ Complete,		AdrNone,		decD },
	}, { //Ab
		{ Complete,		AdrNone,		clearA },
		{ Complete,		AdrNone,		clearB },
		{ Complete,		AdrNone,		clearC },
		{ Complete,		AdrNone,		clearD },
		{ Complete,		AdrNone,		AcopyB },
		{ Complete,		AdrNone,		BcopyC },
		{ Complete,		AdrNone,		CcopyA },
		{ Complete,		AdrNone,		DcopyC },
		{ Complete,		AdrNone,		BcopyA },
		{ Complete,		AdrNone,		CcopyB },
		{ Complete,		AdrNone,		AcopyC },
		{ Complete,		AdrNone,		CcopyD },
		{ Complete,		AdrNone,		AswapB },
		{ Complete,		AdrNone,		BswapC },
		{ Complete,		AdrNone,		CswapA },
		{ Complete,		AdrNone,		DswapC }
	}, { //B
		{ OpcodeBa,		AdrA },
		{ OpcodeBa,		AdrA },
		{ OpcodeBa,		AdrA },
		{ OpcodeBa,		AdrA },
		{ OpcodeBa,		AdrA },
		{ OpcodeBa,		AdrA },
		{ OpcodeBa,		AdrA },
		{ OpcodeBa,		AdrA },
		{ OpcodeBb,		AdrB },
		{ OpcodeBb,		AdrB },
		{ OpcodeBb,		AdrB },
		{ OpcodeBb,		AdrB },
		{ OpcodeBb,		AdrB },
		{ OpcodeBb,		AdrB },
		{ OpcodeBb,		AdrB },
		{ OpcodeBb,		AdrB },
	}, { //Ba
		{ Complete,		AdrNone,		AsubB },
		{ Complete,		AdrNone,		BsubC },
		{ Complete,		AdrNone,		CsubA },
		{ Complete,		AdrNone,		DsubC },
		{ Complete,		AdrNone,		incA },
		{ Complete,		AdrNone,		incB },
		{ Complete,		AdrNone,		incC },
		{ Complete,		AdrNone,		incD },
		{ Complete,		AdrNone,		BsubA },
		{ Complete,		AdrNone,		CsubB },
		{ Complete,		AdrNone,		AsubC },
		{ Complete,		AdrNone,		CsubD },
		{ Complete,		AdrNone,		AsubnB },
		{ Complete,		AdrNone,		BsubnC },
		{ Complete,		AdrNone,		CsubnA },
		{ Complete,		AdrNone,		DsubnC },
	}, { //Bb
		{ Complete,		AdrNone,		Ashiftleft },
		{ Complete,		AdrNone,		Bshiftleft },
		{ Complete,		AdrNone,		Cshiftleft },
		{ Complete,		AdrNone,		Dshiftleft },
		{ Complete,		AdrNone,		Ashiftright },
		{ Complete,		AdrNone,		Bshiftright },
		{ Complete,		AdrNone,		Cshiftright },
		{ Complete,		AdrNone,		Dshiftright },
		{ Complete,		AdrNone,		negateA },
		{ Complete,		AdrNone,		negateB },
		{ Complete,		AdrNone,		negateC },
		{ Complete,		AdrNone,		negateD },
		{ Complete,		AdrNone,		notA },
		{ Complete,		AdrNone,		notB },
		{ Complete,		AdrNone,		notC },
		{ Complete,		AdrNone,		notD }
	}, { //C
		{ Complete,		FieldA,		AaddB },
		{ Complete,		FieldA,		BaddC },
		{ Complete,		FieldA,		CaddA },
		{ Complete,		FieldA,		DaddC },
		{ Complete,		FieldA,		AaddA },
		{ Complete,		FieldA,		BaddB },
		{ Complete,		FieldA,		CaddC },
		{ Complete,		FieldA,		DaddD },
		{ Complete,		FieldA,		BaddA },
		{ Complete,		FieldA,		CaddB },
		{ Complete,		FieldA,		AaddC },
		{ Complete,		FieldA,		CaddD },
		{ Complete,		FieldA,		decA },
		{ Complete,		FieldA,		decB },
		{ Complete,		FieldA,		decC },
		{ Complete,		FieldA,		decD }
	}, { //D
		{ Complete,		FieldA,		clearA },
		{ Complete,		FieldA,		clearB },
		{ Complete,		FieldA,		clearC },
		{ Complete,		FieldA,		clearD },
		{ Complete,		FieldA,		AcopyB },
		{ Complete,		FieldA,		BcopyC },
		{ Complete,		FieldA,		CcopyA },
		{ Complete,		FieldA,		DcopyC },
		{ Complete,		FieldA,		BcopyA },
		{ Complete,		FieldA,		CcopyB },
		{ Complete,		FieldA,		AcopyC },
		{ Complete,		FieldA,		CcopyD },
		{ Complete,		FieldA,		AswapB },
		{ Complete,		FieldA,		BswapC },
		{ Complete,		FieldA,		CswapA },
		{ Complete,		FieldA,		DswapC }
	}, { //E
		{ Complete,		FieldA,		AsubB },
		{ Complete,		FieldA,		BsubC },
		{ Complete,		FieldA,		CsubA },
		{ Complete,		FieldA,		DsubC },
		{ Complete,		FieldA,		incA },
		{ Complete,		FieldA,		incB },
		{ Complete,		FieldA,		incC },
		{ Complete,		FieldA,		incD },
		{ Complete,		FieldA,		BsubA },
		{ Complete,		FieldA,		CsubB },
		{ Complete,		FieldA,		AsubC },
		{ Complete,		FieldA,		CsubD },
		{ Complete,		FieldA,		AsubnB },
		{ Complete,		FieldA,		BsubnC },
		{ Complete,		FieldA,		CsubnA },
		{ Complete,		FieldA,		DsubnC }
	}, { //F
		{ Complete,		FieldA,		Ashiftleft },
		{ Complete,		FieldA,		Bshiftleft },
		{ Complete,		FieldA,		Cshiftleft },
		{ Complete,		FieldA,		Dshiftleft },
		{ Complete,		FieldA,		Ashiftright },
		{ Complete,		FieldA,		Bshiftright },
		{ Complete,		FieldA,		Cshiftright },
		{ Complete,		FieldA,		Dshiftright },
		{ Complete,		FieldA,		negateA },
		{ Complete,		FieldA,		negateB },
		{ Complete,		FieldA,		negateC },
		{ Complete,		FieldA,		negateD },
		{ Complete,		FieldA,		notA },
		{ Complete,		FieldA,		notB },
		{ Complete,		FieldA,		notC },
		{ Complete,		FieldA,		notD }
	}
};

static const int field_adr_af[]=
{ FieldP, FieldWP, FieldXS, FieldX, FieldS, FieldM, FieldB, FieldW, 0, 0, 0, 0, 0, 0, 0, FieldA };

static const int field_adr_a[]=
{ FieldP, FieldWP, FieldXS, FieldX, FieldS, FieldM, FieldB, FieldW};

static const int field_adr_b[]=
{ FieldP, FieldWP, FieldXS, FieldX, FieldS, FieldM, FieldB, FieldW };

unsigned saturn_dasm(char *dst, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	int adr=0;

	int cont=1; // operation still not complete disassembled
	char bin[10]; int binsize=0; // protocollizing fetched nibbles
	char number[17];
	OPCODE *level=opcodes[0]; //pointer to current digit
	int op; // currently fetched nibble
	int	pos = 0;

	int i,c,v;

	while (cont)
	{
		op = oprom[pos++];
		level+=op;
		switch (level->sel) {
		case Illegal:
			cont=0;
			bin[binsize++]=number_2_hex[op];
			bin[binsize]=0;
			sprintf(dst, "???%s",bin);
			break;
		default:
			bin[binsize++]=number_2_hex[op];
			switch (level->adr) {
			case AdrNone: break;
			case AdrA:
				adr=field_adr_a[op];
				break;
			case AdrAF:
				adr=field_adr_af[op];
				break;
			case AdrB:
				adr=field_adr_b[op&7];
				break;
			default:
				cont = 0;
				bin[binsize++]=number_2_hex[op];
				bin[binsize]=0;
				sprintf(dst, "???%s",bin);
				break;
			}
			break;
		case Complete:
			cont=0;
			switch (level->adr==AdrNone?adr:level->adr) {
			case AdrNone:
				strcpy(dst, mnemonics[level->mnemonic].name[set]);
				break;
			case Imm:
				sprintf(dst, mnemonics[level->mnemonic].name[set], oprom[pos++]);
				break;
			case ImmCount:
				sprintf(dst, mnemonics[level->mnemonic].name[set], oprom[pos++]+1);
				break;
			case AdrCount: // mnemonics have string %s for address field
				snprintf(number,sizeof(number),"%x",oprom[pos++]+1);
				sprintf(dst, mnemonics[level->mnemonic].name[set], number);
				break;
			case Imm2:
				v=oprom[pos++];
				v|=oprom[pos++]<<4;
				sprintf(dst, mnemonics[level->mnemonic].name[set], v);
				break;
			case Imm4:
				v=oprom[pos++];
				v|=oprom[pos++]<<4;
				v|=oprom[pos++]<<8;
				v|=oprom[pos++]<<12;
				sprintf(dst, mnemonics[level->mnemonic].name[set], v);
				break;
			case Imm5:
				v=oprom[pos++];
				v|=oprom[pos++]<<4;
				v|=oprom[pos++]<<8;
				v|=oprom[pos++]<<12;
				v|=oprom[pos++]<<16;
				sprintf(dst, mnemonics[level->mnemonic].name[set], v);
				break;
			case ImmCload:
				c=i=oprom[pos++];
				number[i+1]=0;
				for (;i>=0; i--) number[i]=number_2_hex[oprom[pos++]];
				sprintf(dst, mnemonics[level->mnemonic].name[set], c+1, number);
				break;
			case Dis3:
				SATURN_PEEKOP_DIS12(v);
				c=(pc+pos-3+v)%0xfffff;
				sprintf(dst, mnemonics[level->mnemonic].name[set], c );
				break;
			case Dis3Call:
				SATURN_PEEKOP_DIS12(v);
				c=(pc+pos-3+v)%0xfffff;
				sprintf(dst, mnemonics[level->mnemonic].name[set], c );
				break;
			case Dis4:
				SATURN_PEEKOP_DIS16(v);
				c=(pc+pos-4+v)%0xfffff;
				sprintf(dst, mnemonics[level->mnemonic].name[set], c );
				break;
			case Dis4Call:
				SATURN_PEEKOP_DIS16(v);
				c=(pc+pos-4+v)%0xfffff;
				sprintf(dst, mnemonics[level->mnemonic].name[set], c );
				break;
			case Abs:
				SATURN_PEEKOP_ADR(v);
				sprintf(dst, mnemonics[level->mnemonic].name[set], v );
				break;
			case BranchReturn:
				SATURN_PEEKOP_DIS8(v);
				if (v==0) {
					strcpy(dst, mnemonics[level->mnemonic+1].name[set]);
				} else {
					c=(pc+pos-2+v)&0xfffff;
					sprintf(dst, mnemonics[level->mnemonic].name[set], c);
				}
				break;
			case ABranchReturn:
				SATURN_PEEKOP_DIS8(v);
				if (v==0) {
					sprintf(dst, mnemonics[level->mnemonic+1].name[set], A);
				} else {
					c=(pc+pos-2+v)&0xfffff;
					sprintf(dst, mnemonics[level->mnemonic].name[set], A, c);
				}
				break;
			case xBranchReturn:
				SATURN_PEEKOP_DIS8(v);
				if (v==0) {
					sprintf(dst, mnemonics[level->mnemonic+1].name[set], field_2_string(adr));
				} else {
					c=(pc+pos-2+v)&0xfffff;
					sprintf(dst, mnemonics[level->mnemonic].name[set], field_2_string(adr), c);
				}
				break;
			case TestBranchRet:
				i=oprom[pos++];
				SATURN_PEEKOP_DIS8(v);
				if (v==0) {
					sprintf(dst, mnemonics[level->mnemonic+1].name[set], i);
				} else {
					c=(pc+pos-2+v)&0xfffff;
					sprintf(dst, mnemonics[level->mnemonic].name[set], i, c);
				}
				break;
			case ImmBranch:
				i=oprom[pos++];
				SATURN_PEEKOP_DIS8(v);
				c=(pc+pos-2+v)&0xfffff;
				sprintf(dst, mnemonics[level->mnemonic].name[set], i, c);
				break;
			case FieldP:
				sprintf(dst, mnemonics[level->mnemonic].name[set], P );
				break;
			case FieldWP:
				sprintf(dst, mnemonics[level->mnemonic].name[set], WP );
				break;
			case FieldXS:
				sprintf(dst, mnemonics[level->mnemonic].name[set], XS );
				break;
			case FieldX:
				sprintf(dst, mnemonics[level->mnemonic].name[set], X );
				break;
			case FieldS:
				sprintf(dst, mnemonics[level->mnemonic].name[set], S );
				break;
			case FieldM:
				sprintf(dst, mnemonics[level->mnemonic].name[set], M );
				break;
			case FieldB:
				sprintf(dst, mnemonics[level->mnemonic].name[set], B );
				break;
			case FieldA:
				sprintf(dst, mnemonics[level->mnemonic].name[set], A );
				break;
			case FieldW:
				sprintf(dst, mnemonics[level->mnemonic].name[set], W );
				break;
			case AdrA:
				sprintf(dst, mnemonics[level->mnemonic].name[set], adr_a[oprom[pos++]] );
				break;
			case AdrAF:
				sprintf(dst, mnemonics[level->mnemonic].name[set], adr_af[oprom[pos++]] );
				break;
			case AdrB:
				sprintf(dst, mnemonics[level->mnemonic].name[set], adr_b[oprom[pos++]&0x7] );
				break;
			}
			break;
		}
		level = opcodes[level->sel];
	}

	return pos;
}
