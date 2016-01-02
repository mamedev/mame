// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff,R. Belmont,Ryan Holtz
/*****************************************************************************
 *
 *   arm7dasm.c
 *   Portable ARM7TDMI Core Emulator - Disassembler
 *
 *   Copyright Steve Ellenoff, all rights reserved.
 *
 *  This work is based on:
 *  #1) 'Atmel Corporation ARM7TDMI (Thumb) Datasheet - January 1999'
 *  #2) Arm 2/3/6 emulator By Bryan McPhail (bmcphail@tendril.co.uk) and Phil Stroffolino (MAME CORE 0.76)
 *
 *****************************************************************************/
/******************************************************************************
 *  Notes:
 *
 *  Because Co-Processor functions are highly specialized to the actual co-proc
 *  implementation being used, I've setup callback handlers to allow for custom
 *  dasm display of the co-proc functions so that the implementation specific
 *  commands/interpretation can be used. If not used, the default handlers which
 *  implement the ARM7TDMI guideline format is used
 ******************************************************************************/

#include "emu.h"
#include "arm7core.h"

static char *WritePadding( char *pBuf, const char *pBuf0 )
{
	pBuf0 += 8;
	while( pBuf<pBuf0 )
	{
		*pBuf++ = ' ';
	}
	return pBuf;
}

static char *DasmCoProc_RT( char *pBuf, UINT32 opcode, const char *pConditionCode, const char *pBuf0)
{
	/* co processor register transfer */
	/* xxxx 1110 oooL nnnn dddd cccc ppp1 mmmm */
	if( opcode&0x00100000 )     //Bit 20 = Load or Store
	{
		pBuf += sprintf( pBuf, "MRC" );
	}
	else
	{
		pBuf += sprintf( pBuf, "MCR" );
	}
	pBuf += sprintf( pBuf, "%s", pConditionCode );
	pBuf = WritePadding( pBuf, pBuf0 );
	pBuf += sprintf( pBuf, "p%d, %d, R%d, c%d, c%d",
					(opcode>>8)&0xf, (opcode>>21)&7, (opcode>>12)&0xf, (opcode>>16)&0xf, opcode&0xf );
	if((opcode>>5)&7) pBuf += sprintf( pBuf, ", %d",(opcode>>5)&7);
	return pBuf;
}

static char *DasmCoProc_DT( char *pBuf, UINT32 opcode, const char *pConditionCode, const char *pBuf0 )
{
	/* co processor data transfer */
	/* xxxx 111P UNWL nnnn dddd pppp oooooooo */
	//todo: test this on valid instructions

	pBuf += sprintf(pBuf, "%s%s",(opcode&0x00100000)?"LDC":"STC",pConditionCode);   //Bit 20 = 1 for Load, 0 for Store
	//Long Operation
	if(opcode & 0x400000)   pBuf += sprintf(pBuf, "L");
	pBuf = WritePadding( pBuf, pBuf0 );

	//P# & CD #
	pBuf += sprintf(pBuf, "p%d, c%d, ",(opcode>>8)&0x0f,(opcode>>12)&0x0f);

	//Base Register (Rn)
	pBuf += sprintf(pBuf, "[R%d%s",(opcode>>16)&0x0f,(opcode&0x1000000)?"":"]");    //If Bit 24 = 1, Pre-increment, otherwise, Post increment so close brace

	//immediate value ( 8 bit value is << 2 according to manual )
	if(opcode & 0xff)   pBuf += sprintf(pBuf, ",%s#$%x",(opcode&0x800000)?"":"-",(opcode & 0xff)<<2);

	//Pre-Inc brace & Write back
	pBuf += sprintf(pBuf, "%s%s",(opcode&0x1000000)?"]":"",(opcode&0x200000)?"{!}":"");
	return pBuf;
}

static char *DasmCoProc_DO( char *pBuf, UINT32 opcode, const char *pConditionCode, const char *pBuf0 )
{
	/* co processor data operation */
	/* xxxx 1110 oooo nnnn dddd cccc ppp0 mmmm */
	pBuf += sprintf( pBuf, "CDP" );
	pBuf += sprintf( pBuf, "%s", pConditionCode );
	pBuf = WritePadding( pBuf, pBuf0 );
	//p#,CPOpc,cd,cn,cm
	pBuf += sprintf( pBuf, "p%d, %d, c%d, c%d, c%d",
		(opcode>>8)&0xf, (opcode>>20)&0xf, (opcode>>12)&0xf, (opcode>>16)&0xf, opcode&0xf );
	if((opcode>>5)&7) pBuf += sprintf( pBuf, ", %d",(opcode>>5)&7);
	return pBuf;
}

static char *WriteImmediateOperand( char *pBuf, UINT32 opcode )
{
	/* rrrrbbbbbbbb */
	UINT32 imm;
	int r;

	imm = opcode&0xff;
	r = ((opcode>>8)&0xf)*2;
	imm = (imm>>r)|(r?(imm<<(32-r)):0);
	pBuf += sprintf( pBuf, ", #$%x", imm );
	return pBuf;
}

static char *WriteDataProcessingOperand( char *pBuf, UINT32 opcode, int printOp0, int printOp1, int printOp2 )
{
	/* ccccctttmmmm */
	static const char *const pRegOp[4] = { "LSL","LSR","ASR","ROR" };

	if (printOp0)
		pBuf += sprintf(pBuf,"R%d, ", (opcode>>12)&0xf);
	if (printOp1)
		pBuf += sprintf(pBuf,"R%d, ", (opcode>>16)&0xf);

	/* Immediate Op2 */
	if( opcode&0x02000000 )
		return WriteImmediateOperand(pBuf-2,opcode);

	/* Register Op2 */
	if (printOp2)
//SJE:  pBuf += sprintf(pBuf,"R%d, ", (opcode>>0)&0xf);
		pBuf += sprintf(pBuf,"R%d ", (opcode>>0)&0xf);

	//SJE: ignore if LSL#0 for register shift
	if( ((opcode&0x2000000) == 0) && (((opcode>>4) & 0xff)==0) )
		return pBuf;

	pBuf += sprintf(pBuf, ",%s ", pRegOp[(opcode>>5)&3] );
	//SJE: pBuf += sprintf(pBuf, "%s ", pRegOp[(opcode>>5)&3] );

	if( opcode&0x10 ) /* Shift amount specified in bottom bits of RS */
	{
		pBuf += sprintf( pBuf, "R%d", (opcode>>8)&0xf );
	}
	else /* Shift amount immediate 5 bit unsigned integer */
	{
		int c=(opcode>>7)&0x1f;
		if( c==0 ) c = 32;
		pBuf += sprintf( pBuf, "#%d", c );
	}
	return pBuf;
}

static char *WriteRegisterOperand1( char *pBuf, UINT32 opcode )
{
	/* ccccctttmmmm */
	static const char *const pRegOp[4] = { "LSL","LSR","ASR","ROR" };

	pBuf += sprintf(
		pBuf,
		", R%d", /* Operand 1 register, Operand 2 register, shift type */
		(opcode>> 0)&0xf);

	//check for LSL 0
	if( (((opcode>>5)&3)==0) && (((opcode>>7)&0xf)==0) )
		return pBuf;
	else
	//Add rotation type
		pBuf += sprintf(pBuf," %s ",pRegOp[(opcode>>5)&3]);

	if( opcode&0x10 ) /* Shift amount specified in bottom bits of RS */
	{
		pBuf += sprintf( pBuf, "R%d", (opcode>>7)&0xf );
	}
	else /* Shift amount immediate 5 bit unsigned integer */
	{
		int c=(opcode>>7)&0x1f;
		if( c==0 ) c = 32;
		pBuf += sprintf( pBuf, "#%d", c );
	}
	return pBuf;
} /* WriteRegisterOperand */


static char *WriteBranchAddress( char *pBuf, UINT32 pc, UINT32 opcode )
{
	opcode &= 0x00ffffff;
	if( opcode&0x00800000 )
	{
		opcode |= 0xff000000; /* sign-extend */
	}
	pc += 8+4*opcode;
	sprintf( pBuf, "$%x", pc );
	return pBuf;
} /* WriteBranchAddress */

static UINT32 arm7_disasm( char *pBuf, UINT32 pc, UINT32 opcode )
{
	const char *pBuf0;

	static const char *const pConditionCodeTable[16] =
	{
		"EQ","NE","CS","CC",
		"MI","PL","VS","VC",
		"HI","LS","GE","LT",
		"GT","LE","","NV"
	};
	static const char *const pOperation[16] =
	{
		"AND","EOR","SUB","RSB",
		"ADD","ADC","SBC","RSC",
		"TST","TEQ","CMP","CMN",
		"ORR","MOV","BIC","MVN"
	};
	const char *pConditionCode;
	UINT32 dasmflags = 0;

	pConditionCode= pConditionCodeTable[opcode>>28];
	pBuf0 = pBuf;

	if( (opcode&0x0ffffff0)==0x012fff10 ) { //bits 27-4 == 000100101111111111110001
		/* Branch and Exchange (BX) */
		pBuf += sprintf( pBuf, "B");
		pBuf += sprintf( pBuf, "%sX", pConditionCode );
		pBuf = WritePadding( pBuf, pBuf0 );
		pBuf += sprintf( pBuf, "R%d",(opcode&0xf));
		if ((opcode & 0x0f) == 14)
			dasmflags = DASMFLAG_STEP_OUT;
	}
		else if ((opcode & 0x0ff000f0) == 0x01600010)   // CLZ - v5
	{
		pBuf += sprintf(pBuf, "CLZ");
		pBuf = WritePadding( pBuf, pBuf0 );
		pBuf += sprintf(pBuf, "R%d, R%d", (opcode>>12)&0xf, opcode&0xf);
	}
		else if ((opcode & 0x0ff000f0) == 0x01000050)   // QADD - v5
	{
		pBuf += sprintf(pBuf, "QADD");
		pBuf = WritePadding( pBuf, pBuf0 );
		pBuf += sprintf(pBuf, "R%d, R%d, R%d", (opcode>>12)&0xf, opcode&0xf, (opcode>>16)&0xf);
	}
		else if ((opcode & 0x0ff000f0) == 0x01400050)   // QDADD - v5
	{
		pBuf += sprintf(pBuf, "QDADD");
		pBuf = WritePadding( pBuf, pBuf0 );
		pBuf += sprintf(pBuf, "R%d, R%d, R%d", (opcode>>12)&0xf, opcode&0xf, (opcode>>16)&0xf);
	}
		else if ((opcode & 0x0ff000f0) == 0x01200050)   // QSUB - v5
	{
		pBuf += sprintf(pBuf, "QSUB");
		pBuf = WritePadding( pBuf, pBuf0 );
		pBuf += sprintf(pBuf, "R%d, R%d, R%d", (opcode>>12)&0xf, opcode&0xf, (opcode>>16)&0xf);
	}
		else if ((opcode & 0x0ff000f0) == 0x01600050)   // QDSUB - v5
	{
		pBuf += sprintf(pBuf, "QDSUB");
		pBuf = WritePadding( pBuf, pBuf0 );
		pBuf += sprintf(pBuf, "R%d, R%d, R%d", (opcode>>12)&0xf, opcode&0xf, (opcode>>16)&0xf);
	}
		else if ((opcode & 0x0ff00090) == 0x01000080)   // SMLAxy - v5
	{
		pBuf += sprintf(pBuf, "SMLA%c%c", (opcode&0x20) ? 'T' : 'B', (opcode&0x40) ? 'T' : 'B');
		pBuf = WritePadding( pBuf, pBuf0 );
		pBuf += sprintf(pBuf, "R%d, R%d, R%d, R%d", (opcode>>16)&0xf, (opcode>>12)&0xf, opcode&0xf, (opcode>>8)&0xf);
	}
		else if ((opcode & 0x0ff00090) == 0x01400080)   // SMLALxy - v5
	{
		pBuf += sprintf(pBuf, "SMLAL%c%c", (opcode&0x20) ? 'T' : 'B', (opcode&0x40) ? 'T' : 'B');
		pBuf = WritePadding( pBuf, pBuf0 );
		pBuf += sprintf(pBuf, "R%d, R%d, R%d, R%d", (opcode>>16)&0xf, (opcode>>12)&0xf, opcode&0xf, (opcode>>8)&0xf);
	}
		else if ((opcode & 0x0ff00090) == 0x01600080)   // SMULxy - v5
	{
		pBuf += sprintf(pBuf, "SMUL%c%c", (opcode&0x20) ? 'T' : 'B', (opcode&0x40) ? 'T' : 'B');
		pBuf = WritePadding( pBuf, pBuf0 );
		pBuf += sprintf(pBuf, "R%d, R%d, R%d", (opcode>>16)&0xf, opcode&0xf, (opcode>>12)&0xf);
	}
		else if ((opcode & 0x0ff000b0) == 0x012000a0)   // SMULWy - v5
	{
		pBuf += sprintf(pBuf, "SMULW%c", (opcode&0x40) ? 'T' : 'B');
		pBuf = WritePadding( pBuf, pBuf0 );
		pBuf += sprintf(pBuf, "R%d, R%d, R%d", (opcode>>16)&0xf, opcode&0xf, (opcode>>8)&0xf);
	}
		else if ((opcode & 0x0ff000b0) == 0x01200080)   // SMLAWy - v5
	{
		pBuf += sprintf(pBuf, "SMLAW%c", (opcode&0x40) ? 'T' : 'B');
		pBuf = WritePadding( pBuf, pBuf0 );
		pBuf += sprintf(pBuf, "R%d, R%d, R%d, R%d", (opcode>>16)&0xf, opcode&0xf, (opcode>>8)&0xf, (opcode>>12)&0xf);
	}
	else if( (opcode&0x0e000000)==0 && (opcode&0x80) && (opcode&0x10) ) //bits 27-25 == 000, bit 7=1, bit 4=1
	{
		/* multiply or swap or half word data transfer */
		if(opcode&0x60)
		{   //bits = 6-5 != 00
			/* half word data transfer */
			if (((opcode & 0x60) == 0x40) && !(opcode & 0x100000))  // bit 20 = 0, bits 5&6 = 10 is ARMv5 LDRD
			{
				pBuf += sprintf(pBuf, "LDRD%s", pConditionCode);
			}
			else if (((opcode & 0x60) == 0x60) && !(opcode & 0x100000)) // bit 20 = 0, bits 5&6 = 11 is ARMv5 STRD
			{
				pBuf += sprintf(pBuf, "STRD%s", pConditionCode);
			}
			else
			{
				pBuf += sprintf(pBuf, "%s%s",(opcode&0x00100000)?"LDR":"STR",pConditionCode);   //Bit 20 = 1 for Load, 0 for Store

				//Signed? (if not, always unsigned half word)
				if(opcode&0x40)
				{
					pBuf += sprintf(pBuf, "%s",(opcode&0x20)?"SH":"SB");    //Bit 5 = 1 for Half Word, 0 for Byte
				}
				else
				{
					pBuf += sprintf(pBuf, "H");
				}
			}

			pBuf = WritePadding( pBuf, pBuf0 );

			//Dest Register
			pBuf += sprintf(pBuf, "R%d, ",(opcode>>12)&0x0f);
			//Base Register
			pBuf += sprintf(pBuf, "[R%d%s",(opcode>>16)&0x0f,(opcode&0x1000000)?"":"]");    //If Bit 24 = 1, Pre-increment, otherwise, Post increment so close brace

			//Immediate or Register Offset?
			if(opcode&0x400000) {           //Bit 22 - 1 = immediate, 0 = register
				//immediate         ( imm. value in high nibble (bits 8-11) and lo nibble (bit 0-3) )
				pBuf += sprintf(pBuf, ",%s#$%x",(opcode&0x800000)?"":"-",( (((opcode>>8)&0x0f)<<4) | (opcode&0x0f)));
			}
			else {
				//register
				pBuf += sprintf(pBuf, ",%sR%d",(opcode&0x800000)?"":"-",(opcode & 0x0f));
			}

			//Pre-Inc brace & Write back
			pBuf += sprintf(pBuf, "%s%s",(opcode&0x1000000)?"]":"",(opcode&0x200000)?"{!}":"");
		}
		else {
			if(opcode&0x01000000) {     //bit 24 = 1
			/* swap */
			//todo: Test on valid instructions
				/* xxxx 0001 0B00 nnnn dddd 0000 1001 mmmm */
				pBuf += sprintf( pBuf, "SWP" );
				pBuf += sprintf( pBuf, "%s%s", pConditionCode, (opcode & 0x400000)?"B":"" );    //Bit 22 = Byte/Word selection
				//Rd, Rm, [Rn]
				pBuf += sprintf( pBuf, "R%d, R%d, [R%d]",
								(opcode>>12)&0xf, opcode&0xf, (opcode>>16)&0xf );
			}
			else {
				/* multiply or multiply long */

				if( opcode&0x800000 )   //Bit 23 = 1 for Multiply Long
				{
					/* Multiply Long */
					/* xxxx0001 UAShhhhllllnnnn1001mmmm */

					/* Signed? */
					if( opcode&0x00400000 )
						pBuf += sprintf( pBuf, "S" );
					else
						pBuf += sprintf( pBuf, "U" );

					/* Multiply & Accumulate? */
					if( opcode&0x00200000 )
					{
						pBuf += sprintf( pBuf, "MLAL" );
					}
					else
					{
						pBuf += sprintf( pBuf, "MULL" );
					}
					pBuf += sprintf( pBuf, "%s", pConditionCode );

					/* Set Status Flags */
					if( opcode&0x00100000 )
					{
						*pBuf++ = 'S';
					}
					pBuf = WritePadding( pBuf, pBuf0 );

					//Format is RLo,RHi,Rm,Rs
					pBuf += sprintf( pBuf,
						"R%d, R%d, R%d, R%d",
						(opcode>>12)&0xf,
						(opcode>>16)&0xf,
						(opcode&0xf),
						(opcode>>8)&0xf);
				}
				else
				{
					/* Multiply */
					/* xxxx0000 00ASdddd nnnnssss 1001mmmm */

					/* Multiply & Accumulate? */
					if( opcode&0x00200000 )
					{
						pBuf += sprintf( pBuf, "MLA" );
					}
					/* Multiply */
					else
					{
						pBuf += sprintf( pBuf, "MUL" );
					}
					pBuf += sprintf( pBuf, "%s", pConditionCode );
					if( opcode&0x00100000 )
					{
						*pBuf++ = 'S';
					}
					pBuf = WritePadding( pBuf, pBuf0 );

					pBuf += sprintf( pBuf,
						"R%d, R%d, R%d",
						(opcode>>16)&0xf,
						(opcode&0xf),
						(opcode>>8)&0xf );

					if( opcode&0x00200000 )
					{
						pBuf += sprintf( pBuf, ", R%d", (opcode>>12)&0xf );
					}
				}
			}
		}
	}
	else if( (opcode&0x0c000000)==0 )       //bits 27-26 == 00 - This check can only exist properly after Multiplication check above
	{
		/* Data Processing OR PSR Transfer */

		//SJE: check for MRS & MSR ( S bit must be clear, and bit 24,23 = 10 )
		if( ((opcode&0x00100000)==0) && ((opcode&0x01800000)==0x01000000) ) {
			char strpsr[6];
			sprintf(strpsr, "%s",(opcode&0x400000)?"SPSR":"CPSR");

			//MSR ( bit 21 set )
			if( (opcode&0x00200000) ) {
				pBuf += sprintf(pBuf, "MSR%s",pConditionCode );
				//Flag Bits Only? (Bit 16 Clear)
				if( (opcode&0x10000)==0)    pBuf += sprintf(pBuf, "F");
				pBuf = WritePadding( pBuf, pBuf0 );
				pBuf += sprintf(pBuf, "%s,",strpsr);
				WriteDataProcessingOperand(pBuf, opcode, (opcode&0x02000000)?1:0, 0, 1);
			}
			//MRS ( bit 21 clear )
			else {
				pBuf += sprintf(pBuf, "MRS%s",pConditionCode );
				pBuf = WritePadding( pBuf, pBuf0 );
				pBuf += sprintf(pBuf, "R%d,",(opcode>>12)&0x0f);
				pBuf += sprintf(pBuf, "%s",strpsr);
			}
		}
		else {
			/* Data Processing */
			/* xxxx001a aaaSnnnn ddddrrrr bbbbbbbb */
			/* xxxx000a aaaSnnnn ddddcccc ctttmmmm */
			int op=(opcode>>21)&0xf;
			pBuf += sprintf(
				pBuf, "%s%s",
				pOperation[op],
				pConditionCode );

			//SJE: corrected S-Bit bug here
			//if( (opcode&0x01000000) )
			if( (opcode&0x0100000) )
			{
				*pBuf++ = 'S';
			}

			pBuf = WritePadding( pBuf, pBuf0 );

			switch (op) {
			case 0x00:
			case 0x01:
			case 0x02:
			case 0x03:
			case 0x04:
			case 0x05:
			case 0x06:
			case 0x07:
			case 0x0c:
			case 0x0e:
				WriteDataProcessingOperand(pBuf, opcode, 1, 1, 1);
				break;
			case 0x08:
			case 0x09:
			case 0x0a:
			case 0x0b:
				WriteDataProcessingOperand(pBuf, opcode, 0, 1, 1);
				break;
			case 0x0d:
				/* look for mov pc,lr */
				if (((opcode >> 12) & 0x0f) == 15 && ((opcode >> 0) & 0x0f) == 14 && (opcode & 0x02000000) == 0)
					dasmflags = DASMFLAG_STEP_OUT;
			case 0x0f:
				WriteDataProcessingOperand(pBuf, opcode, 1, 0, 1);
				break;
			}
		}
	}
	else if( (opcode&0x0c000000)==0x04000000 )      //bits 27-26 == 01
	{
		UINT32 rn;
		UINT32 rnv = 0;

		/* Data Transfer */

		/* xxxx010P UBWLnnnn ddddoooo oooooooo  Immediate form */
		/* xxxx011P UBWLnnnn ddddcccc ctt0mmmm  Register form */
		if( opcode&0x00100000 )
		{
			pBuf += sprintf( pBuf, "LDR" );
		}
		else
		{
			pBuf += sprintf( pBuf, "STR" );
		}
		pBuf += sprintf( pBuf, "%s", pConditionCode );

		if( opcode&0x00400000 )
		{
			pBuf += sprintf( pBuf, "B" );
		}

		if( opcode&0x00200000 )
		{
			/* writeback addr */
			if( opcode&0x01000000 )
			{
				/* pre-indexed addressing */
				pBuf += sprintf( pBuf, "!" );
			}
			else
			{
				/* post-indexed addressing */
				pBuf += sprintf( pBuf, "T" );
			}
		}

		pBuf = WritePadding( pBuf, pBuf0 );
		pBuf += sprintf( pBuf, "R%d, [R%d",
			(opcode>>12)&0xf, (opcode>>16)&0xf );

		//grab value of pc if used as base register
		rn = (opcode>>16)&0xf;
		if(rn==15) rnv = pc+8;

		if( opcode&0x02000000 )
		{
			/* register form */
			pBuf += sprintf( pBuf, "%s",(opcode&0x01000000)?"":"]" );
			pBuf = WriteRegisterOperand1( pBuf, opcode );
			pBuf += sprintf( pBuf, "%s",(opcode&0x01000000)?"]":"" );
		}
		else
		{
			/* immediate form */
			pBuf += sprintf( pBuf, "%s",(opcode&0x01000000)?"":"]" );
			//hide zero offsets
			if(opcode&0xfff) {
				if( opcode&0x00800000 )
				{
					pBuf += sprintf( pBuf, ", #$%x", opcode&0xfff );
					rnv += (rnv)?opcode&0xfff:0;
				}
				else
				{
					pBuf += sprintf( pBuf, ", -#$%x", opcode&0xfff );
					rnv -= (rnv)?opcode&0xfff:0;
				}
			}
			pBuf += sprintf( pBuf, "%s",(opcode&0x01000000)?"]":"" );
			//show where the read will occur if we found a value
			if(rnv) pBuf += sprintf( pBuf, " (%x)",rnv);
		}
	}
	else if( (opcode&0x0e000000) == 0x08000000 )        //bits 27-25 == 100
	{
		/* xxxx100P USWLnnnn llllllll llllllll */
		/* Block Data Transfer */

		if( opcode&0x00100000 )
		{
			pBuf += sprintf( pBuf, "LDM" );
		}
		else
		{
			pBuf += sprintf( pBuf, "STM" );
		}
		pBuf += sprintf( pBuf, "%s", pConditionCode );

		if( opcode&0x01000000 )
		{
			pBuf += sprintf( pBuf, "P" );
		}
		if( opcode&0x00800000 )
		{
			pBuf += sprintf( pBuf, "U" );
		}
		if( opcode&0x00400000 )
		{
			pBuf += sprintf( pBuf, "^" );
		}
		if( opcode&0x00200000 )
		{
			pBuf += sprintf( pBuf, "W" );
		}

		pBuf = WritePadding( pBuf, pBuf0 );
		pBuf += sprintf( pBuf, "[R%d], {",(opcode>>16)&0xf);

		{
			int j=0,last=0,found=0;
			for (j=0; j<16; j++) {
				if (opcode&(1<<j) && found==0) {
					found=1;
					last=j;
				}
				else if ((opcode&(1<<j))==0 && found) {
					if (last==j-1)
						pBuf += sprintf( pBuf, " R%d,",last);
					else
						pBuf += sprintf( pBuf, " R%d-R%d,",last,j-1);
					found=0;
				}
			}
			if (found && last==15)
				pBuf += sprintf( pBuf, " R15,");
			else if (found)
				pBuf += sprintf( pBuf, " R%d-R%d,",last,15);
		}

		pBuf--;
		pBuf += sprintf( pBuf, " }");
	}
	else if( (opcode&0x0e000000)==0x0a000000 )      //bits 27-25 == 101
	{
		/* branch instruction */
		/* xxxx101L oooooooo oooooooo oooooooo */
		if( opcode&0x01000000 )
		{
			pBuf += sprintf( pBuf, "BL" );
			dasmflags = DASMFLAG_STEP_OVER;
		}
		else
		{
			pBuf += sprintf( pBuf, "B" );
		}

		pBuf += sprintf( pBuf, "%s", pConditionCode );

		pBuf = WritePadding( pBuf, pBuf0 );

		pBuf = WriteBranchAddress( pBuf, pc, opcode );
	}
	else if( (opcode&0x0e000000)==0x0c000000 )      //bits 27-25 == 110
	{
		/* co processor data transfer */
		DasmCoProc_DT(pBuf,opcode,(char*)pConditionCode,(char*)pBuf0);
	}
	else if( (opcode&0x0f000000)==0x0e000000 )      //bits 27-24 == 1110
	{
		/* co processor data operation or register transfer */

		//Register Transfer
		if(opcode&0x10)
		{
			DasmCoProc_RT(pBuf,opcode,pConditionCode,pBuf0);
		}
		//Data Op
		else
		{
			DasmCoProc_DO(pBuf,opcode,pConditionCode,pBuf0);
		}
	}
	else if( (opcode&0x0f000000) == 0x0f000000 )    //bits 27-24 == 1111
	{
		/* Software Interrupt */
		pBuf += sprintf( pBuf, "SWI%s $%x",
			pConditionCode,
			opcode&0x00ffffff );
		dasmflags = DASMFLAG_STEP_OVER;
	}
	else
	{
		pBuf += sprintf( pBuf, "Undefined" );
	}
	return dasmflags | DASMFLAG_SUPPORTED;
}

static UINT32 thumb_disasm( char *pBuf, UINT32 pc, UINT16 opcode )
{
	const char *pBuf0;
	UINT32 dasmflags = 0;

//      UINT32 readword;
		UINT32 addr;
		UINT32 rm, rn, rs, rd, op2, imm;//, rrs;
		INT32 offs;

	pBuf0 = pBuf;
	pBuf = WritePadding( pBuf, pBuf0 );


		switch( ( opcode & THUMB_INSN_TYPE ) >> THUMB_INSN_TYPE_SHIFT )
		{
			case 0x0: /* Logical shifting */
				if( opcode & THUMB_SHIFT_R ) /* Shift right */
				{
					rs = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
					rd = ( opcode & THUMB_ADDSUB_RD ) >> THUMB_ADDSUB_RD_SHIFT;
					offs = ( opcode & THUMB_SHIFT_AMT ) >> THUMB_SHIFT_AMT_SHIFT;
					pBuf += sprintf( pBuf, "LSR R%d, R%d, %d", rd, rs, offs);
				}
				else /* Shift left */
				{
					rs = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
					rd = ( opcode & THUMB_ADDSUB_RD ) >> THUMB_ADDSUB_RD_SHIFT;
					offs = ( opcode & THUMB_SHIFT_AMT ) >> THUMB_SHIFT_AMT_SHIFT;
					pBuf += sprintf( pBuf, "LSL R%d, R%d, %d", rd, rs, offs);
				}
				break;
			case 0x1: /* Arithmetic */
				if( opcode & THUMB_INSN_ADDSUB )
				{
					switch( ( opcode & THUMB_ADDSUB_TYPE ) >> THUMB_ADDSUB_TYPE_SHIFT )
					{
						case 0x0: /* ADD Rd, Rs, Rn */
							rn = ( opcode & THUMB_ADDSUB_RNIMM ) >> THUMB_ADDSUB_RNIMM_SHIFT;
							rs = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
							rd = ( opcode & THUMB_ADDSUB_RD ) >> THUMB_ADDSUB_RD_SHIFT;
							pBuf += sprintf( pBuf, "ADD R%d, R%d, R%d", rd, rs, rn );
							break;
						case 0x1: /* SUB Rd, Rs, Rn */
							rn = ( opcode & THUMB_ADDSUB_RNIMM ) >> THUMB_ADDSUB_RNIMM_SHIFT;
							rs = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
							rd = ( opcode & THUMB_ADDSUB_RD ) >> THUMB_ADDSUB_RD_SHIFT;
							pBuf += sprintf( pBuf, "SUB R%d, R%d, R%d", rd, rs, rn );
							break;
						case 0x2: /* ADD Rd, Rs, #imm */
							imm = ( opcode & THUMB_ADDSUB_RNIMM ) >> THUMB_ADDSUB_RNIMM_SHIFT;
							rs = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
							rd = ( opcode & THUMB_ADDSUB_RD ) >> THUMB_ADDSUB_RD_SHIFT;
							pBuf += sprintf( pBuf, "ADD R%d, R%d, #%d", rd, rs, imm );
							break;
						case 0x3: /* SUB Rd, Rs, #imm */
							imm = ( opcode & THUMB_ADDSUB_RNIMM ) >> THUMB_ADDSUB_RNIMM_SHIFT;
							rs = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
							rd = ( opcode & THUMB_ADDSUB_RD ) >> THUMB_ADDSUB_RD_SHIFT;
							pBuf += sprintf( pBuf, "SUB R%d, R%d, #%d", rd, rs, imm );
							break;
						default:
							sprintf( pBuf, "INVALID %04x", opcode);
							break;
					}
				}
				else
				{
					rs = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
					rd = ( opcode & THUMB_ADDSUB_RD ) >> THUMB_ADDSUB_RD_SHIFT;
					offs = ( opcode & THUMB_SHIFT_AMT ) >> THUMB_SHIFT_AMT_SHIFT;
					pBuf += sprintf( pBuf, "ASR R%d, R%d, %d", rd, rs, offs);
				}
				break;
			case 0x2: /* CMP / MOV */
				if( opcode & THUMB_INSN_CMP )
				{
					rn = ( opcode & THUMB_INSN_IMM_RD ) >> THUMB_INSN_IMM_RD_SHIFT;
					op2 = ( opcode & THUMB_INSN_IMM );
					pBuf += sprintf( pBuf, "CMP R%d, %02x", rn, op2 );
				}
				else
				{
					rd = ( opcode & THUMB_INSN_IMM_RD ) >> THUMB_INSN_IMM_RD_SHIFT;
					op2 = ( opcode & THUMB_INSN_IMM );
					pBuf += sprintf( pBuf, "MOV R%d, %02x", rd, op2 );
				}
				break;
			case 0x3: /* ADD/SUB immediate */
				if( opcode & THUMB_INSN_SUB ) /* SUB Rd, #Offset8 */
				{
					rn = ( opcode & THUMB_INSN_IMM_RD ) >> THUMB_INSN_IMM_RD_SHIFT;
					op2 = ( opcode & THUMB_INSN_IMM );
					pBuf += sprintf( pBuf, "SUB R%d, %02x", rn, op2 ); // fixed, rd -> rn
				}
				else /* ADD Rd, #Offset8 */
				{
					rn = ( opcode & THUMB_INSN_IMM_RD ) >> THUMB_INSN_IMM_RD_SHIFT;
					op2 = opcode & THUMB_INSN_IMM;
					pBuf += sprintf( pBuf, "ADD R%d, %02x", rn, op2 );
				}
				break;
			case 0x4: /* Rd & Rm instructions */
				switch( ( opcode & THUMB_GROUP4_TYPE ) >> THUMB_GROUP4_TYPE_SHIFT )
				{
					case 0x0:
						switch( ( opcode & THUMB_ALUOP_TYPE ) >> THUMB_ALUOP_TYPE_SHIFT )
						{
							case 0x0: /* AND Rd, Rs */
								rs = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
								rd = ( opcode & THUMB_ADDSUB_RD ) >> THUMB_ADDSUB_RD_SHIFT;
								pBuf += sprintf( pBuf, "AND R%d, R%d", rd, rs );
								break;
							case 0x1: /* EOR Rd, Rs */
								rs = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
								rd = ( opcode & THUMB_ADDSUB_RD ) >> THUMB_ADDSUB_RD_SHIFT;
								pBuf += sprintf( pBuf, "EOR R%d, R%d", rd, rs );
								break;
							case 0x2: /* LSL Rd, Rs */
								rs = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
								rd = ( opcode & THUMB_ADDSUB_RD ) >> THUMB_ADDSUB_RD_SHIFT;
								pBuf += sprintf( pBuf, "LSL R%d, R%d", rd, rs );
								break;
							case 0x3: /* LSR Rd, Rs */
								rs = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
								rd = ( opcode & THUMB_ADDSUB_RD ) >> THUMB_ADDSUB_RD_SHIFT;
								pBuf += sprintf( pBuf, "LSR R%d, R%d", rd, rs );
								break;
							case 0x4: /* ASR Rd, Rs */
								rs = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
								rd = ( opcode & THUMB_ADDSUB_RD ) >> THUMB_ADDSUB_RD_SHIFT;
								pBuf += sprintf( pBuf, "ASR R%d, R%d", rd, rs );
								break;
							case 0x5: /* ADC Rd, Rs */
								rs = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
								rd = ( opcode & THUMB_ADDSUB_RD ) >> THUMB_ADDSUB_RD_SHIFT;
								pBuf += sprintf( pBuf, "ADC R%d, R%d", rd, rs );
								break;
							case 0x6: /* SBC Rd, Rs */
								rs = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
								rd = ( opcode & THUMB_ADDSUB_RD ) >> THUMB_ADDSUB_RD_SHIFT;
								pBuf += sprintf( pBuf, "SBC R%d, R%d", rd, rs );
								break;
							case 0x7: /* ROR Rd, Rs */
								rs = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
								rd = ( opcode & THUMB_ADDSUB_RD ) >> THUMB_ADDSUB_RD_SHIFT;
								pBuf += sprintf( pBuf, "ROR R%d, R%d", rd, rs );
								break;
							case 0x8: /* TST Rd, Rs */
								rs = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
								rd = ( opcode & THUMB_ADDSUB_RD ) >> THUMB_ADDSUB_RD_SHIFT;
								pBuf += sprintf( pBuf, "TST R%d, R%d", rd, rs );
								break;
							case 0x9: /* NEG Rd, Rs */
								rs = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
								rd = ( opcode & THUMB_ADDSUB_RD ) >> THUMB_ADDSUB_RD_SHIFT;
								pBuf += sprintf( pBuf, "NEG R%d, R%d", rd, rs );
								break;
							case 0xa: /* CMP Rd, Rs */
								rs = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
								rd = ( opcode & THUMB_ADDSUB_RD ) >> THUMB_ADDSUB_RD_SHIFT;
								pBuf += sprintf( pBuf, "CMP R%d, R%d", rd, rs );
								break;
							case 0xb: /* CMN Rd, Rs - check flags, add dasm */
								rs = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
								rd = ( opcode & THUMB_ADDSUB_RD ) >> THUMB_ADDSUB_RD_SHIFT;
								pBuf += sprintf( pBuf, "CMN R%d, R%d", rd, rs );
								break;
							case 0xc: /* ORR Rd, Rs */
								rs = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
								rd = ( opcode & THUMB_ADDSUB_RD ) >> THUMB_ADDSUB_RD_SHIFT;
								pBuf += sprintf( pBuf, "ORR R%d, R%d", rd, rs );
								break;
							case 0xd: /* MUL Rd, Rs */
								rs = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
								rd = ( opcode & THUMB_ADDSUB_RD ) >> THUMB_ADDSUB_RD_SHIFT;
								pBuf += sprintf( pBuf, "MUL R%d, R%d", rd, rs );
								break;
							case 0xe: /* MUL Rd, Rs */
								rs = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
								rd = ( opcode & THUMB_ADDSUB_RD ) >> THUMB_ADDSUB_RD_SHIFT;
								pBuf += sprintf( pBuf, "BIC R%d, R%d", rd, rs );
								break;
							case 0xf: /* MVN Rd, Rs */
								rs = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
								rd = ( opcode & THUMB_ADDSUB_RD ) >> THUMB_ADDSUB_RD_SHIFT;
								pBuf += sprintf( pBuf, "MVN R%d, R%d", rd, rs );
								break;
							default:
								sprintf( pBuf, "INVALID %04x", opcode);
								break;
						}
						break;
					case 0x1:
						switch( ( opcode & THUMB_HIREG_OP ) >> THUMB_HIREG_OP_SHIFT )
						{
							case 0x0: /* ADD Rd, Rs */
								rs = ( opcode & THUMB_HIREG_RS ) >> THUMB_HIREG_RS_SHIFT;
								rd = opcode & THUMB_HIREG_RD;
								switch( ( opcode & THUMB_HIREG_H ) >> THUMB_HIREG_H_SHIFT )
								{
									case 0x1: /* ADD Rd, HRs */
										pBuf += sprintf( pBuf, "ADD R%d, R%d", rd, rs + 8 );
										break;
									case 0x2: /* ADD HRd, Rs */
										pBuf += sprintf( pBuf, "ADD R%d, R%d", rd + 8, rs );
										break;
									case 0x3: /* ADD HRd, HRs */
										pBuf += sprintf( pBuf, "ADD R%d, R%d", rd + 8, rs + 8 );
										break;
									default:
										sprintf( pBuf, "INVALID %04x", opcode);
										break;
								}
								break;
							case 0x1: /* CMP */
								switch( ( opcode & THUMB_HIREG_H ) >> THUMB_HIREG_H_SHIFT )
								{
									case 0x0: /* CMP Rd, Rs */
										rs = ( opcode & THUMB_HIREG_RS ) >> THUMB_HIREG_RS_SHIFT;
										rd = opcode & THUMB_HIREG_RD;
										pBuf += sprintf( pBuf, "CMP R%d, R%d", rd, rs );
										break;
									case 0x1: /* CMP Rd, HRs */
										rs = ( opcode & THUMB_HIREG_RS ) >> THUMB_HIREG_RS_SHIFT;
										rd = opcode & THUMB_HIREG_RD;
										pBuf += sprintf( pBuf, "CMP R%d, R%d", rd, rs + 8 );
										break;
									case 0x2: /* CMP Hd, Rs */
										rs = ( opcode & THUMB_HIREG_RS ) >> THUMB_HIREG_RS_SHIFT;
										rd = opcode & THUMB_HIREG_RD;
										pBuf += sprintf( pBuf, "CMP R%d, R%d", rd + 8, rs );
										break;
									case 0x3: /* CMP Hd, Hs */
										rs = ( opcode & THUMB_HIREG_RS ) >> THUMB_HIREG_RS_SHIFT;
										rd = opcode & THUMB_HIREG_RD;
										pBuf += sprintf( pBuf, "CMP R%d, R%d", rd + 8, rs + 8 );
										break;
									default:
										sprintf( pBuf, "INVALID %04x", opcode);
										break;
								}
								break;
							case 0x2: /* MOV */
								switch( ( opcode & THUMB_HIREG_H ) >> THUMB_HIREG_H_SHIFT )
								{
									case 0x0:
										rs = ( opcode & THUMB_HIREG_RS ) >> THUMB_HIREG_RS_SHIFT;
										rd = opcode & THUMB_HIREG_RD;
										pBuf += sprintf( pBuf, "MOV R%d, R%d", rd, rs );
										break;
									case 0x1:
										rs = ( opcode & THUMB_HIREG_RS ) >> THUMB_HIREG_RS_SHIFT;
										rd = opcode & THUMB_HIREG_RD;
										pBuf += sprintf( pBuf, "MOV R%d, R%d", rd, rs + 8 );
										break;
									case 0x2:
										rs = ( opcode & THUMB_HIREG_RS ) >> THUMB_HIREG_RS_SHIFT;
										rd = opcode & THUMB_HIREG_RD;
										pBuf += sprintf( pBuf, "MOV R%d, R%d", rd + 8, rs );
										break;
									case 0x3:
										rs = ( opcode & THUMB_HIREG_RS ) >> THUMB_HIREG_RS_SHIFT;
										rd = opcode & THUMB_HIREG_RD;
										pBuf += sprintf( pBuf, "MOV R%d, R%d", rd + 8, rs + 8 );
										break;
									default:
										sprintf( pBuf, "INVALID %04x", opcode);
										break;
								}
								break;
							case 0x3:
								switch( ( opcode & THUMB_HIREG_H ) >> THUMB_HIREG_H_SHIFT )
								{
									case 0x0:
										rd = ( opcode & THUMB_HIREG_RS ) >> THUMB_HIREG_RS_SHIFT;
										pBuf += sprintf( pBuf, "BX R%d", rd );
										break;
									case 0x1:
										rd = ( ( opcode & THUMB_HIREG_RS ) >> THUMB_HIREG_RS_SHIFT ) + 8;
										pBuf += sprintf( pBuf, "BX R%d", rd );
										if (rd == 14)
											dasmflags = DASMFLAG_STEP_OUT;
										break;
									case 0x2:
										rd = ( opcode & THUMB_HIREG_RS ) >> THUMB_HIREG_RS_SHIFT;
										pBuf += sprintf( pBuf, "BLX R%d", rd );
										break;
									default:
										sprintf( pBuf, "INVALID %04x", opcode);
										break;
								}
								break;
							default:
								sprintf( pBuf, "INVALID %04x", opcode);
								break;
						}
						break;
					case 0x2:
					case 0x3:
						rd = ( opcode & THUMB_INSN_IMM_RD ) >> THUMB_INSN_IMM_RD_SHIFT;
						addr = ( opcode & THUMB_INSN_IMM ) << 2;
						pBuf += sprintf( pBuf, "LDR R%d, [PC, #%03x]", rd, addr );
						break;
					default:
						sprintf( pBuf, "INVALID %04x", opcode);
						break;
				}
				break;
			case 0x5: /* LDR* STR* */
				switch( ( opcode & THUMB_GROUP5_TYPE ) >> THUMB_GROUP5_TYPE_SHIFT )
				{
					case 0x0: /* STR Rd, [Rn, Rm] */
						rm = ( opcode & THUMB_GROUP5_RM ) >> THUMB_GROUP5_RM_SHIFT;
						rn = ( opcode & THUMB_GROUP5_RN ) >> THUMB_GROUP5_RN_SHIFT;
						rd = ( opcode & THUMB_GROUP5_RD ) >> THUMB_GROUP5_RD_SHIFT;
						pBuf += sprintf( pBuf, "STR R%d, [R%d, R%d]", rd, rn, rm );
						break;
					case 0x1: /* STRH Rd, [Rn, Rm] */
						rm = ( opcode & THUMB_GROUP5_RM ) >> THUMB_GROUP5_RM_SHIFT;
						rn = ( opcode & THUMB_GROUP5_RN ) >> THUMB_GROUP5_RN_SHIFT;
						rd = ( opcode & THUMB_GROUP5_RD ) >> THUMB_GROUP5_RD_SHIFT;
						pBuf += sprintf( pBuf, "STRH R%d, [R%d, R%d]", rd, rn, rm );
						break;
					case 0x2: /* STRB Rd, [Rn, Rm] */ /* check */
						rm = ( opcode & THUMB_GROUP5_RM ) >> THUMB_GROUP5_RM_SHIFT;
						rn = ( opcode & THUMB_GROUP5_RN ) >> THUMB_GROUP5_RN_SHIFT;
						rd = ( opcode & THUMB_GROUP5_RD ) >> THUMB_GROUP5_RD_SHIFT;
						pBuf += sprintf( pBuf, "STRB R%d, [R%d, R%d]", rd, rn, rm );
						break;
					case 0x3: /* LDRSB Rd, [Rn, Rm] */
						rm = ( opcode & THUMB_GROUP5_RM ) >> THUMB_GROUP5_RM_SHIFT;
						rn = ( opcode & THUMB_GROUP5_RN ) >> THUMB_GROUP5_RN_SHIFT;
						rd = ( opcode & THUMB_GROUP5_RD ) >> THUMB_GROUP5_RD_SHIFT;
						pBuf += sprintf( pBuf, "LDRSB R%d, [R%d, R%d]", rd, rn, rm );
						break;
					case 0x4: /* LDR Rd, [Rn, Rm] */ /* check */
						rm = ( opcode & THUMB_GROUP5_RM ) >> THUMB_GROUP5_RM_SHIFT;
						rn = ( opcode & THUMB_GROUP5_RN ) >> THUMB_GROUP5_RN_SHIFT;
						rd = ( opcode & THUMB_GROUP5_RD ) >> THUMB_GROUP5_RD_SHIFT;
						pBuf += sprintf( pBuf, "LDR R%d, [R%d, R%d]", rd, rn, rm );
						break;
					case 0x5: /* LDRH Rd, [Rn, Rm] */
						rm = ( opcode & THUMB_GROUP5_RM ) >> THUMB_GROUP5_RM_SHIFT;
						rn = ( opcode & THUMB_GROUP5_RN ) >> THUMB_GROUP5_RN_SHIFT;
						rd = ( opcode & THUMB_GROUP5_RD ) >> THUMB_GROUP5_RD_SHIFT;
						pBuf += sprintf( pBuf, "LDRH R%d, [R%d, R%d]", rd, rn, rm );
						break;

					case 0x6: /* LDRB Rd, [Rn, Rm] */
						rm = ( opcode & THUMB_GROUP5_RM ) >> THUMB_GROUP5_RM_SHIFT;
						rn = ( opcode & THUMB_GROUP5_RN ) >> THUMB_GROUP5_RN_SHIFT;
						rd = ( opcode & THUMB_GROUP5_RD ) >> THUMB_GROUP5_RD_SHIFT;
						pBuf += sprintf( pBuf, "LDRB R%d, [R%d, R%d]", rd, rn, rm );
						break;
					case 0x7: /* LDSH Rd, [Rn, Rm] */
						rm = ( opcode & THUMB_GROUP5_RM ) >> THUMB_GROUP5_RM_SHIFT;
						rn = ( opcode & THUMB_GROUP5_RN ) >> THUMB_GROUP5_RN_SHIFT;
						rd = ( opcode & THUMB_GROUP5_RD ) >> THUMB_GROUP5_RD_SHIFT;
						pBuf += sprintf( pBuf, "LDSH R%d, [R%d, R%d]", rd, rn, rm );
						break;
					default:
						sprintf( pBuf, "INVALID %04x", opcode);
						break;
				}
				break;
			case 0x6: /* Word Store w/ Immediate Offset */
				if( opcode & THUMB_LSOP_L ) /* Load */
				{
					rn = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
					rd = opcode & THUMB_ADDSUB_RD;
					offs = ( ( opcode & THUMB_LSOP_OFFS ) >> THUMB_LSOP_OFFS_SHIFT ) << 2;
					pBuf += sprintf( pBuf, "LDR R%d [R%d + #%02x]", rd, rn, offs );
				}
				else /* Store */
				{
					rn = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
					rd = opcode & THUMB_ADDSUB_RD;
					offs = ( ( opcode & THUMB_LSOP_OFFS ) >> THUMB_LSOP_OFFS_SHIFT ) << 2;
					pBuf += sprintf( pBuf, "STR R%d, [R%d + #%02x] ", rd, rn, offs );
				}
				break;
			case 0x7: /* Byte Store w/ Immeidate Offset */
				if( opcode & THUMB_LSOP_L ) /* Load */
				{
					rn = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
					rd = opcode & THUMB_ADDSUB_RD;
					offs = ( opcode & THUMB_LSOP_OFFS ) >> THUMB_LSOP_OFFS_SHIFT;
					pBuf += sprintf( pBuf, "LDRB R%d, [R%d + #%02x]", rd, rn, offs );
				}
				else /* Store */
				{
					rn = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
					rd = opcode & THUMB_ADDSUB_RD;
					offs = ( opcode & THUMB_LSOP_OFFS ) >> THUMB_LSOP_OFFS_SHIFT;
					pBuf += sprintf( pBuf, "STRB R%d, [R%d + #%02x] ", rd, rn, offs );
				}
				break;
			case 0x8: /* Load/Store Halfword */
				if( opcode & THUMB_HALFOP_L ) /* Load */
				{
					imm = ( opcode & THUMB_HALFOP_OFFS ) >> THUMB_HALFOP_OFFS_SHIFT;
					rs = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
					rd = ( opcode & THUMB_ADDSUB_RD ) >> THUMB_ADDSUB_RD_SHIFT;
					pBuf += sprintf( pBuf, "LDRH R%d, [R%d, #%03x]", rd, rs, imm << 1 );
				}
				else /* Store */
				{
					imm = ( opcode & THUMB_HALFOP_OFFS ) >> THUMB_HALFOP_OFFS_SHIFT;
					rs = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
					rd = ( opcode & THUMB_ADDSUB_RD ) >> THUMB_ADDSUB_RD_SHIFT;
					pBuf += sprintf( pBuf, "STRH R%d, [R%d, #%03x]", rd, rs, imm << 1 );
				}
				break;
			case 0x9: /* Stack-Relative Load/Store */
				if( opcode & THUMB_STACKOP_L )
				{
					rd = ( opcode & THUMB_STACKOP_RD ) >> THUMB_STACKOP_RD_SHIFT;
					offs = (UINT8)( opcode & THUMB_INSN_IMM );
					pBuf += sprintf( pBuf, "LDR R%d, [SP, #%03x]", rd, offs << 2 );
				}
				else
				{
					rd = ( opcode & THUMB_STACKOP_RD ) >> THUMB_STACKOP_RD_SHIFT;
					offs = (UINT8)( opcode & THUMB_INSN_IMM );
					pBuf += sprintf( pBuf, "STR R%d, [SP, #%03x]", rd, offs << 2 );
				}
				break;
			case 0xa: /* Get relative address */
				if( opcode & THUMB_RELADDR_SP ) /* ADD Rd, SP, #nn */
				{
					rd = ( opcode & THUMB_RELADDR_RD ) >> THUMB_RELADDR_RD_SHIFT;
					offs = (UINT8)( opcode & THUMB_INSN_IMM ) << 2;
					pBuf += sprintf( pBuf, "ADD R%d, SP, #%03x", rd, offs );
				}
				else /* ADD Rd, PC, #nn */
				{
					rd = ( opcode & THUMB_RELADDR_RD ) >> THUMB_RELADDR_RD_SHIFT;
					offs = (UINT8)( opcode & THUMB_INSN_IMM ) << 2;
					pBuf += sprintf( pBuf, "ADD R%d, PC, #%03x", rd, offs );
				}
				break;
			case 0xb: /* Stack-Related Opcodes */
				switch( ( opcode & THUMB_STACKOP_TYPE ) >> THUMB_STACKOP_TYPE_SHIFT )
				{
					case 0x0: /* ADD SP, #imm */
						addr = ( opcode & THUMB_INSN_IMM );
						addr &= ~THUMB_INSN_IMM_S;
						pBuf += sprintf( pBuf, "ADD SP, #");
						if( opcode & THUMB_INSN_IMM_S )
						{
							pBuf += sprintf( pBuf, "-");
						}
						pBuf += sprintf( pBuf, "%03x", addr << 2);
						break;
					case 0x5: /* PUSH {Rlist}{LR} */
						pBuf += sprintf( pBuf, "PUSH {LR, ");
						for( offs = 7; offs >= 0; offs-- )
						{
							if( opcode & ( 1 << offs ) )
							{
								pBuf += sprintf( pBuf, "R%d, ", offs);
							}
						}
						pBuf += sprintf( pBuf, "}");
						break;
					case 0x4: /* PUSH {Rlist} */
						pBuf += sprintf( pBuf, "PUSH {");
						for( offs = 7; offs >= 0; offs-- )
						{
							if( opcode & ( 1 << offs ) )
							{
								pBuf += sprintf( pBuf, "R%d, ", offs);
							}
						}
						pBuf += sprintf( pBuf, "}");
						break;
					case 0xc: /* POP {Rlist} */
						pBuf += sprintf( pBuf, "POP {");
						for( offs = 0; offs < 8; offs++ )
						{
							if( opcode & ( 1 << offs ) )
							{
								pBuf += sprintf( pBuf, "R%d, ", offs);
							}
						}
						pBuf += sprintf( pBuf, "}");
						break;
					case 0xd: /* POP {Rlist}{PC} */
						pBuf += sprintf( pBuf, "POP {");
						for( offs = 0; offs < 8; offs++ )
						{
							if( opcode & ( 1 << offs ) )
							{
								pBuf += sprintf( pBuf, "R%d, ", offs);
							}
						}
						pBuf += sprintf( pBuf, "PC}");
						break;
					default:
						sprintf( pBuf, "INVALID %04x", opcode);
						break;
				}
				break;
			case 0xc: /* Multiple Load/Store */
				if( opcode & THUMB_MULTLS ) /* Load */
				{
					rd = ( opcode & THUMB_MULTLS_BASE ) >> THUMB_MULTLS_BASE_SHIFT;
					pBuf += sprintf( pBuf, "LDMIA R%d!,{", rd);
					for( offs = 0; offs < 8; offs++ )
					{
						if( opcode & ( 1 << offs ) )
						{
							pBuf += sprintf( pBuf, "R%d, ", offs);
						}
					}
					pBuf += sprintf( pBuf, "}");
				}
				else /* Store */
				{
					rd = ( opcode & THUMB_MULTLS_BASE ) >> THUMB_MULTLS_BASE_SHIFT;
					pBuf += sprintf( pBuf, "STMIA R%d!,{", rd);
					for( offs = 7; offs >= 0; offs-- )
					{
						if( opcode & ( 1 << offs ) )
						{
							pBuf += sprintf( pBuf, "R%d, ", offs);
						}
					}
					pBuf += sprintf( pBuf, "}");
				}
				break;
			case 0xd: /* Conditional Branch */
				offs = (INT8)( opcode & THUMB_INSN_IMM );
				switch( ( opcode & THUMB_COND_TYPE ) >> THUMB_COND_TYPE_SHIFT )
				{
					case COND_EQ:
						pBuf += sprintf( pBuf, "BEQ %08x (%02x)", pc + 4 + (offs << 1), offs << 1);
						break;
					case COND_NE:
						pBuf += sprintf( pBuf, "BNE %08x (%02x)", pc + 4 + (offs << 1), offs << 1);
						break;
					case COND_CS:
						pBuf += sprintf( pBuf, "BCS %08x (%02x)", pc + 4 + (offs << 1), offs << 1);
						break;
					case COND_CC:
						pBuf += sprintf( pBuf, "BCC %08x (%02x)", pc + 4 + (offs << 1), offs << 1);
						break;
					case COND_MI:
						pBuf += sprintf( pBuf, "BMI %08x (%02x)", pc + 4 + (offs << 1), offs << 1);
						break;
					case COND_PL:
						pBuf += sprintf( pBuf, "BPL %08x (%02x)", pc + 4 + (offs << 1), offs << 1);
						break;
					case COND_VS:
						pBuf += sprintf( pBuf, "BVS %08x (%02x)", pc + 4 + (offs << 1), offs << 1);
						break;
					case COND_VC:
						pBuf += sprintf( pBuf, "BVC %08x (%02x)", pc + 4 + (offs << 1), offs << 1);
						break;
					case COND_HI:
						pBuf += sprintf( pBuf, "BHI %08x (%02x)", pc + 4 + (offs << 1), offs << 1);
						break;
					case COND_LS:
						pBuf += sprintf( pBuf, "BLS %08x (%02x)", pc + 4 + (offs << 1), offs << 1);
						break;
					case COND_GE:
						pBuf += sprintf( pBuf, "BGE %08x (%02x)", pc + 4 + (offs << 1), offs << 1);
						break;
					case COND_LT:
						pBuf += sprintf( pBuf, "BLT %08x (%02x)", pc + 4 + (offs << 1), offs << 1);
						break;
					case COND_GT:
						pBuf += sprintf( pBuf, "BGT %08x (%02x)", pc + 4 + (offs << 1), offs << 1);
						break;
					case COND_LE:
						pBuf += sprintf( pBuf, "BLE %08x (%02x)", pc + 4 + (offs << 1), offs << 1);
						break;
					case COND_AL:
						pBuf += sprintf( pBuf, "INVALID");
						break;
					case COND_NV:
						pBuf += sprintf( pBuf, "SWI %02x\n", opcode & 0xff);
						break;
				}
				break;
			case 0xe: /* B #offs */
				if( opcode  & THUMB_BLOP_LO )
				{
					addr = ( ( opcode & THUMB_BLOP_OFFS ) << 1 ) & 0xfffc;
					pBuf += sprintf( pBuf, "BLX (LO) %08x", addr );
					dasmflags = DASMFLAG_STEP_OVER;
				}
				else
				{
					offs = ( opcode & THUMB_BRANCH_OFFS ) << 1;
					if( offs & 0x00000800 )
					{
						offs |= 0xfffff800;
					}
					pBuf += sprintf( pBuf, "B #%08x (%08x)", offs, pc + 4 + offs);
				}
				break;
			case 0xf: /* BL */
				if( opcode & THUMB_BLOP_LO )
				{
					pBuf += sprintf( pBuf, "BL (LO) %08x", ( opcode & THUMB_BLOP_OFFS ) << 1 );
					dasmflags = DASMFLAG_STEP_OVER;
				}
				else
				{
					addr = ( opcode & THUMB_BLOP_OFFS ) << 12;
					if( addr & ( 1 << 22 ) )
					{
						addr |= 0xff800000;
					}
					pBuf += sprintf( pBuf, "BL (HI) %08x", addr );
					dasmflags = DASMFLAG_STEP_OVER;
				}
				break;
			default:
				sprintf( pBuf, "INVALID %04x", opcode);
				break;
		}

	return dasmflags | DASMFLAG_SUPPORTED;
}

CPU_DISASSEMBLE( arm7arm )
{
	return arm7_disasm(buffer, pc, oprom[0] | (oprom[1] << 8) | (oprom[2] << 16) | (oprom[3] << 24)) | 4;
}

CPU_DISASSEMBLE( arm7arm_be )
{
	return arm7_disasm(buffer, pc, oprom[3] | (oprom[2] << 8) | (oprom[1] << 16) | (oprom[0] << 24)) | 4;
}

CPU_DISASSEMBLE( arm7thumb )
{
	return thumb_disasm(buffer, pc, oprom[0] | (oprom[1] << 8)) | 2;
}

CPU_DISASSEMBLE( arm7thumb_be )
{
	return thumb_disasm(buffer, pc, oprom[1] | (oprom[0] << 8)) | 2;
}
