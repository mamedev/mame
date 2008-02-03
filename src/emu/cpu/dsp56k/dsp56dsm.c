/***************************************************************************

    dsp56dsm.c
    Disassembler for the portable Motorola/Freescale dsp56k emulator.
    Written by Andrew Gardner

***************************************************************************/

#include "dsp56k.h"

/* todo: fix the syntax for many opcodes to be true DSP56k assembly
 *
 *
*/


// Main opcode categories
static unsigned DecodeDataALUOpcode(char *buffer, UINT16 op, unsigned pc, int parallelType) ;
//static unsigned DecodeDXMDROpcode(char *buffer, UINT16 op, unsigned pc) ;
static unsigned DecodeNPMOpcode(char *buffer, UINT16 op, unsigned pc, const UINT8 *oprom) ;
static unsigned DecodeMisfitOpcode(char *buffer, UINT16 op, unsigned pc, const UINT8 *oprom) ;
static unsigned DecodeSpecialOpcode(char *buffer, UINT16 op, unsigned pc, const UINT8 *oprom) ;


// Parallel memory operation decoding
static void AppendXMDM(char *buffer, UINT16 op) ;
static void AppendXMDMSpecial(char *buffer, UINT16 op, char *working) ;
static void AppendARU(char *buffer, UINT16 op) ;
static void AppendRRDM(char *buffer, UINT16 op, char *working) ;

//static void AppendDXMDR(char *buffer, UINT16 op) ;


// Helper functions
#define BITS(CUR,MASK) (Dsp56kOpMask(CUR,MASK))
static UINT16 Dsp56kOpMask(UINT16 op, UINT16 mask) ;


enum bbbType  { BBB_UPPER, BBB_MIDDLE, BBB_LOWER } ;

// Table decoder functions...
static int  DecodeBBBTable  (UINT16 BBB             ) ;
static void DecodeccccTable (UINT16 cccc,           char *mnemonic) ;
static void DecodeDDDDDTable(UINT16 DDDDD,          char *SD) ;
static void DecodeDDTable   (UINT16 DD,             char *SD) ;
//static void DecodeDDFTable  (UINT16 DD,   UINT16 F, char *S, char *D) ;
static void DecodeEETable   (UINT16 EE,             char *D) ;
static void DecodeFTable    (UINT16 F,              char *SD) ;
static void Decodeh0hFTable (UINT16 h0h,  UINT16 F, char *S, char *D) ;
static void DecodeHHTable   (UINT16 HH,             char *SD) ;
static void DecodeHHHTable  (UINT16 HHH,            char *SD) ;
static void DecodeIIIITable (UINT16 IIII,           char *S, char *D) ;
static void DecodeJJJFTable (UINT16 JJJ,  UINT16 F, char *S, char *D) ;
static void DecodeJJFTable  (UINT16 JJ,   UINT16 F, char *S, char *D) ;
//static void DecodeJFTable   (UINT16 J,    UINT16 F, char *S, char *D) ;
//static void DecodekTable    (UINT16 k,              char *Dnot) ;
//static void DecodekSignTable(UINT16 k,              char *plusMinus) ;
//static void DecodeKKKTable  (UINT16 KKK,            char *D1, char *D2) ;
//static int  DecodeNNTable   (UINT16 NN) ;
//static void DecodeQQFTable  (UINT16 QQ,   UINT16 F, char *S1, char *S2, char *D) ;
static void DecodeQQFTableSp(UINT16 QQ,   UINT16 F, char *S1, char *S2, char *D) ;
//static void DecodeQQQFTable (UINT16 QQQ,  UINT16 F, char *S1, char *S2, char *D) ;
static int  DecodeRRTable   (UINT16 RR) ;
static void DecodesTable    (UINT16 s,              char *arithmetic) ;
//static void DecodessTable   (UINT16 ss,             char *arithmetic) ;
//static void DecodeuuuuFTable(UINT16 uuuu, UINT16 F, char *arg, char *S, char *D) ;
static void DecodeZTable    (UINT16 Z,              char *ea) ;


static void AssembleeaFrommTable (UINT16 m,  int n,          char *ea) ;
//static void AssembleeaFrommmTable(UINT16 mm, int n1, int n2, char *ea1, char *ea2) ;
static void AssembleeaFromMMTable(UINT16 MM, int n,          char *ea) ;
static void AssembleeaFromtTable (UINT16 t,  UINT16 val,     char *ea) ;
static void AssembleeaFromqTable (UINT16 q,  int n,          char *ea) ;
static void AssembleeaFromzTable (UINT16 z,  int n,          char *ea) ;

static void AssembleDFromPTable(UINT16 P, UINT16 ppppp, char *D) ;
static void AssembleArgumentsFromWTable(UINT16 W, char *args, char ma, char *SD, char *ea) ;
static void AssembleRegFromWTable(UINT16 W, char *args, char ma, char *SD, UINT8 xx) ;

static void AssembleAddressFromIOShortAddress(UINT16 pp, char *ea) ;
static void AssembleAddressFrom6BitSignedRelativeShortAddress(UINT16 srs, char *ea) ;

// Main disassembly function
offs_t dsp56k_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	UINT16 op = oprom[0] | (oprom[1] << 8);
	unsigned size = 0 ;

	pc <<= 1 ;

	if (BITS(op,0x8000))													// First, the parallel data move instructions
	{
		size = DecodeDataALUOpcode(buffer, op, pc, PARALLEL_TYPE_XMDM) ;
	}
	else if (BITS(op,0xf000) == 0x5)
	{
		size = DecodeDataALUOpcode(buffer, op, pc, PARALLEL_TYPE_XMDM_SPECIAL) ;
	}
	else if (BITS(op,0xff00) == 0x4a)
	{
		size = DecodeDataALUOpcode(buffer, op, pc, PARALLEL_TYPE_NODM) ;
	}
	else if (BITS(op,0xf000) == 0x04)
	{
		size = DecodeDataALUOpcode(buffer, op, pc, PARALLEL_TYPE_RRDM) ;
	}
	else if (BITS(op,0xf800) == 0x06)
	{
		size = DecodeDataALUOpcode(buffer, op, pc, PARALLEL_TYPE_ARU) ;
	}

	else if (BITS(op,0x4000))
	{
		// size = DecodeDXMDROpcode(buffer, op, pc) ;
		sprintf(buffer, "unknown") ;
		size = 1 ;
	}
	else if (BITS(op,0xf000) == 0x1)
	{
		size = DecodeNPMOpcode(buffer, op, pc, oprom) ;
	}
	else if (BITS(op,0x2000))
	{
		size = DecodeMisfitOpcode(buffer, op, pc, oprom) ;
	}
	else if (BITS(op,0xf000) == 0x0)
	{
		size = DecodeSpecialOpcode(buffer, op, pc, oprom) ;
	}

	if (size == 0)
	{
		sprintf(buffer, "unknown") ;
		size = 1 ;						// Just to get the debugger past the bad opcode
	}

	return size | DASMFLAG_SUPPORTED;
}


static unsigned DecodeDataALUOpcode(char *buffer, UINT16 op, unsigned pc, int parallelType)
{
	unsigned retSize = 0 ;

	char S1[32] ;
	char D[32] ;

	switch(BITS(op,0x0070))
	{
		case 0x0:
			if (BITS(op,0x0007) == 0x1)
			{
				// CLR - 1mRR HHHW 0000 F001
				DecodeFTable(BITS(op,0x0008), D) ;
				sprintf(buffer, "clr       %s", D) ;
				retSize = 1 ;
			}
			else
			{
				// ADD - 1mRR HHHW 0000 FJJJ
				DecodeJJJFTable(BITS(op,0x0007), BITS(op,0x0008), S1, D) ;
				sprintf(buffer, "add       %s,%s", S1, D) ;
				retSize = 1 ;
			}
			break ;

		case 0x1:
			if (BITS(op,0x000f) == 0x1)
			{
				// MOVE - 1mRR HHHW 0001 0001
				// Funny operation :)
				sprintf(buffer, "move      ") ;
				retSize = 1 ;
			}
			else
			{
				// TFR - 1mRR HHHW 0001 FJJJ
				DecodeJJJFTable(BITS(op,0x0007), BITS(op,0x0008), S1, D) ;
				sprintf(buffer, "tfr       %s,%s", S1, D) ;
				retSize = 1 ;
			}
			break ;

		case 0x2:
			if (BITS(op,0x0007) == 0x0)
			{
				// RND - 1mRR HHHW 0010 F000
/*              DecodeFTable(BITS(op,0x0008), D) ;
                sprintf(buffer, "rnd       %s", D) ;
                retSize = 1 ;
*/			}
			else if (BITS(op,0x0007) == 0x1)
			{
				// TST - 1mRR HHHW 0010 F001
	            DecodeFTable(BITS(op,0x0008), D) ;
                sprintf(buffer, "tst       %s", D) ;
                retSize = 1 ;
			}
			else if (BITS(op,0x0007) == 0x2)
			{
				// INC - 1mRR HHHW 0010 F010
/*              DecodeFTable(BITS(op,0x0008), D) ;
                sprintf(buffer, "inc       %s", D) ;
                retSize = 1 ;
*/			}
			else if (BITS(op,0x0007) == 0x3)
			{
				// INC24 - 1mRR HHHW 0010 F011
/*              DecodeFTable(BITS(op,0x0008), D) ;
                sprintf(buffer, "inc24     %s", D) ;
                retSize = 1 ;
*/			}
			else
			{
				// OR - 1mRR HHHW 0010 F1JJ
				DecodeJJFTable(BITS(op,0x0003),BITS(op,0x0008), S1, D) ;
				sprintf(buffer, "or        %s,%s", S1, D) ;
				retSize = 1 ;
			}
			break ;

		case 0x3:
			if (BITS(op,0x0007) == 0x0)
			{
				// ASR - 1mRR HHHW 0011 F000
				DecodeFTable(BITS(op,0x0008), D) ;
				sprintf(buffer, "asr       %s", D) ;
				retSize = 1 ;
			}
			else if (BITS(op,0x0007) == 0x1)
			{
/*
                // ASL - 1mRR HHHW 0011 F001
                DecodeFTable(BITS(op,0x0008), D) ;
                sprintf(buffer, "asl       %s", D) ;
                retSize = 1 ;
*/			}
			else if (BITS(op,0x0007) == 0x2)
			{
				// LSR - 1mRR HHHW 0011 F010
				DecodeFTable(BITS(op,0x0008), D) ;
				sprintf(buffer, "lsr       %s", D) ;
				retSize = 1 ;
			}
			else if (BITS(op,0x0007) == 0x3)
			{
/*
                // LSL - 1mRR HHHW 0011 F011
                DecodeFTable(BITS(op,0x0008), D) ;
                sprintf(buffer, "lsl       %s", D) ;
                retSize = 1 ;
*/			}
			else
			{
				// EOR - 1mRR HHHW 0011 F1JJ
				DecodeJJFTable(BITS(op,0x0003),BITS(op,0x0008), S1, D) ;
				sprintf(buffer, "eor       %s,%s", S1, D) ;
				retSize = 1 ;
			}
			break ;

		case 0x4:
			if (BITS(op,0x0007) == 0x1)
			{
				// SUBL - 1mRR HHHW 0100 F001
				// There's something strange about this opcode - there's only one option for F!
/*              if (!BITS(op,0x0008))
                    sprintf(buffer, "subl      B,A") ;
                else
                    sprintf(buffer, "subl      (other!)") ;
                retSize = 1 ;
*/			}
			else
			{
				// SUB - 1mRR HHHW 0100 FJJJ
/*              DecodeJJJFTable(BITS(op,0x0007), BITS(op,0x0008), S1, D) ;
                sprintf(buffer, "sub       %s,%s", S1, D) ;
                retSize = 1 ;
*/			}
			break ;

		case 0x5:
			if (BITS(op,0x0007) == 0x1)
			{
				// CLR24 - 1mRR HHHW 0101 F001
/*              DecodeFTable(BITS(op,0x0008), D) ;
                sprintf(buffer, "clr24     %s", D) ;
                retSize = 1 ;
*/			}
			else if (BITS(op,0x0006) == 0x1)
			{
				// SBC - 1mRR HHHW 0101 F01J
/*              DecodeJFTable(BITS(op,0x0001), BITS(op,0x0008), S1, D) ;
                sprintf(buffer, "sbc       %s,%s", S1, D) ;
                retSize = 1 ;
*/			}
			else
			{
				// CMP - 1mRR HHHW 0101 FJJJ
				DecodeJJJFTable(BITS(op,0x0007), BITS(op,0x0008), S1, D) ;
				sprintf(buffer, "cmp       %s,%s", S1,D) ;
				retSize = 1 ;
			}
			break ;

		case 0x6:
			if (BITS(op,0x0007) == 0x0)
			{
				// NEG - 1mRR HHHW 0110 F000
/*              DecodeFTable(BITS(op,0x0008), D) ;
                sprintf(buffer, "neg       %s", D) ;
                retSize = 1 ;
*/			}
			else if (BITS(op,0x0007) == 0x1)
			{
				// NOT - 1mRR HHHW 0110 F001
				DecodeFTable(BITS(op,0x0008), D) ;
				sprintf(buffer, "not       %s", D) ;
				retSize = 1 ;
			}
			else if (BITS(op,0x0007) == 0x2)
			{
				// DEC - 1mRR HHHW 0110 F010
/*              DecodeFTable(BITS(op,0x0008), D) ;
                sprintf(buffer, "dec       %s", D) ;
                retSize = 1 ;
*/			}
			else if (BITS(op,0x0007) == 0x3)
			{
				// DEC24 - 1mRR HHHW 0110 F011
				DecodeFTable(BITS(op,0x0008), D) ;
				sprintf(buffer, "dec24     %s", D) ;
				retSize = 1 ;
			}
			else
			{
				// AND - 1mRR HHHW 0110 F1JJ
				DecodeJJFTable(BITS(op,0x0003),BITS(op,0x0008), S1, D) ;
				sprintf(buffer, "and       %s,%s", S1, D) ;
				retSize = 1 ;
			}
			break ;

		case 0x7:
			if (BITS(op,0x0007) == 0x1)
			{
				// ABS - 1mRR HHHW 0111 F001
/*              DecodeFTable(BITS(op,0x0008), D) ;
                sprintf(buffer, "abs       %s", D) ;
                retSize = 1 ;
*/			}
			else if (BITS(op,0x0007) == 0x2)
			{
				// ROR - 1mRR HHHW 0111 F010
/*              DecodeFTable(BITS(op,0x0008), D) ;
                sprintf(buffer, "ror       %s", D) ;
                retSize = 1 ;
*/			}
			else if (BITS(op,0x0007) == 0x3)
			{
				// ROL - 1mRR HHHW 0111 F011
/*              DecodeFTable(BITS(op,0x0008), D) ;
                sprintf(buffer, "rol       %s", D) ;
                retSize = 1 ;
*/			}
			else
			{
				// CMPM - 1mRR HHHW 0111 FJJJ
/*              DecodeJJJFTable(BITS(op,0x0007), BITS(op,0x0008), S1, D) ;
                sprintf(buffer, "cmpm      %s,%s", S1,D) ;
                retSize = 1 ;
*/			}

			break ;
	}

/*
    // Otherwise you're looking at a MPY/MAC operation...
    if (BITS(op,0x0080))                                                                    // Maybe i should un-consolidate here?
    {
        DecodeQQQFTable(BITS(op,0x0007), BITS(op,0x0008), S1, S2, D) ;
        DecodekSignTable(BITS(op,0x0040), sign) ;

        switch(BITS(op,0x00b0))
        {
            // MPY - 1mRR HHHH 1k00 FQQQ
            case 0x4: sprintf(buffer, "mpy       (%s)%s,%s,%s", sign, S2, S1, D)  ; break ;

            // MPYR - 1mRR HHHH 1k01 FQQQ
            case 0x5: sprintf(buffer, "mpyr      (%s)%s,%s,%s", sign, S2, S1, D) ; break ;

            // MAC - 1mRR HHHH 1k10 FQQQ
            case 0x6: sprintf(buffer, "mac       (%s)%s,%s,%s", sign, S2, S1, D)  ; break ;

            // MACR - 1mRR HHHH 1k11 FQQQ
            case 0x7: sprintf(buffer, "macr      (%s)%s,%s,%s", sign, S1, S2, D) ; break ;
            // !! It's a little odd that macr is S1,S2 while everyone else is S2,S1 !!
        }

        retSize = 1 ;
    }
*/

//  mame_printf_debug("op : %04x parallelType : %d\n", op, parallelType) ;

	switch (parallelType)
	{
		case PARALLEL_TYPE_XMDM:
			AppendXMDM(buffer, op) ;
			break ;

		case PARALLEL_TYPE_XMDM_SPECIAL:
			AppendXMDMSpecial(buffer, op, D) ;
			break ;

		case PARALLEL_TYPE_NODM:
			// Do Nothing :)...
			break ;

		case PARALLEL_TYPE_ARU:
			AppendARU(buffer, op) ;
			break ;

		case PARALLEL_TYPE_RRDM:
			AppendRRDM(buffer, op, D) ;
			break ;
	}

	return retSize ;
}


#ifdef UNUSED_FUNCTION
static unsigned DecodeDXMDROpcode(char *buffer, UINT16 op, unsigned pc)
{
	unsigned retSize = 0 ;

	char S1[32] ;
	char S2[32] ;
	char D[32] ;
	char arg[32] ;

	if (!BITS(op,0x0080))
	{
		if (BITS(op,0x0014) != 0x2)
		{
			// ADD - 011m mKKK 0rru Fuuu
			// SUB - 011m mKKK 0rru Fuuu
			DecodeuuuuFTable(BITS(op,0x0013), BITS(op,0x0008), arg, S1, D) ;
			sprintf(buffer, "%s       %s,%s", arg, S1, D) ;
			retSize = 1 ;
		}
		else if (BITS(op,0x0014) == 0x2)
		{
			// TFR - 011m mKKK 0rr1 F0DD
			DecodeDDFTable(BITS(op,0x0003), BITS(op,0x0008), S1, D) ;
			sprintf(buffer, "tfr       %s,%s", S1, D) ;
			retSize = 1 ;

			// MOVE - 011m mKKK 0rr1 0000
			// !!! What?  The opcode is .identical. to the TFR one.  Wait, that makes sense, right?
		}
	}
	else
	{
		switch (BITS(op,0x0014))
		{
			case 0x0:
				// MPY - 011m mKKK 1xx0 F0QQ
				// What the hell are those two x's : i bet they're supposed to be -'s?
				DecodeQQFTable(BITS(op,0x0003), BITS(op,0x0008), S1, S2, D) ;
				sprintf(buffer, "mpy       %s,%s,%s", S1, S2, D) ;
				retSize = 1 ;
				break ;

			case 0x1:
				// MAC - 011m mKKK 1xx0 F1QQ
				// What the hell are those two x's : i bet they're supposed to be -'s?
				DecodeQQFTable(BITS(op,0x0003), BITS(op,0x0008), S1, S2, D) ;
				sprintf(buffer, "mac       %s,%s,%s", S1, S2, D) ;
				retSize = 1 ;
				break ;

			case 0x2:
				// MPYR - 011m mKKK 1--1 F0QQ
				DecodeQQFTable(BITS(op,0x0003), BITS(op,0x0008), S1, S2, D) ;
				sprintf(buffer, "mpyr      %s,%s,%s", S1, S2, D) ;
				retSize = 1 ;
				break ;

			case 0x3:
				// MACR - 011m mKKK 1--1 F1QQ
				DecodeQQFTable(BITS(op,0x0003), BITS(op,0x0008), S1, S2, D) ;
				sprintf(buffer, "macr      %s,%s,%s", S1, S2, D) ;
				retSize = 1 ;
				break ;
		}
	}

	AppendDXMDR(buffer, op) ;

	return retSize ;
}
#endif


static unsigned DecodeNPMOpcode(char *buffer, UINT16 op, unsigned pc, const UINT8 *oprom)
{
	unsigned retSize = 0 ;

	char S1[32] ;
	char S2[32] ;
	char SD[32] ;
	char D[32] ;
	char A[32] ;
	char M[32] ;
	char args[32] ;
	int Rnum ;


	if (BITS(op,0x0f00) == 0x4)
	{
		// Bitfield Immediate
		int upperMiddleLower = -1 ;
		UINT16 iVal = 0x0000 ;
        UINT16 rVal = 0x0000 ;
		char D[32] ;

		UINT32 op2 = oprom[2] | (oprom[3] << 8);

		// Decode the common parts
		upperMiddleLower = DecodeBBBTable(BITS(op2,0xe000)) ;
		iVal = BITS(op2,0x00ff) ;

		switch(upperMiddleLower)
		{
			case BBB_UPPER:  iVal <<= 8; break ;
			case BBB_MIDDLE: iVal <<= 4; break ;
			case BBB_LOWER:  iVal <<= 0; break ;
		}

		switch(BITS(op,0x00e0))
		{
			case 0x6: case 0x7: case 0x2: case 0x3:
				AssembleDFromPTable(BITS(op,0x0020), BITS(op,0x001f), D) ;
				break ;

			case 0x5: case 0x1:
				// !! Probably want to combine this into something that returns a 'string'... !!
				rVal = DecodeRRTable(BITS(op,0x0003)) ;
				sprintf(D, "X:(R%d)", rVal) ;
				break ;

			case 0x4: case 0x0:
				DecodeDDDDDTable(BITS(op,0x001f), D) ;
				break ;
		}

		switch(BITS(op2,0x1f00))
		{
			// !!! retsize = 2 put here for debugging help - put it at the end eventually...
			case 0x12: sprintf(buffer, "bfchg     #%04x,%s", iVal, D)  ; retSize = 2 ; break ;
			case 0x04: sprintf(buffer, "bfclr     #%04x,%s", iVal, D)  ; retSize = 2 ; break ;
			case 0x18: sprintf(buffer, "bfset     #%04x,%s", iVal, D)  ; retSize = 2 ; break ;
//          case 0x10: sprintf(buffer, "bftsth    #%04x,%s", iVal, D) ; retSize = 2 ; break ;
			case 0x00: sprintf(buffer, "bftstl    #%04x,%s", iVal, D) ; retSize = 2 ; break ;
		}

		// uncomment me someday
//      retSize = 2 ;
	}
	else if (BITS(op,0x0f00) == 0x5)
	{
		switch(BITS(op,0x0074))
		{
			case 0x0:
				if (BITS(op,0x0006) == 0x0)
				{
					// TFR(2) - 0001 0101 0000 F00J
/*                  DecodeJFTable(BITS(op,0x0001),BITS(op,0x0008), D, S1) ;
                    sprintf(buffer, "tfr2      %s,%s", S1, D) ;
                    retSize = 1 ;
*/					// !?! Documentation bug !?!
					// !!!  the source and destination are backwards for TFR(2), and no mention of it is made !!!
				}
				else if (BITS(op,0x0006) == 0x1)
				{
					// ADC - 0001 0101 0000 F01J
/*                  DecodeJFTable(BITS(op,0x0001),BITS(op,0x0008), S1, D) ;
                    sprintf(buffer, "adc       %s,%s", S1, D) ;
                    retSize = 1 ;
*/				}
				break ;

			case 0x3:
					// TST(2) - 0001 0101 0001 -1DD
					DecodeDDTable(BITS(op,0x0003), S1) ;
					sprintf(buffer, "tst(2)    %s", S1) ;
					retSize = 1 ;
				break ;

			case 0x4:
					// NORM - 0001 0101 0010 F0RR
/*                  DecodeFTable(BITS(op,0x0008), D) ;
                    Rnum = DecodeRRTable(BITS(op,0x0003)) ;
                    sprintf(buffer, "norm      %s,R%d", S1, Rnum) ;
                    retSize = 1 ;
*/				break ;

			case 0x6:
				if (BITS(op,0x0003) == 0x0)
				{
					// ASR4 - 0001 0101 0011 F000
/*                  DecodeFTable(BITS(op,0x0008), D) ;
                    sprintf(buffer, "asr4      %s", D) ;
                    retSize = 1 ;
*/				}
				else if (BITS(op,0x0003) == 0x1)
				{
					// ASL4 - 0001 0101 0011 F001
/*                  DecodeFTable(BITS(op,0x0008), D) ;
                    sprintf(buffer, "asl4      %s", D) ;
                    retSize = 1 ;
*/				}
				break ;

			case 0x1: case 0x5: case 0x9: case 0xd:
					// DIV - 0001 0101 0--0 F1DD
/*                  DecodeDDFTable(BITS(op,0x0003), BITS(op,0x0008), S1, D) ;
                    sprintf(buffer, "div       %s,%s", S1, D) ;
                    retSize = 1 ;
*/				break ;

			case 0xa:
				if (BITS(op,0x0003) == 0x0)
				{
					// ZERO - 0001 0101 0101 F000
/*                  DecodeFTable(BITS(op,0x0008), D) ;
                    sprintf(buffer, "zero      %s", D) ;
                    retSize = 1 ;
*/				}
				else if (BITS(op,0x0003) == 0x2)
				{
					// EXT - 0001 0101 0101 F010
/*                  DecodeFTable(BITS(op,0x0008), D) ;
                    sprintf(buffer, "ext       %s", D) ;
                    retSize = 1 ;
*/				}
				break ;

			case 0xc:
				if (BITS(op,0x0003) == 0x0)
				{
					// NEGC - 0001 0101 0110 F000
/*                  DecodeFTable(BITS(op,0x0008), D) ;
                    sprintf(buffer, "negc      %s", D) ;
                    retSize = 1 ;
*/				}
				break ;

			case 0xe:
				if (BITS(op,0x0003) == 0x0)
				{
					// ASR16 - 0001 0101 0111 F000
/*                  DecodeFTable(BITS(op,0x0008), D) ;
                    sprintf(buffer, "asr16     %s", D) ;
                    retSize = 1 ;
*/				}
				else if (BITS(op,0x0003) == 0x1)
				{
					// SWAP - 0001 0101 0111 F001
/*                  DecodeFTable(BITS(op,0x0008), D) ;
                    sprintf(buffer, "swap      %s", D) ;
                    retSize = 1 ;
*/				}
				break ;
		}

		switch(BITS(op,0x00f0))
		{
			case 0x8:
				// IMPY - 0001 0101 1000 FQQQ
/*              DecodeQQQFTable(BITS(op,0x0007), BITS(op,0x0008), S1, S2, D) ;
                sprintf(buffer, "impy      %s,%s,%s", S1, S2, D) ;
                retSize = 1 ;
*/				break ;
			case 0xa:
				// IMAC - 0001 0101 1010 FQQQ
/*              DecodeQQQFTable(BITS(op,0x0007), BITS(op,0x0008), S1, S2, D) ;
                sprintf(buffer, "imac      %s,%s,%s", S1, S2, D) ;
                retSize = 1 ;
*/				break ;
			case 0x9: case 0xb:
				// DMAC(ss,su,uu) - 0001 0101 10s1 FsQQ
/*              DecodessTable(BITS(op,0x0024), A) ;
                DecodeQQFTableSp(BITS(op,0x0003), BITS(op,0x0008), S2, S1, D) ;     // Special QQF
                sprintf(buffer, "dmac(%s)  %s,%s,%s", A, S1, S2, D) ;
                retSize = 1 ;
*/				break ;
			case 0xc:
				// MPY(su,uu) - 0001 0101 1100 FsQQ
/*              DecodesTable(BITS(op,0x0004), A) ;
                DecodeQQFTableSp(BITS(op,0x0003), BITS(op,0x0008), S2, S1, D) ;     // Special QQF
                sprintf(buffer, "mpy(%s)   %s,%s,%s", A, S1, S2, D) ;
                retSize = 1 ;
*/				break ;
			case 0xe:
				// MAC(su,uu) - 0001 0101 1110 FsQQ
				DecodesTable(BITS(op,0x0004), A) ;
				DecodeQQFTableSp(BITS(op,0x0003), BITS(op,0x0008), S2, S1, D) ;		// Special QQF
				sprintf(buffer, "mac(%s)   %s,%s,%s", A, S1, S2, D) ;
				retSize = 1 ;
				break ;
		}
	}
	else if ((BITS(op,0x0f00) == 0x6))
	{
/*
        char S[32] ;

        // MPY - 0001 0110 RRDD FQQQ
        DecodekTable(BITS(op,0x0100), Dnot) ;
        Rnum = DecodeRRTable(BITS(op,0x00c0)) ;
        DecodeDDTable(BITS(op,0x0030), S) ;
        DecodeQQQFTable(BITS(op,0x0007), BITS(op,0x0008), S1, S2, D) ;
        sprintf(buffer, "mpy       %s,%s,%s %s,(R%d)+N%d %s,%s", S1, S2, D, Dnot, Rnum, Rnum, S, Dnot) ;
        // Strange, but not entirely out of the question - this 'k' parameter is hardcoded
        // I cheat here and do the parallel memory data move above - this specific one is only used twice
        retSize = 1 ;
*/	}
	else if ((BITS(op,0x0f00) == 0x7))
	{
/*
        char S[32] ;

        // MAC - 0001 0111 RRDD FQQQ
        DecodekTable(BITS(op,0x0100), Dnot) ;
        Rnum = DecodeRRTable(BITS(op,0x00c0)) ;
        DecodeDDTable(BITS(op,0x0030), S) ;
        DecodeQQQFTable(BITS(op,0x0007), BITS(op,0x0008), S1, S2, D) ;
        sprintf(buffer, "mac       %s,%s,%s %s,(R%d)+N%d %s,%s", S1, S2, D, Dnot, Rnum, Rnum, S, Dnot) ;
        // Strange, but not entirely out of the question - this 'k' parameter is hardcoded
        // I cheat here and do the parallel memory data move above - this specific one is only used twice
        retSize = 1 ;
*/	}
	else if ((BITS(op,0x0800) == 0x1))
	{
		if (BITS(op,0x0600))
		{
			if (!BITS(op,0x0100))
			{
				// ANDI - 0001 1EE0 iiii iiii
				DecodeEETable(BITS(op,0x0600), D) ;
				sprintf(buffer, "and(i)    #%02x,%s", BITS(op,0x00ff), D) ;
				retSize = 1 ;
			}
			else
			{
				// ORI - 0001 1EE1 iiii iiii
/*              DecodeEETable(BITS(op,0x0600), D) ;
                sprintf(buffer, "or(i)     #%02x,%s", BITS(op,0x00ff), D) ;
                retSize = 1 ;
*/			}
		}
		else
		{
			if (!BITS(op,0x0020))
			{
				// MOVE(S) - 0001 100W HH0a aaaa
/*
                DecodeHHTable(BITS(op,0x00c0), SD) ;
                sprintf(A, "X:%02x", BITS(op,0x001f)) ;
                AssembleArgumentsFromWTable(BITS(op,0x0100), args, 'X', SD, A) ;
                sprintf(buffer, "move(s)   %s", args) ;
                retSize = 1 ;
*/			}
			else
			{
				char fullAddy[128] ;									// Convert Short Absolute Address to full 16-bit

				// MOVE(P) - 0001 100W HH1p pppp
				DecodeHHTable(BITS(op,0x00c0), SD) ;

				AssembleAddressFromIOShortAddress(BITS(op,0x001f), fullAddy) ;

				sprintf(A, "%02x (%s)", BITS(op,0x001f), fullAddy) ;
				AssembleArgumentsFromWTable(BITS(op,0x0100), args, 'X', SD, A) ;
				sprintf(buffer, "move(p)   %s", args) ;
				retSize = 1 ;
			}
		}
	}
	else if ((BITS(op,0x0c00) == 0x0))
	{
		// T.cc - 0001 00cc ccTT Fh0h
		DecodeccccTable(BITS(op,0x03c0), M) ;
		Rnum = DecodeRRTable(BITS(op,0x0030)) ;
		Decodeh0hFTable(BITS(op,0x0007),BITS(op,0x0008), S1, D) ;
		if (S1[0] == D[0] && D[0] == 'A')
			sprintf(buffer, "t.%s  %s,%s", M, S1, D) ;
		else
			sprintf(buffer, "t.%s  %s,%s  R0,R%d", M, S1, D, Rnum) ;
		// !! Might not be right - gotta' double-check !!
		retSize = 1 ;
	}

	return retSize ;
}


unsigned DecodeMisfitOpcode(char *buffer, UINT16 op, unsigned pc, const UINT8 *oprom)
{
	unsigned retSize = 0 ;

	char S1[32] ;
	char D1[32] ;
	char SD[32] ;
	char M[32] ;
	char ea[32] ;
	char args[32] ;
	int Rnum ;

	if (BITS(op,0x1000)== 0x0)
	{
		switch(BITS(op,0x0c00))
		{
			// MOVE(I) - 0010 00DD BBBB BBBB
			case 0x0:
				DecodeDDTable(BITS(op,0x0300), D1) ;
				sprintf(buffer, "move(i)   #%02x,%s", BITS(op,0x00ff), D1) ;
				retSize = 1 ;
				break ;

			// TFR(3) - 0010 01mW RRDD FHHH
			case 0x1:
/*              DecodeDDFTable(BITS(op,0x0030), BITS(op,0x0008), D1, S1) ;          // Intentionally switched
                DecodeHHHTable(BITS(op,0x0007), SD) ;
                Rnum = DecodeRRTable(BITS(op,0x00c0)) ;
                AssembleeaFrommTable(BITS(op,0x0200), Rnum, ea) ;
                AssembleArgumentsFromWTable(BITS(op,0x0100), args, 'X', SD, ea) ;
                sprintf(buffer, "tfr3      %s,%s %s", S1, D1, args) ;
                retSize = 1 ;
*/				break ;

			// MOVE(C) - 0010 10dd dddD DDDD
			case 0x2:
				DecodeDDDDDTable(BITS(op,0x03e0), S1) ;
				DecodeDDDDDTable(BITS(op,0x001f), D1) ;
				sprintf(buffer, "move(c)   %s,%s", S1, D1) ;
				retSize = 1 ;
				break ;

			// B.cc - 0010 11cc ccee eeee
			case 0x3:
				DecodeccccTable(BITS(op,0x3c0), M) ;
				AssembleAddressFrom6BitSignedRelativeShortAddress(BITS(op,0x003f), ea) ;
				sprintf(buffer, "b.%s  %s (%02x)", M, ea, BITS(op,0x003f)) ;
				retSize = 1 ;
				break ;
		}
	}
	else
	{
		if (BITS(op,0x0010) == 0x0)
		{
			// MOVE(C) - 0011 1WDD DDD0 MMRR
			DecodeDDDDDTable(BITS(op,0x03e0), SD) ;
			Rnum = DecodeRRTable(BITS(op,0x0003)) ;
			AssembleeaFromMMTable(BITS(op,0x000c), Rnum, ea) ;
			AssembleArgumentsFromWTable(BITS(op,0x0400), args, 'X', SD, ea) ;
			sprintf(buffer, "move(c)   %s", args) ;
			retSize = 1 ;
		}
		else
		{
			if (BITS(op,0x0004) == 0x0)
			{
				// MOVE(C) - 0011 1WDD DDD1 q0RR
	            DecodeDDDDDTable(BITS(op,0x03e0), SD) ;
                Rnum = DecodeRRTable(BITS(op,0x0003)) ;
                AssembleeaFromqTable(BITS(op,0x0008), Rnum, ea) ;
                AssembleArgumentsFromWTable(BITS(op,0x0400), args, 'X', SD, ea) ;
                sprintf(buffer, "move(c)   %s", args) ;
                retSize = 1 ;
			}
			else
			{
				switch(BITS(op,0x0006))
				{
					// MOVE(C) - 0011 1WDD DDD1 t10- xxxx xxxx xxxx xxxx
					case 0x2:
						DecodeDDDDDTable(BITS(op,0x03e0), SD) ;
						AssembleeaFromtTable(BITS(op,0x0008), oprom[2] | (oprom[3] << 8), ea) ;
						// !!! I'm pretty sure this is  in the right order - same issue as the WTables !!!
						if (BITS(op,0x0400))												// fixed - 02/03/05
							sprintf(args, "%s,%s", ea, SD) ;
						else
							sprintf(args, "%s,%s", SD, ea) ;
						sprintf(buffer, "move(c)   %s", args) ;
						retSize = 2 ;
						break ;

					// MOVE(C) - 0011 1WDD DDD1 Z11-
					case 0x3:
                        DecodeDDDDDTable(BITS(op,0x03e0), SD) ;
                        DecodeZTable(BITS(op,0x0008), ea) ;				// Fixed - 11/26/06
                        AssembleArgumentsFromWTable(BITS(op,0x0400), args, 'X', SD, ea) ;
                        sprintf(buffer, "move(c)   %s", args) ;
                        retSize = 1 ;
						break ;
				}
			}
		}
	}

	return retSize ;
}


static unsigned DecodeSpecialOpcode(char *buffer, UINT16 op, unsigned pc, const UINT8 *oprom)
{
	unsigned retSize = 0 ;

	char S1[32] ;
	char SD[32] ;
	char M[32] ;
	char args[32] ;
	char ea[32] ;
	int Rnum ;

	if (BITS(op,0x0ff0) == 0x0)
	{
		switch (BITS(op,0x000f))
		{
			// NOP - 0000 0000 0000 0000
			case 0x0:
				sprintf(buffer, "nop") ;
				retSize = 1 ;
				break ;

			// Debug - 0000 0000 0000 0001
			case 0x1:
/*              sprintf(buffer, "debug") ;
                retSize = 1 ;
*/				break ;

			// DO FOREVER - 0000 0000 0000 0010 xxxx xxxx xxxx xxxx
			case 0x2:
				sprintf(buffer, "doForever %04x", oprom[2] | (oprom[3] << 8)) ;
				retSize = 2 ;
				break ;

			// chkaau - 0000 0000 0000 0100
			case 0x4:
/*              sprintf(buffer, "chkaau") ;
                retSize = 1 ;
*/				break ;

			// SWI - 0000 0000 0000 0101
			case 0x5:
/*              sprintf(buffer, "swi") ;
                retSize = 1 ;
*/				break ;

			// RTS - 0000 0000 0000 0110
			case 0x6:
				sprintf(buffer, "rts") ;
				retSize = 1 | DASMFLAG_STEP_OUT;
				break ;

			// RTI - 0000 0000 0000 0111
			case 0x7:
	            sprintf(buffer, "rti") ;
                retSize = 1 | DASMFLAG_STEP_OUT;
				break ;

			// RESET - 0000 0000 0000 1000
			case 0x8:
/*              sprintf(buffer, "reset") ;
                retSize = 1 ;
*/				break ;

			// Enddo - 0000 0000 0000 1001
			case 0x9:
/*              sprintf(buffer, "endDo") ;
                retSize = 1 ;
*/				break ;

			// STOP - 0000 0000 0000 1010
			case 0xa:
/*              sprintf(buffer, "stop") ;
                retSize = 1 ;
*/				break ;

			// WAIT - 0000 0000 0000 1011
			case 0xb:
/*              sprintf(buffer, "wait") ;
                retSize = 1 ;
*/				break ;

			// ILLEGAL - 0000 0000 0000 1111
			case 0xf:
/*              sprintf(buffer, "illegal") ;
                retSize = 1 ;
*/				break ;

		}
	}
	else if (BITS(op,0x0f00) == 0x0)
	{
		switch(BITS(op,0x00e0))
		{
			case 0x2:
				// DEBUG.cc - 0000 0000 0101 cccc
/*              DecodeccccTable(BITS(op,0x000f), M) ;
                sprintf(buffer, "debug.%s", M) ;
                retSize = 1 ;
*/				break ;

			case 0x6:
				// DO - 0000 0000 110- --RR xxxx xxxx xxxx xxxx
/*              Rnum = DecodeRRTable(BITS(op,0x0003)) ;
                sprintf(buffer, "do        X:(R%d),%02x", Rnum, oprom[2] | (oprom[3] << 8)) ;
                retSize = 2 ;
*/				break ;

			case 0x7:
				// REP - 0000 0000 111- --RR
/*              Rnum = DecodeRRTable(BITS(op,0x0003)) ;
                sprintf(buffer, "rep       X:(R%d)", Rnum) ;
                retSize = 1 ;
*/				break ;
		}
	}
	else if (BITS(op,0x0f00) == 0x1)
	{
		if (BITS(op,0x00f0) == 0x1)
		{
			// BRK.cc - 0000 0001 0001 cccc
/*          DecodeccccTable(BITS(op,0x000f), M) ;
            sprintf(buffer, "brk.%s", M) ;
            retSize = 1 ;
*/		}
		else if (BITS(op,0x00f0) == 0x2)
		{
			switch(BITS(op,0x000c))								// Consolidation can happen here
			{
				// JSR - 0000 0001 0010 00RR
				case 0x0:
/*                  Rnum = DecodeRRTable(BITS(op,0x0003)) ;
                    sprintf(buffer, "jsr       R%d", Rnum) ;
                    retSize = 1 ; // I think aaron guessed incorrectly? | DASMFLAG_STEP_OVER;
*/					break ;

				// JMP - 0000 0001 0010 01RR
				case 0x1:
					Rnum = DecodeRRTable(BITS(op,0x0003)) ;
                    sprintf(buffer, "jmp       R%d", Rnum) ;
                    retSize = 1 ;
					break ;

				// BSR - 0000 0001 0010 10RR
				case 0x2:
/*                  Rnum = DecodeRRTable(BITS(op,0x0003)) ;
                    sprintf(buffer, "bsr       R%d", Rnum) ;
                    retSize = 1 | DASMFLAG_STEP_OVER;
*/					break ;

				// BRA - 0000 0001 0010 11RR
				case 0x3:
/*                  Rnum = DecodeRRTable(BITS(op,0x0003)) ;
                    sprintf(buffer, "bra       R%d", Rnum) ;
                    retSize = 1 ;
*/					break ;
			}
		}
		else if (BITS(op,0x00f0) == 0x3)
		{
			switch(BITS(op,0x000c))
			{
				// JSR - 0000 0001 0011 00-- xxxx xxxx xxxx xxxx
				case 0x0:
	                sprintf(buffer, "jsr       %04x", oprom[2] | (oprom[3] << 8)) ;
                    retSize = 2 | DASMFLAG_STEP_OVER;
					break ;

				// JMP - 0000 0001 0011 01-- xxxx xxxx xxxx xxxx
				case 0x1:
					sprintf(buffer, "jmp       %04x", oprom[2] | (oprom[3] << 8)) ;
					retSize = 2 ;
					break ;

				// BSR - 0000 0001 0011 10-- xxxx xxxx xxxx xxxx
				case 0x2:
					sprintf(buffer, "bsr       %d (0x%04x)", oprom[2] | (oprom[3] << 8), oprom[2] | (oprom[3] << 8)) ;
					retSize = 2 | DASMFLAG_STEP_OVER;
					break ;

				// BRA - 0000 0001 0011 11-- xxxx xxxx xxxx xxxx
				case 0x3:
/*                  sprintf(buffer, "bra       %d (%04x)", oprom[2] | (oprom[3] << 8), oprom[2] | (oprom[3] << 8)) ;
                    retSize = 2 ;
*/					break ;
			}
		}
		else if (BITS(op,0x00f0) == 0x5)
		{
			// REP.cc - 0000 0001 0101 cccc
/*          DecodeccccTable(BITS(op,0x000f), M) ;
            sprintf(buffer, "rep.%s", M) ;
            retSize = 1 ;
*/			// !!! Should I decode the next instruction and put it here ???  probably...
		}
		else if (BITS(op,0x0080) == 0x1)
		{
			// LEA - 0000 0001 10TT MMRR - 0000 0001 11NN MMRR
/*          Rnum = DecodeRRTable(BITS(op,0x0030)) ;
            AssembleeaFromMMTable(BITS(op,0x000c), BITS(op,0x0003), ea) ;
            if (BITS(op,0x0040))
                sprintf(buffer, "lea       %s,R%d", ea, Rnum) ;
            else
                sprintf(buffer, "lea       %s,N%d", ea, Rnum) ;
            retSize = 1 ;
*/		}
	}
	else if (BITS(op,0x0e00) == 0x1)
	{

		if (BITS(op,0x0020) == 0x0)
		{
			// MOVE(M) - 0000 001W RR0M MHHH
			Rnum = DecodeRRTable(BITS(op,0x00c0)) ;
			DecodeHHHTable(BITS(op,0x0007), SD) ;
			AssembleeaFromMMTable(BITS(op,0x0018), Rnum, ea) ;
			AssembleArgumentsFromWTable(BITS(op,0x0100), args, 'P', SD, ea) ;
			sprintf(buffer, "move(m)   %s", args) ;
			// !!! The docs list the read/write order backwards for all move(m)'s - crackbabies ???
			retSize = 1 ;
		}
		else
		{
			// MOVE(M) - 0000 001W RR11 mmRR
/*          AssembleeaFrommmTable(BITS(op,0x000c), BITS(op,0x00c0), BITS(op,0x0003), ea, ea2) ;
            sprintf(SD, "P:%s", ea) ;
            AssembleArgumentsFromWTable(BITS(op,0x0100), args, 'X', SD, ea2) ;
            // !!! The docs list the read/write order backwards for all move(m)'s - crackbabies ???
            sprintf(buffer, "move(m)   %s", args) ;
            retSize = 1 ;
*/		}
	}
	else if (BITS(op,0x0f00) == 0x4)
	{
		if (BITS(op,0x0020) == 0x0)
		{
			// DO - 0000 0100 000D DDDD xxxx xxxx xxxx xxxx
			DecodeDDDDDTable(BITS(op,0x001f), S1) ;
			sprintf(buffer, "do        %s,%04x", S1, oprom[2] | (oprom[3] << 8)) ;
			retSize = 2 ;
		}
		else
		{
			// REP - 0000 0100 001D DDDD
            DecodeDDDDDTable(BITS(op,0x001f), S1) ;
            sprintf(buffer, "rep       %s", S1) ;
            retSize = 1 ;
		}
	}
	else if (BITS(op,0x0f00) == 0x5)
	{
		UINT8 B ;
		UINT16 op2 = oprom[2] | (oprom[3] << 8) ;

		if (BITS(op2,0xfe20) == 0x02)
		{
			// MOVE(M) - 0000 0101 BBBB BBBB | 0000 001W --0- -HHH
			B = BITS(op,0x00ff) ;
			DecodeHHHTable(BITS(op2,0x0007), SD) ;
			AssembleRegFromWTable(BITS(op2,0x0100), args, 'P', SD, B) ;
			sprintf(buffer, "move(m)   %s", args) ;
			// !!! The docs list the read/write order backwards for all move(m)'s - crackbabies ???
			retSize = 2 ;
		}
		else if (BITS(op2,0xf810) == 0x0e)
		{
			// MOVE(C) - 0000 0101 BBBB BBBB | 0011 1WDD DDD0 ----
/*          B = BITS(op,0x00ff) ;
            DecodeDDDDDTable(BITS(op2,0x03e0), SD) ;
            AssembleRegFromWTable(BITS(op2,0x0400), args, 'X', SD, B) ;
            sprintf(buffer, "move(c)   %s", args) ;
            retSize = 2 ;
*/		}
		else if (BITS(op2,0x00ff) == 0x11)
		{
			// MOVE - 0000 0101 BBBB BBBB | ---- HHHW 0001 0001
/*          B = BITS(op,0x00ff) ;
            DecodeHHHTable(BITS(op2,0x0e00), SD) ;
            AssembleRegFromWTable(BITS(op2,0x0100), args, 'X', SD, B) ;
            sprintf(buffer, "move      %s", args) ;
            retSize = 2 ;
*/		}
	}
	else if (BITS(op,0x0f00) == 0x6)
	{
		switch(BITS(op,0x0030))
		{
			case 0x0:
				// JS.cc - 0000 0110 RR00 cccc
/*              DecodeccccTable(BITS(op,0x000f), M) ;
                Rnum = DecodeRRTable(BITS(op,0x00c0)) ;
                sprintf(buffer, "js.%s  R%d\n", M, Rnum) ;
                retSize = 1 ;
*/				break ;

			case 0x1:
				// JS.cc - 0000 0110 --01 cccc xxxx xxxx xxxx xxxx
/*              DecodeccccTable(BITS(op,0x000f), M) ;
                sprintf(buffer, "js.%s  %04x", M, oprom[2] | (oprom[3] << 8)) ;
                retSize = 2 ;
*/				break ;

			case 0x2:
				// J.cc - 0000 0110 RR10 cccc
/*              DecodeccccTable(BITS(op,0x000f), M) ;
                Rnum = DecodeRRTable(BITS(op,0x00c0)) ;
                sprintf(buffer, "j.%s  R%d", M, Rnum) ;
                retSize = 1 ;
*/				break ;

			case 0x3:
				// J.cc - 0000 0110 --11 cccc xxxx xxxx xxxx xxxx
/*              DecodeccccTable(BITS(op,0x000f), M) ;
                sprintf(buffer, "j.%s  %04x", M, oprom[2] | (oprom[3] << 8)) ;
                retSize = 2 ;
*/				break ;
		}
	}
	else if (BITS(op,0x0f00) == 0x7)
	{
		switch(BITS(op,0x0030))
		{
			case 0x0:
				// BS.cc - 0000 0111 RR00 cccc
/*              DecodeccccTable(BITS(op,0x000f), M) ;
                Rnum = DecodeRRTable(BITS(op,0x00c0)) ;
                sprintf(buffer, "bs.%s  R%d\n", M, Rnum) ;
                retSize = 1 ;
*/				break ;

			case 0x1:
				// BS.cc - 0000 0111 --01 cccc xxxx xxxx xxxx xxxx
				DecodeccccTable(BITS(op,0x000f), M) ;
				sprintf(buffer, "bs.%s %d (0x%04x)", M, (INT16)(oprom[2] | (oprom[3] << 8)), oprom[2] | (oprom[3] << 8)) ;
				retSize = 2 ;
				break ;

			case 0x2:
				// B.cc - 0000 0111 RR10 cccc
/*              DecodeccccTable(BITS(op,0x000f), M) ;
                Rnum = DecodeRRTable(BITS(op,0x00c0)) ;
                sprintf(buffer, "b.%s  R%d", M, Rnum) ;
                retSize = 1 ;
*/				break ;

			case 0x3:
				// B.cc - 0000 0111 --11 cccc xxxx xxxx xxxx xxxx
				DecodeccccTable(BITS(op,0x000f), M) ;
				sprintf(buffer, "b.%s  %04x (%d)", M, oprom[2] | (oprom[3] << 8), oprom[2] | (oprom[3] << 8)) ;
				retSize = 2 ;
				break ;
		}
	}
	else if (BITS(op,0x0800))
	{
		switch (BITS(op,0x0700))
		{
			// JSR - 0000 1010 AAAA AAAA
			case 0x2:
/*              sprintf(buffer, "jsr       %02x", BITS(op,0x00ff)) ;
                retSize = 1 ;
*/				break ;

			// BRA - 0000 1011 aaaa aaaa
			case 0x3:
				sprintf(buffer, "bra       %d (0x%02x)", (INT8)BITS(op,0x00ff), BITS(op,0x00ff)) ;
				retSize = 1 ;
				break ;

			// MOVE(P) - 0000 110W RRmp pppp
			case 0x4: case 0x5:
			{
				char fullAddy[128] ;									// Convert Short Absolute Address to full 16-bit
				Rnum = DecodeRRTable(BITS(op,0x00c0)) ;
				AssembleeaFrommTable(BITS(op,0x0020), Rnum, ea) ;

				AssembleAddressFromIOShortAddress(BITS(op,0x001f), fullAddy) ;

				sprintf(SD, "X:%02x (%s)", BITS(op,0x001f), fullAddy) ;

				// !! order (pretty sure pp is S/D in the docs) !!
				AssembleArgumentsFromWTable(BITS(op,0x0100), args, 'X', SD, ea) ;
				sprintf(buffer, "move(p)   %s", args) ;
				retSize = 1 ;
				break ;
			}

			// DO - 0000 1110 iiii iiii xxxx xxxx xxxx xxxx
			case 0x6:
				sprintf(buffer, "do        #%02x,%04x", BITS(op,0x00ff), oprom[2] | (oprom[3] << 8)) ;
				retSize = 2 ;
				break ;

			// REP - 0000 1111 iiii iiii
			case 0x7:
				sprintf(buffer, "rep       #%02x", BITS(op,0x00ff)) ;
				retSize = 1 ;
				break ;
		}
	}

	return retSize ;
}





////////////////////////////////////////
// PARALLEL MEMORY OPERATION DECODING //
////////////////////////////////////////

static void AppendXMDM(char *buffer, UINT16 op)
{
	int Rnum ;
	char SD[32] ;
	char ea[32] ;
	char args[32] ;

	// 1mRR HHHW ---- ----
	Rnum = DecodeRRTable(BITS(op,0x3000)) ;
	DecodeHHHTable(BITS(op,0x0e00), SD) ;
	AssembleeaFrommTable(BITS(op,0x4000), Rnum, ea) ;
	AssembleArgumentsFromWTable(BITS(op,0x0100), args, 'X', SD, ea) ;

	strcat(buffer, "  ")  ;
	strcat(buffer, args) ;
}

// !!! NOT FULLY IMPLEMENTED !!!
static void AppendXMDMSpecial(char *buffer, UINT16 op, char *working)
{
	// This is actually a little tricky 'cuz X:(F1) comes from the 'Data ALU' operation...
	// (page 326 of the family manual)

	// !! Also, p326 mentions 'F1' is the upper word of the accumulator which is not used by the parallel data ALU operation
	//                        (in case of no Data ALU operation, A1 is chosen as F)
	//    my question is : when is there .not. a Data ALU operation ??

	char SD[32] ;
	char args[32] ;
	char dest[32] ;

	if (working[0] == 'B')
		sprintf(dest, "(A1)") ;
	else if (working[0] == 'A')
		sprintf(dest, "(B1)") ;
	else
		sprintf(dest, "(wtf)") ;

	DecodeHHHTable(BITS(op,0x0e00), SD) ;
	AssembleArgumentsFromWTable(BITS(op,0x0100), args, 'X', SD, dest) ;

	strcat(buffer, "  ") ;
	strcat(buffer, args) ;
}

static void AppendARU(char *buffer, UINT16 op)
{
	char ea[32] ;
	int rr = DecodeRRTable(BITS(op,0x0300)) ;

	AssembleeaFromzTable(BITS(op,0x0400), rr, ea) ;

	strcat(buffer, ea) ;
}

static void AppendRRDM(char *buffer, UINT16 op, char *working)
{
	char S[32], D[32] ;
	char final[128] ;

	DecodeIIIITable(BITS(op,0x0f00), S, D) ;

	if (D[0] == '^')
	{
		if (working[0] == 'B')
			sprintf(D, "A") ;
		else if (working[0] == 'A')
			sprintf(D, "B") ;
		else
			sprintf(D, "(wtf)") ;
	}

	if (S[0] == 'F')
	{
		sprintf(S, "%s", working) ;
	}

	sprintf(final, "%s,%s", S, D) ;

	strcat(buffer, "  ") ;
	strcat(buffer, final) ;
}

#ifdef UNUSED_FUNCTION
static void AppendDXMDR(char *buffer, UINT16 op)
{
	char D1[32] ;
	char D2[32] ;
	char ea1[32] ;
	char ea2[32] ;
	int Rnum ;

	char temp[128] ;

	// 011m mKKK -rr- ----
	Rnum = BITS(op,0x0060) ;
	DecodeKKKTable(BITS(op,0x0700), D1, D2) ;
	AssembleeaFrommmTable(BITS(op,0x1800), Rnum, 3, ea1, ea2) ;

	sprintf(temp, "  X:%s,%s X:%s,%s", ea1, D1, ea2, D2) ;
	strcat(buffer, temp) ;
}
#endif



/////////////////////////////
// TABLE DECODER FUNCTIONS //
/////////////////////////////

static int DecodeBBBTable(UINT16 BBB)
{
	switch(BBB)
	{
		case 0x4: return BBB_UPPER  ; break ;
		case 0x2: return BBB_MIDDLE ; break ;
		case 0x1: return BBB_LOWER  ; break ;
	}

	return BBB_LOWER ;                          // Not really safe...
}

static void DecodeccccTable(UINT16 cccc, char *mnemonic)
{
	switch (cccc)
	{
		case 0x0: sprintf(mnemonic, "cc(hs)") ; break ;
		case 0x1: sprintf(mnemonic, "ge    ") ; break ;
		case 0x2: sprintf(mnemonic, "ne    ") ; break ;
		case 0x3: sprintf(mnemonic, "pl    ") ; break ;
		case 0x4: sprintf(mnemonic, "nn    ") ; break ;
		case 0x5: sprintf(mnemonic, "ec    ") ; break ;
		case 0x6: sprintf(mnemonic, "lc    ") ; break ;
		case 0x7: sprintf(mnemonic, "gt    ") ; break ;
		case 0x8: sprintf(mnemonic, "cs(lo)") ; break ;
		case 0x9: sprintf(mnemonic, "lt    ") ; break ;
		case 0xa: sprintf(mnemonic, "eq    ") ; break ;
		case 0xb: sprintf(mnemonic, "mi    ") ; break ;
		case 0xc: sprintf(mnemonic, "nr    ") ; break ;
		case 0xd: sprintf(mnemonic, "es    ") ; break ;
		case 0xe: sprintf(mnemonic, "ls    ") ; break ;
		case 0xf: sprintf(mnemonic, "le    ") ; break ;
	}
}

static void DecodeDDDDDTable(UINT16 DDDDD, char *SD)
{
	switch(DDDDD)
	{
		case 0x00: sprintf(SD, "X0") ;  break ;
		case 0x01: sprintf(SD, "Y0") ;  break ;
		case 0x02: sprintf(SD, "X1") ;  break ;
		case 0x03: sprintf(SD, "Y1") ;  break ;
		case 0x04: sprintf(SD, "A") ;   break ;
		case 0x05: sprintf(SD, "B") ;   break ;
		case 0x06: sprintf(SD, "A0") ;  break ;
		case 0x07: sprintf(SD, "B0") ;  break ;
		case 0x08: sprintf(SD, "LC") ;  break ;
		case 0x09: sprintf(SD, "SR") ;  break ;
		case 0x0a: sprintf(SD, "OMR") ; break ;
		case 0x0b: sprintf(SD, "SP") ;  break ;
		case 0x0c: sprintf(SD, "A1") ;  break ;
		case 0x0d: sprintf(SD, "B1") ;  break ;
		case 0x0e: sprintf(SD, "A2") ;  break ;
		case 0x0f: sprintf(SD, "B2") ;  break ;

		case 0x10: sprintf(SD, "R0") ;  break ;
		case 0x11: sprintf(SD, "R1") ;  break ;
		case 0x12: sprintf(SD, "R2") ;  break ;
		case 0x13: sprintf(SD, "R3") ;  break ;
		case 0x14: sprintf(SD, "M0") ;  break ;
		case 0x15: sprintf(SD, "M1") ;  break ;
		case 0x16: sprintf(SD, "M2") ;  break ;
		case 0x17: sprintf(SD, "M3") ;  break ;
		case 0x18: sprintf(SD, "SSH") ; break ;
		case 0x19: sprintf(SD, "SSL") ; break ;
		case 0x1a: sprintf(SD, "LA") ;  break ;
		//no 0x1b
		case 0x1c: sprintf(SD, "N0") ;  break ;
		case 0x1d: sprintf(SD, "N1") ;  break ;
		case 0x1e: sprintf(SD, "N2") ;  break ;
		case 0x1f: sprintf(SD, "N3") ;  break ;
	}
}

static void DecodeDDTable(UINT16 DD, char *SD)
{
	switch (DD)
	{
		case 0x0: sprintf(SD, "X0") ; break ;
		case 0x1: sprintf(SD, "Y0") ; break ;
		case 0x2: sprintf(SD, "X1") ; break ;
		case 0x3: sprintf(SD, "Y1") ; break ;
	}
}

#ifdef UNUSED_FUNCTION
static void DecodeDDFTable(UINT16 DD, UINT16 F, char *S, char *D)
{
	UINT16 switchVal = (DD << 1) | F ;

	switch (switchVal)
	{
		case 0x0: sprintf(S, "X0") ; sprintf(D, "A") ; break ;
		case 0x1: sprintf(S, "X0") ; sprintf(D, "B") ; break ;
		case 0x2: sprintf(S, "Y0") ; sprintf(D, "A") ; break ;
		case 0x3: sprintf(S, "Y0") ; sprintf(D, "B") ; break ;
		case 0x4: sprintf(S, "X1") ; sprintf(D, "A") ; break ;
		case 0x5: sprintf(S, "X1") ; sprintf(D, "B") ; break ;
		case 0x6: sprintf(S, "Y1") ; sprintf(D, "A") ; break ;
		case 0x7: sprintf(S, "Y1") ; sprintf(D, "B") ; break ;
	}
}
#endif

static void DecodeEETable(UINT16 EE, char *D)
{
	switch(EE)
	{
		case 0x1: sprintf(D, "MR") ;  break ;
		case 0x3: sprintf(D, "CCR") ; break ;
		case 0x2: sprintf(D, "OMR") ; break ;
	}
}

static void DecodeFTable(UINT16 F, char *SD)
{
	switch(F)
	{
		case 0x0: sprintf(SD, "A") ; break ;
		case 0x1: sprintf(SD, "B") ; break ;
	}
}

static void Decodeh0hFTable(UINT16 h0h, UINT16 F, char *S, char *D)
{
	UINT16 switchVal = (h0h << 1) | F ;

	switch (switchVal)
	{
		case 0x8: sprintf(S, "X0") ; sprintf(D, "A") ; break ;
		case 0x9: sprintf(S, "X0") ; sprintf(D, "B") ; break ;
		case 0xa: sprintf(S, "Y0") ; sprintf(D, "A") ; break ;
		case 0xb: sprintf(S, "Y0") ; sprintf(D, "B") ; break ;
		case 0x2: sprintf(S, "A")  ; sprintf(D, "A") ; break ;
		case 0x1: sprintf(S, "A")  ; sprintf(D, "B") ; break ;
		case 0x0: sprintf(S, "B")  ; sprintf(D, "A") ; break ;
		case 0x3: sprintf(S, "B")  ; sprintf(D, "B") ; break ;
	}
}

static void DecodeHHTable(UINT16 HH, char *SD)
{
	switch(HH)
	{
		case 0x0: sprintf(SD, "X0") ; break ;
		case 0x1: sprintf(SD, "Y0") ; break ;
		case 0x2: sprintf(SD, "A")  ; break ;
		case 0x3: sprintf(SD, "B")  ; break ;
	}
}

static void DecodeHHHTable(UINT16 HHH, char *SD)
{
	switch(HHH)
	{
		case 0x0: sprintf(SD, "X0") ; break ;
		case 0x1: sprintf(SD, "Y0") ; break ;
		case 0x2: sprintf(SD, "X1") ; break ;
		case 0x3: sprintf(SD, "Y1") ; break ;
		case 0x4: sprintf(SD, "A")  ; break ;
		case 0x5: sprintf(SD, "B")  ; break ;
		case 0x6: sprintf(SD, "A0") ; break ;
		case 0x7: sprintf(SD, "B0") ; break ;
	}
}

// I don't know if this is ever used?
static void DecodeIIIITable(UINT16 IIII, char *S, char *D)
{
	switch(IIII)
	{
		case 0x0: sprintf(S, "X0") ; sprintf(D, "^F^") ; break ;
		case 0x1: sprintf(S, "Y0") ; sprintf(D, "^F^") ; break ;
		case 0x2: sprintf(S, "X1") ; sprintf(D, "^F^") ; break ;
		case 0x3: sprintf(S, "Y1") ; sprintf(D, "^F^") ; break ;
		case 0x4: sprintf(S, "A") ;  sprintf(D, "X0")  ; break ;
		case 0x5: sprintf(S, "B") ;  sprintf(D, "Y0")  ; break ;
		case 0x6: sprintf(S, "A0") ; sprintf(D, "X0")  ; break ;
		case 0x7: sprintf(S, "B0") ; sprintf(D, "Y0")  ; break ;
		case 0x8: sprintf(S, "F") ;  sprintf(D, "^F^") ; break ;
		case 0x9: sprintf(S, "F") ;  sprintf(D, "^F^") ; break ;
		case 0xc: sprintf(S, "A") ;  sprintf(D, "X1")  ; break ;
		case 0xd: sprintf(S, "B") ;  sprintf(D, "Y1")  ; break ;
		case 0xe: sprintf(S, "A0") ; sprintf(D, "X1")  ; break ;
		case 0xf: sprintf(S, "B0") ; sprintf(D, "Y1")  ; break ;
	}
}

static void DecodeJJJFTable(UINT16 JJJ, UINT16 F, char *S, char *D)
{
	UINT16 switchVal = (JJJ << 1) | F ;

	switch(switchVal)
	{
		case 0x0: sprintf(S, "B")  ; sprintf(D, "A") ; break ;
		case 0x1: sprintf(S, "A")  ; sprintf(D, "B") ; break ;
		case 0x4: sprintf(S, "X")  ; sprintf(D, "A") ; break ;
		case 0x5: sprintf(S, "X")  ; sprintf(D, "B") ; break ;
		case 0x6: sprintf(S, "Y")  ; sprintf(D, "A") ; break ;
		case 0x7: sprintf(S, "Y")  ; sprintf(D, "B") ; break ;
		case 0x8: sprintf(S, "X0") ; sprintf(D, "A") ; break ;
		case 0x9: sprintf(S, "X0") ; sprintf(D, "B") ; break ;
		case 0xa: sprintf(S, "Y0") ; sprintf(D, "A") ; break ;
		case 0xb: sprintf(S, "Y0") ; sprintf(D, "B") ; break ;
		case 0xc: sprintf(S, "X1") ; sprintf(D, "A") ; break ;
		case 0xd: sprintf(S, "X1") ; sprintf(D, "B") ; break ;
		case 0xe: sprintf(S, "Y1") ; sprintf(D, "A") ; break ;
		case 0xf: sprintf(S, "Y1") ; sprintf(D, "B") ; break ;
	}
}

static void DecodeJJFTable(UINT16 JJ, UINT16 F, char *S, char *D)
{
	UINT16 switchVal = (JJ << 1) | F ;

	switch (switchVal)
	{
		case 0x0: sprintf(S, "X0") ; sprintf(D, "A") ; break ;
		case 0x1: sprintf(S, "X0") ; sprintf(D, "B") ; break ;
		case 0x2: sprintf(S, "Y0") ; sprintf(D, "A") ; break ;
		case 0x3: sprintf(S, "Y0") ; sprintf(D, "B") ; break ;
		case 0x4: sprintf(S, "X1") ; sprintf(D, "A") ; break ;
		case 0x5: sprintf(S, "X1") ; sprintf(D, "B") ; break ;
		case 0x6: sprintf(S, "Y1") ; sprintf(D, "A") ; break ;
		case 0x7: sprintf(S, "Y1") ; sprintf(D, "B") ; break ;
	}
}

#ifdef UNUSED_FUNCTION
static void DecodeJFTable(UINT16 J, UINT16 F, char *S, char *D)
{
	UINT16 switchVal = (J << 1) | F ;

	switch(switchVal)
	{
		case 0x0: sprintf(S, "X") ; sprintf(D, "A") ; break ;
		case 0x1: sprintf(S, "X") ; sprintf(D, "B") ; break ;
		case 0x2: sprintf(S, "Y") ; sprintf(D, "A") ; break ;
		case 0x3: sprintf(S, "Y") ; sprintf(D, "B") ; break ;
	}
}

static void DecodekTable(UINT16 k, char *Dnot)
{
	switch(k)
	{
		case 0x0: sprintf(Dnot, "B") ; break ;
		case 0x1: sprintf(Dnot, "A") ; break ;
	}
}

static void DecodekSignTable(UINT16 k, char *plusMinus)
{
	switch(k)
	{
		case 0x0: sprintf(plusMinus, "+") ; break ;
		case 0x1: sprintf(plusMinus, "-") ; break ;
	}
}

static void DecodeKKKTable  (UINT16 KKK, char *D1, char *D2)
{
	switch(KKK)
	{
		case 0x0: sprintf(D1, "^F^") ; sprintf(D2, "X0") ; break ;
		case 0x1: sprintf(D1, "Y0")  ; sprintf(D2, "X0") ; break ;
		case 0x2: sprintf(D1, "X1")  ; sprintf(D2, "X0") ; break ;
		case 0x3: sprintf(D1, "Y1")  ; sprintf(D2, "X0") ; break ;
		case 0x4: sprintf(D1, "X0")  ; sprintf(D2, "X1") ; break ;
		case 0x5: sprintf(D1, "Y0")  ; sprintf(D2, "X1") ; break ;
		case 0x6: sprintf(D1, "^F^") ; sprintf(D2, "Y0") ; break ;
		case 0x7: sprintf(D1, "Y1")  ; sprintf(D2, "X1") ; break ;
	}
}

static int DecodeNNTable(UINT16 NN)
{
	return NN ;
}

static void DecodeQQFTable(UINT16 QQ, UINT16 F, char *S1, char *S2, char *D)
{
	UINT16 switchVal = (QQ << 1) | F ;

	switch(switchVal)
	{
		case 0x0: sprintf(S1, "X0") ; sprintf(S2, "Y0") ; sprintf(D, "A") ; break ;
		case 0x1: sprintf(S1, "X0") ; sprintf(S2, "Y0") ; sprintf(D, "B") ; break ;
		case 0x2: sprintf(S1, "X0") ; sprintf(S2, "Y1") ; sprintf(D, "A") ; break ;
		case 0x3: sprintf(S1, "X0") ; sprintf(S2, "Y1") ; sprintf(D, "B") ; break ;
		case 0x4: sprintf(S1, "X1") ; sprintf(S2, "Y0") ; sprintf(D, "A") ; break ;
		case 0x5: sprintf(S1, "X1") ; sprintf(S2, "Y0") ; sprintf(D, "B") ; break ;
		case 0x6: sprintf(S1, "X1") ; sprintf(S2, "Y1") ; sprintf(D, "A") ; break ;
		case 0x7: sprintf(S1, "X1") ; sprintf(S2, "Y1") ; sprintf(D, "B") ; break ;
	}
}
#endif

static void DecodeQQFTableSp(UINT16 QQ, UINT16 F, char *S1, char *S2, char *D)
{
	UINT16 switchVal = (QQ << 1) | F ;

	switch(switchVal)
	{
		case 0x0: sprintf(S1, "Y0") ; sprintf(S2, "X0") ; sprintf(D, "A") ; break ;
		case 0x1: sprintf(S1, "Y0") ; sprintf(S2, "X0") ; sprintf(D, "B") ; break ;
		case 0x2: sprintf(S1, "Y1") ; sprintf(S2, "X0") ; sprintf(D, "A") ; break ;
		case 0x3: sprintf(S1, "Y1") ; sprintf(S2, "X0") ; sprintf(D, "B") ; break ;
		case 0x4: sprintf(S1, "X1") ; sprintf(S2, "Y0") ; sprintf(D, "A") ; break ;
		case 0x5: sprintf(S1, "X1") ; sprintf(S2, "Y0") ; sprintf(D, "B") ; break ;
		case 0x6: sprintf(S1, "X1") ; sprintf(S2, "Y1") ; sprintf(D, "A") ; break ;
		case 0x7: sprintf(S1, "X1") ; sprintf(S2, "Y1") ; sprintf(D, "B") ; break ;
	}
}

#ifdef UNUSED_FUNCTION
static void DecodeQQQFTable(UINT16 QQQ, UINT16 F, char *S1, char *S2, char *D)
{
	UINT16 switchVal = (QQQ << 1) | F ;

	switch(switchVal)
	{
		case 0x0: sprintf(S1, "X0") ; sprintf(S2, "X0") ; sprintf(D, "A") ; break ;
		case 0x1: sprintf(S1, "X0") ; sprintf(S2, "X0") ; sprintf(D, "B") ; break ;
		case 0x2: sprintf(S1, "X1") ; sprintf(S2, "X0") ; sprintf(D, "A") ; break ;
		case 0x3: sprintf(S1, "X1") ; sprintf(S2, "X0") ; sprintf(D, "B") ; break ;
		case 0x4: sprintf(S1, "A1") ; sprintf(S2, "Y0") ; sprintf(D, "A") ; break ;
		case 0x5: sprintf(S1, "A1") ; sprintf(S2, "Y0") ; sprintf(D, "B") ; break ;
		case 0x6: sprintf(S1, "B1") ; sprintf(S2, "X0") ; sprintf(D, "A") ; break ;
		case 0x7: sprintf(S1, "B1") ; sprintf(S2, "X0") ; sprintf(D, "B") ; break ;
		case 0x8: sprintf(S1, "Y0") ; sprintf(S2, "X0") ; sprintf(D, "A") ; break ;
		case 0x9: sprintf(S1, "Y0") ; sprintf(S2, "X0") ; sprintf(D, "B") ; break ;
		case 0xa: sprintf(S1, "Y1") ; sprintf(S2, "X0") ; sprintf(D, "A") ; break ;
		case 0xb: sprintf(S1, "Y1") ; sprintf(S2, "X0") ; sprintf(D, "B") ; break ;
		case 0xc: sprintf(S1, "Y0") ; sprintf(S2, "X1") ; sprintf(D, "A") ; break ;
		case 0xd: sprintf(S1, "Y0") ; sprintf(S2, "X1") ; sprintf(D, "B") ; break ;
		case 0xe: sprintf(S1, "Y1") ; sprintf(S2, "X1") ; sprintf(D, "A") ; break ;
		case 0xf: sprintf(S1, "Y1") ; sprintf(S2, "X1") ; sprintf(D, "B") ; break ;
	}
}
#endif

static int DecodeRRTable(UINT16 RR)
{
	return RR ;
}

static void DecodesTable(UINT16 s, char *arithmetic)
{
	switch(s)
	{
		case 0x0: sprintf(arithmetic, "su") ; break ;
		case 0x1: sprintf(arithmetic, "uu") ; break ;
	}
}

#ifdef UNUSED_FUNCTION
static void DecodessTable(UINT16 ss, char *arithmetic)
{
	switch(ss)
	{
		case 0x0: sprintf(arithmetic, "ss") ; break ;
		case 0x1: sprintf(arithmetic, "ss") ; break ;
		case 0x2: sprintf(arithmetic, "su") ; break ;
		case 0x3: sprintf(arithmetic, "uu") ; break ;
	}
}

static void DecodeuuuuFTable(UINT16 uuuu, UINT16 F, char *arg, char *S, char *D)
{
	UINT16 switchVal = (uuuu << 1) | F ;

	switch(switchVal)
	{
		// The add and sub cases are piled on top of one another
		case 0x00: sprintf(arg, "add") ; case 0x08: sprintf(arg, "sub") ; sprintf(S, "X0") ; sprintf(D, "A") ; break ;
		case 0x01: sprintf(arg, "add") ; case 0x09: sprintf(arg, "sub") ; sprintf(S, "X0") ; sprintf(D, "B") ; break ;
		case 0x02: sprintf(arg, "add") ; case 0x0a: sprintf(arg, "sub") ; sprintf(S, "Y0") ; sprintf(D, "A") ; break ;
		case 0x03: sprintf(arg, "add") ; case 0x0b: sprintf(arg, "sub") ; sprintf(S, "Y0") ; sprintf(D, "B") ; break ;
		case 0x04: sprintf(arg, "add") ; case 0x0c: sprintf(arg, "sub") ; sprintf(S, "X1") ; sprintf(D, "A") ; break ;
		case 0x05: sprintf(arg, "add") ; case 0x0d: sprintf(arg, "sub") ; sprintf(S, "X1") ; sprintf(D, "B") ; break ;
		case 0x06: sprintf(arg, "add") ; case 0x0e: sprintf(arg, "sub") ; sprintf(S, "Y1") ; sprintf(D, "A") ; break ;
		case 0x07: sprintf(arg, "add") ; case 0x0f: sprintf(arg, "sub") ; sprintf(S, "Y1") ; sprintf(D, "B") ; break ;
		case 0x18: sprintf(arg, "add") ; case 0x1a: sprintf(arg, "sub") ; sprintf(S, "B")  ; sprintf(D, "A") ; break ;
		case 0x19: sprintf(arg, "add") ; case 0x1b: sprintf(arg, "sub") ; sprintf(S, "A")  ; sprintf(D, "B") ; break ;
	}
}
#endif

static void DecodeZTable(UINT16 Z, char *ea)
{
	// This is fixed as per the Family Manual addendum
	switch(Z)
	{
		case 0x1: sprintf(ea, "(A1)") ; break ;
		case 0x0: sprintf(ea, "(B1)") ; break ;
	}
}


static void AssembleeaFrommTable(UINT16 m, int n, char *ea)
{
	switch(m)
	{
		case 0x0: sprintf(ea, "(R%d)+",n)        ; break ;
		case 0x1: sprintf(ea, "(R%d)+N%d", n, n) ; break ;
	}
}

#ifdef UNUSED_FUNCTION
static void AssembleeaFrommmTable(UINT16 mm, int n1, int n2, char *ea1, char *ea2)
{
	switch(mm)
	{
		case 0x0: sprintf(ea1, "(R%d)+",    n1)     ;
			      sprintf(ea2, "(R%d)+",    n2)     ; break ;
		case 0x1: sprintf(ea1, "(R%d)+",    n1)     ;
			      sprintf(ea2, "(R%d)+N%d", n2, n2) ; break ;
		case 0x2: sprintf(ea1, "(R%d)+N%d", n1, n1) ;
			      sprintf(ea2, "(R%d)+",    n2)     ; break ;
		case 0x3: sprintf(ea1, "(R%d)+N%d", n1, n1) ;
			      sprintf(ea2, "(R%d)+N%d", n2, n2) ; break ;
	}
}
#endif

static void AssembleeaFromMMTable(UINT16 MM, int n, char *ea)
{
	switch(MM)
	{
		case 0x0: sprintf(ea, "(R%d)",     n)    ; break ;
		case 0x1: sprintf(ea, "(R%d)+",    n)    ; break ;
		case 0x2: sprintf(ea, "(R%d)-",    n)    ; break ;
		case 0x3: sprintf(ea, "(R%d)+N%d", n, n) ; break ;
	}
}

static void AssembleeaFromqTable(UINT16 q, int n, char *ea)
{
	switch(q)
	{
		case 0x0: sprintf(ea, "(R%d+N%d)", n, n) ; break ;
		case 0x1: sprintf(ea, "-(R%d)",    n)    ; break ;
	}
}

static void AssembleeaFromtTable(UINT16 t,  UINT16 val, char *ea)
{
	switch(t)
	{
		// !!! Looking at page 336 of UM, I'm not sure if 0 is X: or if 0 is just # !!!
		case 0x0: sprintf(ea, "X:%04x", val) ; break ;
		case 0x1: sprintf(ea, "#%04x", val) ;  break ;
	}
}

// Not sure if this one is even used?
static void AssembleeaFromzTable(UINT16 z, int n, char *ea)
{
	switch(z)
	{
		case 0x0: sprintf(ea, "(R%d)-",    n)    ; break ;
		case 0x1: sprintf(ea, "(R%d)+N%d", n, n) ; break ;
	}
}

static void AssembleDFromPTable(UINT16 P, UINT16 ppppp, char *D)
{
	char fullAddy[128] ;		// Convert Short Absolute Address to full 16-bit

	switch(P)
	{
		case 0x0: sprintf(D, "X:%02x", ppppp) ; break ;
		case 0x1:
			AssembleAddressFromIOShortAddress(ppppp, fullAddy) ;
			sprintf(D, "X:%02x (%s)", ppppp, fullAddy) ;
			break ;
	}
}

static void AssembleArgumentsFromWTable(UINT16 W, char *args, char ma, char *SD, char *ea)
{
	switch(W)
	{
		case 0x0: sprintf(args, "%s,%c:%s", SD, ma, ea) ; break ;
		case 0x1: sprintf(args, "%c:%s,%s", ma, ea, SD) ; break ;
	}
}

static void AssembleRegFromWTable(UINT16 W, char *args, char ma, char *SD, UINT8 xx)
{
	switch(W)
	{
		case 0x0: sprintf(args, "%s,%c:(R2+%02x)", SD, ma, xx) ; break ;
		case 0x1: sprintf(args, "%c:(R2+%02x),%s", ma, xx, SD) ; break ;
	}
}

static void AssembleAddressFromIOShortAddress(UINT16 pp, char *ea)
{
	UINT16 fullAddy = 0xffe0 ;
	fullAddy |= pp ;

	sprintf(ea, "%.04x", fullAddy) ;
}

static void AssembleAddressFrom6BitSignedRelativeShortAddress(UINT16 srs, char *ea)
{
	UINT16 fullAddy = srs ;
	if (fullAddy & 0x0020) fullAddy |= 0xffc0 ;

	sprintf(ea, "%d", (INT16)fullAddy) ;
}



//////////////////////
// HELPER FUNCTIONS //
//////////////////////

static UINT16 Dsp56kOpMask(UINT16 cur, UINT16 mask)
{
	int i ;

	UINT16 retVal = (cur & mask) ;
	UINT16 temp = 0x0000 ;
	int offsetCount = 0 ;

	// Shift everything right, eliminating 'whitespace'...
	for (i = 0; i < 16; i++)
	{
		if (mask & (0x1<<i))		// If mask bit is non-zero
		{
			temp |= (((retVal >> i) & 0x1) << offsetCount) ;
			offsetCount++ ;
		}
	}

	return temp ;
}



/*

Data ALU Ops
--------------------

CLR             X       1xxx xxxx 0000 x001
ADD             X       1xxx xxxx 0000 xxxx

MOVE            X       1xxx xxxx 0001 0001
TFR             X       1xxx xxxx 0001 xxxx

RND             X       1xxx xxxx 0010 x000
TST             X       1xxx xxxx 0010 x001
INC             X       1xxx xxxx 0010 x010
INC24           X       1xxx xxxx 0010 x011
OR              X       1xxx xxxx 0010 x1xx

ASR             X       1xxx xxxx 0011 x000
ASL             X       1xxx xxxx 0011 x001
LSR             X       1xxx xxxx 0011 x010
LSL             X       1xxx xxxx 0011 x011
EOR             X       1xxx xxxx 0011 x1xx

SUBL            X       1xxx xxxx 0100 x001
SUB             X       1xxx xxxx 0100 xxxx

CLR24           X       1xxx xxxx 0101 x001
SBC             X       1xxx xxxx 0101 x01x
CMP             X       1xxx xxxx 0101 xxxx

NEG             X       1xxx xxxx 0110 x000
NOT             X       1xxx xxxx 0110 x001
DEC             X       1xxx xxxx 0110 x010
DEC24           X       1xxx xxxx 0110 x011
AND             X       1xxx xxxx 0110 x1xx

ABS             X       1xxx xxxx 0111 x001
ROR             X       1xxx xxxx 0111 x010
ROL             X       1xxx xxxx 0111 x011
CMPM            X       1xxx xxxx 0111 xxxx

MPY             X       1xxx xxxx 1x00 xxxx
MPYR            X       1xxx xxxx 1x01 xxxx
MAC             X       1xxx xxxx 1x10 xxxx
MACR            X       1xxx xxxx 1x11 xxxx


VERY STRANGE XMDM!      0101 HHHW xxxx xxxx


DXMDR
-----------------

ADD             X       011x xxxx 0xxx xxxx
SUB             X       011x xxxx 0xxx xxxx
TFR             X       011x xxxx 0xx1 x0xx
MOVE            X       011x xxxx 0xx1 0000

MPY             X       011x xxxx 1xx0 x0xx
MAC             X       011x xxxx 1xx0 x1xx
MPYR            X       011x xxxx 1XX1 x0xx
MACR            X       011x xxxx 1XX1 x1xx


BITFIELD
-----------------

BFTSTL          X       0001 0100 000x xxxx . xxx0 0000 xxxx xxxx
BFTSTH          X       0001 0100 000x xxxx . xxx1 0000 xxxx xxxx
BFTSTL          X       0001 0100 001X XXxx . xxx0 0000 xxxx xxxx
BFTSTH          X       0001 0100 001X XXxx . xxx1 0000 xxxx xxxx
BFTSTL          X       0001 0100 01xx xxxx . xxx0 0000 xxxx xxxx
BFTSTH          X       0001 0100 01xx xxxx . xxx1 0000 xxxx xxxx
BFCLR           X       0001 0100 100x xxxx . xxx0 0100 xxxx xxxx
BFCHG           X       0001 0100 100x xxxx . xxx1 0010 xxxx xxxx
BFSET           X       0001 0100 100x xxxx . xxx1 1000 xxxx xxxx
BFCLR           X       0001 0100 101X XXxx . xxx0 0100 xxxx xxxx
BFCHG           X       0001 0100 101X XXxx . xxx1 0010 xxxx xxxx
BFSET           X       0001 0100 101X XXxx . xxx1 1000 xxxx xxxx
BFCHG           X       0001 0100 11xx xxxx . xxx1 0010 xxxx xxxx
BFCLR           X       0001 0100 11xx xxxx . xxx0 0100 xxxx xxxx
BFSET           X       0001 0100 11xx xxxx . xxx1 1000 xxxx xxxx


NPM
-----------------

TFR2            X       0001 0101 0000 x00x
ADC             X       0001 0101 0000 x01x

TST2            X       0001 0101 0001 X1xx

NORM            X       0001 0101 0010 x0xx

ASR4            X       0001 0101 0011 x000
ASL4            X       0001 0101 0011 x001

DIV             X       0001 0101 0XX0 x1xx

ZERO            X       0001 0101 0101 x000
EXT             X       0001 0101 0101 x010

NEGC            X       0001 0101 0110 x000

ASR16           X       0001 0101 0111 x000
SWAP            X       0001 0101 0111 x001

IMPY            X       0001 0101 1000 xxxx
IMAC            X       0001 0101 1010 xxxx
DMAC(ss,su,uu)  X       0001 0101 10x1 xxxx
MPY(su,uu)      X       0001 0101 1100 xxxx
MAC(su,uu)      X       0001 0101 1110 xxxx


XMDWRDM
-----------------

MPY             X       0001 0110 xxxx xxxx
MAC             X       0001 0111 xxxx xxxx


IMMEDIATE
-----------------

ANDI            X       0001 1xx0 xxxx xxxx
ORI             X       0001 1xx1 xxxx xxxx


SPECIAL
-----------------

MOVE(S)         X       0001 100x xx0x xxxx
MOVE(P)         X       0001 100x xx1x xxxx

-----------------

Tcc             X       0001 00xx xxxx xx0x

-----------------

MOVE(I)         X       0010 00xx xxxx xxxx
TFR3            X       0010 01xx xxxx xxxx
MOVE(C)         X       0010 10xx xxxx xxxx
Bcc             X       0010 11xx xxxx xxxx

-----------------

MOVE(C)         X       0011 1xxx xxx0 xxxx
MOVE(C)         X       0011 1xxx xxx1 x0xx
MOVE(C)         X       0011 1xxx xxx1 x10X xxxx xxxx xxxx xxxx
MOVE(C)         X       0011 1xxx xxx1 x11X

-----------------

NOP             X       0000 0000 0000 0000
DEBUG           X       0000 0000 0000 0001
DO FOREVER      X       0000 0000 0000 0010 xxxx xxxx xxxx xxxx
CHKAAU          X       0000 0000 0000 0100
SWI             X       0000 0000 0000 0101
RTS             X       0000 0000 0000 0110
RTI             X       0000 0000 0000 0111
RESET           X       0000 0000 0000 1000
ENDDO           X       0000 0000 0000 1001
STOP            X       0000 0000 0000 1010
WAIT            X       0000 0000 0000 1011
ILLEGAL         X       0000 0000 0000 1111

DEBUGcc         X       0000 0000 0101 xxxx
DOLoop          X       0000 0000 110X XXxx xxxx xxxx xxxx xxxx
REP             X       0000 0000 111X XXxx

BRKcc           X       0000 0001 0001 xxxx

JSR             X       0000 0001 0010 00xx
JMP             X       0000 0001 0010 01xx
BSR             X       0000 0001 0010 10xx
BRA             X       0000 0001 0010 11xx

JSR             X       0000 0001 0011 00XX xxxx xxxx xxxx xxxx
JMP             X       0000 0001 0011 01XX xxxx xxxx xxxx xxxx
BSR             X       0000 0001 0011 10XX xxxx xxxx xxxx xxxx
BRA             X       0000 0001 0011 11XX xxxx xxxx xxxx xxxx

REPcc           X       0000 0001 0101 xxxx

LEA             X       0000 0001 10xx xxxx
LEA             X       0000 0001 11xx xxxx

-----------------

MOVE(M)         X       0000 001x xx0x xxxx
MOVE(M)         X       0000 001x xx11 xxxx

-----------------

DOLoop          X       0000 0100 000x xxxx xxxx xxxx xxxx xxxx
REP             X       0000 0100 001x xxxx

-----------------

MOVE(M)         X       0000 0101 xxxx xxxx 0000 001x XX0X Xxxx
MOVE(C)         X       0000 0101 xxxx xxxx 0011 1xxx xxx0 XXXX
MOVE            X       0000 0101 xxxx xxxx XXXX xxxx 0001 0001

-----------------

JScc            X       0000 0110 xx00 xxxx
JScc            X       0000 0110 XX01 xxxx xxxx xxxx xxxx xxxx
Jcc             X       0000 0110 xx10 xxxx
Jcc             X       0000 0110 XX11 xxxx xxxx xxxx xxxx xxxx

BScc            X       0000 0111 xx00 xxxx
BScc            X       0000 0111 XX01 xxxx xxxx xxxx xxxx xxxx
Bcc             X       0000 0111 xx10 xxxx
Bcc             X       0000 0111 XX11 xxxx xxxx xxxx xxxx xxxx

-----------------

JSR             X       0000 1010 xxxx xxxx
BRA             X       0000 1011 xxxx xxxx
MOVE(P)         X       0000 110x xxxx xxxx
DOLoop          X       0000 1110 xxxx xxxx xxxx xxxx xxxx xxxx
REP             X       0000 1111 xxxx xxxx

*/
