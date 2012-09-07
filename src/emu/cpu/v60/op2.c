

#define F2END(cs) \
	return 2 + (cs)->amlength1 + (cs)->amlength2;

#define F2LOADOPFLOAT(cs, num)								\
	if ((cs)->flag##num)									\
		appf = u2f((cs)->reg[(cs)->op##num]);				\
	else													\
		appf = u2f((cs)->program->read_dword_unaligned((cs)->op##num));

#define F2STOREOPFLOAT(cs,num)								\
	if ((cs)->flag##num)									\
		(cs)->reg[(cs)->op##num] = f2u(appf);				\
	else													\
		(cs)->program->write_dword_unaligned((cs)->op##num, f2u(appf));

static void F2DecodeFirstOperand(v60_state *cpustate, UINT32 (*DecodeOp1)(v60_state *), UINT8 dim1)
{
	cpustate->moddim = dim1;
	cpustate->modm = cpustate->instflags & 0x40;
	cpustate->modadd = cpustate->PC + 2;
	cpustate->amlength1 = DecodeOp1(cpustate);
	cpustate->op1 = cpustate->amout;
	cpustate->flag1 = cpustate->amflag;
}

static void F2DecodeSecondOperand(v60_state *cpustate, UINT32 (*DecodeOp2)(v60_state *), UINT8 dim2)
{
	cpustate->moddim = dim2;
	cpustate->modm = cpustate->instflags & 0x20;
	cpustate->modadd = cpustate->PC + 2 + cpustate->amlength1;
	cpustate->amlength2 = DecodeOp2(cpustate);
	cpustate->op2 = cpustate->amout;
	cpustate->flag2 = cpustate->amflag;
}

static void F2WriteSecondOperand(v60_state *cpustate, UINT8 dim2)
{
	cpustate->moddim = dim2;
	cpustate->modm = cpustate->instflags & 0x20;
	cpustate->modadd = cpustate->PC + 2 + cpustate->amlength1;
	cpustate->amlength2 = WriteAM(cpustate);
}

static UINT32 opCVTWS(v60_state *cpustate)
{
	float val;

	F2DecodeFirstOperand(cpustate, ReadAM, 2);

	// Convert to float
	val = (float)(INT32)cpustate->op1;
	cpustate->modwritevalw = f2u(val);

	cpustate->_OV = 0;
	cpustate->_CY = (val < 0.0f);
	cpustate->_S = ((cpustate->modwritevalw & 0x80000000) != 0);
	cpustate->_Z = (val == 0.0f);

	F2WriteSecondOperand(cpustate, 2);
	F2END(cpustate);
}

static UINT32 opCVTSW(v60_state *cpustate)
{
	float val;

	F2DecodeFirstOperand(cpustate, ReadAM, 2);

	// Convert to UINT32
	val = u2f(cpustate->op1);
	cpustate->modwritevalw = (UINT32)val;

	cpustate->_OV = 0;
	cpustate->_CY =(val < 0.0f);
	cpustate->_S = ((cpustate->modwritevalw & 0x80000000) != 0);
	cpustate->_Z = (val == 0.0f);

	F2WriteSecondOperand(cpustate, 2);
	F2END(cpustate);
}

static UINT32 opMOVFS(v60_state *cpustate)
{
	F2DecodeFirstOperand(cpustate, ReadAM, 2);
	cpustate->modwritevalw = cpustate->op1;
	F2WriteSecondOperand(cpustate, 2);
	F2END(cpustate);
}

static UINT32 opNEGFS(v60_state *cpustate)
{
	float appf;

	F2DecodeFirstOperand(cpustate, ReadAM, 2);
	F2DecodeSecondOperand(cpustate, ReadAMAddress, 2);

	appf = -u2f(cpustate->op1);

	cpustate->_OV = 0;
	cpustate->_CY = (appf < 0.0f);
	cpustate->_S = ((f2u(appf) & 0x80000000) != 0);
	cpustate->_Z = (appf == 0.0f);

	F2STOREOPFLOAT(cpustate, 2);
	F2END(cpustate)
}

static UINT32 opABSFS(v60_state *cpustate)
{
	float appf;

	F2DecodeFirstOperand(cpustate, ReadAM, 2);
	F2DecodeSecondOperand(cpustate, ReadAMAddress, 2);

	appf = u2f(cpustate->op1);

	if(appf < 0)
		appf = -appf;

	cpustate->_OV = 0;
	cpustate->_CY = 0;
	cpustate->_S = ((f2u(appf) & 0x80000000) != 0);
	cpustate->_Z = (appf == 0.0f);

	F2STOREOPFLOAT(cpustate, 2);
	F2END(cpustate)
}

static UINT32 opADDFS(v60_state *cpustate)
{
	UINT32 appw;
	float appf;

	F2DecodeFirstOperand(cpustate, ReadAM, 2);
	F2DecodeSecondOperand(cpustate, ReadAMAddress, 2);

	F2LOADOPFLOAT(cpustate, 2);

	appf += u2f(cpustate->op1);

	appw = f2u(appf);
	cpustate->_OV = cpustate->_CY = 0;
	cpustate->_S = ((appw & 0x80000000) != 0);
	cpustate->_Z = (appw == 0);

	F2STOREOPFLOAT(cpustate, 2);
	F2END(cpustate)
}

static UINT32 opSUBFS(v60_state *cpustate)
{
	UINT32 appw;
	float appf;

	F2DecodeFirstOperand(cpustate, ReadAM, 2);
	F2DecodeSecondOperand(cpustate, ReadAMAddress, 2);

	F2LOADOPFLOAT(cpustate, 2);

	appf -= u2f(cpustate->op1);

	appw = f2u(appf);
	cpustate->_OV = cpustate->_CY = 0;
	cpustate->_S = ((appw & 0x80000000) != 0);
	cpustate->_Z = (appw == 0);

	F2STOREOPFLOAT(cpustate, 2);
	F2END(cpustate)
}

static UINT32 opMULFS(v60_state *cpustate)
{
	UINT32 appw;
	float appf;

	F2DecodeFirstOperand(cpustate, ReadAM, 2);
	F2DecodeSecondOperand(cpustate, ReadAMAddress, 2);

	F2LOADOPFLOAT(cpustate, 2);

	appf *= u2f(cpustate->op1);

	appw = f2u(appf);
	cpustate->_OV = cpustate->_CY = 0;
	cpustate->_S = ((appw & 0x80000000) != 0);
	cpustate->_Z = (appw == 0);

	F2STOREOPFLOAT(cpustate, 2);
	F2END(cpustate)
}

static UINT32 opDIVFS(v60_state *cpustate)
{
	UINT32 appw;
	float appf;

	F2DecodeFirstOperand(cpustate, ReadAM, 2);
	F2DecodeSecondOperand(cpustate, ReadAMAddress, 2);

	F2LOADOPFLOAT(cpustate, 2);

	appf /= u2f(cpustate->op1);

	appw = f2u(appf);
	cpustate->_OV = cpustate->_CY = 0;
	cpustate->_S = ((appw & 0x80000000) != 0);
	cpustate->_Z = (appw == 0);

	F2STOREOPFLOAT(cpustate, 2);
	F2END(cpustate)
}

static UINT32 opSCLFS(v60_state *cpustate)
{
	UINT32 appw;
	float appf;

	F2DecodeFirstOperand(cpustate, ReadAM, 1);
	F2DecodeSecondOperand(cpustate, ReadAMAddress, 2);

	F2LOADOPFLOAT(cpustate, 2);

	if ((INT16)cpustate->op1 < 0)
		appf /= 1 << -(INT16)cpustate->op1;
	else
		appf *= 1 << cpustate->op1;

	appw = f2u(appf);
	cpustate->_OV = cpustate->_CY = 0;
	cpustate->_S = ((appw & 0x80000000) != 0);
	cpustate->_Z = (appw == 0);

	F2STOREOPFLOAT(cpustate, 2);
	F2END(cpustate)
}

static UINT32 opCMPF(v60_state *cpustate)
{
	float appf;

	F2DecodeFirstOperand(cpustate, ReadAM, 2);
	F2DecodeSecondOperand(cpustate, ReadAM, 2);

	appf = u2f(cpustate->op2) - u2f(cpustate->op1);

	cpustate->_Z = (appf == 0);
	cpustate->_S = (appf < 0);
	cpustate->_OV = 0;
	cpustate->_CY = 0;

	F2END(cpustate);
}

static UINT32 op5FUNHANDLED(v60_state *cpustate)
{
	fatalerror("Unhandled 5F opcode at %08x\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 op5CUNHANDLED(v60_state *cpustate)
{
	fatalerror("Unhandled 5C opcode at %08x\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 (*const Op5FTable[32])(v60_state *) =
{
	opCVTWS,
	opCVTSW,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED
};

static UINT32 (*const Op5CTable[32])(v60_state *) =
{
	opCMPF,
	op5CUNHANDLED,
	op5CUNHANDLED,
	op5CUNHANDLED,
	op5CUNHANDLED,
	op5CUNHANDLED,
	op5CUNHANDLED,
	op5CUNHANDLED,
	opMOVFS,
	opNEGFS,
	opABSFS,
	op5CUNHANDLED,
	op5CUNHANDLED,
	op5CUNHANDLED,
	op5CUNHANDLED,
	op5CUNHANDLED,

	opSCLFS,
	op5CUNHANDLED,
	op5CUNHANDLED,
	op5CUNHANDLED,
	op5CUNHANDLED,
	op5CUNHANDLED,
	op5CUNHANDLED,
	op5CUNHANDLED,
	opADDFS,
	opSUBFS,
	opMULFS,
	opDIVFS,
	op5CUNHANDLED,
	op5CUNHANDLED,
	op5CUNHANDLED,
	op5CUNHANDLED
};


static UINT32 op5F(v60_state *cpustate)
{
	cpustate->instflags = OpRead8(cpustate, cpustate->PC + 1);
	return Op5FTable[cpustate->instflags & 0x1F](cpustate);
}


static UINT32 op5C(v60_state *cpustate)
{
	cpustate->instflags = OpRead8(cpustate, cpustate->PC + 1);
	return Op5CTable[cpustate->instflags & 0x1F](cpustate);
}
