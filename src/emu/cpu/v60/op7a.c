
/*
 * CMPC: What happens to cpustate->_S flag if the strings are identical?
 *   I suppose that it will be cleared. And is it set or cleared
 *   when the first one is a substring of the second? I suppose
 *   cleared (since cpustate->_S should be (src > dst))
 * MOVC: Why MOVCS does not exist in downward version?
 * SHCHDB / SHCHDH: cpustate->R27 is filled with the offset from the start or from the end?
 *
 * Strange stuff:
 *   SCHC opcodes does *not* modify cpustate->_Z flag as stated in V60 manual:
 *   they do the opposite (set if not found, reset if found)
 */

#define F7AEND(cs)  \
	return (cs)->amlength1 + (cs)->amlength2 + 4;

#define F7BEND(cs)  \
	return (cs)->amlength1 + (cs)->amlength2 + 3;

#define F7CEND(cs)  \
	return (cs)->amlength1 + (cs)->amlength2 + 3;

#define F7BCREATEBITMASK(x) \
	x = ((1 << (x)) - 1)

#define F7CCREATEBITMASK(x) \
	x = ((1 << (x)) - 1)

static void F7aDecodeOperands(v60_state *cpustate, UINT32 (*DecodeOp1)(v60_state *), UINT8 dim1, UINT32 (*DecodeOp2)(v60_state *), UINT8 dim2)
{
	UINT8 appb;
	// Decode first operand
	cpustate->moddim = dim1;
	cpustate->modm = cpustate->subop & 0x40;
	cpustate->modadd = cpustate->PC + 2;
	cpustate->amlength1 = DecodeOp1(cpustate);
	cpustate->flag1 = cpustate->amflag;
	cpustate->op1 = cpustate->amout;

	// Decode length
	appb = OpRead8(cpustate, cpustate->PC + 2 + cpustate->amlength1);
	if (appb & 0x80)
		cpustate->lenop1 = cpustate->reg[appb & 0x1F];
	else
		cpustate->lenop1 = appb;

	// Decode second operand
	cpustate->moddim = dim2;
	cpustate->modm = cpustate->subop & 0x20;
	cpustate->modadd = cpustate->PC + 3 + cpustate->amlength1;
	cpustate->amlength2 = DecodeOp2(cpustate);
	cpustate->flag2 = cpustate->amflag;
	cpustate->op2 = cpustate->amout;

	// Decode length
	appb = OpRead8(cpustate, cpustate->PC + 3 + cpustate->amlength1 + cpustate->amlength2);
	if (appb & 0x80)
		cpustate->lenop2 = cpustate->reg[appb & 0x1F];
	else
		cpustate->lenop2 = appb;
}

static void F7bDecodeFirstOperand(v60_state *cpustate, UINT32 (*DecodeOp1)(v60_state *), UINT8 dim1)
{
	UINT8 appb;
	// Decode first operand
	cpustate->moddim = dim1;
	cpustate->modm = cpustate->subop & 0x40;
	cpustate->modadd = cpustate->PC + 2;
	cpustate->amlength1 = DecodeOp1(cpustate);
	cpustate->flag1 = cpustate->amflag;
	cpustate->op1 = cpustate->amout;

	// Decode ext
	appb = OpRead8(cpustate, cpustate->PC + 2 + cpustate->amlength1);
	if (appb & 0x80)
		cpustate->lenop1 = cpustate->reg[appb & 0x1F];
	else
		cpustate->lenop1 = appb;
}


static void F7bWriteSecondOperand(v60_state *cpustate, UINT8 dim2)
{
	cpustate->moddim = dim2;
	cpustate->modm = cpustate->subop & 0x20;
	cpustate->modadd = cpustate->PC + 3 + cpustate->amlength1;
	cpustate->amlength2 = WriteAM(cpustate);
}


static void F7bDecodeOperands(v60_state *cpustate, UINT32 (*DecodeOp1)(v60_state *), UINT8 dim1, UINT32 (*DecodeOp2)(v60_state *), UINT8 dim2)
{
	// Decode first operand
	F7bDecodeFirstOperand(cpustate, DecodeOp1, dim1);
	cpustate->bamoffset1 = cpustate->bamoffset;

	// Decode second operand
	cpustate->moddim = dim2;
	cpustate->modm = cpustate->subop & 0x20;
	cpustate->modadd = cpustate->PC + 3 + cpustate->amlength1;
	cpustate->amlength2 = DecodeOp2(cpustate);
	cpustate->flag2 = cpustate->amflag;
	cpustate->op2 = cpustate->amout;
	cpustate->bamoffset2 = cpustate->bamoffset;
}

static void F7cDecodeOperands(v60_state *cpustate, UINT32 (*DecodeOp1)(v60_state *), UINT8 dim1, UINT32 (*DecodeOp2)(v60_state *), UINT8 dim2)
{
	UINT8 appb;
	// Decode first operand
	cpustate->moddim = dim1;
	cpustate->modm = cpustate->subop & 0x40;
	cpustate->modadd = cpustate->PC + 2;
	cpustate->amlength1 = DecodeOp1(cpustate);
	cpustate->flag1 = cpustate->amflag;
	cpustate->op1 = cpustate->amout;

	// Decode second operand
	cpustate->moddim = dim2;
	cpustate->modm = cpustate->subop & 0x20;
	cpustate->modadd = cpustate->PC + 2 + cpustate->amlength1;
	cpustate->amlength2 = DecodeOp2(cpustate);
	cpustate->flag2 = cpustate->amflag;
	cpustate->op2 = cpustate->amout;

	// Decode ext
	appb = OpRead8(cpustate, cpustate->PC + 2 + cpustate->amlength1 + cpustate->amlength2);
	if (appb & 0x80)
		cpustate->lenop1 = cpustate->reg[appb & 0x1F];
	else
		cpustate->lenop1 = appb;
}

#define F7CLOADOP1BYTE(cs,appb) \
	if ((cs)->flag1) \
		appb = (UINT8)((cs)->reg[(cs)->op1]&0xFF); \
	else \
		appb = (cs)->program->read_byte((cs)->op1);

#define F7CLOADOP2BYTE(cs,appb) \
	if ((cs)->flag2) \
		appb = (UINT8)((cs)->reg[(cs)->op2]&0xFF); \
	else \
		appb = (cs)->program->read_byte((cs)->op2);


#define F7CSTOREOP2BYTE(cs) \
	if ((cs)->flag2) \
		SETREG8((cs)->reg[(cs)->op2], appb); \
	else \
		(cs)->program->write_byte((cs)->op2, appb);

#define F7CSTOREOP2HALF(cs) \
	if ((cs)->flag2) \
		SETREG16((cs)->reg[(cs)->op2], apph); \
	else \
		(cs)->program->write_word_unaligned((cs)->op2, apph);

static UINT32 opCMPSTRB(v60_state *cpustate, UINT8 bFill, UINT8 bStop)
{
	UINT32 i, dest;
	UINT8 c1, c2;

	F7aDecodeOperands(cpustate, ReadAMAddress, 0,ReadAMAddress, 0);

	// Filling
	if (bFill)
	{
		if (cpustate->lenop1 < cpustate->lenop2)
		{
			for (i = cpustate->lenop1; i < cpustate->lenop2; i++)
				cpustate->program->write_byte(cpustate->op1 + i,(UINT8)cpustate->R26);
		}
		else if (cpustate->lenop2 < cpustate->lenop1)
		{
			for (i = cpustate->lenop2; i < cpustate->lenop1; i++)
				cpustate->program->write_byte(cpustate->op2 + i,(UINT8)cpustate->R26);
		}
	}

	dest = (cpustate->lenop1 < cpustate->lenop2 ? cpustate->lenop1 : cpustate->lenop2);

	cpustate->_Z = 0;
	cpustate->_S = 0;
	if (bStop) cpustate->_CY = 1;

	for (i = 0; i < dest; i++)
	{
		c1 = cpustate->program->read_byte(cpustate->op1 + i);
		c2 = cpustate->program->read_byte(cpustate->op2 + i);

		if (c1 > c2)
		{
			cpustate->_S = 1;   break;
		}
		else if (c2 > c1)
		{
			cpustate->_S = 0;   break;
		}

		if (bStop)
			if (c1 == (UINT8)cpustate->R26 || c2 == (UINT8)cpustate->R26)
			{
				cpustate->_CY = 0;
				break;
			}
	}

	cpustate->R28 = cpustate->lenop1 + i;
	cpustate->R27 = cpustate->lenop2 + i;

	if (i == dest)
	{
		if (cpustate->lenop1 > cpustate->lenop2)
			cpustate->_S = 1;
		else if (cpustate->lenop2 > cpustate->lenop1)
			cpustate->_S = 0;
		else
			cpustate->_Z = 1;
	}

	F7AEND(cpustate);
}

static UINT32 opCMPSTRH(v60_state *cpustate, UINT8 bFill, UINT8 bStop)
{
	UINT32 i, dest;
	UINT16 c1, c2;

	F7aDecodeOperands(cpustate, ReadAMAddress, 0,ReadAMAddress, 0);

	// Filling
	if (bFill)
	{
		if (cpustate->lenop1 < cpustate->lenop2)
		{
			for (i = cpustate->lenop1; i < cpustate->lenop2; i++)
				cpustate->program->write_word_unaligned(cpustate->op1 + i * 2,(UINT16)cpustate->R26);
		}
		else if (cpustate->lenop2 < cpustate->lenop1)
		{
			for (i = cpustate->lenop2; i < cpustate->lenop1; i++)
				cpustate->program->write_word_unaligned(cpustate->op2 + i * 2,(UINT16)cpustate->R26);
		}
	}

	dest = (cpustate->lenop1 < cpustate->lenop2 ? cpustate->lenop1 : cpustate->lenop2);

	cpustate->_Z = 0;
	cpustate->_S = 0;
	if (bStop) cpustate->_CY = 1;

	for (i = 0; i < dest; i++)
	{
		c1 = cpustate->program->read_word_unaligned(cpustate->op1 + i * 2);
		c2 = cpustate->program->read_word_unaligned(cpustate->op2 + i * 2);

		if (c1 > c2)
		{
			cpustate->_S = 1;   break;
		}
		else if (c2 > c1)
		{
			cpustate->_S = 0;   break;
		}

		if (bStop)
			if (c1 == (UINT16)cpustate->R26 || c2 == (UINT16)cpustate->R26)
			{
				cpustate->_CY = 0;
				break;
			}
	}

	cpustate->R28 = cpustate->lenop1 + i * 2;
	cpustate->R27 = cpustate->lenop2 + i * 2;

	if (i == dest)
	{
		if (cpustate->lenop1 > cpustate->lenop2)
			cpustate->_S = 1;
		else if (cpustate->lenop2 > cpustate->lenop1)
			cpustate->_S = 0;
		else
			cpustate->_Z = 1;
	}

	F7AEND(cpustate);
}



static UINT32 opMOVSTRUB(v60_state *cpustate, UINT8 bFill, UINT8 bStop) /* TRUSTED (0, 0) (1, 0) */
{
	UINT32 i, dest;
	UINT8 c1;

//  if (bStop)
//  {
//      int a = 1;
//  }

	F7aDecodeOperands(cpustate, ReadAMAddress, 0,ReadAMAddress, 0);

	dest = (cpustate->lenop1 < cpustate->lenop2 ? cpustate->lenop1 : cpustate->lenop2);

	for (i = 0; i < dest; i++)
	{
		cpustate->program->write_byte(cpustate->op2 + i,(c1 = cpustate->program->read_byte(cpustate->op1 + i)));

		if (bStop && c1 == (UINT8)cpustate->R26)
			break;
	}

	cpustate->R28 = cpustate->op1 + i;
	cpustate->R27 = cpustate->op2 + i;

	if (bFill && cpustate->lenop1 < cpustate->lenop2)
	{
		for (;i < cpustate->lenop2; i++)
			cpustate->program->write_byte(cpustate->op2 + i,(UINT8)cpustate->R26);

		cpustate->R27 = cpustate->op2 + i;
	}


	F7AEND(cpustate);
}

static UINT32 opMOVSTRDB(v60_state *cpustate, UINT8 bFill, UINT8 bStop)
{
	UINT32 i, dest;
	UINT8 c1;

	F7aDecodeOperands(cpustate, ReadAMAddress, 0,ReadAMAddress, 0);

	dest = (cpustate->lenop1 < cpustate->lenop2 ? cpustate->lenop1 : cpustate->lenop2);

	for (i = 0; i < dest; i++)
	{
		cpustate->program->write_byte(cpustate->op2 + (dest - i - 1),(c1 = cpustate->program->read_byte(cpustate->op1 + (dest - i - 1))));

		if (bStop && c1 == (UINT8)cpustate->R26)
			break;
	}

	cpustate->R28 = cpustate->op1 + (cpustate->lenop1 - i - 1);
	cpustate->R27 = cpustate->op2 + (cpustate->lenop2 - i - 1);

	if (bFill && cpustate->lenop1 < cpustate->lenop2)
	{
		for (;i < cpustate->lenop2; i++)
			cpustate->program->write_byte(cpustate->op2 + dest + (cpustate->lenop2 - i - 1),(UINT8)cpustate->R26);

		cpustate->R27 = cpustate->op2 + (cpustate->lenop2 - i - 1);
	}


	F7AEND(cpustate);
}


static UINT32 opMOVSTRUH(v60_state *cpustate, UINT8 bFill, UINT8 bStop) /* TRUSTED (0, 0) (1, 0) */
{
	UINT32 i, dest;
	UINT16 c1;

//  if (bStop)
//  {   int a = 1; }

	F7aDecodeOperands(cpustate, ReadAMAddress, 1,ReadAMAddress, 1);

	dest = (cpustate->lenop1 < cpustate->lenop2 ? cpustate->lenop1 : cpustate->lenop2);

	for (i = 0; i < dest; i++)
	{
		cpustate->program->write_word_unaligned(cpustate->op2 + i * 2,(c1 = cpustate->program->read_word_unaligned(cpustate->op1 + i * 2)));

		if (bStop && c1 == (UINT16)cpustate->R26)
			break;
	}

	cpustate->R28 = cpustate->op1 + i * 2;
	cpustate->R27 = cpustate->op2 + i * 2;

	if (bFill && cpustate->lenop1 < cpustate->lenop2)
	{
		for (;i < cpustate->lenop2; i++)
			cpustate->program->write_word_unaligned(cpustate->op2 + i * 2,(UINT16)cpustate->R26);

		cpustate->R27 = cpustate->op2 + i * 2;
	}

	F7AEND(cpustate);
}

static UINT32 opMOVSTRDH(v60_state *cpustate, UINT8 bFill, UINT8 bStop)
{
	UINT32 i, dest;
	UINT16 c1;

//  if (bFill | bStop)
//  { int a = 1; }

	F7aDecodeOperands(cpustate, ReadAMAddress, 1,ReadAMAddress, 1);

//  if (cpustate->lenop1 != cpustate->lenop2)
//  { int a = 1; }

	dest = (cpustate->lenop1 < cpustate->lenop2 ? cpustate->lenop1 : cpustate->lenop2);

	for (i = 0; i < dest; i++)
	{
		cpustate->program->write_word_unaligned(cpustate->op2 + (dest - i - 1) * 2,(c1 = cpustate->program->read_word_unaligned(cpustate->op1 + (dest - i - 1) * 2)));

		if (bStop && c1 == (UINT16)cpustate->R26)
			break;
	}

	cpustate->R28 = cpustate->op1 + (cpustate->lenop1 - i - 1) * 2;
	cpustate->R27 = cpustate->op2 + (cpustate->lenop2 - i - 1) * 2;

	if (bFill && cpustate->lenop1 < cpustate->lenop2)
	{
		for (;i < cpustate->lenop2; i++)
			cpustate->program->write_word_unaligned(cpustate->op2 + (cpustate->lenop2 - i - 1) * 2,(UINT16)cpustate->R26);

		cpustate->R27 = cpustate->op2 + (cpustate->lenop2 - i - 1) * 2;
	}

	F7AEND(cpustate);
}

static UINT32 opSEARCHUB(v60_state *cpustate, UINT8 bSearch)
{
	UINT8 appb;
	UINT32 i;

	F7bDecodeOperands(cpustate, ReadAMAddress, 0,ReadAM, 0);

	for (i = 0; i < cpustate->lenop1; i++)
	{
		appb = (cpustate->program->read_byte(cpustate->op1 + i) == (UINT8)cpustate->op2);
		if ((bSearch && appb) || (!bSearch && !appb))
			break;
	}

	cpustate->R28 = cpustate->op1 + i;
	cpustate->R27 = i;

	// This is the opposite as stated in V60 manual...
	if (i != cpustate->lenop1)
		cpustate->_Z = 0;
	else
		cpustate->_Z = 1;

	F7BEND(cpustate);
}

static UINT32 opSEARCHUH(v60_state *cpustate, UINT8 bSearch)
{
	UINT8 appb;
	UINT32 i;

	F7bDecodeOperands(cpustate, ReadAMAddress, 1,ReadAM, 1);

	for (i = 0; i < cpustate->lenop1; i++)
	{
		appb = (cpustate->program->read_word_unaligned(cpustate->op1 + i * 2) == (UINT16)cpustate->op2);
		if ((bSearch && appb) || (!bSearch && !appb))
			break;
	}

	cpustate->R28 = cpustate->op1 + i * 2;
	cpustate->R27 = i;

	if (i != cpustate->lenop1)
		cpustate->_Z = 0;
	else
		cpustate->_Z = 1;

	F7BEND(cpustate);
}

static UINT32 opSEARCHDB(v60_state *cpustate, UINT8 bSearch)
{
	UINT8 appb;
	INT32 i;

	F7bDecodeOperands(cpustate, ReadAMAddress, 0,ReadAM, 0);

	for (i = cpustate->lenop1; i >= 0; i--)
	{
		appb = (cpustate->program->read_byte(cpustate->op1 + i) == (UINT8)cpustate->op2);
		if ((bSearch && appb) || (!bSearch && !appb))
			break;
	}

	cpustate->R28 = cpustate->op1 + i;
	cpustate->R27 = i;

	// This is the opposite as stated in V60 manual...
	if ((UINT32)i != cpustate->lenop1)
		cpustate->_Z = 0;
	else
		cpustate->_Z = 1;

	F7BEND(cpustate);
}

static UINT32 opSEARCHDH(v60_state *cpustate, UINT8 bSearch)
{
	UINT8 appb;
	INT32 i;

	F7bDecodeOperands(cpustate, ReadAMAddress, 1,ReadAM, 1);

	for (i = cpustate->lenop1 - 1; i >= 0; i--)
	{
		appb = (cpustate->program->read_word_unaligned(cpustate->op1 + i * 2) == (UINT16)cpustate->op2);
		if ((bSearch && appb) || (!bSearch && !appb))
			break;
	}

	cpustate->R28 = cpustate->op1 + i * 2;
	cpustate->R27 = i;

	if ((UINT32)i != cpustate->lenop1)
		cpustate->_Z = 0;
	else
		cpustate->_Z = 1;

	F7BEND(cpustate);
}


static UINT32 opSCHCUB(v60_state *cpustate) { return opSEARCHUB(cpustate, 1); }
static UINT32 opSCHCUH(v60_state *cpustate) { return opSEARCHUH(cpustate, 1); }
static UINT32 opSCHCDB(v60_state *cpustate) { return opSEARCHDB(cpustate, 1); }
static UINT32 opSCHCDH(v60_state *cpustate) { return opSEARCHDH(cpustate, 1); }
static UINT32 opSKPCUB(v60_state *cpustate) { return opSEARCHUB(cpustate, 0); }
static UINT32 opSKPCUH(v60_state *cpustate) { return opSEARCHUH(cpustate, 0); }
static UINT32 opSKPCDB(v60_state *cpustate) { return opSEARCHDB(cpustate, 0); }
static UINT32 opSKPCDH(v60_state *cpustate) { return opSEARCHDH(cpustate, 0); }

static UINT32 opCMPCB(v60_state *cpustate) { return opCMPSTRB(cpustate, 0, 0); }
static UINT32 opCMPCH(v60_state *cpustate) { return opCMPSTRH(cpustate, 0, 0); }
static UINT32 opCMPCFB(v60_state *cpustate) { return opCMPSTRB(cpustate, 1, 0); }
static UINT32 opCMPCFH(v60_state *cpustate) { return opCMPSTRH(cpustate, 1, 0); }
static UINT32 opCMPCSB(v60_state *cpustate) { return opCMPSTRB(cpustate, 0, 1); }
static UINT32 opCMPCSH(v60_state *cpustate) { return opCMPSTRH(cpustate, 0, 1); }

static UINT32 opMOVCUB(v60_state *cpustate) { return opMOVSTRUB(cpustate, 0, 0); }
static UINT32 opMOVCUH(v60_state *cpustate) { return opMOVSTRUH(cpustate, 0, 0); }
static UINT32 opMOVCFUB(v60_state *cpustate) { return opMOVSTRUB(cpustate, 1, 0); }
static UINT32 opMOVCFUH(v60_state *cpustate) { return opMOVSTRUH(cpustate, 1, 0); }
static UINT32 opMOVCSUB(v60_state *cpustate) { return opMOVSTRUB(cpustate, 0, 1); }
static UINT32 opMOVCSUH(v60_state *cpustate) { return opMOVSTRUH(cpustate, 0, 1); }

static UINT32 opMOVCDB(v60_state *cpustate) { return opMOVSTRDB(cpustate, 0, 0); }
static UINT32 opMOVCDH(v60_state *cpustate) { return opMOVSTRDH(cpustate, 0, 0); }
static UINT32 opMOVCFDB(v60_state *cpustate) { return opMOVSTRDB(cpustate, 1, 0); }
static UINT32 opMOVCFDH(v60_state *cpustate) { return opMOVSTRDH(cpustate, 1, 0); }

static UINT32 opEXTBFZ(v60_state *cpustate) /* TRUSTED */
{
	F7bDecodeFirstOperand(cpustate, BitReadAM, 11);

	F7BCREATEBITMASK(cpustate->lenop1);

	cpustate->modwritevalw = (cpustate->op1 >> cpustate->bamoffset) & cpustate->lenop1;

	F7bWriteSecondOperand(cpustate, 2);

	F7BEND(cpustate);
}

static UINT32 opEXTBFS(v60_state *cpustate) /* TRUSTED */
{
	F7bDecodeFirstOperand(cpustate, BitReadAM, 11);

	F7BCREATEBITMASK(cpustate->lenop1);

	cpustate->modwritevalw = (cpustate->op1 >> cpustate->bamoffset) & cpustate->lenop1;
	if (cpustate->modwritevalw & ((cpustate->lenop1 + 1) >> 1))
		cpustate->modwritevalw |= ~cpustate->lenop1;

	F7bWriteSecondOperand(cpustate, 2);

	F7BEND(cpustate);
}

static UINT32 opEXTBFL(v60_state *cpustate)
{
	UINT32 appw;

	F7bDecodeFirstOperand(cpustate, BitReadAM, 11);

	appw = cpustate->lenop1;
	F7BCREATEBITMASK(cpustate->lenop1);

	cpustate->modwritevalw = (cpustate->op1 >> cpustate->bamoffset) & cpustate->lenop1;
	cpustate->modwritevalw <<= 32 - appw;

	F7bWriteSecondOperand(cpustate, 2);

	F7BEND(cpustate);
}

static UINT32 opSCHBS(v60_state *cpustate, UINT32 bSearch1)
{
	UINT32 i, data;
	UINT32 offset;

	F7bDecodeFirstOperand(cpustate, BitReadAMAddress, 10);

	// Read first UINT8
	cpustate->op1 += cpustate->bamoffset / 8;
	data = cpustate->program->read_byte(cpustate->op1);
	offset = cpustate->bamoffset & 7;

	// Scan bitstring
	for (i = 0; i < cpustate->lenop1; i++)
	{
		// Update the work register
		cpustate->R28 = cpustate->op1;

		// There is a 0 / 1 at current offset?
		if ((bSearch1 && (data&(1 << offset))) ||
			(!bSearch1 && !(data&(1 << offset))))
			break;

		// Next bit please
		offset++;
		if (offset == 8)
		{
			// Next UINT8 please
			offset = 0;
			cpustate->op1++;
			data = cpustate->program->read_byte(cpustate->op1);
		}
	}

	// Set zero if bit not found
	cpustate->_Z = (i == cpustate->lenop1);

	// Write to destination the final offset
	cpustate->modwritevalw = i;
	F7bWriteSecondOperand(cpustate, 2);

	F7BEND(cpustate);
}

static UINT32 opSCH0BSU(v60_state *cpustate) { return opSCHBS(cpustate, 0); }
static UINT32 opSCH1BSU(v60_state *cpustate) { return opSCHBS(cpustate, 1); }

static UINT32 opINSBFR(v60_state *cpustate)
{
	UINT32 appw;
	F7cDecodeOperands(cpustate, ReadAM, 2,BitReadAMAddress, 11);

	F7CCREATEBITMASK(cpustate->lenop1);

	cpustate->op2 += cpustate->bamoffset / 8;
	appw = cpustate->program->read_dword_unaligned(cpustate->op2);
	cpustate->bamoffset &= 7;

	appw &= ~(cpustate->lenop1 << cpustate->bamoffset);
	appw |=  (cpustate->lenop1 & cpustate->op1) << cpustate->bamoffset;

	cpustate->program->write_dword_unaligned(cpustate->op2, appw);

	F7CEND(cpustate);
}

static UINT32 opINSBFL(v60_state *cpustate)
{
	UINT32 appw;
	F7cDecodeOperands(cpustate, ReadAM, 2,BitReadAMAddress, 11);

	cpustate->op1 >>= (32 - cpustate->lenop1);

	F7CCREATEBITMASK(cpustate->lenop1);

	cpustate->op2 += cpustate->bamoffset / 8;
	appw = cpustate->program->read_dword_unaligned(cpustate->op2);
	cpustate->bamoffset &= 7;

	appw &= ~(cpustate->lenop1 << cpustate->bamoffset);
	appw |=  (cpustate->lenop1 & cpustate->op1) << cpustate->bamoffset;

	cpustate->program->write_dword_unaligned(cpustate->op2, appw);

	F7CEND(cpustate);
}

static UINT32 opMOVBSD(v60_state *cpustate)
{
	UINT32 i;
	UINT8 srcdata, dstdata;

	F7bDecodeOperands(cpustate, BitReadAMAddress, 10, BitReadAMAddress, 10);

//  if (cpustate->lenop1 != 1)
//  { int a = 1; }

	cpustate->bamoffset1 += cpustate->lenop1 - 1;
	cpustate->bamoffset2 += cpustate->lenop1 - 1;

	cpustate->op1 += cpustate->bamoffset1 / 8;
	cpustate->op2 += cpustate->bamoffset2 / 8;

	cpustate->bamoffset1 &= 7;
	cpustate->bamoffset2 &= 7;

	srcdata = cpustate->program->read_byte(cpustate->op1);
	dstdata = cpustate->program->read_byte(cpustate->op2);

	for (i = 0; i < cpustate->lenop1; i++)
	{
		// Update work registers
		cpustate->R28 = cpustate->op1;
		cpustate->R27 = cpustate->op2;

		dstdata &= ~(1 << cpustate->bamoffset2);
		dstdata |= ((srcdata >> cpustate->bamoffset1) & 1) << cpustate->bamoffset2;

		if (cpustate->bamoffset1 == 0)
		{
			cpustate->bamoffset1 = 8;
			cpustate->op1--;
			srcdata = cpustate->program->read_byte(cpustate->op1);
		}
		if (cpustate->bamoffset2 == 0)
		{
			cpustate->program->write_byte(cpustate->op2, dstdata);
			cpustate->bamoffset2 = 8;
			cpustate->op2--;
			dstdata = cpustate->program->read_byte(cpustate->op2);
		}

		cpustate->bamoffset1--;
		cpustate->bamoffset2--;
	}

	// Flush of the final data
	if (cpustate->bamoffset2 != 7)
		cpustate->program->write_byte(cpustate->op2, dstdata);

	F7BEND(cpustate);
}

static UINT32 opMOVBSU(v60_state *cpustate)
{
	UINT32 i;
	UINT8 srcdata, dstdata;

	F7bDecodeOperands(cpustate, BitReadAMAddress, 10, BitReadAMAddress, 10);

	cpustate->op1 += cpustate->bamoffset1 / 8;
	cpustate->op2 += cpustate->bamoffset2 / 8;

	cpustate->bamoffset1 &= 7;
	cpustate->bamoffset2 &= 7;

	srcdata = cpustate->program->read_byte(cpustate->op1);
	dstdata = cpustate->program->read_byte(cpustate->op2);

	for (i = 0; i < cpustate->lenop1; i++)
	{
		// Update work registers
		cpustate->R28 = cpustate->op1;
		cpustate->R27 = cpustate->op2;

		dstdata &= ~(1 << cpustate->bamoffset2);
		dstdata |= ((srcdata >> cpustate->bamoffset1) & 1) << cpustate->bamoffset2;

		cpustate->bamoffset1++;
		cpustate->bamoffset2++;
		if (cpustate->bamoffset1 == 8)
		{
			cpustate->bamoffset1 = 0;
			cpustate->op1++;
			srcdata = cpustate->program->read_byte(cpustate->op1);
		}
		if (cpustate->bamoffset2 == 8)
		{
			cpustate->program->write_byte(cpustate->op2, dstdata);
			cpustate->bamoffset2 = 0;
			cpustate->op2++;
			dstdata = cpustate->program->read_byte(cpustate->op2);
		}
	}

	// Flush of the final data
	if (cpustate->bamoffset2 != 0)
		cpustate->program->write_byte(cpustate->op2, dstdata);

	F7BEND(cpustate);
}

// RADM 0x20f4b8 holds the time left

static UINT32 opADDDC(v60_state *cpustate)
{
	UINT8 appb;
	UINT8 src, dst;

	F7cDecodeOperands(cpustate, ReadAM, 0, ReadAMAddress, 0);

	if (cpustate->lenop1 != 0)
	{
		logerror("ADDDC %x (pat: %x)\n", cpustate->op1, cpustate->lenop1);
	}

	F7CLOADOP2BYTE(cpustate, appb);

	src = (UINT8)(cpustate->op1 >> 4) * 10 + (UINT8)(cpustate->op1 & 0xF);
	dst = (appb >> 4) * 10 + (appb & 0xF);

	appb = src + dst + (cpustate->_CY?1:0);

	if (appb >= 100)
	{
		appb -= 100;
		cpustate->_CY = 1;
	}
	else
		cpustate->_CY = 0;

	// compute z flag:
	// cleared if result non-zero or carry generated
	// unchanged otherwise
	if (appb != 0 || cpustate->_CY)
		cpustate->_Z = 0;

	appb = ((appb / 10) << 4) | (appb % 10);

	F7CSTOREOP2BYTE(cpustate);
	F7CEND(cpustate);
}

static UINT32 opSUBDC(v60_state *cpustate)
{
	INT8 appb;
	UINT32 src, dst;

	F7cDecodeOperands(cpustate, ReadAM, 0, ReadAMAddress, 0);

	if (cpustate->lenop1 != 0)
	{
		logerror("SUBDC %x (pat: %x)\n", cpustate->op1, cpustate->lenop1);
	}

	F7CLOADOP2BYTE(cpustate, appb);

	src = (UINT32)(cpustate->op1 >> 4) * 10 + (UINT32)(cpustate->op1 & 0xF);
	dst = ((appb & 0xF0) >> 4) * 10 + (appb & 0xF);

	// Note that this APPB must be SIGNED!
	appb = (INT32)dst - (INT32)src - (cpustate->_CY?1:0);

	if (appb < 0)
	{
		appb += 100;
		cpustate->_CY = 1;
	}
	else
		cpustate->_CY = 0;

	// compute z flag:
	// cleared if result non-zero or carry generated
	// unchanged otherwise
	if (appb != 0 || cpustate->_CY)
		cpustate->_Z = 0;

	appb = ((appb / 10) << 4) | (appb % 10);

	F7CSTOREOP2BYTE(cpustate);
	F7CEND(cpustate);
}

static UINT32 opSUBRDC(v60_state *cpustate)
{
	INT8 appb;
	UINT32 src, dst;

	F7cDecodeOperands(cpustate, ReadAM, 0, ReadAMAddress, 0);

	if (cpustate->lenop1 != 0)
	{
		logerror("SUBRDC %x (pat: %x)\n", cpustate->op1, cpustate->lenop1);
	}

	F7CLOADOP2BYTE(cpustate, appb);

	src = (UINT32)(cpustate->op1 >> 4) * 10 + (UINT32)(cpustate->op1 & 0xF);
	dst = ((appb & 0xF0) >> 4) * 10 + (appb & 0xF);

	// Note that this APPB must be SIGNED!
	appb = (INT32)src - (INT32)dst - (cpustate->_CY?1:0);

	if (appb < 0)
	{
		appb += 100;
		cpustate->_CY = 1;
	}
	else
		cpustate->_CY = 0;

	// compute z flag:
	// cleared if result non-zero or carry generated
	// unchanged otherwise
	if (appb != 0 || cpustate->_CY)
		cpustate->_Z = 0;

	appb = ((appb / 10) << 4) | (appb % 10);

	F7CSTOREOP2BYTE(cpustate);
	F7CEND(cpustate);
}

static UINT32 opCVTDPZ(v60_state *cpustate)
{
	UINT16 apph;

	F7cDecodeOperands(cpustate, ReadAM, 0, ReadAMAddress, 1);

	apph = (UINT16)(((cpustate->op1 >> 4) & 0xF) | ((cpustate->op1 & 0xF) << 8));
	apph |= (cpustate->lenop1);
	apph |= (cpustate->lenop1 << 8);

	// Z flag is unchanged if src is zero, cleared otherwise
	if (cpustate->op1 != 0) cpustate->_Z = 0;

	F7CSTOREOP2HALF(cpustate);
	F7CEND(cpustate);
}

static UINT32 opCVTDZP(v60_state *cpustate)
{
	UINT8 appb;
	F7cDecodeOperands(cpustate, ReadAM, 1, ReadAMAddress, 0);

	if ((cpustate->op1 & 0xF0) != (cpustate->lenop1 & 0xF0) || ((cpustate->op1 >> 8) & 0xF0) != (cpustate->lenop1 & 0xF0))
	{
		// Decimal exception
		logerror("CVTD.ZP Decimal exception #1!\n");
	}

	if ((cpustate->op1 & 0xF) > 9 || ((cpustate->op1 >> 8) & 0xF) > 9)
	{
		// Decimal exception
		logerror("CVTD.ZP Decimal exception #2!\n");
	}

	appb = (UINT8)(((cpustate->op1 >> 8) & 0xF) | ((cpustate->op1 & 0xF) << 4));
	if (appb != 0) cpustate->_Z = 0;

	F7CSTOREOP2BYTE(cpustate);
	F7CEND(cpustate);
}

static UINT32 op58UNHANDLED(v60_state *cpustate)
{
	fatalerror("Unhandled 58 opcode at cpustate->PC: /%06x\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 op5AUNHANDLED(v60_state *cpustate)
{
	fatalerror("Unhandled 5A opcode at cpustate->PC: /%06x\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 op5BUNHANDLED(v60_state *cpustate)
{
	fatalerror("Unhandled 5B opcode at cpustate->PC: /%06x\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 op5DUNHANDLED(v60_state *cpustate)
{
	fatalerror("Unhandled 5D opcode at cpustate->PC: /%06x\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 op59UNHANDLED(v60_state *cpustate)
{
	fatalerror("Unhandled 59 opcode at cpustate->PC: /%06x\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 (*const Op59Table[32])(v60_state *) =
{
	opADDDC,
	opSUBDC,
	opSUBRDC,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	opCVTDPZ,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	opCVTDZP,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED
};


static UINT32 (*const Op5BTable[32])(v60_state *) =
{
	opSCH0BSU,
	op5BUNHANDLED,
	opSCH1BSU,
		op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	opMOVBSU,
	opMOVBSD,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED
};


static UINT32 (*const Op5DTable[32])(v60_state *) =
{
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	opEXTBFS,
	opEXTBFZ,
	opEXTBFL,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	opINSBFR,
	opINSBFL,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED
};

static UINT32 (*const Op58Table[32])(v60_state *) =
{
	opCMPCB,
	opCMPCFB,
	opCMPCSB,
	op58UNHANDLED,
	op58UNHANDLED,
	op58UNHANDLED,
	op58UNHANDLED,
	op58UNHANDLED,
	opMOVCUB,
	opMOVCDB,
	opMOVCFUB,
	opMOVCFDB,
	opMOVCSUB,
	op58UNHANDLED,
	op58UNHANDLED,
	op58UNHANDLED,
	op58UNHANDLED,
	op58UNHANDLED,
	op58UNHANDLED,
	op58UNHANDLED,
	op58UNHANDLED,
	op58UNHANDLED,
	op58UNHANDLED,
	op58UNHANDLED,
	opSCHCUB,
	opSCHCDB,
	opSKPCUB,
	opSKPCDB,
	op58UNHANDLED,
	op58UNHANDLED,
	op58UNHANDLED,
	op58UNHANDLED
};

static UINT32 (*const Op5ATable[32])(v60_state *) =
{
	opCMPCH,
	opCMPCFH,
	opCMPCSH,
	op5AUNHANDLED,
	op5AUNHANDLED,
	op5AUNHANDLED,
	op5AUNHANDLED,
	op5AUNHANDLED,
	opMOVCUH,
	opMOVCDH,
	opMOVCFUH,
	opMOVCFDH,
	opMOVCSUH,
	op5AUNHANDLED,
	op5AUNHANDLED,
	op5AUNHANDLED,
	op5AUNHANDLED,
	op5AUNHANDLED,
	op5AUNHANDLED,
	op5AUNHANDLED,
	op5AUNHANDLED,
	op5AUNHANDLED,
	op5AUNHANDLED,
	op5AUNHANDLED,
	opSCHCUH,
	opSCHCDH,
	opSKPCUH,
	opSKPCDH,
	op5AUNHANDLED,
	op5AUNHANDLED,
	op5AUNHANDLED,
	op5AUNHANDLED
};

static UINT32 op58(v60_state *cpustate)
{
	cpustate->subop = OpRead8(cpustate, cpustate->PC + 1);

	return Op58Table[cpustate->subop & 0x1F](cpustate);
}

static UINT32 op5A(v60_state *cpustate)
{
	cpustate->subop = OpRead8(cpustate, cpustate->PC + 1);

	return Op5ATable[cpustate->subop & 0x1F](cpustate);
}

static UINT32 op5B(v60_state *cpustate)
{
	cpustate->subop = OpRead8(cpustate, cpustate->PC + 1);

	return Op5BTable[cpustate->subop & 0x1F](cpustate);
}

static UINT32 op5D(v60_state *cpustate)
{
	cpustate->subop = OpRead8(cpustate, cpustate->PC + 1);

	return Op5DTable[cpustate->subop & 0x1F](cpustate);
}

static UINT32 op59(v60_state *cpustate)
{
	cpustate->subop = OpRead8(cpustate, cpustate->PC + 1);

	return Op59Table[cpustate->subop & 0x1F](cpustate);
}
