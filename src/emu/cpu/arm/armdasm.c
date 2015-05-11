// license:BSD-3-Clause
// copyright-holders:Bryan McPhail,Phil Stroffolino
/*
    ARM 2/3 disassembler

    (c) 2002-2006 Bryan McPhail (bmcphail@tendril.co.uk) and Phil Stroffolino
*/

#include "emu.h"
#include "arm.h"

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
		pBuf += sprintf(pBuf,"R%d, ", (opcode>>0)&0xf);

	pBuf += sprintf(pBuf, "%s ", pRegOp[(opcode>>5)&3] );

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
		", R%d %s ", /* Operand 1 register, Operand 2 register, shift type */
		(opcode>> 0)&0xf,
		pRegOp[(opcode>>5)&3] );

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

static char *WritePadding( char *pBuf, const char *pBuf0 )
{
	pBuf0 += 8;
	while( pBuf<pBuf0 )
	{
		*pBuf++ = ' ';
	}
	return pBuf;
}

static UINT32 arm_disasm( char *pBuf, UINT32 pc, UINT32 opcode )
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

	if( (opcode&0x0fc000f0)==0x00000090u )
	{
		/* multiply */
		/* xxxx0000 00ASdddd nnnnssss 1001mmmm */
		if( opcode&0x00200000 )
		{
			pBuf += sprintf( pBuf, "MLA" );
		}
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
	else if( (opcode&0x0c000000)==0 )
	{
		int op=(opcode>>21)&0xf;

		/* Data Processing */
		/* xxxx001a aaaSnnnn ddddrrrr bbbbbbbb */
		/* xxxx000a aaaSnnnn ddddcccc ctttmmmm */

		pBuf += sprintf(
			pBuf, "%s%s",
			pOperation[op],
			pConditionCode );

		if( (opcode&0x01000000) )
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
	else if( (opcode&0x0c000000)==0x04000000 )
	{
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

		if( opcode&0x02000000 )
		{
			/* register form */
			pBuf = WriteRegisterOperand1( pBuf, opcode );
			pBuf += sprintf( pBuf, "]" );
		}
		else
		{
			/* immediate form */
			pBuf += sprintf( pBuf, "]" );
			if( opcode&0x00800000 )
			{
				pBuf += sprintf( pBuf, ", #$%x", opcode&0xfff );
			}
			else
			{
				pBuf += sprintf( pBuf, ", -#$%x", opcode&0xfff );
			}
		}
	}
	else if( (opcode&0x0e000000) == 0x08000000 )
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
	else if( (opcode&0x0e000000)==0x0a000000 )
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
	else if( (opcode&0x0f000000) == 0x0e000000 )
	{
		/* co-processor */
		/* xxxx1110 oooLNNNN ddddpppp qqq1MMMM MRC/MCR */
		if( (opcode&0x0f100000)==0x0e100000 )
		{
			if( (opcode&0x0f100010)==0x0e100010 )
			{
				pBuf += sprintf( pBuf, "MRC" );
			}
			else if( (opcode&0x0f100010)==0x0e000010 )
			{
				pBuf += sprintf( pBuf, "MCR" );
			}
			else
			{
				pBuf += sprintf( pBuf, "???" );
			}

			pBuf += sprintf( pBuf, "%s", pConditionCode );
			pBuf = WritePadding( pBuf, pBuf0 );
			pBuf += sprintf( pBuf, "R%d, CR%d {CRM%d, q%d}",(opcode>>12)&0xf, (opcode>>16)&0xf, (opcode>>0)&0xf, (opcode>>5)&0x7);
			/* Nb:  full form should be o, p, R, CR, CRM, q */
		}
		else if( (opcode&0x0f000010)==0x0e000000 )
		{
			pBuf += sprintf( pBuf, "CDP" );
			pBuf += sprintf( pBuf, "%s", pConditionCode );
			pBuf = WritePadding( pBuf, pBuf0 );
			pBuf += sprintf( pBuf, "%08x", opcode );
		}
		else
		{
			pBuf += sprintf( pBuf, "???" );
		}
	}
	else if( (opcode&0x0f000000) == 0x0f000000 )
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

CPU_DISASSEMBLE( arm )
{
	UINT32 opcode = oprom[0] | (oprom[1] << 8) | (oprom[2] << 16) | (oprom[3] << 24);
	return 4 | arm_disasm(buffer, pc, opcode);
}

CPU_DISASSEMBLE( arm_be )
{
	UINT32 opcode = oprom[3] | (oprom[2] << 8) | (oprom[1] << 16) | (oprom[0] << 24);
	return 4 | arm_disasm(buffer, pc, opcode);
}
