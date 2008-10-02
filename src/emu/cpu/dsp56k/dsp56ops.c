/***************************************************************************

    dsp56ops.c
    Core implementation for the portable Motorola/Freescale DSP56k emulator.
    Written by Andrew Gardner

***************************************************************************/

// NOTES For register setting:
// FM.3-4 : When A2 or B2 is read, the register contents occupy the low-order portion
//          (bits 7-0) of the word; the high-order portion (bits 16-8) is sign-extended. When A2 or B2
//          is written, the register receives the low-order portion of the word; the high-order portion is not used
//        : ...much more!
//        : ...shifter/limiter/overflow notes too.
//
//

// Helper functions and macros
#define BITS(CUR,MASK) (Dsp56kOpMask(CUR,MASK))
static UINT16 Dsp56kOpMask(UINT16 op, UINT16 mask) ;

enum dataType { DT_BYTE, DT_WORD, DT_DOUBLE_WORD, DT_LONG_WORD } ;

// This function's areguments are written source->destination to fall in line with the processor's paradigm...
static void SetDestinationValue(void *sourcePointer, unsigned char sourceType,
								void *destinationPointer, unsigned char destinationType) ;

static void SetDataMemoryValue   (void *sourcePointer, unsigned char sourceType, UINT32 destinationAddr) ;
static void SetProgramMemoryValue(void *sourcePointer, unsigned char sourceType, UINT32 destinationAddr) ;


// Main opcode categories
static unsigned ExecuteDataALUOpcode(int parallelType) ;
static unsigned ExecuteDXMDROpcode(void) ;
static unsigned ExecuteNPMOpcode(void) ;
static unsigned ExecuteMisfitOpcode(void) ;
static unsigned ExecuteSpecialOpcode(void) ;


// Actual opcode implementations

// Data ALU Ops
static int ClrOperation(void **wd, UINT64 *pa) ;
static int AddOperation(void **wd, UINT64 *pa) ;
static int NotOperation(void **wd, UINT64 *pa) ;
static int LsrOperation(void **wd, UINT64 *pa) ;
static int AsrOperation(void **wd, UINT64 *pa) ;
static int TfrDataALUOperation(void **wd, UINT64 *pa) ;
static int EorOperation(void **wd, UINT64 *pa) ;
static int CmpOperation(void **wd, UINT64 *pa) ;
static int Dec24Operation(void **wd, UINT64 *pa) ;
static int AndOperation(void **wd, UINT64 *pa) ;
static int OrOperation(void **wd, UINT64 *pa) ;
static int TstOperation(void **wd, UINT64 *pa) ;

static int MoveCOperation(void) ;
static int MoveMOperation(void) ;
static int MoveIOperation(void) ;
static int MovePOperation(void) ;

static int AndiOperation(void) ;
//static int OriOperation(void) ;

static int BitfieldOperation(void) ;
static int JmpOperation(void) ;
static int BsrOperation(void) ;
static int JsrOperation(void) ;
static int DoOperation(void) ;
static int TccOperation(void) ;
static int BraOperation(void) ;
static int BsccOperation(void) ;
static int BccOperation(void) ;
static int Tst2Operation(void) ;
static int MACsuuuOperation(void) ;
static int RepOperation(void) ;

static void EndOfLoopProcessing(void) ;
static void EndOfRepProcessing(void) ;

// Parallel memory data moves
static void XMDMOperation(UINT16 parameters, void *working, UINT64 *pa) ;
static void XMDMSpecialOperation(UINT16 parameters, void *working) ;
static void ARUOperation(UINT16 parameters) ;
static void RRDMOperation(UINT16 parameters, void *working,  UINT64 *pa) ;

// For decoding the different types of tables...
static void *DecodeDDDDDTable(UINT16 DDDDD, unsigned char *dt) ;
static void *DecodeRRTable(UINT16 RR, unsigned char *dt) ;
static void *DecodeDDTable(UINT16 DD, unsigned char *dt) ;
static void *DecodeHHHTable(UINT16 HHH, unsigned char *dt) ;
static void *DecodeHHTable(UINT16 HH, unsigned char *dt) ;
static void *DecodeZTable(UINT16 Z, unsigned char *dt) ;
static UINT16 DecodeBBBBitMask(UINT16 BBB, UINT16 *iVal) ;
static int DecodeccccTable(UINT16 cccc) ;
static void Decodeh0hFTable(UINT16 h0h, UINT16 F, void **source, unsigned char *st, void **dest, unsigned char *dt) ;
static void *DecodeFTable(UINT16 F, unsigned char *dt) ;
static void DecodeJJJFTable(UINT16 JJJ, UINT16 F, void **source, unsigned char *st, void **dest, unsigned char *dt) ;
static void DecodeJJFTable(UINT16 JJ, UINT16 F, void **source, unsigned char *st, void **dest, unsigned char *dt) ;
static void DecodeQQFTableSp(UINT16 QQ, UINT16 F, void **S1, void **S2, void **D) ;
static void DecodeIIIITable(UINT16 IIII, void **source, unsigned char *st, void **dest, unsigned char *dt, void *working) ;

static void ExecuteMMTable(int x, UINT16 MM) ;
static void ExecutemTable(int x, UINT16 m) ;
static void ExecutezTable(int x, UINT16 z) ;
static UINT16 ExecuteqTable(int x, UINT16 q) ;

static UINT16 AssembleDFromPTable(UINT16 P, UINT16 ppppp) ;

static UINT16 AssembleAddressFromIOShortAddress(UINT16 pp) ;
static UINT16 AssembleAddressFrom6BitSignedRelativeShortAddress(UINT16 srs) ;


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

#ifdef UNUSED_FUNCTION
static void unimplemented(void)
{
	fatalerror("Unimplemented OP @ %04X: %04X", PC-2, OP) ;
}
#endif


static void execute_one(void)
{
	unsigned size = 666 ;

	debugger_instruction_hook(Machine, PC);
	OP = ROPCODE(PC<<1);

	if (BITS(OP,0x8000))											// First, the parallel data move instructions
	{
		size = ExecuteDataALUOpcode(PARALLEL_TYPE_XMDM) ;
	}
	else if (BITS(OP,0xf000) == 0x5)
	{
		size = ExecuteDataALUOpcode(PARALLEL_TYPE_XMDM_SPECIAL) ;
	}
	else if (BITS(OP,0xff00) == 0x4a)
	{
		size = ExecuteDataALUOpcode(PARALLEL_TYPE_NODM) ;
	}
	else if (BITS(OP,0xf000) == 0x04)
	{
		size = ExecuteDataALUOpcode(PARALLEL_TYPE_RRDM) ;
	}
	else if (BITS(OP,0xf800) == 0x06)
	{
		size = ExecuteDataALUOpcode(PARALLEL_TYPE_ARU) ;
	}


	else if (BITS(OP,0x4000))
	{
		size = ExecuteDXMDROpcode() ;
	}
	else if (BITS(OP,0xf000) == 0x1)
	{
		size = ExecuteNPMOpcode() ;
	}
	else if (BITS(OP,0x2000))
	{
		size = ExecuteMisfitOpcode() ;
	}
	else if (BITS(OP,0xf000) == 0x0)
	{
		size = ExecuteSpecialOpcode();
	}

	if (size == 666)
	{
		mame_printf_debug("unimplemented at %04x\n", PC) ;
		size = 1 ;						// Just to get the debugger past the bad opcode
	}


	PC += size ;
	change_pc(PC) ;

	// Loop processing
	if (LF_bit())
	{
		if (PC == LA)
		{
			// You're on the last instruction of the loop...
			if (LC != 1)								// Strange, the == 1 thing...
			{
				LC-- ;
				PC = SSH ;
				change_pc(PC) ;
			}
			else
			{
				EndOfLoopProcessing() ;
			}
		}
	}

	// Rep processing
	if (core.repFlag)
	{
		if (PC == core.repAddr)
		{
			if (LC != 1)
			{
				LC-- ;
				PC -= size ;								// Eh, seems reasonable :)
				change_pc(PC) ;
			}
			else
			{
				EndOfRepProcessing() ;
			}
		}
	}

	dsp56k_icount -= 4;					/* Temporarily hard-coded at 4 clocks per opcode */
}


static unsigned ExecuteDataALUOpcode(int parallelType)
{
	unsigned retSize = 666 ;

	void *workingDest = 0x00 ;

	// Whenever an instruction uses an accumulator as both a destination operand for a Data ALU operation and
	//   as a source for a parallel move operation, the parallel move operation will use the value in the accumulator
	//   prior to execution of any Data ALU operation.
	UINT64 previousAccum = 0 ;

	switch(BITS(OP,0x0070))
	{

		case 0x0:
			if (BITS(OP,0x0007) == 0x1)
			{
				// CLR - 1mRR HHHW 0000 F001
                retSize = ClrOperation(&workingDest, &previousAccum) ;
			}
			else
			{
				// ADD - 1mRR HHHW 0000 FJJJ
				retSize = AddOperation(&workingDest, &previousAccum) ;
			}
			break ;

		case 0x1:
			if (BITS(OP,0x000f) == 0x1)
			{
				// MOVE - 1mRR HHHW 0001 0001
				retSize = 1 ;
			}
			else
			{
				// TFR - 1mRR HHHW 0001 FJJJ
				retSize = TfrDataALUOperation(&workingDest, &previousAccum) ;
			}
			break ;

		case 0x2:
			if (BITS(OP,0x0007) == 0x0)
			{
				// RND - 1mRR HHHW 0010 F000
			}
			else if (BITS(OP,0x0007) == 0x1)
			{
				// TST - 1mRR HHHW 0010 F001
				retSize = TstOperation(&workingDest, &previousAccum);
			}
			else if (BITS(OP,0x0007) == 0x2)
			{
				// INC - 1mRR HHHW 0010 F010
			}
			else if (BITS(OP,0x0007) == 0x3)
			{
				// INC24 - 1mRR HHHW 0010 F011
			}
			else
			{
				// OR - 1mRR HHHW 0010 F1JJ
				retSize = OrOperation(&workingDest, &previousAccum) ;
			}
			break ;

		case 0x3:
			if (BITS(OP,0x0007) == 0x0)
			{
				// ASR - 1mRR HHHW 0011 F000
				retSize = AsrOperation(&workingDest, &previousAccum) ;
			}
			else if (BITS(OP,0x0007) == 0x1)
			{
				// ASL - 1mRR HHHW 0011 F001
			}
			else if (BITS(OP,0x0007) == 0x2)
			{
				// LSR - 1mRR HHHW 0011 F010
				retSize = LsrOperation(&workingDest, &previousAccum) ;
			}
			else if (BITS(OP,0x0007) == 0x3)
			{
				// LSL - 1mRR HHHW 0011 F011
			}
			else
			{
				// EOR - 1mRR HHHW 0011 F1JJ
				retSize = EorOperation(&workingDest, &previousAccum) ;
			}
			break ;
/*
        case 0x4:
            if (BITS(op,0x0007) == 0x1)
            {
                // SUBL - 1mRR HHHW 0100 F001
            }
            else
            {
                // SUB - 1mRR HHHW 0100 FJJJ
            }
            break ;
*/
		case 0x5:
			if (BITS(OP,0x0007) == 0x1)
			{
				// CLR24 - 1mRR HHHW 0101 F001
			}
			else if (BITS(OP,0x0006) == 0x1)
			{
				// SBC - 1mRR HHHW 0101 F01J
			}
			else
			{
				// CMP - 1mRR HHHW 0101 FJJJ
				retSize = CmpOperation(&workingDest, &previousAccum) ;
			}
			break ;

		case 0x6:
			if (BITS(OP,0x0007) == 0x0)
			{
				// NEG - 1mRR HHHW 0110 F000
			}
			else if (BITS(OP,0x0007) == 0x1)
			{
				// NOT - 1mRR HHHW 0110 F001
				retSize = NotOperation(&workingDest, &previousAccum) ;
			}
			else if (BITS(OP,0x0007) == 0x2)
			{
				// DEC - 1mRR HHHW 0110 F010
			}
			else if (BITS(OP,0x0007) == 0x3)
			{
				// DEC24 - 1mRR HHHW 0110 F011
				retSize = Dec24Operation(&workingDest, &previousAccum) ;
			}
			else
			{
				// AND - 1mRR HHHW 0110 F1JJ
				retSize = AndOperation(&workingDest, &previousAccum) ;
			}
			break ;
/*
        case 0x7:
            if (BITS(op,0x0007) == 0x1)
            {
                // ABS - 1mRR HHHW 0111 F001
            }
            if (BITS(op,0x0007) == 0x2)
            {
                // ROR - 1mRR HHHW 0111 F010
            }
            if (BITS(op,0x0007) == 0x3)
            {
                // ROL - 1mRR HHHW 0111 F011
            }
            else
            {
                // CMPM - 1mRR HHHW 0111 FJJJ
            }

            break ;
*/
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

	// Do the parallel move parallel'y :)
    // Someday we may have to check against the special cases to throw exceptions !!
	switch (parallelType)
	{
		case PARALLEL_TYPE_XMDM:
			XMDMOperation(BITS(OP,0xff00), workingDest, &previousAccum) ;
			break ;

		case PARALLEL_TYPE_RRDM:
			RRDMOperation(BITS(OP,0xff00), workingDest, &previousAccum) ;
			break ;

		case PARALLEL_TYPE_XMDM_SPECIAL:
			// pa is not needed because the XMDM dest is always the .opposite. of the regular dest.
			XMDMSpecialOperation(BITS(OP,0xff00), workingDest) ;
			break ;

		case PARALLEL_TYPE_NODM:
			// do nothing
			break ;

		case PARALLEL_TYPE_ARU:
			ARUOperation(BITS(OP,0xff00)) ;
			break ;
	}

	return retSize ;
}

static unsigned ExecuteDXMDROpcode(void)
{
	unsigned retSize = 666 ;

	return retSize ;
}

static unsigned ExecuteNPMOpcode(void)
{
	unsigned retSize = 666 ;

	if (BITS(OP,0x0f00) == 0x4)
	{
		retSize = BitfieldOperation() ;
	}
	else if (BITS(OP,0x0f00) == 0x5)
	{
		switch(BITS(OP,0x0074))
		{
			case 0x0:
				if (BITS(OP,0x0006) == 0x0)
				{
					// TFR(2) - 0001 0101 0000 F00J
					retSize = 1 ;
				}
				else if (BITS(OP,0x0006) == 0x1)
				{
					// ADC - 0001 0101 0000 F01J
					retSize = 1 ;
				}
				break ;

			case 0x3:
					// TST(2) - 0001 0101 0001 -1DD
					retSize = Tst2Operation() ; break ;

			case 0x4:
					// NORM - 0001 0101 0010 F0RR
					retSize = 1 ; break ;

			case 0x6:
				if (BITS(OP,0x0003) == 0x0)
				{
					// ASR4 - 0001 0101 0011 F000
					retSize = 1 ;
				}
				else if (BITS(OP,0x0003) == 0x1)
				{
					// ASL4 - 0001 0101 0011 F001
					retSize = 1 ;
				}
				break ;

			case 0x1: case 0x5: case 0x9: case 0xd:
					// DIV - 0001 0101 0--0 F1DD
					retSize = 1 ; break ;

			case 0xa:
				if (BITS(OP,0x0003) == 0x0)
				{
					// ZERO - 0001 0101 0101 F000
					retSize = 1 ;
				}
				else if (BITS(OP,0x0003) == 0x2)
				{
					// EXT - 0001 0101 0101 F010
					retSize = 1 ;
				}
				break ;

			case 0xc:
				if (BITS(OP,0x0003) == 0x0)
				{
					// NEGC - 0001 0101 0110 F000
					retSize = 1 ;
				}
				break ;

			case 0xe:
				if (BITS(OP,0x0003) == 0x0)
				{
					// ASR16 - 0001 0101 0111 F000
					retSize = 1 ;
				}
				else if (BITS(OP,0x0003) == 0x1)
				{
					// SWAP - 0001 0101 0111 F001
					retSize = 1 ;
				}
				break ;
		}

		switch(BITS(OP,0x00f0))
		{
			case 0x8:
				// IMPY - 0001 0101 1000 FQQQ
				retSize = 1 ; break ;
			case 0xa:
				// IMAC - 0001 0101 1010 FQQQ
				retSize = 1 ; break ;
			case 0x9: case 0xb:
				// DMAC(ss,su,uu) - 0001 0101 10s1 FsQQ
				retSize = 1 ; break ;
			case 0xc:
				// MPY(su,uu) - 0001 0101 1100 FsQQ
				retSize = 1 ; break ;
			case 0xe:
				// MAC(su,uu) - 0001 0101 1110 FsQQ
				retSize = MACsuuuOperation() ; break ;
		}
	}
	else if ((BITS(OP,0x0f00) == 0x6))
	{
		retSize = 1 ;
	}
	else if ((BITS(OP,0x0f00) == 0x7))
	{
		retSize = 1 ;
	}
	else if ((BITS(OP,0x0800) == 0x1))
	{
		if (BITS(OP,0x0600))
		{
			if (!BITS(OP,0x0100))
			{
				// ANDI - 0001 1EE0 iiii iiii
				retSize = AndiOperation() ;
			}
			else
			{
				// ORI - 0001 1EE1 iiii iiii
			}
		}
		else
		{
			if (!BITS(OP,0x0020))
			{
				// MOVE(S) - 0001 100W HH0a aaaa
			}
			else
			{
				// MOVE(P) - 0001 100W HH1p pppp
				retSize = MovePOperation() ;
			}
		}
	}
	else if ((BITS(OP,0x0c00) == 0x0))
	{
		retSize = TccOperation() ;
	}

	return retSize ;
}

static unsigned ExecuteMisfitOpcode(void)
{
	unsigned retSize = 666 ;

	if (BITS(OP,0x1000)== 0x0)
	{
		switch(BITS(OP,0x0c00))
		{
			// MOVE(I) - 0010 00DD BBBB BBBB
			case 0x0:
				retSize = MoveIOperation() ;
				break ;

/*
            case 0x1:
                // TFR(3) - 0010 01mW RRDD FHHH
                break ;
*/
			case 0x2:
				// MOVE(C) - 0010 10dd dddD DDDD
				retSize = MoveCOperation() ;
				break ;

			case 0x3:
				// B.cc - 0010 11cc ccee eeee
				retSize = BccOperation() ;
				break ;
		}
	}
	else
	{
		// MOVE(C) - 0011 1WDD DDD0 MMRR
		// MOVE(C) - 0011 1WDD DDD1 q0RR
		// MOVE(C) - 0011 1WDD DDD1 t10- xxxx xxxx xxxx xxxx
		// MOVE(C) - 0011 1WDD DDD1 Z11-
		retSize = MoveCOperation() ;
	}

	return retSize ;
}

static unsigned ExecuteSpecialOpcode(void)
{
	unsigned retSize = 666 ;

	if (BITS(OP,0x0ff0) == 0x0)
	{
		switch (BITS(OP,0x000f))
		{
			// NOP - 0000 0000 0000 0000
			case 0x0:
				retSize = 1 ;
				break ;

			// DO FOREVER - 0000 0000 0000 0010 xxxx xxxx xxxx xxxx
			case 0x2:
				// !!! IMPLEMENT ME NEXT;
				retSize = 0;
				break;

			// RTS - 0000 0000 0000 0110
			case 0x6:
				PC = SSH ;
				SR = SSL ;
				SP-- ;
				retSize = 0 ;
				change_pc(PC) ;
				break ;

			// RTI - 0000 0000 0000 0111
			case 0x7:
				PC = SSH;
				SR = SSL;
				SP--;
				retSize = 0;
				change_pc(PC);
				break;
		}
	}
	else if (BITS(OP,0x0f00) == 0x0)
	{
		retSize = 1 ;
	}
	else if (BITS(OP,0x0f00) == 0x1)
	{
		if (BITS(OP,0x00f0) == 0x1)
		{
			retSize = 1 ;
		}
		else if (BITS(OP,0x00f0) == 0x2)
		{
			switch(BITS(OP,0x000c))								// Consolidation can happen here
			{
				// JSR - 0000 0001 0010 00RR
				// JMP - 0000 0001 0010 01RR
				case 0x1: retSize = JmpOperation(); break;
				// BSR - 0000 0001 0010 10RR
				// BRA - 0000 0001 0010 11RR
				default:  retSize = 1;
			}
		}
		else if (BITS(OP,0x00f0) == 0x3)
		{
			switch(BITS(OP,0x000c))
			{
				// JSR - 0000 0001 0011 00-- xxxx xxxx xxxx xxxx
				case 0x0: retSize = JsrOperation() ; break ;
				// JMP - 0000 0001 0011 01-- xxxx xxxx xxxx xxxx
				case 0x1: retSize = JmpOperation() ; break ;
				// BSR - 0000 0001 0011 10-- xxxx xxxx xxxx xxxx
				case 0x2: retSize = BsrOperation() ; break ;
			}
		}
		else if (BITS(OP,0x00f0) == 0x5)
		{
			retSize = 1 ;
		}
		else if (BITS(OP,0x0080) == 0x1)
		{
			retSize = 1 ;
		}
	}
	else if (BITS(OP,0x0e00) == 0x1)
	{
		if (BITS(OP,0x0020) == 0x0)
		{
			// MOVE(M) - 0000 001W RR0M MHHH
			retSize = MoveMOperation() ;
		}
	}
	else if (BITS(OP,0x0f00) == 0x4)
	{
		if (BITS(OP,0x0020) == 0x0)
		{
			// DO - 0000 0100 000D DDDD xxxx xxxx xxxx xxxx
			retSize = DoOperation() ;
		}
		else
		{
			// REP - 0000 0100 001D DDDD
			retSize = RepOperation() ;
		}
	}
	else if (BITS(OP,0x0f00) == 0x5)
	{
	    UINT16 op2 = ROPCODE((PC<<1)+2) ;

		if (BITS(op2,0xfe20) == 0x02)
		{
			// MOVE(M) - 0000 0101 BBBB BBBB | 0000 001W --0- -HHH
			retSize = MoveMOperation() ;
		}
		else if (BITS(op2,0xf810) == 0x0e)
		{
			// MOVE(C) - 0000 0101 BBBB BBBB | 0011 1WDD DDD0 ----
		}
		else if (BITS(op2,0x00ff) == 0x11)
		{
			// MOVE - 0000 0101 BBBB BBBB | ---- HHHW 0001 0001
		}
	}
	else if (BITS(OP,0x0f00) == 0x6)
	{
		retSize = 1 ;
	}
	else if (BITS(OP,0x0f00) == 0x7)
	{
		switch(BITS(OP,0x0030))
		{
			// BS.cc - 0000 0111 RR00 cccc
			case 0x0: retSize = 1 ; break ;

			// BS.cc - 0000 0111 --01 cccc xxxx xxxx xxxx xxxx
			case 0x1: retSize = BsccOperation() ; break ;

			// B.cc - 0000 0111 RR10 cccc
			case 0x2: retSize = 1 ; break ;

			// B.cc - 0000 0111 --11 cccc xxxx xxxx xxxx xxxx
			case 0x3: retSize = BccOperation() ; break ;
		}
	}
	else if (BITS(OP,0x0800))
	{
		switch (BITS(OP,0x0700))
		{
			// BRA - 0000 1011 aaaa aaaa
			case 0x3: retSize = BraOperation() ; break ;

			// MOVE(P) - 0000 110W RRmp pppp
			case 0x4: case 0x5: retSize = MovePOperation() ; break ;

			// DO - 0000 1110 iiii iiii xxxx xxxx xxxx xxxx
			case 0x6: retSize = DoOperation() ;  break ;

			// REP - 0000 1111 iiii iiii
			case 0x7: retSize = RepOperation() ; break ;
		}
	}

	return retSize ;
}

/*
IMPLEMENTATIONS
*/

static int BitfieldOperation(void)
{
	int retSize = 0 ;

	// BITFIELD OPERATIONS
	// ONLY PARTIALLY IMPLEMENTED !!!

	// Get the next part of the opcode...
	UINT16 op2 = ROPCODE((PC<<1)+2) ;

	unsigned char dt = 0x00 ;
	void *destinationReg = 0x00 ;
	void *rReg = 0x00 ;

	UINT16 destAddr = 0x00 ;

    UINT16 opVal ;

	// Get the universal immediate value and its proper mask...
	UINT16 iVal    = op2 & 0x00ff ;
	DecodeBBBBitMask(BITS(op2,0xe000), &iVal) ;

	// Figure out where data you'll be dealing with comes from...
	switch(BITS(OP,0x00e0))
	{
		case 0x6: case 0x7: case 0x2: case 0x3:
			destAddr = AssembleDFromPTable(BITS(OP,0x0020), BITS(OP,0x001f)) ;
			break ;

		case 0x5:             case 0x1:
			rReg = DecodeRRTable(BITS(OP,0x0003), &dt) ;
			destAddr = *((UINT16*)rReg) ;
			break ;

		case 0x4: case 0x0:
			destinationReg = DecodeDDDDDTable(BITS(OP,0x001f), &dt) ;
			break ;
	}

	// Grab the value to operate on...
	if (destinationReg)
	{
		if (dt == DT_LONG_WORD)
			opVal = (*((PAIR64*)destinationReg)).w.h ;
		else
			opVal = *((UINT16*)destinationReg) ;
	}
	else
		opVal = data_read_word_16le(destAddr<<1) ;

	// Do the necessary bitfield operation...
	switch(BITS(op2,0x1f00))
	{
		// bfchg
		case 0x12:
			if ((iVal & opVal) == iVal)   C_bit_set(1);		// test
			else					      C_bit_set(0) ;

			opVal ^= iVal ;									// and change
			break ;

		// bfclr
		case 0x04:
			opVal &= (~iVal) ;
			break ;

		// bfset
		case 0x18:
			opVal |= iVal ;
			break ;

		// bfsth
		case 0x10:
			mame_printf_debug("BFSTH - not here yet") ;
			break ;

		// bftstl
		case 0x00:
			if ((iVal & opVal) == 0)   C_bit_set(1);
			else				       C_bit_set(0);
			retSize = 2; return retSize;				/* It's just a test - no need to go on */
			break ;
	}

	// Put the data back where it belongs
	// !!! When setting A and B, I'm pretty sure if it's supposed to pass through the normal 'setDestValue' pipe but not entirely !!!
	//     (in other words, does clearing ff.ffff.ffff with 001c really result in ff.ffe3.0000 or ff.ffe3.ffff) ???
	if (destinationReg)
		SetDestinationValue(&opVal, DT_WORD, destinationReg, dt) ;
	else
		SetDataMemoryValue(&opVal, DT_WORD, destAddr<<1) ;

	retSize = 2 ;
	return retSize ;
}

static int MoveCOperation(void)
{
	int retSize = 0 ;

	unsigned char W = 0x00 ;

	unsigned char sdt = 0x00 ;
	void *SD ;

	// ONLY PARTIALLY IMPLEMENTED !!!

	SD = DecodeDDDDDTable(BITS(OP,0x03e0), &sdt) ;
	W  = BITS(OP,0x0400) ;

	if (BITS(OP,0x1000))
	{
		// Single source/destination move(c)
		if (!BITS(OP,0x0010))
		{
			// MM in move(c)

			// Get the r register...
			unsigned char rt = 0x00 ;
			void *R ;

			R = DecodeRRTable(BITS(OP,0x0003), &rt) ;

			if (W)
			{
				// !!! does this work ???
				UINT16 data = data_read_word_16le((*((UINT16*)R))<<1) ;
				SetDestinationValue(&data, DT_WORD, SD, sdt) ;
			}
			else
			{
				SetDataMemoryValue(SD, sdt, (*((UINT16*)R))<<1) ;
			}

			ExecuteMMTable(BITS(OP,0x0003), BITS(OP,0x000c)) ;
			retSize = 1 ;
		}
		else
		{
			if (!BITS(OP,0x0004))
			{
				// q in move(c)

				// Figure out the true addr...
				UINT16 addr = ExecuteqTable(BITS(OP,0x0003), BITS(OP,0x0008));

				if (W)
				{
					// !!! does this work ???
					UINT16 data = data_read_word_16le(addr<<1) ;
					SetDestinationValue(&data, DT_WORD, SD, sdt) ;
				}
				else
				{
					SetDataMemoryValue(SD, sdt, addr<<1);
				}

				retSize = 1 ;
			}
			else
			{
				if (BITS(OP,0x0002))
				{
					// Z in move(c)
					unsigned char rt = 0x00;
					void *Z;

					Z = DecodeZTable(BITS(OP,0x0008), &rt);

					if (W)
					{
						// Unimplemented
						retSize = 666 ;
					}
					else
					{
						SetDataMemoryValue(SD, sdt, (*((UINT16*)Z))<<1);
						retSize = 1;
					}
				}
				else
				{
					// 2-word move(c)
				    UINT16 nextWord = ROPCODE((PC<<1)+2) ;

					if (BITS(OP,0x0008))
					{
						// #xxxx - value read directly out of the opcode
						SetDestinationValue(&nextWord, DT_WORD, SD, sdt) ;
					}
					else
					{
						// X:xxxx = value resides in X memory...
						if (W)
						{
							// Read from X-data memory...
							// !!! Does this work ???
							UINT16 data = data_read_word_16le(nextWord<<1) ;
							SetDestinationValue(&data, DT_WORD, SD, sdt) ;
						}
						else
						{
							// Write to X-data memory...
							SetDataMemoryValue(SD, sdt, nextWord<<1) ;
						}
					}

					retSize = 2 ;
				}
			}
		}
	}
	else if (BITS(OP,0x2000))
	{
		// Source and Destination explicit
		unsigned char dt = 0x00 ;
		void *D ;

		D = DecodeDDDDDTable(BITS(OP,0x001f), &dt) ;
		SetDestinationValue(SD, sdt, D, dt) ;

		retSize = 1 ;
	}
	else
	{
		// Double source/destination move(c)

		retSize = 666 ;
	}

	return retSize ;
}

static int MoveMOperation(void)
{
	int retSize = 0 ;

	// ONLY PARTIALLY IMPLEMENTED !!!

	if (BITS(OP,0x0400))
	{
		// The double word MoveM operation...
	    UINT16 nextWord = ROPCODE((PC<<1)+2) ;

		UINT8 increment = BITS(OP,0x00ff) ;

		int W = BITS(nextWord, 0x0100) ;

		unsigned char sdt = 0x00 ;
		void *sdReg = 0x00 ;
		sdReg = DecodeHHHTable(BITS(nextWord,0x0007), &sdt) ;

		if (W)
		{
			// Read from Program Memory...
			UINT16 data = program_read_word_16le((R2<<1)+(increment<<1)) ;
			SetDestinationValue(&data, DT_WORD, sdReg, sdt) ;
		}
		else
		{
			// Write to Program Memory...
			// !!! Does this work ???
			// SetMemoryValue(sdReg, sdt, ((R2<<1)+(increment<<1))) ;
		}

		retSize = 2 ;
	}
	else
	{
		// The single word MoveM operation...

		// Decode which way the data is going...
		int W = BITS(OP,0x0100) ;

		// Decode which R Register we're working with...
		unsigned char rt = 0x00 ;
		void *rReg = 0x00 ;
		rReg = DecodeRRTable(BITS(OP,0x00c0), &rt) ;

		if (BITS(OP,0x0020))
		{
			// R and R operation
			retSize = 666 ;
		}
		else
		{
			// R and SD operation
			// Decode the SD register...
			unsigned char sdt = 0x00 ;
			void *sdReg = 0x00 ;
			sdReg = DecodeHHHTable(BITS(OP,0x0007), &sdt) ;

			if (W)
			{
				// Read from Program Memory...
				UINT16 data = program_read_word_16le( (*((UINT16*)rReg)) << 1 ) ;
				SetDestinationValue(&data, DT_WORD, sdReg, sdt) ;
			}
			else
			{
				// Write to Program Memory...
				SetProgramMemoryValue(sdReg, sdt, (*((UINT16*)rReg)) << 1 ) ;
			}

			// Execute the increment part of the opcode...
			ExecuteMMTable(BITS(OP,0x00c0), BITS(OP,0x0018)) ;

			retSize = 1 ;
		}
	}

	return retSize ;
}

static int MoveIOperation(void)
{
	int retSize = 1 ;

	// The 8-bit signed immediate operand is stored in the low byte of destination
	//   register D after having been sign extended.

	INT8 immediateVal = OP & 0x00ff ;
	UINT16 immediateWord ;

	unsigned char dt = 0x00 ;
	void *destinationReg = 0x00 ;

	destinationReg = DecodeDDTable(BITS(OP,0x0300), &dt) ;

	// Sign extend the immediate value...
	// !!! On Intel architecture this works correctly, does it work everywhere ???
	immediateWord = (INT16)immediateVal ;

	SetDestinationValue(&immediateWord, DT_WORD, destinationReg, dt) ;

	return retSize ;
}

static int MovePOperation(void)
{
	int retSize = 1 ;

	if (BITS(OP,0x1000))
	{
		// X:<pp> and SD
		UINT16 pp ;
		UINT16 W  ;

		unsigned char sdt = 0x00 ;
		void *SD = 0x00 ;

		SD = DecodeHHTable(BITS(OP,0x00c0), &sdt) ;

		pp = BITS(OP,0x001f) ;
		pp = AssembleAddressFromIOShortAddress(pp) ;

		W = BITS(OP,0x0100) ;

		if (W)
		{
			UINT16 data = data_read_word_16le(pp<<1) ;
			SetDestinationValue(&data, DT_WORD, SD, sdt) ;
		}
		else
		{
			SetDataMemoryValue(SD, sdt, pp<<1) ;
		}
	}
	else
	{
		// X:<Rx> and X:<pp>
		UINT16 pp ;
		UINT16 W  ;

		unsigned char sdrt = 0x00 ;
		void *sdrReg       = 0x00 ;
		sdrReg = DecodeRRTable(BITS(OP,0x00c0), &sdrt) ;

		pp = BITS(OP,0x001f) ;
		pp = AssembleAddressFromIOShortAddress(pp) ;

		W = BITS(OP,0x0100) ;

		// A little different than most W if's - opposite read and write thingies...
		if (W)
		{
			UINT16 data = data_read_word_16le( (*((UINT16*)sdrReg))<<1 ) ;		// read the word out
			SetDataMemoryValue(&data, DT_WORD, pp<<1) ;
		}
		else
		{
			mame_printf_debug("NOTHING HERE (yet)\n") ;
		}

		// Postincrement
		ExecutemTable(BITS(OP,0x00c0), BITS(OP,0x0020)) ;
	}

	return retSize ;
}


static int JmpOperation(void)
{
	int retSize = 0 ;

	core.ppc = PC;

	if (BITS(OP,0x0010))
	{
		PC = (ROPCODE((PC<<1)+2)) ;		// Can this offset by 2 bytes be handled better?  Say, for example, by moving "1 word"?
		change_pc(PC) ;
	}
	else
	{
		unsigned char rt = 0x00 ;
		void *R = 0x00 ;

		R = DecodeRRTable(BITS(OP,0x0003), &rt);

		PC = *((UINT16*)R);
		change_pc(PC);
	}

	retSize = 0 ;					// Jump with no offset.

	return retSize ;
}

static int BsrOperation(void)
{
	int retSize = 0 ;

	INT16 branchOffset = (ROPCODE((PC<<1)+2)) ;	// Can this offset by 2 bytes be handled better?  Say, for example, by moving "1 word"?

	// "The PC Contains the address of the next instruction"
	PC += 2 ;
	core.ppc = PC ;

	// Push the PC to the stack...
	SP++ ;					// !!! I'm not so sure about this 'pre-increment'
							//     The reason it might be there is that SSH and SSL are supposed to point to the top of the
							//     stack (and the docs seem to imply this behaviour).  But it wastes the first stack entry ???
							//     !!! I think i've read somewhere that PC == 1 when the stack is empty ???
							//     !!! I also .know. there are only 15 stack entries - could this be why ???
	SSH = PC ;
	SSL = SR ;

	PC += branchOffset ;

	retSize = 0 ;									// Branch and then no offset.

	change_pc(PC) ;

	return retSize ;
}

static int JsrOperation(void)
{
	int retSize = 0;

	// Handle other JSR types!
	INT16 branchOffset = (ROPCODE((PC<<1)+2)) ;	// Can this offset by 2 bytes be handled better?  Say, for example, by moving "1 word"?

	// The docs say nothing of this, but I swear it *must* be true
	PC += 2 ;
	core.ppc = PC ;

	// See bsr operation
	SP++;
	SSH = PC;
	SSL = SR;

	PC = branchOffset;

	return retSize;
}

static int BraOperation(void)
{
	int retSize = 0 ;

	// Branch to the location in program memory at location PC+displacement.
	INT16 branchOffset = 0x00;

	if (BITS(OP,0x0800))
	{
		// 8 bit immediate (relative) offset
		branchOffset = (INT8)BITS(OP,0x00ff) ;

		// "The PC Contains the address of the next instruction"
		PC += 1 ;
	}
	else if (BITS(OP,0x0010))
	{
		// 16 bit immediate (relative) offset
		retSize = 666 ;
	}
	else if (!BITS(OP,0x0010))
	{
		// 16 bit Rx offset
		retSize = 666 ;
	}

	core.ppc = PC ;
	PC += branchOffset ;

	change_pc(PC) ;

	return retSize ;
}


static int DoOperation(void)
{
	int retSize = 0 ;

	UINT16 offset ;

	// First instruction cycle
	SP++ ;
	SSH = LA ;
	SSL = LC ;

	if (BITS(OP,0x0800))
	{
		// #xx,expr
		LC = BITS(OP,0x00ff) ;
		retSize = 2 ;
	}
	else if (BITS(OP,0x0400))
	{
		// S,expr
		unsigned char sdt = 0x00 ;
		void *SD ;

		// !!! Watch out for the special cases (FamilyManual page 276) !!!
		SD = DecodeDDDDDTable(BITS(OP,0x001f), &sdt) ;
		SetDestinationValue(SD, sdt, &LC, DT_WORD) ;
		retSize = 2 ;
	}
	else
	{
		// X:(Rn),expr
		retSize = 666 ;
	}


	// Second instruction cycle
	SP++ ;
	SSH = PC+2 ;			// The value in the Program Counter (PC) register pushed onto the system stack is the address of the first instruction following the DO instruction
	SSL = SR ;

	offset = 0x00 ;			// The offset is the same for all DO instructions...
	offset = (ROPCODE((PC<<1)+2)) ;
	offset += PC+2 ;
	LA = offset ;


	// Third instruction cycle
	LF_bit_set(1);

	return retSize ;
}

static void EndOfLoopProcessing(void)
{
	SR = SSL ;
	SP-- ;
	LA = SSH ;
	LC = SSL ;
	SP-- ;
}

static int Dec24Operation(void **wd, UINT64 *pa)
{
	int retSize = 1 ;

	UINT64 decMe ;

	unsigned char drt = 0x00 ;
	void *D = 0x00 ;
	D = DecodeFTable(BITS(OP,0x0008), &drt) ;

	*pa = *((UINT64*)D) ;	// backup the previous value

	// !! I wonder if this should be a signed int instead !!
	decMe = (*((UINT64*)D) & (UINT64)U64(0xffffff0000)) >> 16 ;
	decMe-- ;
	decMe &= (UINT64)U64(0x0000000000ffffff) ;						// For wraparound - Make sure no bits get set improperly in A/B

//  UINT8 carryBit = (*((UINT64*)D) & 0x0000008000000000) >> 39 ;   // For determinating carry status...

	*((UINT64*)D) &= (UINT64)U64(0x000000000000ffff) ;				// Prepare the destination to be set...
	*((UINT64*)D) |= (decMe << 16) ;						// Set the top 24 bits accordingly...

	// Carry flag (C) - HACK
//  if (decMe == 0x0000000000ff7fff)
//      SET_cBIT() ;
//  else
//      CLEAR_cBIT() ;

	// Carry flag (C)
//  UINT8 postCarryBit = (*((UINT64*)D) & 0x0000008000000000) >> 39 ;
//  if (postCarryBit != carryBit)   SET_cBIT() ;
//  else                          CLEAR_cBIT() ;

    *wd = D ;

/*
    UINT32 decMe = ((*((PAIR64*)D)).b.h4 << 16) | (*((PAIR64*)D)).w.h ;

    int fflag = 0 ;
    if (decMe == 0x00007fff) fflag = 1 ;

    if (fflag) mame_printf_debug("%08x\n", decMe) ;

    decMe-- ;

    if (fflag) mame_printf_debug("%08x\n", decMe) ;

    decMe &= 0x00ffffff ;

    if (fflag) mame_printf_debug("%08x\n", decMe) ;

    UINT8 carryBit = ((*((PAIR64*)D)).b.h4 & 0x80) >> 7 ;

    (*((PAIR64*)D)).b.h4 = (decMe & 0x00ff0000) >> 16 ;
    (*((PAIR64*)D)).w.h  = (decMe & 0x0000ffff) ;

    if (fflag) mame_printf_debug("%2x\n", (*((PAIR64*)D)).b.h4) ;
    if (fflag) mame_printf_debug("%4x\n", (*((PAIR64*)D)).w.h) ;

    // Carry flag (C)
    UINT8 postCarryBit = ((*((PAIR64*)D)).b.h4 & 0x80) >> 7 ;
    if (postCarryBit != carryBit)   SET_cBIT() ;
    else                          CLEAR_cBIT() ;

    *wd = D ;
*/
	return retSize ;
}

static int AddOperation(void **wd, UINT64 *pa)
{
	UINT64 addVal = 0x00;

	unsigned char st = 0x00, dt = 0x00 ;
	void *S = 0x00, *D = 0x00;

	DecodeJJJFTable(BITS(OP,0x0007),BITS(OP,0x0008), &S, &st, &D, &dt) ;

	*pa = *((UINT64*)D) ;	// backup the previous value

	switch(st)
	{
		case DT_WORD:        addVal = (UINT64)*((UINT16*)S) << 16 ; break ;
		case DT_DOUBLE_WORD: addVal = (UINT64)*((UINT32*)S) ;       break ;
		case DT_LONG_WORD:   addVal = (UINT64)*((UINT64*)S) ;       break ;
	}

	*((UINT64*)D) += addVal ;

	*wd = D ;

    return 1 ;
}

static int LsrOperation(void **wd, UINT64 *pa)
{
	int retSize = 1 ;

	UINT16 result ;

	unsigned char dt = 0x00 ;
	void *D = 0x00 ;

	D = DecodeFTable(BITS(OP,0x0008), &dt) ;

	*pa = *((UINT64*)D) ;	// backup the previous value

    // !!! Make sure to hold the MS bit and shift the LS bit properly !!!

	result = (*((PAIR64*)D)).w.h >> 1 ;
	(*((PAIR64*)D)).w.h = result ;

	*wd = D ;
	return retSize ;
}

static int AsrOperation(void **wd, UINT64 *pa)
{
	int retSize = 1 ;

	unsigned char dt = 0x00 ;
	void *D = 0x00 ;

	D = DecodeFTable(BITS(OP,0x0008), &dt) ;

	*pa = *((UINT64*)D) ;	// backup the previous value

    // !!! Make sure to hold the MS bit and shift the LS bit properly !!!

	*((UINT64*)D) = (*((UINT64*)D)) >> 1 ;

	*wd = D ;
	return retSize ;
}

static int NotOperation(void **wd, UINT64 *pa)
{
	int retSize = 1 ;

	unsigned char dt = 0x00 ;
	void *D = 0x00 ;

	D = DecodeFTable(BITS(OP,0x0008), &dt) ;

	*pa = *((UINT64*)D) ;	// backup the previous value

	(*((PAIR64*)D)).w.h ^= 0xffff ;

	*wd = D ;
	return retSize ;
}


static int EorOperation(void **wd, UINT64 *pa)
{
	int retSize = 1 ;

	UINT16 result ;

	unsigned char st = 0x00, dt = 0x00 ;
	void *S = 0x00, *D = 0x00;

	DecodeJJFTable(BITS(OP,0x0003),BITS(OP,0x0008), &S, &st, &D, &dt) ;

	*pa = *((UINT64*)D) ;	// backup the previous value

	// This operation does an XOR of a word of S with A1/B1...
	result = *((UINT16*)S) ^ (*((PAIR64*)D)).w.h ;
	(*((PAIR64*)D)).w.h = result ;

	*wd = D ;
	return retSize ;
}

static int TfrDataALUOperation(void **wd, UINT64 *pa)
{
	int retSize = 1 ;

	unsigned char st = 0x00, dt = 0x00 ;
	void *S = 0x00, *D = 0x00;

	DecodeJJJFTable(BITS(OP,0x0007),BITS(OP,0x0008), &S, &st, &D, &dt) ;

	*pa = *((UINT64*)D) ;	// backup the previous value

	SetDestinationValue(S, st, D, dt) ;

	*wd = D ;
	return retSize ;
}

static int AndOperation(void **wd, UINT64 *pa)
{
	int retSize = 1 ;

	UINT16 result ;

	unsigned char st = 0x00, dt = 0x00 ;
	void *S = 0x00, *D = 0x00;

	DecodeJJFTable(BITS(OP,0x0003),BITS(OP,0x0008), &S, &st, &D, &dt) ;

	*pa = *((UINT64*)D) ;	// backup the previous value

	// This operation does an AND of a word of S with A1/B1...
	result = *((UINT16*)S) & (*((PAIR64*)D)).w.h ;
	(*((PAIR64*)D)).w.h = result ;

	*wd = D ;
	return retSize ;
}

static int OrOperation(void **wd, UINT64 *pa)
{
	int retSize = 1 ;

	UINT16 result ;

	unsigned char st = 0x00, dt = 0x00 ;
	void *S = 0x00, *D = 0x00;

	DecodeJJFTable(BITS(OP,0x0003),BITS(OP,0x0008), &S, &st, &D, &dt) ;

	*pa = *((UINT64*)D) ;	// backup the previous value

	// This operation does an OR of a word of S with A1/B1...
	result = *((UINT16*)S) | (*((PAIR64*)D)).w.h ;
	(*((PAIR64*)D)).w.h = result ;

	*wd = D ;
	return retSize ;
}


static int TccOperation(void)
{
	int retSize = 1 ;

	int transfer = DecodeccccTable(BITS(OP,0x03c0)) ;

    // !!! WARNING === TESTING !!!
	// transfer = 1 ;

	if (transfer)
	{
		unsigned char st = 0x00, dt = 0x00 ;
		void *S = 0x00, *D = 0x00;

		unsigned char drt    = 0x00 ;
		void *destinationReg = 0x00 ;

		Decodeh0hFTable(BITS(OP,0x0007),BITS(OP,0x0008), &S, &st, &D, &dt) ;

		SetDestinationValue(S, st, D, dt) ;

		// !!! What's up with that A,A* thing in the docs?  Does that mean "do not transfer R0 to Rn ???
		destinationReg = DecodeRRTable(BITS(OP,0x0030), &drt) ; // Actually a TT table, but it's the same

		SetDestinationValue(&R0, DT_WORD, destinationReg, drt) ;
	}

	// Otherwise it's a NOP

	return retSize ;
}

static int ClrOperation(void **wd, UINT64 *pa)
{
	int retSize = 1 ;

	UINT64 clearValue = 0 ;

	unsigned char drt    = 0x00 ;
	void *destinationReg = 0x00 ;

	destinationReg = DecodeFTable(BITS(OP,0x0008), &drt) ;

	*pa = *((UINT64*)destinationReg) ;	// backup the previous value

	SetDestinationValue(&clearValue, DT_LONG_WORD, destinationReg, drt) ;

    *wd = destinationReg ;
	return retSize ;
}

static int TstOperation(void **wd, UINT64 *pa)
{
	int retSize = 1 ;

	unsigned char drt    = 0x00 ;
	void *destinationReg = 0x00 ;
	destinationReg = DecodeFTable(BITS(OP,0x0008), &drt) ;

	// S - "computed according to the standard definition (section A.4)"
	// L - can't be done until after data move
	// E - set if signed integer portion is in use
	// U - set according to the standard definition of the U bit
	// N - set if bit 39 is set
	if (*((UINT64*)destinationReg) & (UINT64)U64(0x8000000000)) N_bit_set(1);
	else										                N_bit_set(0);

	// Z - set if == 0
	if (*((UINT64*)destinationReg) == 0) Z_bit_set(1);
	else								 Z_bit_set(0);

	// V&C - always cleared
	V_bit_set(0);
	C_bit_set(0);

	*pa = *((UINT64*)destinationReg) ;	// backup the previous value (no change here)
	*wd = destinationReg;

	return retSize ;
}

static int AndiOperation(void)
{
	int retSize = 1 ;

	UINT16 andMe = BITS(OP,0x00ff) ;

	// Because I don't have a good way to access MR and CCR right now, i'm going to
	//   hard-code decoding EE here and in the OriOperation as well...
	switch(BITS(OP,0x0600))
	{
		// MR
		case 0x01:
			SR  &= ((andMe << 8) | 0x00ff) ;
			break ;

		// CCR
		case 0x03:
			SR  &= ((andMe)      | 0xff00) ;
			break ;

		// OMR
		case 0x02:
			OMR &= (UINT8)(andMe) ;
			break ;

		default:
			fatalerror("DSP56k - BAD EE value in andi operation") ;
	}


	return retSize ;
}

#ifdef UNUSED_FUNCTION
// !!! NOT IMPLEMENTED YET - JUST PUT HERE TO BE NEXT TO ANDI SOMEDAY !!!
static int OriOperation(void)
{
	int retSize = 1 ;

	return retSize ;
}
#endif

static int CmpOperation(void **wd, UINT64 *pa)
{
	int retSize = 1 ;

	UINT64 cmpVal ;
	UINT64 result ;

	unsigned char st = 0x00, dt = 0x00 ;
	void *S = 0x00, *D = 0x00;

	DecodeJJJFTable(BITS(OP,0x0007),BITS(OP,0x0008), &S, &st, &D, &dt) ;

	*pa = *((UINT64*)D) ;	// backup the previous value - not necessary for cmp :)

	switch(st)
	{
		default:
		case DT_WORD:        cmpVal = (UINT64)*((UINT16*)S) ; break ;
		case DT_DOUBLE_WORD: cmpVal = (UINT64)*((UINT32*)S) ; break ;
		case DT_LONG_WORD:   cmpVal = (UINT64)*((UINT64*)S) ; break ;
	}

	result = *((UINT64*)D) - cmpVal ;

	if (result == 0)
		Z_bit_set(1) ;
	else
		Z_bit_set(0) ;

    *wd = D ;
	return retSize ;
}

static int BsccOperation(void)
{
	int retSize = 0 ;

	// Decode the condition
	int transfer = DecodeccccTable(BITS(OP,0x000f)) ;

	// TESTING !!!
	// transfer = 0 ;

	// 1 or 2 word operation...
	if (transfer)
	{
		if (BITS(OP,0x0010))
		{
			// 2 word immediate offset...
			INT16 branchOffset = (INT16)ROPCODE((PC<<1)+2) ;

			// "The PC Contains the address of the next instruction"
			PC += 2 ;
			core.ppc = PC ;

			SP++ ;
			SSH = PC ;
			SSL = SR ;

			PC += branchOffset ;

			change_pc(PC) ;
		}
		else
		{
			// 1 word Rx offset...
		}

		retSize = 0 ;
	}
	else
	{
		if (BITS(OP,0x0010))
			retSize = 2 ;
		else
			retSize = 1 ;
	}

	return retSize ;
}

static int BccOperation(void)
{
	int retSize = 0 ;

	if (!BITS(OP,0x2000))
	{
		int branch = DecodeccccTable(BITS(OP,0x000f)) ;

		if (branch)
		{
			if (BITS(OP,0x0010))
			{
				// B.cc xxxx
			    INT16 op2 = ROPCODE((PC<<1)+2) ;

				// The PC contains the address of the next instruction
				PC += 2 ;
    			core.ppc = PC ;
				PC += op2 ;

				change_pc(PC) ;
			}
			else
			{
				// B.cc Rn
			}
		}
		else
		{
			if (BITS(OP,0x0010))
				retSize = 2 ;
			else
				retSize = 1 ;
		}
	}
	else
	{
		// B.cc aa
		int branch = DecodeccccTable(BITS(OP,0x03c0)) ;

		if (branch)
		{
			INT16 offset = (INT16)AssembleAddressFrom6BitSignedRelativeShortAddress(BITS(OP,0x003f)) ;

			PC += 1 ;
			core.ppc = PC ;
			PC += offset ;

			change_pc(PC) ;
		}
		else
		{
			retSize = 1 ;
		}
	}

	return retSize ;
}

static int Tst2Operation(void)
{
	int retSize = 1 ;

	unsigned char dt = 0x00 ;
	void *destinationReg = 0x00 ;

	destinationReg = DecodeDDTable(BITS(OP,0x0003), &dt) ;

	// unnormalized

	// negative
	// bit 31 of A or B ???

	// zero
	if ( (*((UINT16*)destinationReg)) == 0)
		Z_bit_set(1);
	else
		Z_bit_set(0);

	// always clear C flag
	C_bit_set(0) ;

	return retSize ;
}

static int MACsuuuOperation(void)
{
	int retSize = 1 ;

	INT64 result = 0 ;

	int s ;
	void *S1 = 0x00, *S2 = 0x00, *D = 0x00 ;

	DecodeQQFTableSp(BITS(OP,0x0003), BITS(OP,0x0008), &S1, &S2, &D) ;

	s = BITS(OP,0x0004) ;

	// !!! There is no way this is right - wtf is EXT:MSP:LSP format ???
	if (!s)
	{
		// su
		result = ( (INT32)(*((UINT16*)S1)) *  (UINT32)(*((UINT16*)S2)) ) << 1 ;
	}
	else
	{
		// uu
		result = ( (UINT32)(*((UINT16*)S1)) * (UINT32)(*((UINT16*)S2)) ) << 1 ;
	}

	(*((UINT64*)D)) += result ;

	return retSize ;
}

static int RepOperation(void)
{
	int retSize = 1 ;

	UINT16 count = 0x0000 ;

	if (BITS(OP,0x0f00) == 0xf)
	{
		// immediate
		count = BITS(OP,0x00ff) ;
	}
	else if (BITS(OP,0x0f00) == 0x4)
	{
		// SD
		unsigned char st = 0x00 ;
		void *S ;

		S = DecodeDDDDDTable(BITS(OP,0x001f), &st) ;
		SetDestinationValue(S, st, &count, DT_WORD);
	}
	else
	{
		// X:(Rn)
	}

	TEMP = LC ;
	LC = count ;

	core.repFlag = 1 ;
	core.repAddr = PC + (retSize<<1) ;

	return retSize ;
}

static void EndOfRepProcessing(void)
{
	LC = TEMP ;

	core.repFlag = 0 ;
	core.repAddr = 0x0000 ;
}


/*
DECODERS
*/
static void *DecodeDDDDDTable(UINT16 DDDDD, unsigned char *dt)
{
	void *retAddress = 0x00 ;

	switch(DDDDD)
	{
		case 0x00: retAddress = &X0  ; *dt = DT_WORD ;      break ;
		case 0x01: retAddress = &Y0  ; *dt = DT_WORD ;      break ;
		case 0x02: retAddress = &X1  ; *dt = DT_WORD ;      break ;
		case 0x03: retAddress = &Y1  ; *dt = DT_WORD ;      break ;
		case 0x04: retAddress = &A   ; *dt = DT_LONG_WORD ; break ;
		case 0x05: retAddress = &B   ; *dt = DT_LONG_WORD ; break ;
		case 0x06: retAddress = &A0  ; *dt = DT_WORD ;      break ;
		case 0x07: retAddress = &B0  ; *dt = DT_WORD ;      break ;
		case 0x08: retAddress = &LC  ; *dt = DT_WORD ;      break ;
		case 0x09: retAddress = &SR  ; *dt = DT_WORD ;      break ;
		case 0x0a: retAddress = &OMR ; *dt = DT_BYTE ;      break ;
		case 0x0b: retAddress = &SP  ; *dt = DT_BYTE ;      break ;
		case 0x0c: retAddress = &A1  ; *dt = DT_WORD ;      break ;
		case 0x0d: retAddress = &B1  ; *dt = DT_WORD ;      break ;
		case 0x0e: retAddress = &A2  ; *dt = DT_BYTE ;      break ;
		case 0x0f: retAddress = &B2  ; *dt = DT_BYTE ;      break ;

		case 0x10: retAddress = &R0  ; *dt = DT_WORD ;      break ;
		case 0x11: retAddress = &R1  ; *dt = DT_WORD ;      break ;
		case 0x12: retAddress = &R2  ; *dt = DT_WORD ;      break ;
		case 0x13: retAddress = &R3  ; *dt = DT_WORD ;      break ;
		case 0x14: retAddress = &M0  ; *dt = DT_WORD ;      break ;
		case 0x15: retAddress = &M1  ; *dt = DT_WORD ;      break ;
		case 0x16: retAddress = &M2  ; *dt = DT_WORD ;      break ;
		case 0x17: retAddress = &M3  ; *dt = DT_WORD ;      break ;
		case 0x18: retAddress = &SSH ; *dt = DT_WORD ;      break ;
		case 0x19: retAddress = &SSL ; *dt = DT_WORD ;      break ;
		case 0x1a: retAddress = &LA  ; *dt = DT_WORD ;      break ;
		//no 0x1b
		case 0x1c: retAddress = &N0  ; *dt = DT_WORD ;      break ;
		case 0x1d: retAddress = &N1  ; *dt = DT_WORD ;      break ;
		case 0x1e: retAddress = &N2  ; *dt = DT_WORD ;      break ;
		case 0x1f: retAddress = &N3  ; *dt = DT_WORD ;      break ;
	}

	return retAddress ;
}

static void *DecodeRRTable(UINT16 RR, unsigned char *dt)
{
	void *retAddress = 0x00 ;

	switch(RR)
	{
		case 0x00: retAddress = &R0 ; *dt = DT_WORD ; break ;
		case 0x01: retAddress = &R1 ; *dt = DT_WORD ; break ;
		case 0x02: retAddress = &R2 ; *dt = DT_WORD ; break ;
		case 0x03: retAddress = &R3 ; *dt = DT_WORD ; break ;
	}

	return retAddress ;
}

static void *DecodeDDTable(UINT16 DD, unsigned char *dt)
{
	void *retAddress = 0x00 ;

	switch(DD)
	{
		case 0x00: retAddress = &X0 ; *dt = DT_WORD ; break ;
		case 0x01: retAddress = &Y0 ; *dt = DT_WORD ; break ;
		case 0x02: retAddress = &X1 ; *dt = DT_WORD ; break ;
		case 0x03: retAddress = &Y1 ; *dt = DT_WORD ; break ;
	}

	return retAddress ;
}

static void *DecodeHHHTable(UINT16 HHH, unsigned char *dt)
{
	void *retAddress = 0x00 ;

	switch(HHH)
	{
		case 0x0: retAddress = &X0 ; *dt = DT_WORD ;      break ;
		case 0x1: retAddress = &Y0 ; *dt = DT_WORD ;      break ;
		case 0x2: retAddress = &X1 ; *dt = DT_WORD ;      break ;
		case 0x3: retAddress = &Y1 ; *dt = DT_WORD ;      break ;
		case 0x4: retAddress = &A  ; *dt = DT_LONG_WORD ; break ;
		case 0x5: retAddress = &B  ; *dt = DT_LONG_WORD ; break ;
		case 0x6: retAddress = &A0 ; *dt = DT_WORD ;      break ;
		case 0x7: retAddress = &B0 ; *dt = DT_WORD ;      break ;
	}

	return retAddress ;
}

static void *DecodeHHTable(UINT16 HH, unsigned char *dt)
{
	void *retAddress = 0x00 ;

	switch(HH)
	{
		case 0x0: retAddress = &X0 ; *dt = DT_WORD ;      break ;
		case 0x1: retAddress = &Y0 ; *dt = DT_WORD ;      break ;
		case 0x2: retAddress = &A  ; *dt = DT_LONG_WORD ; break ;
		case 0x3: retAddress = &B  ; *dt = DT_LONG_WORD ; break ;
	}

	return retAddress ;
}

static void *DecodeZTable(UINT16 Z, unsigned char *dt)
{
	void *retAddress = 0x00 ;

	switch(Z)
	{
		// Fixed as per the Family Manual addendum
		case 0x01: retAddress = &A1 ; *dt = DT_WORD ; break ;
		case 0x00: retAddress = &B1 ; *dt = DT_WORD ; break ;
	}

	return retAddress ;
}


static UINT16 DecodeBBBBitMask(UINT16 BBB, UINT16 *iVal)
{
	UINT16 retVal = 0x00 ;

	switch(BBB)
	{
		case 0x4: retVal = 0xff00 ; *iVal <<= 8 ; break ;
		case 0x2: retVal = 0x0ff0 ; *iVal <<= 4 ; break ;
		case 0x1: retVal = 0x00ff ; *iVal <<= 0 ; break ;
	}

	return retVal ;
}

static void ExecuteMMTable(int x, UINT16 MM)
{
	UINT16 *rX = 0x00 ;
	UINT16 *nX = 0x00 ;

	switch(x)
	{
		case 0x0: rX = &R0 ; nX = &N0 ; break ;
		case 0x1: rX = &R1 ; nX = &N1 ; break ;
		case 0x2: rX = &R2 ; nX = &N2 ; break ;
		case 0x3: rX = &R3 ; nX = &N3 ; break ;
	}

	switch(MM)
	{
		case 0x0: /* do nothing */      break ;
		case 0x1: (*rX)++ ;             break ;
		case 0x2: (*rX)-- ;             break ;
		case 0x3: (*rX) = (*rX)+(*nX) ; break ;
	}
}

static void ExecutemTable(int x, UINT16 m)
{
	UINT16 *rX = 0x00 ;
	UINT16 *nX = 0x00 ;

	switch(x)
	{
		case 0x0: rX = &R0 ; nX = &N0 ; break ;
		case 0x1: rX = &R1 ; nX = &N1 ; break ;
		case 0x2: rX = &R2 ; nX = &N2 ; break ;
		case 0x3: rX = &R3 ; nX = &N3 ; break ;
	}

	switch(m)
	{
		case 0x0: (*rX)++ ;             break ;
		case 0x1: (*rX) = (*rX)+(*nX) ; break ;
	}
}

static void ExecutezTable(int x, UINT16 z)
{
	UINT16 *rX = 0x00 ;
	UINT16 *nX = 0x00 ;

	switch(x)
	{
		case 0x0: rX = &R0 ; nX = &N0 ; break ;
		case 0x1: rX = &R1 ; nX = &N1 ; break ;
		case 0x2: rX = &R2 ; nX = &N2 ; break ;
		case 0x3: rX = &R3 ; nX = &N3 ; break ;
	}

	if (!z)
	{
		(*rX)-- ;
	}
	else
	{
		(*rX) = (*rX)+(*nX) ;
	}
}

// Returns R address
static UINT16 ExecuteqTable(int x, UINT16 q)
{
	UINT16 *rX = 0x00 ;
	UINT16 *nX = 0x00 ;

	switch(x)
	{
		case 0x0: rX = &R0 ; nX = &N0 ; break ;
		case 0x1: rX = &R1 ; nX = &N1 ; break ;
		case 0x2: rX = &R2 ; nX = &N2 ; break ;
		case 0x3: rX = &R3 ; nX = &N3 ; break ;
	}

	switch(q)
	{
		case 0x0: /* No permanent changes */ ; return (*rX)+(*nX); break;
		case 0x1: (*rX)--;					   return (*rX);	   break;	// This one is special - it's a *PRE-decrement*!
	}

	exit(1);
	return 0x00;
}

static UINT16 AssembleDFromPTable(UINT16 P, UINT16 ppppp)
{
	UINT16 destAddr = 0x00 ;

	switch (P)
	{
		case 0x0: destAddr = ppppp ; break ;
		case 0x1: destAddr = AssembleAddressFromIOShortAddress(ppppp) ; break ;
	}

	return destAddr ;
}


static UINT16 AssembleAddressFromIOShortAddress(UINT16 pp)
{
	UINT16 fullAddy = 0xffe0 ;
	fullAddy |= pp ;
	return fullAddy ;
}

static UINT16 AssembleAddressFrom6BitSignedRelativeShortAddress(UINT16 srs)
{
	UINT16 fullAddy = srs ;
	if (fullAddy & 0x0020) fullAddy |= 0xffc0 ;

	return fullAddy ;
}

static int DecodeccccTable(UINT16 cccc)
{
	int retVal = 0 ;

	// !!! go back and check me again someday - potential bugs !!!
	switch (cccc)
	{
		// Arranged according to mnemonic table - not decoding table...
		case 0x0: if( C_bit() == 0)                              retVal = 1 ; break ; // cc(hs)
		case 0x8: if( C_bit() == 1)                              retVal = 1 ; break ; // cs(lo)
		case 0x5: if( E_bit() == 0)                              retVal = 1 ; break ; // ec
		case 0xa: if( Z_bit() == 1)                              retVal = 1 ; break ; // eq
		case 0xd: if( E_bit() == 1)                              retVal = 1 ; break ; // es
		case 0x1: if((N_bit() ^ V_bit()) == 0)                   retVal = 1 ; break ; // ge
		case 0x7: if((Z_bit() + (N_bit() ^ V_bit())) == 0)       retVal = 1 ; break ; // gt
		case 0x6: if( L_bit() == 0)                              retVal = 1 ; break ; // lc
		case 0xf: if((Z_bit() + (N_bit() ^ V_bit())) == 1)       retVal = 1 ; break ; // le
		case 0xe: if( L_bit() == 1)                              retVal = 1 ; break ; // ls
		case 0x9: if((N_bit() ^ V_bit()) == 1)                   retVal = 1 ; break ; // lt
		case 0xb: if( N_bit() == 1)                              retVal = 1 ; break ; // mi
		case 0x2: if( Z_bit() == 0)                              retVal = 1 ; break ; // ne
		case 0xc: if((Z_bit() + ((!U_bit()) & (!E_bit()))) == 1) retVal = 1 ; break ; // nr
		case 0x3: if( N_bit() == 0)                              retVal = 1 ; break ; // pl
		case 0x4: if((Z_bit() + ((!U_bit()) & (!E_bit()))) == 0) retVal = 1 ; break ; // nn
	}

	return retVal ;
}

static void Decodeh0hFTable(UINT16 h0h, UINT16 F,
					 void **source, unsigned char *st,
					 void **dest,   unsigned char *dt)
{
	UINT16 switchVal = (h0h << 1) | F ;

	switch (switchVal)
	{
		case 0x8: *source = &X0 ; *st = DT_WORD ;      *dest = &A ; *dt = DT_LONG_WORD ; break ;
		case 0x9: *source = &X0 ; *st = DT_WORD ;      *dest = &B ; *dt = DT_LONG_WORD ; break ;
		case 0xa: *source = &Y0 ; *st = DT_WORD ;      *dest = &A ; *dt = DT_LONG_WORD ; break ;
		case 0xb: *source = &Y0 ; *st = DT_WORD ;      *dest = &B ; *dt = DT_LONG_WORD ; break ;
		case 0x2: *source = &A  ; *st = DT_LONG_WORD ; *dest = &A ; *dt = DT_LONG_WORD ; break ;
		case 0x1: *source = &A  ; *st = DT_LONG_WORD ; *dest = &B ; *dt = DT_LONG_WORD ; break ;
		case 0x0: *source = &B  ; *st = DT_LONG_WORD ; *dest = &A ; *dt = DT_LONG_WORD ; break ;
		case 0x3: *source = &B  ; *st = DT_LONG_WORD ; *dest = &B ; *dt = DT_LONG_WORD ; break ;
	}
}

static void *DecodeFTable(UINT16 F, unsigned char *dt)
{
	void *retAddress = 0x00 ;

	switch(F)
	{
		case 0x0: retAddress = &A ; *dt = DT_LONG_WORD ; break ;
		case 0x1: retAddress = &B ; *dt = DT_LONG_WORD ; break ;
	}

	return retAddress ;
}

static void DecodeJJJFTable(UINT16 JJJ, UINT16 F,
					 void **source, unsigned char *st,
					 void **dest,   unsigned char *dt)
{
	UINT16 switchVal = (JJJ << 1) | F ;

	switch(switchVal)
	{
		case 0x0: *source = &B  ; *st = DT_LONG_WORD   ; *dest = &A ; *dt = DT_LONG_WORD ; break ;
		case 0x1: *source = &A  ; *st = DT_LONG_WORD   ; *dest = &B ; *dt = DT_LONG_WORD ; break ;
		case 0x4: *source = &X  ; *st = DT_DOUBLE_WORD ; *dest = &A ; *dt = DT_LONG_WORD ; break ;
		case 0x5: *source = &X  ; *st = DT_DOUBLE_WORD ; *dest = &B ; *dt = DT_LONG_WORD ; break ;
		case 0x6: *source = &Y  ; *st = DT_DOUBLE_WORD ; *dest = &A ; *dt = DT_LONG_WORD ; break ;
		case 0x7: *source = &Y  ; *st = DT_DOUBLE_WORD ; *dest = &B ; *dt = DT_LONG_WORD ; break ;
		case 0x8: *source = &X0 ; *st = DT_WORD        ; *dest = &A ; *dt = DT_LONG_WORD ; break ;
		case 0x9: *source = &X0 ; *st = DT_WORD        ; *dest = &B ; *dt = DT_LONG_WORD ; break ;
		case 0xa: *source = &Y0 ; *st = DT_WORD        ; *dest = &A ; *dt = DT_LONG_WORD ; break ;
		case 0xb: *source = &Y0 ; *st = DT_WORD        ; *dest = &B ; *dt = DT_LONG_WORD ; break ;
		case 0xc: *source = &X1 ; *st = DT_WORD        ; *dest = &A ; *dt = DT_LONG_WORD ; break ;
		case 0xd: *source = &X1 ; *st = DT_WORD        ; *dest = &B ; *dt = DT_LONG_WORD ; break ;
		case 0xe: *source = &Y1 ; *st = DT_WORD        ; *dest = &A ; *dt = DT_LONG_WORD ; break ;
		case 0xf: *source = &Y1 ; *st = DT_WORD        ; *dest = &B ; *dt = DT_LONG_WORD ; break ;
	}
}

static void DecodeJJFTable(UINT16 JJ, UINT16 F,
					void **source, unsigned char *st,
					void **dest, unsigned char *dt)
{
	UINT16 switchVal = (JJ << 1) | F ;

	switch (switchVal)
	{
		case 0x0: *source = &X0 ; *st = DT_WORD ; *dest = &A ; *dt = DT_LONG_WORD ; break ;
		case 0x1: *source = &X0 ; *st = DT_WORD ; *dest = &B ; *dt = DT_LONG_WORD ; break ;
		case 0x2: *source = &Y0 ; *st = DT_WORD ; *dest = &A ; *dt = DT_LONG_WORD ; break ;
		case 0x3: *source = &Y0 ; *st = DT_WORD ; *dest = &B ; *dt = DT_LONG_WORD ; break ;
		case 0x4: *source = &X1 ; *st = DT_WORD ; *dest = &A ; *dt = DT_LONG_WORD ; break ;
		case 0x5: *source = &X1 ; *st = DT_WORD ; *dest = &B ; *dt = DT_LONG_WORD ; break ;
		case 0x6: *source = &Y1 ; *st = DT_WORD ; *dest = &A ; *dt = DT_LONG_WORD ; break ;
		case 0x7: *source = &Y1 ; *st = DT_WORD ; *dest = &B ; *dt = DT_LONG_WORD ; break ;
	}
}

// Types not necessary - we know what we are...
static void DecodeQQFTableSp(UINT16 QQ, UINT16 F, void **S1, void **S2, void **D)
{
	UINT16 switchVal = (QQ << 1) | F ;

	switch(switchVal)
	{
		case 0x0: *S1 = &Y0 ; *S2 = &X0 ; *D = &A ; break ;
		case 0x1: *S1 = &Y0 ; *S2 = &X0 ; *D = &B ; break ;
		case 0x2: *S1 = &Y1 ; *S2 = &X0 ; *D = &A ; break ;
		case 0x3: *S1 = &Y1 ; *S2 = &X0 ; *D = &B ; break ;
		case 0x4: *S1 = &X1 ; *S2 = &Y0 ; *D = &A ; break ;
		case 0x5: *S1 = &X1 ; *S2 = &Y0 ; *D = &B ; break ;
		case 0x6: *S1 = &X1 ; *S2 = &Y1 ; *D = &A ; break ;
		case 0x7: *S1 = &X1 ; *S2 = &Y1 ; *D = &B ; break ;
	}
}

static void DecodeIIIITable(UINT16 IIII,
					 void **source, unsigned char *st,
					 void **dest, unsigned char *dt,
					 void *working)
{
	void *opposite = 0x00 ;

	if (working == &A) opposite = &B ;
	else               opposite = &A ;

	switch(IIII)
	{
		case 0x0: *source = &X0      ; *st = DT_WORD ;      *dest = opposite ; *dt = DT_LONG_WORD ; break ;
		case 0x1: *source = &Y0      ; *st = DT_WORD ;      *dest = opposite ; *dt = DT_LONG_WORD ; break ;
		case 0x2: *source = &X1      ; *st = DT_WORD ;      *dest = opposite ; *dt = DT_LONG_WORD ; break ;
		case 0x3: *source = &Y1      ; *st = DT_WORD ;      *dest = opposite ; *dt = DT_LONG_WORD ; break ;
		case 0x4: *source = &A       ; *st = DT_LONG_WORD ; *dest = &X0      ; *dt = DT_WORD ;      break ;
		case 0x5: *source = &B       ; *st = DT_LONG_WORD ; *dest = &Y0      ; *dt = DT_WORD ;      break ;
		case 0x6: *source = &A0      ; *st = DT_WORD ;      *dest = &X0      ; *dt = DT_WORD ;      break ;
		case 0x7: *source = &B0      ; *st = DT_WORD ;      *dest = &Y0      ; *dt = DT_WORD ;      break ;
		case 0x8: *source = working  ; *st = DT_LONG_WORD ; *dest = opposite ; *dt = DT_LONG_WORD ; break ;
		case 0x9: *source = working  ; *st = DT_LONG_WORD ; *dest = opposite ; *dt = DT_LONG_WORD ; break ;
		case 0xc: *source = &A       ; *st = DT_LONG_WORD ; *dest = &X1      ; *dt = DT_WORD ;      break ;
		case 0xd: *source = &B       ; *st = DT_LONG_WORD ; *dest = &Y1      ; *dt = DT_WORD ;      break ;
		case 0xe: *source = &A0      ; *st = DT_WORD ;      *dest = &X1      ; *dt = DT_WORD ;      break ;
		case 0xf: *source = &B0      ; *st = DT_WORD ;      *dest = &Y1      ; *dt = DT_WORD ;      break ;
	}
}

/*
MEMORY OPS
*/

static void XMDMOperation(UINT16 parameters, void *working, UINT64 *pa)
{
	UINT16 W ;
	unsigned char rdt = 0x00 ;
	void *R = 0x00 ;

	unsigned char sdt = 0x00 ;
	void *SD = 0x00 ;

	// Get the source
	SD = DecodeHHHTable(BITS(parameters,0x000e), &sdt) ;

	// Get the destination
	R = DecodeRRTable(BITS(parameters,0x0030), &rdt) ;

	// Move the data
	W = BITS(parameters,0x0001) ;

	if (W)
	{
		// from X:<ea> to SD...
		// !!! TODO : Fill in similar comments for other "if (W)'s" !!!
		UINT16 data = data_read_word_16le((*((UINT16*)R))<<1) ;		// read the word out
		SetDestinationValue(&data, DT_WORD, SD, sdt) ;
	}
	else
	{
		// from SD to X:<ea>...

		// If the source is the same as the ALU destination, use the Previous Accumulator value
		if (working == SD)
		{
			// the value before the ALU operation was executed...
			SetDataMemoryValue(pa, DT_LONG_WORD, (*((UINT16*)R))<<1) ;
		}
		else
		{
			// Expected memory move
			SetDataMemoryValue(SD, sdt, (*((UINT16*)R))<<1) ;
		}
	}

	// Postincrement
	ExecutemTable(BITS(parameters,0x0030), BITS(parameters,0x0040)) ;
}

static void RRDMOperation(UINT16 parameters, void *working, UINT64 *pa)
{
	unsigned char st = 0x00, dt = 0x00 ;
	void *S = 0x00, *D = 0x00;

	DecodeIIIITable(BITS(parameters,0x000f), &S, &st, &D, &dt, working) ;

	// If the source is what we're working with, use the old value
	if (S == working)
		SetDestinationValue(pa, DT_LONG_WORD, D, dt) ;
	else
		SetDestinationValue(S, st, D, dt) ;
}


static void XMDMSpecialOperation(UINT16 parameters, void *working)
{
    UINT16 W ;
	UINT16 *dest = 0x00 ;

	// Get the source/destination
	unsigned char sdt = 0x00 ;
	void *SD = 0x00 ;
	SD = DecodeHHHTable(BITS(parameters,0x000e), &sdt) ;

	// Get the opposite A/B depending on what was used before
    if (working == &A)
        dest = &B1 ;
    else if (working == &B)
        dest = &A1 ;
    else
    {
        mame_printf_debug("WTF MF\n") ;
        return ;
    }

	// Move the data
	W = BITS(parameters,0x0001) ;

	if (W)
	{
		// from X:<ea> to SD...
		// !!! DOES THIS WORK ???
		UINT16 data = data_read_word_16le((*dest)<<1) ;		// read the word out
		SetDestinationValue(&data, DT_WORD, SD, sdt) ;
	}
	else
	{
		// Normal memory move
		SetDataMemoryValue(SD, sdt, (*dest)<<1) ;
	}
}

static void ARUOperation(UINT16 parameters)
{
	ExecutezTable(BITS(parameters, 0x0003), BITS(parameters, 0x0004)) ;
}


/*
HELPERS
*/
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


static void SetDestinationValue(void *sourcePointer, unsigned char sourceType,
						 void *destinationPointer, unsigned char destinationType)
{
	UINT64 destinationValue = 0 ;

	switch(destinationType)
	{
		case DT_BYTE:
			switch(sourceType)
			{
				case DT_BYTE:        *((UINT8*)destinationPointer) = *((UINT8*) sourcePointer) ; break ;
				case DT_WORD:        *((UINT8*)destinationPointer) = *((UINT16*)sourcePointer) ; break ;
				case DT_DOUBLE_WORD: *((UINT8*)destinationPointer) = *((UINT32*)sourcePointer) ; break ;
				case DT_LONG_WORD:   *((UINT8*)destinationPointer) = *((UINT64*)sourcePointer) ; break ;
			}
		break ;

		case DT_WORD:
			switch(sourceType)
			{
				case DT_BYTE:        *((UINT16*)destinationPointer) = *((UINT8*) sourcePointer) ; break ;
				case DT_WORD:        *((UINT16*)destinationPointer) = *((UINT16*)sourcePointer) ; break ;
				case DT_DOUBLE_WORD: *((UINT16*)destinationPointer) = *((UINT32*)sourcePointer) ; break ;
				// !!! This one is interesting too - shifter limiter action !!!
				case DT_LONG_WORD:   *((UINT16*)destinationPointer) = *((UINT64*)sourcePointer) ; break ;
			}
		break ;

		case DT_DOUBLE_WORD:
			switch(sourceType)
			{
				case DT_BYTE:        *((UINT32*)destinationPointer) = *((UINT8*) sourcePointer) ; break ;
				case DT_WORD:        *((UINT32*)destinationPointer) = *((UINT16*)sourcePointer) ; break ;
				case DT_DOUBLE_WORD: *((UINT32*)destinationPointer) = *((UINT32*)sourcePointer) ; break ;
				case DT_LONG_WORD:   *((UINT32*)destinationPointer) = *((UINT64*)sourcePointer) ; break ;
			}
		break ;

		case DT_LONG_WORD:
			switch(sourceType)
			{
				case DT_BYTE:        *((UINT64*)destinationPointer) = *((UINT8*) sourcePointer) ; break ;

				// !!! sign extending works on intel, others ???
                // !!! Need to check the shift limiter here too !!!
				case DT_WORD:        destinationValue = (*((INT16*)sourcePointer)) << 16 ;		  // Insure sign extending
									 destinationValue &= (UINT64)U64(0x000000ffffff0000) ;
									 *((UINT64*)destinationPointer) = destinationValue ;		  break ;

				case DT_DOUBLE_WORD: *((UINT64*)destinationPointer) = *((UINT32*)sourcePointer) ; break ;
				case DT_LONG_WORD:   *((UINT64*)destinationPointer) = *((UINT64*)sourcePointer) ; break ;
			}
		break ;
	}
}

// !! redundant functions (data and memory) can be handled with function pointers !!
static void SetDataMemoryValue(void *sourcePointer, unsigned char sourceType, UINT32 destinationAddr)
{
	// mame_printf_debug("%d %x\n", sourceType, destinationAddr) ;

	// I wonder if this is how this should be done???
	switch(sourceType)
	{
		case DT_BYTE:        data_write_word_16le(destinationAddr, (UINT16)( (*((UINT8*) sourcePointer) & 0xff)               ) ) ; break ;
		case DT_WORD:        data_write_word_16le(destinationAddr, (UINT16)( (*((UINT16*)sourcePointer) & 0xffff)             ) ) ; break ;
		case DT_DOUBLE_WORD: data_write_word_16le(destinationAddr, (UINT16)( (*((UINT32*)sourcePointer) & 0x0000ffff)         ) ) ; break ;

		// !!! Is this universal ???
		// !!! Forget not, yon shift-limiter !!!
		case DT_LONG_WORD:   data_write_word_16le(destinationAddr, (UINT16)( ((*((UINT64*)sourcePointer)) & (UINT64)U64(0x00000000ffff0000)) >> 16) ) ; break ;
	}
}

static void SetProgramMemoryValue(void *sourcePointer, unsigned char sourceType, UINT32 destinationAddr)
{
	// I wonder if this is how this should be done???
	switch(sourceType)
	{
		case DT_BYTE:        program_write_word_16le(destinationAddr, (UINT16)( (*((UINT8*) sourcePointer) & 0xff)               ) ) ; break ;
		case DT_WORD:        program_write_word_16le(destinationAddr, (UINT16)( (*((UINT16*)sourcePointer) & 0xffff)             ) ) ; break ;
		case DT_DOUBLE_WORD: program_write_word_16le(destinationAddr, (UINT16)( (*((UINT32*)sourcePointer) & 0x0000ffff)         ) ) ; break ;

		// !!! Is this universal ???
		// !!! Forget not, yon shift-limiter !!!
		case DT_LONG_WORD:   program_write_word_16le(destinationAddr, (UINT16)( ((*((UINT64*)sourcePointer)) & (UINT64)U64(0x00000000ffff0000)) >> 16) ) ; break ;
	}
}

