
/*
 * CMPC: What happens to _S flag if the strings are identical?
 *   I suppose that it will be cleared. And is it set or cleared
 *   when the first one is a substring of the second? I suppose
 *   cleared (since _S should be (src > dst))
 * MOVC: Why MOVCS does not exist in downward version?
 * SHCHDB/SHCHDH: R27 is filled with the offset from the start or from the end?
 *
 * Strange stuff:
 *   SCHC opcodes does *not* modify _Z flag as stated in V60 manual:
 *   they do the opposite (set if not found, reset if found)
 */

static UINT32 f7aOp1, f7aOp2;
static UINT8 f7aFlag1, f7aFlag2;
static UINT32 f7aLenOp1, f7aLenOp2;
static UINT8 subOp;

static UINT32 f7bBamOffset1, f7bBamOffset2;

#define f7bOp1 f7aOp1
#define f7bFlag1 f7aFlag1
#define f7bOp2 f7aOp2
#define f7bFlag2 f7aFlag2
#define f7bLen  f7aLenOp1

#define f7cOp1 f7aOp1
#define f7cOp2 f7aOp2
#define f7cLen f7aLenOp1
#define f7cFlag1 f7aFlag1
#define f7cFlag2 f7aFlag2

#define F7AEND()	\
	return amLength1 + amLength2 + 4;

#define F7BEND()	\
	return amLength1 + amLength2 + 3;

#define F7CEND()	\
	return amLength1 + amLength2 + 3;

#define F7BCREATEBITMASK(x)	\
	x=((1<<(x))-1)

#define F7CCREATEBITMASK(x)	\
	x=((1<<(x))-1)

static void F7aDecodeOperands(UINT32 (*DecodeOp1)(void), UINT8 dim1, UINT32 (*DecodeOp2)(void), UINT8 dim2)
{
	UINT8 appb;
	// Decode first operand
	modDim=dim1;
	modM=subOp&0x40;
	modAdd=PC+2;
	amLength1=DecodeOp1();
	f7aFlag1=amFlag;
	f7aOp1=amOut;

	// Decode length
	appb=OpRead8(PC+2+amLength1);
	if (appb&0x80)
		f7aLenOp1=v60.reg[appb&0x1F];
	else
		f7aLenOp1=appb;

	// Decode second operand
	modDim=dim2;
	modM=subOp&0x20;
	modAdd=PC+3+amLength1;
	amLength2=DecodeOp2();
	f7aFlag2=amFlag;
	f7aOp2=amOut;

	// Decode length
	appb=OpRead8(PC+3+amLength1+amLength2);
	if (appb&0x80)
		f7aLenOp2=v60.reg[appb&0x1F];
	else
		f7aLenOp2=appb;
}

static void F7bDecodeFirstOperand(UINT32 (*DecodeOp1)(void), UINT8 dim1)
{
	UINT8 appb;
	// Decode first operand
	modDim=dim1;
	modM=subOp&0x40;
	modAdd=PC+2;
	amLength1=DecodeOp1();
	f7bFlag1=amFlag;
	f7bOp1=amOut;

	// Decode ext
	appb=OpRead8(PC+2+amLength1);
	if (appb&0x80)
		f7bLen=v60.reg[appb&0x1F];
	else
		f7bLen=appb;
}


static void F7bWriteSecondOperand(UINT8 dim2)
{
	modDim=dim2;
	modM=subOp&0x20;
	modAdd=PC+3+amLength1;
	amLength2=WriteAM();
}


static void F7bDecodeOperands(UINT32 (*DecodeOp1)(void), UINT8 dim1, UINT32 (*DecodeOp2)(void), UINT8 dim2)
{
	// Decode first operand
	F7bDecodeFirstOperand(DecodeOp1,dim1);
	f7bBamOffset1 = bamOffset;

	// Decode second operand
	modDim=dim2;
	modM=subOp&0x20;
	modAdd=PC+3+amLength1;
	amLength2=DecodeOp2();
	f7bFlag2=amFlag;
	f7bOp2=amOut;
	f7bBamOffset2 = bamOffset;
}

static void F7cDecodeOperands(UINT32 (*DecodeOp1)(void), UINT8 dim1, UINT32 (*DecodeOp2)(void), UINT8 dim2)
{
	UINT8 appb;
	// Decode first operand
	modDim=dim1;
	modM=subOp&0x40;
	modAdd=PC+2;
	amLength1=DecodeOp1();
	f7cFlag1=amFlag;
	f7cOp1=amOut;

	// Decode second operand
	modDim=dim2;
	modM=subOp&0x20;
	modAdd=PC+2+amLength1;
	amLength2=DecodeOp2();
	f7cFlag2=amFlag;
	f7cOp2=amOut;

	// Decode ext
	appb=OpRead8(PC+2+amLength1+amLength2);
	if (appb&0x80)
		f7cLen=v60.reg[appb&0x1F];
	else
		f7cLen=appb;
}

#define F7CLOADOP1BYTE(appb) \
	if (f7cFlag1) \
		appb = (UINT8)(v60.reg[f7cOp1]&0xFF); \
	else \
		appb = MemRead8(f7cOp1);

#define F7CLOADOP2BYTE(appb) \
	if (f7cFlag2) \
		appb = (UINT8)(v60.reg[f7cOp2]&0xFF); \
	else \
		appb = MemRead8(f7cOp2);


#define F7CSTOREOP2BYTE() \
	if (f7cFlag2) \
		SETREG8(v60.reg[f7cOp2], appb); \
	else \
		MemWrite8(f7cOp2, appb);

#define F7CSTOREOP2HALF() \
	if (f7cFlag2) \
		SETREG16(v60.reg[f7cOp2], apph); \
	else \
		MemWrite16(f7cOp2, apph);

static UINT32 opCMPSTRB(UINT8 bFill, UINT8 bStop)
{
	UINT32 i,dest;
	UINT8 c1,c2;

	F7aDecodeOperands(ReadAMAddress,0,ReadAMAddress,0);

	// Filling
	if (bFill)
	{
		if (f7aLenOp1 < f7aLenOp2)
		{
			for (i=f7aLenOp1;i<f7aLenOp2;i++)
				MemWrite8(f7aOp1+i,(UINT8)R26);
		}
		else if (f7aLenOp2 < f7aLenOp1)
		{
			for (i=f7aLenOp2;i<f7aLenOp1;i++)
				MemWrite8(f7aOp2+i,(UINT8)R26);
		}
	}

	dest=(f7aLenOp1 < f7aLenOp2 ? f7aLenOp1 : f7aLenOp2);

	_Z = 0;
	_S = 0;
	if (bStop) _CY = 1;

	for (i=0;i<dest;i++)
	{
		c1=MemRead8(f7aOp1+i);
		c2=MemRead8(f7aOp2+i);

		if (c1>c2)
		{
			_S=1;	break;
		}
		else if (c2>c1)
		{
			_S=0;	break;
		}

		if (bStop)
			if (c1==(UINT8)R26 || c2==(UINT8)R26)
			{
				_CY=0;
				break;
			}
	}

	R28=f7aLenOp1+i;
	R27=f7aLenOp2+i;

	if (i==dest)
	{
		if (f7aLenOp1 > f7aLenOp2)
			_S=1;
		else if (f7aLenOp2 > f7aLenOp1)
			_S=0;
		else
			_Z=1;
	}

	F7AEND();
}

static UINT32 opCMPSTRH(UINT8 bFill, UINT8 bStop)
{
	UINT32 i,dest;
	UINT16 c1,c2;

	F7aDecodeOperands(ReadAMAddress,0,ReadAMAddress,0);

	// Filling
	if (bFill)
	{
		if (f7aLenOp1 < f7aLenOp2)
		{
			for (i=f7aLenOp1;i<f7aLenOp2;i++)
				MemWrite16(f7aOp1+i*2,(UINT16)R26);
		}
		else if (f7aLenOp2 < f7aLenOp1)
		{
			for (i=f7aLenOp2;i<f7aLenOp1;i++)
				MemWrite16(f7aOp2+i*2,(UINT16)R26);
		}
	}

	dest=(f7aLenOp1 < f7aLenOp2 ? f7aLenOp1 : f7aLenOp2);

	_Z = 0;
	_S = 0;
	if (bStop) _CY = 1;

	for (i=0;i<dest;i++)
	{
		c1=MemRead16(f7aOp1+i*2);
		c2=MemRead16(f7aOp2+i*2);

		if (c1>c2)
		{
			_S=1;	break;
		}
		else if (c2>c1)
		{
			_S=0;	break;
		}

		if (bStop)
			if (c1==(UINT16)R26 || c2==(UINT16)R26)
			{
				_CY=0;
				break;
			}
	}

	R28=f7aLenOp1+i*2;
	R27=f7aLenOp2+i*2;

	if (i==dest)
	{
		if (f7aLenOp1 > f7aLenOp2)
			_S=1;
		else if (f7aLenOp2 > f7aLenOp1)
			_S=0;
		else
			_Z=1;
	}

	F7AEND();
}



static UINT32 opMOVSTRUB(UINT8 bFill, UINT8 bStop) /* TRUSTED (0,0) (1,0) */
{
	UINT32 i,dest;
	UINT8 c1;

//  if (bStop)
//  {
//      int a=1;
//  }

	F7aDecodeOperands(ReadAMAddress,0,ReadAMAddress,0);

	dest=(f7aLenOp1 < f7aLenOp2 ? f7aLenOp1 : f7aLenOp2);

	for (i=0;i<dest;i++)
	{
		MemWrite8(f7aOp2+i,(c1=MemRead8(f7aOp1+i)));

		if (bStop && c1==(UINT8)R26)
			break;
	}

	R28=f7aOp1+i;
	R27=f7aOp2+i;

	if (bFill && f7aLenOp1 < f7aLenOp2)
	{
		for (;i<f7aLenOp2;i++)
			MemWrite8(f7aOp2+i,(UINT8)R26);

		R27=f7aOp2+i;
	}


	F7AEND();
}

static UINT32 opMOVSTRDB(UINT8 bFill, UINT8 bStop)
{
	UINT32 i,dest;
	UINT8 c1;

	F7aDecodeOperands(ReadAMAddress,0,ReadAMAddress,0);

	dest=(f7aLenOp1 < f7aLenOp2 ? f7aLenOp1 : f7aLenOp2);

	for (i=0;i<dest;i++)
	{
		MemWrite8(f7aOp2+(dest-i-1),(c1=MemRead8(f7aOp1+(dest-i-1))));

		if (bStop && c1==(UINT8)R26)
			break;
	}

	R28=f7aOp1+(f7aLenOp1-i-1);
	R27=f7aOp2+(f7aLenOp2-i-1);

	if (bFill && f7aLenOp1 < f7aLenOp2)
	{
		for (;i<f7aLenOp2;i++)
			MemWrite8(f7aOp2+dest+(f7aLenOp2-i-1),(UINT8)R26);

		R27=f7aOp2+(f7aLenOp2-i-1);
	}


	F7AEND();
}


static UINT32 opMOVSTRUH(UINT8 bFill, UINT8 bStop) /* TRUSTED (0,0) (1,0) */
{
	UINT32 i,dest;
	UINT16 c1;

//  if (bStop)
//  {   int a=1; }

	F7aDecodeOperands(ReadAMAddress,1,ReadAMAddress,1);

	dest=(f7aLenOp1 < f7aLenOp2 ? f7aLenOp1 : f7aLenOp2);

	for (i=0;i<dest;i++)
	{
		MemWrite16(f7aOp2+i*2,(c1=MemRead16(f7aOp1+i*2)));

		if (bStop && c1==(UINT16)R26)
			break;
	}

	R28=f7aOp1+i*2;
	R27=f7aOp2+i*2;

	if (bFill && f7aLenOp1 < f7aLenOp2)
	{
		for (;i<f7aLenOp2;i++)
			MemWrite16(f7aOp2+i*2,(UINT16)R26);

		R27=f7aOp2+i*2;
	}

	F7AEND();
}

static UINT32 opMOVSTRDH(UINT8 bFill, UINT8 bStop)
{
	UINT32 i,dest;
	UINT16 c1;

//  if (bFill | bStop)
//  { int a=1; }

	F7aDecodeOperands(ReadAMAddress,1,ReadAMAddress,1);

//  if (f7aLenOp1 != f7aLenOp2)
//  { int a=1; }

	dest=(f7aLenOp1 < f7aLenOp2 ? f7aLenOp1 : f7aLenOp2);

	for (i=0;i<dest;i++)
	{
		MemWrite16(f7aOp2+(dest-i-1)*2,(c1=MemRead16(f7aOp1+(dest-i-1)*2)));

		if (bStop && c1==(UINT16)R26)
			break;
	}

	R28=f7aOp1+(f7aLenOp1-i-1)*2;
	R27=f7aOp2+(f7aLenOp2-i-1)*2;

	if (bFill && f7aLenOp1 < f7aLenOp2)
	{
		for (;i<f7aLenOp2;i++)
			MemWrite16(f7aOp2+(f7aLenOp2-i-1)*2,(UINT16)R26);

		R27=f7aOp2+(f7aLenOp2-i-1)*2;
	}

	F7AEND();
}

static UINT32 opSEARCHUB(UINT8 bSearch)
{
	UINT8 appb;
	UINT32 i;

	F7bDecodeOperands(ReadAMAddress,0,ReadAM,0);

	for (i=0;i<f7bLen;i++)
	{
		appb = (MemRead8(f7bOp1+i)==(UINT8)f7bOp2);
		if ((bSearch && appb) || (!bSearch && !appb))
			break;
	}

	R28=f7bOp1+i;
	R27=i;

	// This is the opposite as stated in V60 manual...
	if (i!=f7bLen)
		_Z=0;
	else
		_Z=1;

	F7BEND();
}

static UINT32 opSEARCHUH(UINT8 bSearch)
{
	UINT8 appb;
	UINT32 i;

	F7bDecodeOperands(ReadAMAddress,1,ReadAM,1);

	for (i=0;i<f7bLen;i++)
	{
		appb = (MemRead16(f7bOp1+i*2)==(UINT16)f7bOp2);
		if ((bSearch && appb) || (!bSearch && !appb))
			break;
	}

	R28=f7bOp1+i*2;
	R27=i;

	if (i!=f7bLen)
		_Z=0;
	else
		_Z=1;

	F7BEND();
}

static UINT32 opSEARCHDB(UINT8 bSearch)
{
	UINT8 appb;
	INT32 i;

	F7bDecodeOperands(ReadAMAddress,0,ReadAM,0);

	for (i=f7bLen;i>=0;i--)
	{
		appb = (MemRead8(f7bOp1+i)==(UINT8)f7bOp2);
		if ((bSearch && appb) || (!bSearch && !appb))
			break;
	}

	R28=f7bOp1+i;
	R27=i;

	// This is the opposite as stated in V60 manual...
	if ((UINT32)i!=f7bLen)
		_Z=0;
	else
		_Z=1;

	F7BEND();
}

static UINT32 opSEARCHDH(UINT8 bSearch)
{
	UINT8 appb;
	INT32 i;

	F7bDecodeOperands(ReadAMAddress,1,ReadAM,1);

	for (i=f7bLen-1;i>=0;i--)
	{
		appb = (MemRead16(f7bOp1+i*2)==(UINT16)f7bOp2);
		if ((bSearch && appb) || (!bSearch && !appb))
			break;
	}

	R28=f7bOp1+i*2;
	R27=i;

	if ((UINT32)i!=f7bLen)
		_Z=0;
	else
		_Z=1;

	F7BEND();
}


static UINT32 opSCHCUB(void) { return opSEARCHUB(1); }
static UINT32 opSCHCUH(void) { return opSEARCHUH(1); }
static UINT32 opSCHCDB(void) { return opSEARCHDB(1); }
static UINT32 opSCHCDH(void) { return opSEARCHDH(1); }
static UINT32 opSKPCUB(void) { return opSEARCHUB(0); }
static UINT32 opSKPCUH(void) { return opSEARCHUH(0); }
static UINT32 opSKPCDB(void) { return opSEARCHDB(0); }
static UINT32 opSKPCDH(void) { return opSEARCHDH(0); }

static UINT32 opCMPCB(void) { return opCMPSTRB(0,0); }
static UINT32 opCMPCH(void) { return opCMPSTRH(0,0); }
static UINT32 opCMPCFB(void) { return opCMPSTRB(1,0); }
static UINT32 opCMPCFH(void) { return opCMPSTRH(1,0); }
static UINT32 opCMPCSB(void) { return opCMPSTRB(0,1); }
static UINT32 opCMPCSH(void) { return opCMPSTRH(0,1); }

static UINT32 opMOVCUB(void) { return opMOVSTRUB(0,0); }
static UINT32 opMOVCUH(void) { return opMOVSTRUH(0,0); }
static UINT32 opMOVCFUB(void) { return opMOVSTRUB(1,0); }
static UINT32 opMOVCFUH(void) { return opMOVSTRUH(1,0); }
static UINT32 opMOVCSUB(void) { return opMOVSTRUB(0,1); }
static UINT32 opMOVCSUH(void) { return opMOVSTRUH(0,1); }

static UINT32 opMOVCDB(void) { return opMOVSTRDB(0,0); }
static UINT32 opMOVCDH(void) { return opMOVSTRDH(0,0); }
static UINT32 opMOVCFDB(void) { return opMOVSTRDB(1,0); }
static UINT32 opMOVCFDH(void) { return opMOVSTRDH(1,0); }

static UINT32 opEXTBFZ(void) /* TRUSTED */
{
	F7bDecodeFirstOperand(BitReadAM, 11);

	F7BCREATEBITMASK(f7bLen);

	modWriteValW=(f7bOp1 >> bamOffset) & f7bLen;

	F7bWriteSecondOperand(2);

	F7BEND();
}

static UINT32 opEXTBFS(void) /* TRUSTED */
{
 	F7bDecodeFirstOperand(BitReadAM, 11);

	F7BCREATEBITMASK(f7bLen);

	modWriteValW=(f7bOp1 >> bamOffset) & f7bLen;
	if (modWriteValW & ((f7bLen+1)>>1))
		modWriteValW |= ~f7bLen;

	F7bWriteSecondOperand(2);

	F7BEND();
}

static UINT32 opEXTBFL(void)
{
	UINT32 appw;

	F7bDecodeFirstOperand(BitReadAM, 11);

	appw=f7bLen;
	F7BCREATEBITMASK(f7bLen);

	modWriteValW=(f7bOp1 >> bamOffset) & f7bLen;
	modWriteValW<<=32-appw;

	F7bWriteSecondOperand(2);

	F7BEND();
}

static UINT32 opSCHBS(UINT32 bSearch1)
{
	UINT32 i,data;
	UINT32 offset;

	F7bDecodeFirstOperand(BitReadAMAddress,10);

	// Read first UINT8
	f7bOp1 += bamOffset/8;
	data = MemRead8(f7bOp1);
	offset = bamOffset&7;

	// Scan bitstring
	for (i=0;i<f7bLen;i++)
	{
		// Update the work register
		R28 = f7bOp1;

		// There is a 0/1 at current offset?
		if ((bSearch1 && (data&(1<<offset))) ||
			(!bSearch1 && !(data&(1<<offset))))
			break;

		// Next bit please
		offset++;
		if (offset==8)
		{
			// Next UINT8 please
			offset=0;
			f7bOp1++;
			data = MemRead8(f7bOp1);
		}
	}

	// Set zero if bit not found
	_Z = (i == f7bLen);

	// Write to destination the final offset
	modWriteValW = i;
	F7bWriteSecondOperand(2);

	F7BEND();
}

static UINT32 opSCH0BSU(void) { return opSCHBS(0); }
static UINT32 opSCH1BSU(void) { return opSCHBS(1); }

static UINT32 opINSBFR(void)
{
	UINT32 appw;
	F7cDecodeOperands(ReadAM,2,BitReadAMAddress,11);

	F7CCREATEBITMASK(f7cLen);

	f7cOp2 += bamOffset/8;
	appw = MemRead32(f7cOp2);
	bamOffset &= 7;

	appw &= ~(f7cLen << bamOffset);
	appw |=  (f7cLen & f7cOp1) << bamOffset;

	MemWrite32(f7cOp2, appw);

	F7CEND();
}

static UINT32 opINSBFL(void)
{
	UINT32 appw;
	F7cDecodeOperands(ReadAM,2,BitReadAMAddress,11);

	f7cOp1 >>= (32-f7cLen);

	F7CCREATEBITMASK(f7cLen);

	f7cOp2 += bamOffset/8;
	appw = MemRead32(f7cOp2);
	bamOffset &= 7;

	appw &= ~(f7cLen << bamOffset);
	appw |=  (f7cLen & f7cOp1) << bamOffset;

	MemWrite32(f7cOp2, appw);

	F7CEND();
}

static UINT32 opMOVBSD(void)
{
	UINT32 i;
	UINT8 srcdata, dstdata;

	F7bDecodeOperands(BitReadAMAddress,10,BitReadAMAddress,10);

//  if (f7bLen!=1)
//  { int a=1; }

	f7bBamOffset1 += f7bLen-1;
	f7bBamOffset2 += f7bLen-1;

	f7bOp1 += f7bBamOffset1/8;
	f7bOp2 += f7bBamOffset2/8;

	f7bBamOffset1 &= 7;
	f7bBamOffset2 &= 7;

	srcdata = MemRead8(f7bOp1);
	dstdata = MemRead8(f7bOp2);

	for (i=0;i<f7bLen;i++)
	{
		// Update work registers
		R28 = f7bOp1;
		R27 = f7bOp2;

		dstdata &= ~(1 << f7bBamOffset2);
		dstdata |= ((srcdata >> f7bBamOffset1) & 1) << f7bBamOffset2;

		if (f7bBamOffset1 == 0)
		{
			f7bBamOffset1 = 8;
			f7bOp1--;
			srcdata = MemRead8(f7bOp1);
		}
		if (f7bBamOffset2 == 0)
		{
			MemWrite8(f7bOp2, dstdata);
			f7bBamOffset2 = 8;
			f7bOp2--;
			dstdata = MemRead8(f7bOp2);
		}

		f7bBamOffset1--;
		f7bBamOffset2--;
	}

	// Flush of the final data
	if (f7bBamOffset2 != 7)
		MemWrite8(f7bOp2, dstdata);

	F7BEND();
}

static UINT32 opMOVBSU(void)
{
	UINT32 i;
	UINT8 srcdata, dstdata;

	F7bDecodeOperands(BitReadAMAddress,10,BitReadAMAddress,10);

	f7bOp1 += f7bBamOffset1/8;
	f7bOp2 += f7bBamOffset2/8;

	f7bBamOffset1 &= 7;
	f7bBamOffset2 &= 7;

	srcdata = MemRead8(f7bOp1);
	dstdata = MemRead8(f7bOp2);

	for (i=0;i<f7bLen;i++)
	{
		// Update work registers
		R28 = f7bOp1;
		R27 = f7bOp2;

		dstdata &= ~(1 << f7bBamOffset2);
		dstdata |= ((srcdata >> f7bBamOffset1) & 1) << f7bBamOffset2;

		f7bBamOffset1++;
		f7bBamOffset2++;
		if (f7bBamOffset1 == 8)
		{
			f7bBamOffset1 = 0;
			f7bOp1++;
			srcdata = MemRead8(f7bOp1);
		}
		if (f7bBamOffset2 == 8)
		{
			MemWrite8(f7bOp2, dstdata);
			f7bBamOffset2 = 0;
			f7bOp2++;
			dstdata = MemRead8(f7bOp2);
		}
	}

	// Flush of the final data
	if (f7bBamOffset2 != 0)
		MemWrite8(f7bOp2, dstdata);

	F7BEND();
}

// RADM 0x20f4b8 holds the time left

static UINT32 opADDDC(void)
{
	UINT8 appb;
	UINT8 src, dst;

	F7cDecodeOperands(ReadAM, 0, ReadAMAddress, 0);

	if (f7cLen != 0)
	{
		logerror("ADDDC %x (pat: %x)\n", f7cOp1, f7cLen);
	}

	F7CLOADOP2BYTE(appb);

	src = (UINT8)(f7cOp1 >> 4) * 10 + (UINT8)(f7cOp1 & 0xF);
	dst = (appb >> 4) * 10 + (appb & 0xF);

	appb = src + dst + (_CY?1:0);

	if (appb >= 100)
	{
		appb -= 100;
		_CY = 1;
	}
	else
		_CY = 0;

	// compute z flag:
	// cleared if result non-zero or carry generated
	// unchanged otherwise
	if (appb != 0 || _CY)
		_Z = 0;

	appb = ((appb/10)<<4) | (appb % 10);

	F7CSTOREOP2BYTE();
	F7CEND();
}

static UINT32 opSUBDC(void)
{
	INT8 appb;
	UINT32 src, dst;

	F7cDecodeOperands(ReadAM, 0, ReadAMAddress, 0);

	if (f7cLen != 0)
	{
		logerror("SUBDC %x (pat: %x)\n", f7cOp1, f7cLen);
	}

	F7CLOADOP2BYTE(appb);

	src = (UINT32)(f7cOp1 >> 4) * 10 + (UINT32)(f7cOp1 & 0xF);
	dst = ((appb & 0xF0) >> 4) * 10 + (appb & 0xF);

	// Note that this APPB must be SIGNED!
	appb = (INT32)dst - (INT32)src - (_CY?1:0);

	if (appb < 0)
	{
		appb += 100;
		_CY = 1;
	}
	else
		_CY = 0;

	// compute z flag:
	// cleared if result non-zero or carry generated
	// unchanged otherwise
	if (appb != 0 || _CY)
		_Z = 0;

	appb = ((appb/10)<<4) | (appb % 10);

	F7CSTOREOP2BYTE();
	F7CEND();
}

static UINT32 opSUBRDC(void)
{
	INT8 appb;
	UINT32 src, dst;

	F7cDecodeOperands(ReadAM, 0, ReadAMAddress, 0);

	if (f7cLen != 0)
	{
		logerror("SUBRDC %x (pat: %x)\n", f7cOp1, f7cLen);
	}

	F7CLOADOP2BYTE(appb);

	src = (UINT32)(f7cOp1 >> 4) * 10 + (UINT32)(f7cOp1 & 0xF);
	dst = ((appb & 0xF0) >> 4) * 10 + (appb & 0xF);

	// Note that this APPB must be SIGNED!
	appb = (INT32)src - (INT32)dst - (_CY?1:0);

	if (appb < 0)
	{
		appb += 100;
		_CY = 1;
	}
	else
		_CY = 0;

	// compute z flag:
	// cleared if result non-zero or carry generated
	// unchanged otherwise
	if (appb != 0 || _CY)
		_Z = 0;

	appb = ((appb/10)<<4) | (appb % 10);

	F7CSTOREOP2BYTE();
	F7CEND();
}

static UINT32 opCVTDPZ(void)
{
	UINT16 apph;

	F7cDecodeOperands(ReadAM, 0, ReadAMAddress, 1);

	apph = (UINT16)(((f7cOp1 >> 4) & 0xF) | ((f7cOp1 & 0xF) << 8));
	apph |= (f7cLen);
	apph |= (f7cLen<<8);

	// Z flag is unchanged if src is zero, cleared otherwise
	if (f7cOp1 != 0) _Z = 0;

	F7CSTOREOP2HALF();
	F7CEND();
}

static UINT32 opCVTDZP(void)
{
	UINT8 appb;
	F7cDecodeOperands(ReadAM, 1, ReadAMAddress, 0);

	if ((f7cOp1 & 0xF0) != (f7cLen & 0xF0) || ((f7cOp1 >> 8) & 0xF0) != (f7cLen & 0xF0))
	{
		// Decimal exception
		logerror("CVTD.ZP Decimal exception #1!\n");
	}

	if ((f7cOp1 & 0xF) > 9 || ((f7cOp1 >> 8) & 0xF) > 9)
	{
		// Decimal exception
		logerror("CVTD.ZP Decimal exception #2!\n");
	}

	appb = (UINT8)(((f7cOp1 >> 8) & 0xF) | ((f7cOp1 & 0xF) << 4));
	if (appb != 0) _Z = 0;

	F7CSTOREOP2BYTE();
	F7CEND();
}

static UINT32 op58UNHANDLED(void)
{
	fatalerror("Unhandled 58 opcode at PC: /%06x", PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 op5AUNHANDLED(void)
{
	fatalerror("Unhandled 5A opcode at PC: /%06x", PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 op5BUNHANDLED(void)
{
	fatalerror("Unhandled 5B opcode at PC: /%06x", PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 op5DUNHANDLED(void)
{
	fatalerror("Unhandled 5D opcode at PC: /%06x", PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 op59UNHANDLED(void)
{
	fatalerror("Unhandled 59 opcode at PC: /%06x", PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 (*const Op59Table[32])(void) =
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


static UINT32 (*const Op5BTable[32])(void) =
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


static UINT32 (*const Op5DTable[32])(void) =
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

static UINT32 (*const Op58Table[32])(void) =
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

static UINT32 (*const Op5ATable[32])(void) =
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

static UINT32 op58(void)
{
	subOp = OpRead8(PC + 1);

	return Op58Table[subOp&0x1F]();
}

static UINT32 op5A(void)
{
	subOp = OpRead8(PC + 1);

	return Op5ATable[subOp&0x1F]();
}

static UINT32 op5B(void)
{
	subOp = OpRead8(PC + 1);

	return Op5BTable[subOp&0x1F]();
}

static UINT32 op5D(void)
{
	subOp = OpRead8(PC + 1);

	return Op5DTable[subOp&0x1F]();
}

static UINT32 op59(void)
{
	subOp = OpRead8(PC + 1);

	return Op59Table[subOp&0x1F]();
}
