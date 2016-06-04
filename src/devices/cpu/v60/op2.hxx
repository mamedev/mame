// license:BSD-3-Clause
// copyright-holders:Farfetch'd, R. Belmont

#define F2END() \
	return 2 + m_amlength1 + m_amlength2;

#define F2LOADOPFLOAT(num)                              \
	if (m_flag##num)                                    \
		appf = u2f(m_reg[m_op##num]);               \
	else                                                    \
		appf = u2f(m_program->read_dword_unaligned(m_op##num));

#define F2STOREOPFLOAT(num)                              \
	if (m_flag##num)                                    \
		m_reg[m_op##num] = f2u(appf);               \
	else                                                    \
		m_program->write_dword_unaligned(m_op##num, f2u(appf));

void v60_device::F2DecodeFirstOperand(am_func DecodeOp1, UINT8 dim1)
{
	m_moddim = dim1;
	m_modm = m_instflags & 0x40;
	m_modadd = PC + 2;
	m_amlength1 = (this->*DecodeOp1)();
	m_op1 = m_amout;
	m_flag1 = m_amflag;
}

void v60_device::F2DecodeSecondOperand(am_func DecodeOp2, UINT8 dim2)
{
	m_moddim = dim2;
	m_modm = m_instflags & 0x20;
	m_modadd = PC + 2 + m_amlength1;
	m_amlength2 = (this->*DecodeOp2)();
	m_op2 = m_amout;
	m_flag2 = m_amflag;
}

void v60_device::F2WriteSecondOperand(UINT8 dim2)
{
	m_moddim = dim2;
	m_modm = m_instflags & 0x20;
	m_modadd = PC + 2 + m_amlength1;
	m_amlength2 = WriteAM();
}

UINT32 v60_device::opCVTWS()
{
	float val;

	F2DecodeFirstOperand(&v60_device::ReadAM, 2);

	// Convert to float
	val = (float)(INT32)m_op1;
	m_modwritevalw = f2u(val);

	_OV = 0;
	_CY = (val < 0.0f);
	_S = ((m_modwritevalw & 0x80000000) != 0);
	_Z = (val == 0.0f);

	F2WriteSecondOperand(2);
	F2END();
}

UINT32 v60_device::opCVTSW()
{
	float val;

	F2DecodeFirstOperand(&v60_device::ReadAM, 2);

	// Convert to UINT32
	val = u2f(m_op1);
	m_modwritevalw = (UINT32)val;

	_OV = 0;
	_CY =(val < 0.0f);
	_S = ((m_modwritevalw & 0x80000000) != 0);
	_Z = (val == 0.0f);

	F2WriteSecondOperand(2);
	F2END();
}

UINT32 v60_device::opMOVFS()
{
	F2DecodeFirstOperand(&v60_device::ReadAM, 2);
	m_modwritevalw = m_op1;
	F2WriteSecondOperand(2);
	F2END();
}

UINT32 v60_device::opNEGFS()
{
	float appf;

	F2DecodeFirstOperand(&v60_device::ReadAM, 2);
	F2DecodeSecondOperand(&v60_device::ReadAMAddress, 2);

	appf = -u2f(m_op1);

	_OV = 0;
	_CY = (appf < 0.0f);
	_S = ((f2u(appf) & 0x80000000) != 0);
	_Z = (appf == 0.0f);

	F2STOREOPFLOAT(2);
	F2END()
}

UINT32 v60_device::opABSFS()
{
	float appf;

	F2DecodeFirstOperand(&v60_device::ReadAM, 2);
	F2DecodeSecondOperand(&v60_device::ReadAMAddress, 2);

	appf = u2f(m_op1);

	if(appf < 0)
		appf = -appf;

	_OV = 0;
	_CY = 0;
	_S = ((f2u(appf) & 0x80000000) != 0);
	_Z = (appf == 0.0f);

	F2STOREOPFLOAT(2);
	F2END()
}

UINT32 v60_device::opADDFS()
{
	UINT32 appw;
	float appf;

	F2DecodeFirstOperand(&v60_device::ReadAM, 2);
	F2DecodeSecondOperand(&v60_device::ReadAMAddress, 2);

	F2LOADOPFLOAT(2);

	appf += u2f(m_op1);

	appw = f2u(appf);
	_OV = _CY = 0;
	_S = ((appw & 0x80000000) != 0);
	_Z = (appw == 0);

	F2STOREOPFLOAT(2);
	F2END()
}

UINT32 v60_device::opSUBFS()
{
	UINT32 appw;
	float appf;

	F2DecodeFirstOperand(&v60_device::ReadAM, 2);
	F2DecodeSecondOperand(&v60_device::ReadAMAddress, 2);

	F2LOADOPFLOAT(2);

	appf -= u2f(m_op1);

	appw = f2u(appf);
	_OV = _CY = 0;
	_S = ((appw & 0x80000000) != 0);
	_Z = (appw == 0);

	F2STOREOPFLOAT(2);
	F2END()
}

UINT32 v60_device::opMULFS()
{
	UINT32 appw;
	float appf;

	F2DecodeFirstOperand(&v60_device::ReadAM, 2);
	F2DecodeSecondOperand(&v60_device::ReadAMAddress, 2);

	F2LOADOPFLOAT(2);

	appf *= u2f(m_op1);

	appw = f2u(appf);
	_OV = _CY = 0;
	_S = ((appw & 0x80000000) != 0);
	_Z = (appw == 0);

	F2STOREOPFLOAT(2);
	F2END()
}

UINT32 v60_device::opDIVFS()
{
	UINT32 appw;
	float appf;

	F2DecodeFirstOperand(&v60_device::ReadAM, 2);
	F2DecodeSecondOperand(&v60_device::ReadAMAddress, 2);

	F2LOADOPFLOAT(2);

	appf /= u2f(m_op1);

	appw = f2u(appf);
	_OV = _CY = 0;
	_S = ((appw & 0x80000000) != 0);
	_Z = (appw == 0);

	F2STOREOPFLOAT(2);
	F2END()
}

UINT32 v60_device::opSCLFS()
{
	UINT32 appw;
	float appf;

	F2DecodeFirstOperand(&v60_device::ReadAM, 1);
	F2DecodeSecondOperand(&v60_device::ReadAMAddress, 2);

	F2LOADOPFLOAT(2);

	if ((INT16)m_op1 < 0)
		appf /= 1 << -(INT16)m_op1;
	else
		appf *= 1 << m_op1;

	appw = f2u(appf);
	_OV = _CY = 0;
	_S = ((appw & 0x80000000) != 0);
	_Z = (appw == 0);

	F2STOREOPFLOAT(2);
	F2END()
}

UINT32 v60_device::opCMPF()
{
	float appf;

	F2DecodeFirstOperand(&v60_device::ReadAM, 2);
	F2DecodeSecondOperand(&v60_device::ReadAM, 2);

	appf = u2f(m_op2) - u2f(m_op1);

	_Z = (appf == 0);
	_S = (appf < 0);
	_OV = 0;
	_CY = 0;

	F2END();
}

UINT32 v60_device::op5FUNHANDLED()
{
	fatalerror("Unhandled 5F opcode at %08x\n", PC);
	return 0; /* never reached, fatalerror won't return */
}

UINT32 v60_device::op5CUNHANDLED()
{
	fatalerror("Unhandled 5C opcode at %08x\n", PC);
	return 0; /* never reached, fatalerror won't return */
}

const v60_device::am_func v60_device::s_Op5FTable[32] =
{
	&v60_device::opCVTWS,
	&v60_device::opCVTSW,
	&v60_device::op5FUNHANDLED,
	&v60_device::op5FUNHANDLED,
	&v60_device::op5FUNHANDLED,
	&v60_device::op5FUNHANDLED,
	&v60_device::op5FUNHANDLED,
	&v60_device::op5FUNHANDLED,
	&v60_device::op5FUNHANDLED,
	&v60_device::op5FUNHANDLED,
	&v60_device::op5FUNHANDLED,
	&v60_device::op5FUNHANDLED,
	&v60_device::op5FUNHANDLED,
	&v60_device::op5FUNHANDLED,
	&v60_device::op5FUNHANDLED,
	&v60_device::op5FUNHANDLED,
	&v60_device::op5FUNHANDLED,
	&v60_device::op5FUNHANDLED,
	&v60_device::op5FUNHANDLED,
	&v60_device::op5FUNHANDLED,
	&v60_device::op5FUNHANDLED,
	&v60_device::op5FUNHANDLED,
	&v60_device::op5FUNHANDLED,
	&v60_device::op5FUNHANDLED,
	&v60_device::op5FUNHANDLED,
	&v60_device::op5FUNHANDLED,
	&v60_device::op5FUNHANDLED,
	&v60_device::op5FUNHANDLED,
	&v60_device::op5FUNHANDLED,
	&v60_device::op5FUNHANDLED,
	&v60_device::op5FUNHANDLED,
	&v60_device::op5FUNHANDLED
};

const v60_device::am_func v60_device::s_Op5CTable[32] =
{
	&v60_device::opCMPF,
	&v60_device::op5CUNHANDLED,
	&v60_device::op5CUNHANDLED,
	&v60_device::op5CUNHANDLED,
	&v60_device::op5CUNHANDLED,
	&v60_device::op5CUNHANDLED,
	&v60_device::op5CUNHANDLED,
	&v60_device::op5CUNHANDLED,
	&v60_device::opMOVFS,
	&v60_device::opNEGFS,
	&v60_device::opABSFS,
	&v60_device::op5CUNHANDLED,
	&v60_device::op5CUNHANDLED,
	&v60_device::op5CUNHANDLED,
	&v60_device::op5CUNHANDLED,
	&v60_device::op5CUNHANDLED,

	&v60_device::opSCLFS,
	&v60_device::op5CUNHANDLED,
	&v60_device::op5CUNHANDLED,
	&v60_device::op5CUNHANDLED,
	&v60_device::op5CUNHANDLED,
	&v60_device::op5CUNHANDLED,
	&v60_device::op5CUNHANDLED,
	&v60_device::op5CUNHANDLED,
	&v60_device::opADDFS,
	&v60_device::opSUBFS,
	&v60_device::opMULFS,
	&v60_device::opDIVFS,
	&v60_device::op5CUNHANDLED,
	&v60_device::op5CUNHANDLED,
	&v60_device::op5CUNHANDLED,
	&v60_device::op5CUNHANDLED
};


UINT32 v60_device::op5F()
{
	m_instflags = OpRead8(PC + 1);
	return (this->*s_Op5FTable[m_instflags & 0x1F])();
}


UINT32 v60_device::op5C()
{
	m_instflags = OpRead8(PC + 1);
	return (this->*s_Op5CTable[m_instflags & 0x1F])();
}
