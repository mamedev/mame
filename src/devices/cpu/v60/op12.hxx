// license:BSD-3-Clause
// copyright-holders:Farfetch'd, R. Belmont
/*
 * MUL* and MULU* do not set OV correctly
 * DIVX: the second operand should be treated as dword instead of word
 * GETATE, GETPTE and GETRA should not be used
 * UPDPSW: _CY and _OV must be cleared or unchanged? I suppose
 *   cleared, like TEST being done on the mask operand.
 * MOVT: I cannot understand exactly what happens to the result
 *   when an overflow occurs
 *
 * Unimplemented opcodes:
 * ROTC, UPDATE, UPDPTE
 */


/*
 *  Macro to access data in operands decoded with ReadAMAddress()
 */

#define F12LOADOPBYTE(num)                          \
	if (m_flag##num)                                \
		appb = (UINT8)m_reg[m_op##num];         \
	else                                                \
		appb = m_program->read_byte(m_op##num);

#define F12LOADOPHALF(num)                          \
	if (m_flag##num)                                \
		apph = (UINT16)m_reg[m_op##num];        \
	else                                                \
		apph = m_program->read_word_unaligned(m_op##num);

#define F12LOADOPWORD(num)                          \
	if (m_flag##num)                                \
		appw = m_reg[m_op##num];                \
	else                                                \
		appw = m_program->read_dword_unaligned(m_op##num);

#define F12STOREOPBYTE(num)                         \
	if (m_flag##num)                                \
		SETREG8(m_reg[m_op##num], appb);        \
	else                                                \
		m_program->write_byte(m_op##num, appb);

#define F12STOREOPHALF(num)                         \
	if (m_flag##num)                                \
		SETREG16(m_reg[m_op##num], apph);       \
	else                                                \
		m_program->write_word_unaligned(m_op##num, apph);

#define F12STOREOPWORD(num)                         \
	if (m_flag##num)                                \
		m_reg[m_op##num] = appw;                \
	else                                                \
		m_program->write_dword_unaligned(m_op##num, appw);

#define F12LOADOP1BYTE()  F12LOADOPBYTE(1)
#define F12LOADOP1HALF()  F12LOADOPHALF(1)
#define F12LOADOP1WORD()  F12LOADOPWORD(1)

#define F12LOADOP2BYTE()  F12LOADOPBYTE(2)
#define F12LOADOP2HALF()  F12LOADOPHALF(2)
#define F12LOADOP2WORD()  F12LOADOPWORD(2)

#define F12STOREOP1BYTE()  F12STOREOPBYTE(1)
#define F12STOREOP1HALF()  F12STOREOPHALF(1)
#define F12STOREOP1WORD()  F12STOREOPWORD(1)

#define F12STOREOP2BYTE()  F12STOREOPBYTE(2)
#define F12STOREOP2HALF()  F12STOREOPHALF(2)
#define F12STOREOP2WORD()  F12STOREOPWORD(2)

#define F12END()                                  \
	return m_amlength1 + m_amlength2 + 2;


// Decode the first operand of the instruction and prepare
// writing to the second operand.
void v60_device::F12DecodeFirstOperand(am_func DecodeOp1, UINT8 dim1)
{
	m_instflags = OpRead8(PC + 1);

	// Check if F1 or F2
	if (m_instflags & 0x80)
	{
		m_moddim = dim1;
		m_modm = m_instflags & 0x40;
		m_modadd = PC + 2;
		m_amlength1 = (this->*DecodeOp1)();
		m_op1 = m_amout;
		m_flag1 = m_amflag;
	}
	else
	{
		// Check D flag
		if (m_instflags & 0x20)
		{
			m_moddim = dim1;
			m_modm = m_instflags & 0x40;
			m_modadd = PC + 2;
			m_amlength1 = (this->*DecodeOp1)();
			m_op1 = m_amout;
			m_flag1 = m_amflag;
		}
		else
		{
			if (DecodeOp1 == &v60_device::ReadAM)
			{
				switch (dim1)
				{
				case 0:
					m_op1 = (UINT8)m_reg[m_instflags & 0x1F];
					break;
				case 1:
					m_op1 = (UINT16)m_reg[m_instflags & 0x1F];
					break;
				case 2:
					m_op1 = m_reg[m_instflags & 0x1F];
					break;
				}

				m_flag1 = 0;
			}
			else
			{
				m_flag1 = 1;
				m_op1 = m_instflags & 0x1F;
			}

			m_amlength1 = 0;
		}
	}
}

void v60_device::F12WriteSecondOperand(UINT8 dim2)
{
	m_moddim = dim2;

	// Check if F1 or F2
	if (m_instflags & 0x80)
	{
		m_modm = m_instflags & 0x20;
		m_modadd = PC + 2 + m_amlength1;
		m_moddim = dim2;
		m_amlength2 = WriteAM();
	}
	else
	{
		// Check D flag
		if (m_instflags & 0x20)
		{
			switch (dim2)
			{
			case 0:
				SETREG8(m_reg[m_instflags & 0x1F], m_modwritevalb);
				break;
			case 1:
				SETREG16(m_reg[m_instflags & 0x1F], m_modwritevalh);
				break;
			case 2:
				m_reg[m_instflags & 0x1F] = m_modwritevalw;
				break;
			}

			m_amlength2 = 0;
		}
		else
		{
			m_modm = m_instflags & 0x40;
			m_modadd = PC + 2;
			m_moddim = dim2;
			m_amlength2 = WriteAM();
		}
	}
}



// Decode both format 1 / 2 operands
void v60_device::F12DecodeOperands(am_func DecodeOp1, UINT8 dim1, am_func DecodeOp2, UINT8 dim2)
{
	UINT8 _if12 = OpRead8(PC + 1);

	// Check if F1 or F2
	if (_if12 & 0x80)
	{
		m_moddim = dim1;
		m_modm = _if12 & 0x40;
		m_modadd = PC + 2;
		m_amlength1 = (this->*DecodeOp1)();
		m_op1 = m_amout;
		m_flag1 = m_amflag;

		m_moddim = dim2;
		m_modm = _if12 & 0x20;
		m_modadd = PC + 2 + m_amlength1;
		m_amlength2 = (this->*DecodeOp2)();
		m_op2 = m_amout;
		m_flag2 = m_amflag;
	}
	else
	{
		// Check D flag
		if (_if12 & 0x20)
		{
			if (DecodeOp2 == &v60_device::ReadAMAddress)
			{
				m_op2 = _if12 & 0x1F;
				m_flag2 = 1;
			}
			else
			{
				switch (dim2)
				{
				case 0:
					m_op2 = (UINT8)m_reg[_if12 & 0x1F];
					break;
				case 1:
					m_op2 = (UINT16)m_reg[_if12 & 0x1F];
					break;
				case 2:
					m_op2 = m_reg[_if12 & 0x1F];
					break;
				}
			}

			m_amlength2 = 0;

			m_moddim = dim1;
			m_modm = _if12 & 0x40;
			m_modadd = PC + 2;
			m_amlength1 = (this->*DecodeOp1)();
			m_op1 = m_amout;
			m_flag1 = m_amflag;
		}
		else
		{
			if (DecodeOp1 == &v60_device::ReadAMAddress)
			{
				m_op1 = _if12 & 0x1F;
				m_flag1 = 1;
			}
			else
			{
				switch (dim1)
				{
				case 0:
					m_op1 = (UINT8)m_reg[_if12 & 0x1F];
					break;
				case 1:
					m_op1 = (UINT16)m_reg[_if12 & 0x1F];
					break;
				case 2:
					m_op1 = m_reg[_if12 & 0x1F];
					break;
				}
			}
			m_amlength1 = 0;

			m_moddim = dim2;
			m_modm = _if12 & 0x40;
			m_modadd = PC + 2 + m_amlength1;
			m_amlength2 = (this->*DecodeOp2)();
			m_op2 = m_amout;
			m_flag2 = m_amflag;
		}
	}
}

UINT32 v60_device::opADDB() /* TRUSTED (C too!)*/
{
	UINT8 appb;
	F12DecodeOperands(&v60_device::ReadAM, 0,&v60_device::ReadAMAddress, 0);

	F12LOADOP2BYTE();

	ADDB(appb, (UINT8)m_op1);

	F12STOREOP2BYTE();
	F12END();
}

UINT32 v60_device::opADDH() /* TRUSTED (C too!)*/
{
	UINT16 apph;
	F12DecodeOperands(&v60_device::ReadAM, 1,&v60_device::ReadAMAddress, 1);

	F12LOADOP2HALF();

	ADDW(apph, (UINT16)m_op1);

	F12STOREOP2HALF();
	F12END();
}

UINT32 v60_device::opADDW() /* TRUSTED (C too!) */
{
	UINT32 appw;
	F12DecodeOperands(&v60_device::ReadAM, 2,&v60_device::ReadAMAddress, 2);

	F12LOADOP2WORD();

	ADDL(appw, (UINT32)m_op1);

	F12STOREOP2WORD();
	F12END();
}

UINT32 v60_device::opADDCB()
{
	UINT8 appb, temp;

	F12DecodeOperands(&v60_device::ReadAM, 0,&v60_device::ReadAMAddress, 0);

	F12LOADOP2BYTE();

	temp = ((UINT8)m_op1 + (_CY?1:0));
	ADDB(appb, temp);

	F12STOREOP2BYTE();
	F12END();
}

UINT32 v60_device::opADDCH()
{
	UINT16 apph, temp;

	F12DecodeOperands(&v60_device::ReadAM, 1,&v60_device::ReadAMAddress, 1);

	F12LOADOP2HALF();

	temp = ((UINT16)m_op1 + (_CY?1:0));
	ADDW(apph, temp);

	F12STOREOP2HALF();
	F12END();
}

UINT32 v60_device::opADDCW()
{
	UINT32 appw, temp;

	F12DecodeOperands(&v60_device::ReadAM, 2,&v60_device::ReadAMAddress, 2);

	F12LOADOP2WORD();

	temp = m_op1 + (_CY?1:0);
	ADDL(appw, temp);

	F12STOREOP2WORD();
	F12END();
}

UINT32 v60_device::opANDB() /* TRUSTED */
{
	UINT8 appb;
	F12DecodeOperands(&v60_device::ReadAM, 0,&v60_device::ReadAMAddress, 0);

	F12LOADOP2BYTE();

	appb &= m_op1;
	_OV = 0;
	_S = ((appb & 0x80) != 0);
	_Z = (appb == 0);

	F12STOREOP2BYTE();
	F12END();
}

UINT32 v60_device::opANDH() /* TRUSTED */
{
	UINT16 apph;
	F12DecodeOperands(&v60_device::ReadAM, 1,&v60_device::ReadAMAddress, 1);

	F12LOADOP2HALF();

	apph &= m_op1;
	_OV = 0;
	_S = ((apph & 0x8000) != 0);
	_Z = (apph == 0);

	F12STOREOP2HALF();
	F12END();
}

UINT32 v60_device::opANDW() /* TRUSTED */
{
	UINT32 appw;
	F12DecodeOperands(&v60_device::ReadAM, 2,&v60_device::ReadAMAddress, 2);

	F12LOADOP2WORD();

	appw &= m_op1;
	_OV = 0;
	_S = ((appw & 0x80000000) != 0);
	_Z = (appw == 0);

	F12STOREOP2WORD();
	F12END();
}

UINT32 v60_device::opCALL() /* TRUSTED */
{
	F12DecodeOperands(&v60_device::ReadAMAddress, 0,&v60_device::ReadAMAddress, 2);

	SP -= 4;
	m_program->write_dword_unaligned(SP, AP);
	AP = m_op2;

	SP -= 4;
	m_program->write_dword_unaligned(SP, PC + m_amlength1 + m_amlength2 + 2);
	PC = m_op1;

	return 0;
}

UINT32 v60_device::opCHKAR()
{
	F12DecodeOperands(&v60_device::ReadAM, 0,&v60_device::ReadAM, 0);

	// No MMU and memory permissions yet @@@
	_Z = 1;
	_CY = 0;
	_S = 0;

	F12END();
}

UINT32 v60_device::opCHKAW()
{
	F12DecodeOperands(&v60_device::ReadAM, 0,&v60_device::ReadAM, 0);

	// No MMU and memory permissions yet @@@
	_Z = 1;
	_CY = 0;
	_S = 0;

	F12END();
}

UINT32 v60_device::opCHKAE()
{
	F12DecodeOperands(&v60_device::ReadAM, 0,&v60_device::ReadAM, 0);

	// No MMU and memory permissions yet @@@
	_Z = 1;
	_CY = 0;
	_S = 0;

	F12END();
}

UINT32 v60_device::opCHLVL()
{
	UINT32 oldPSW;

	F12DecodeOperands(&v60_device::ReadAM, 0,&v60_device::ReadAM, 0);

	if (m_op1 > 3)
	{
		fatalerror("Illegal data field on opCHLVL, PC=%x\n", PC);
	}

	oldPSW = v60_update_psw_for_exception(0, m_op1);

	SP -= 4;
	m_program->write_dword_unaligned(SP, m_op2);

	SP -= 4;
	m_program->write_dword_unaligned(SP, EXCEPTION_CODE_AND_SIZE(0x1800 + m_op1 * 0x100, 8));

	SP -= 4;
	m_program->write_dword_unaligned(SP, oldPSW);

	SP -= 4;
	m_program->write_dword_unaligned(SP, PC + m_amlength1 + m_amlength2 + 2);

	PC = GETINTVECT(24 + m_op1);

	return 0;
}

UINT32 v60_device::opCLR1() /* TRUSTED */
{
	UINT32 appw;
	F12DecodeOperands(&v60_device::ReadAM, 2,&v60_device::ReadAMAddress, 2);

	F12LOADOP2WORD();

	_CY = ((appw & (1 << m_op1)) != 0);
	_Z = !(_CY);

	appw &= ~(1 << m_op1);

	F12STOREOP2WORD();
	F12END();
}

UINT32 v60_device::opCMPB() /* TRUSTED (C too!) */
{
	UINT8 appb;
	F12DecodeOperands(&v60_device::ReadAM, 0,&v60_device::ReadAM, 0);

	appb = (UINT8)m_op2;
	SUBB(appb, (UINT8)m_op1);

	F12END();
}

UINT32 v60_device::opCMPH() /* TRUSTED (C too!) */
{
	UINT16 apph;
	F12DecodeOperands(&v60_device::ReadAM, 1,&v60_device::ReadAM, 1);

	apph = (UINT16)m_op2;
	SUBW(apph, (UINT16)m_op1);

	F12END();
}


UINT32 v60_device::opCMPW() /* TRUSTED (C too!)*/
{
	F12DecodeOperands(&v60_device::ReadAM, 2,&v60_device::ReadAM, 2);

	SUBL(m_op2, (UINT32)m_op1);

	F12END();
}

UINT32 v60_device::opDIVB() /* TRUSTED */
{
	UINT8 appb;
	F12DecodeOperands(&v60_device::ReadAM, 0,&v60_device::ReadAMAddress, 0);

	F12LOADOP2BYTE();

	_OV = ((appb == 0x80) && (m_op1 == 0xFF));
	if (m_op1 && !_OV)
		appb= (INT8)appb / (INT8)m_op1;
	_Z = (appb == 0);
	_S = ((appb & 0x80) != 0);

	F12STOREOP2BYTE();
	F12END();
}

UINT32 v60_device::opDIVH() /* TRUSTED */
{
	UINT16 apph;
	F12DecodeOperands(&v60_device::ReadAM, 1,&v60_device::ReadAMAddress, 1);

	F12LOADOP2HALF();

	_OV = ((apph == 0x8000) && (m_op1 == 0xFFFF));
	if (m_op1 && !_OV)
		apph = (INT16)apph / (INT16)m_op1;
	_Z = (apph == 0);
	_S = ((apph & 0x8000) != 0);

	F12STOREOP2HALF();
	F12END();
}

UINT32 v60_device::opDIVW() /* TRUSTED */
{
	UINT32 appw;
	F12DecodeOperands(&v60_device::ReadAM, 2,&v60_device::ReadAMAddress, 2);

	F12LOADOP2WORD();

	_OV = ((appw == 0x80000000) && (m_op1 == 0xFFFFFFFF));
	if (m_op1 && !_OV)
		appw = (INT32)appw / (INT32)m_op1;
	_Z = (appw == 0);
	_S = ((appw & 0x80000000) != 0);

	F12STOREOP2WORD();
	F12END();
}

UINT32 v60_device::opDIVX()
{
	UINT32 a, b;
	INT64 dv;

	F12DecodeOperands(&v60_device::ReadAM, 2,&v60_device::ReadAMAddress, 3);

	if (m_flag2)
	{
		a = m_reg[m_op2 & 0x1F];
		b = m_reg[(m_op2 & 0x1F) + 1];
	}
	else
	{
		a = m_program->read_dword_unaligned(m_op2);
		b = m_program->read_dword_unaligned(m_op2 + 4);
	}

	dv = ((UINT64)b << 32) | ((UINT64)a);

	a = dv / (INT64)((INT32)m_op1);
	b = dv % (INT64)((INT32)m_op1);

	_S = ((a & 0x80000000) != 0);
	_Z = (a == 0);

	if (m_flag2)
	{
		m_reg[m_op2 & 0x1F] = a;
		m_reg[(m_op2 & 0x1F) + 1] = b;
	}
	else
	{
		m_program->write_dword_unaligned(m_op2, a);
		m_program->write_dword_unaligned(m_op2 + 4, b);
	}

	F12END();
}

UINT32 v60_device::opDIVUX()
{
	UINT32 a, b;
	UINT64 dv;

	F12DecodeOperands(&v60_device::ReadAM, 2,&v60_device::ReadAMAddress, 3);

	if (m_flag2)
	{
		a = m_reg[m_op2 & 0x1F];
		b = m_reg[(m_op2 & 0x1F) + 1];
	}
	else
	{
		a = m_program->read_dword_unaligned(m_op2);
		b = m_program->read_dword_unaligned(m_op2 + 4);
	}

	dv = (UINT64)(((UINT64)b << 32) | (UINT64)a);
	a = (UINT32)(dv / (UINT64)m_op1);
	b = (UINT32)(dv % (UINT64)m_op1);

	_S = ((a & 0x80000000) != 0);
	_Z = (a == 0);

	if (m_flag2)
	{
		m_reg[m_op2 & 0x1F] = a;
		m_reg[(m_op2 & 0x1F) + 1] = b;
	}
	else
	{
		m_program->write_dword_unaligned(m_op2, a);
		m_program->write_dword_unaligned(m_op2 + 4, b);
	}

	F12END();
}


UINT32 v60_device::opDIVUB() /* TRUSTED */
{
	UINT8 appb;
	F12DecodeOperands(&v60_device::ReadAM, 0,&v60_device::ReadAMAddress, 0);

	F12LOADOP2BYTE();

	_OV = 0;
	if (m_op1)  appb /= (UINT8)m_op1;
	_Z = (appb == 0);
	_S = ((appb & 0x80) != 0);

	F12STOREOP2BYTE();
	F12END();
}

UINT32 v60_device::opDIVUH() /* TRUSTED */
{
	UINT16 apph;
	F12DecodeOperands(&v60_device::ReadAM, 1,&v60_device::ReadAMAddress, 1);

	F12LOADOP2HALF();

	_OV = 0;
	if (m_op1)  apph /= (UINT16)m_op1;
	_Z = (apph == 0);
	_S = ((apph & 0x8000) != 0);

	F12STOREOP2HALF();
	F12END();
}

UINT32 v60_device::opDIVUW() /* TRUSTED */
{
	UINT32 appw;
	F12DecodeOperands(&v60_device::ReadAM, 2,&v60_device::ReadAMAddress, 2);

	F12LOADOP2WORD();

	_OV = 0;
	if (m_op1)  appw /= m_op1;
	_Z = (appw == 0);
	_S = ((appw & 0x80000000) != 0);

	F12STOREOP2WORD();
	F12END();
}

UINT32 v60_device::opINB()
{
	F12DecodeFirstOperand(&v60_device::ReadAMAddress, 0);
	m_modwritevalb = m_io->read_byte(m_op1);

	if ( m_stall_io )
	{
		m_stall_io = 0;
		return 0;
	}

	F12WriteSecondOperand(0);
	F12END();
}

UINT32 v60_device::opINH()
{
	F12DecodeFirstOperand(&v60_device::ReadAMAddress, 1);
	m_modwritevalh = m_io->read_word_unaligned(m_op1);

	if ( m_stall_io )
	{
		m_stall_io = 0;
		return 0;
	}

	F12WriteSecondOperand(1);
	F12END();
}

UINT32 v60_device::opINW()
{
	F12DecodeFirstOperand(&v60_device::ReadAMAddress, 2);
	m_modwritevalw = m_io->read_dword_unaligned(m_op1);

	if ( m_stall_io )
	{
		m_stall_io = 0;
		return 0;
	}

	F12WriteSecondOperand(2);
	F12END();
}

UINT32 v60_device::opLDPR()
{
	F12DecodeOperands(&v60_device::ReadAMAddress, 2,&v60_device::ReadAM, 2);
	if (m_op2 <= 28)
	{
		if (m_flag1 &&(!(OpRead8(PC + 1)&0x80 && OpRead8(PC + 2) == 0xf4 ) ))
			m_reg[m_op2 + 36] = m_reg[m_op1];
		else
			m_reg[m_op2 + 36] = m_op1;
	}
	else
	{
		fatalerror("Invalid operand on LDPR PC=%x\n", PC);
	}
	F12END();
}

UINT32 v60_device::opLDTASK()
{
	int i;
	F12DecodeOperands(&v60_device::ReadAMAddress, 2,&v60_device::ReadAM, 2);

	v60WritePSW(v60ReadPSW() & 0xefffffff);

	TR = m_op2;

	TKCW = m_program->read_dword_unaligned(m_op2);
	m_op2 += 4;
	if(SYCW & 0x100) {
		L0SP = m_program->read_dword_unaligned(m_op2);
		m_op2 += 4;
	}
	if(SYCW & 0x200) {
		L1SP = m_program->read_dword_unaligned(m_op2);
		m_op2 += 4;
	}
	if(SYCW & 0x400) {
		L2SP = m_program->read_dword_unaligned(m_op2);
		m_op2 += 4;
	}
	if(SYCW & 0x800) {
		L3SP = m_program->read_dword_unaligned(m_op2);
		m_op2 += 4;
	}

	v60ReloadStack();

	// 31 registers supported, _not_ 32
	for(i = 0; i < 31; i++)
		if(m_op1 & (1 << i)) {
			m_reg[i] = m_program->read_dword_unaligned(m_op2);
			m_op2 += 4;
		}

	// #### Ignore the virtual addressing crap.

	F12END();
}

UINT32 v60_device::opMOVD() /* TRUSTED */
{
	UINT32 a, b;

	F12DecodeOperands(&v60_device::ReadAMAddress, 3,&v60_device::ReadAMAddress, 3);

	if (m_flag1)
	{
		a = m_reg[m_op1 & 0x1F];
		b = m_reg[(m_op1 & 0x1F) + 1];
	}
	else
	{
		a = m_program->read_dword_unaligned(m_op1);
		b = m_program->read_dword_unaligned(m_op1 + 4);
	}

	if (m_flag2)
	{
		m_reg[m_op2 & 0x1F] = a;
		m_reg[(m_op2 & 0x1F) + 1] = b;
	}
	else
	{
		m_program->write_dword_unaligned(m_op2, a);
		m_program->write_dword_unaligned(m_op2 + 4, b);
	}

	F12END();
}

UINT32 v60_device::opMOVB() /* TRUSTED */
{
	F12DecodeFirstOperand(&v60_device::ReadAM, 0);
	m_modwritevalb = (UINT8)m_op1;
	F12WriteSecondOperand(0);
	F12END();
}

UINT32 v60_device::opMOVH() /* TRUSTED */
{
	F12DecodeFirstOperand(&v60_device::ReadAM, 1);
	m_modwritevalh = (UINT16)m_op1;
	F12WriteSecondOperand(1);
	F12END();
}

UINT32 v60_device::opMOVW() /* TRUSTED */
{
	F12DecodeFirstOperand(&v60_device::ReadAM, 2);
	m_modwritevalw = m_op1;
	F12WriteSecondOperand(2);
	F12END();
}

UINT32 v60_device::opMOVEAB() /* TRUSTED */
{
	F12DecodeFirstOperand(&v60_device::ReadAMAddress, 0);
	m_modwritevalw = m_op1;
	F12WriteSecondOperand(2);
	F12END();
}

UINT32 v60_device::opMOVEAH() /* TRUSTED */
{
	F12DecodeFirstOperand(&v60_device::ReadAMAddress, 1);
	m_modwritevalw = m_op1;
	F12WriteSecondOperand(2);
	F12END();
}

UINT32 v60_device::opMOVEAW() /* TRUSTED */
{
	F12DecodeFirstOperand(&v60_device::ReadAMAddress, 2);
	m_modwritevalw = m_op1;
	F12WriteSecondOperand(2);
	F12END();
}

UINT32 v60_device::opMOVSBH() /* TRUSTED */
{
	F12DecodeFirstOperand(&v60_device::ReadAM, 0);
	m_modwritevalh = (INT8)(m_op1 & 0xFF);
	F12WriteSecondOperand(1);
	F12END();
}

UINT32 v60_device::opMOVSBW() /* TRUSTED */
{
	F12DecodeFirstOperand(&v60_device::ReadAM, 0);
	m_modwritevalw = (INT8)(m_op1 & 0xFF);
	F12WriteSecondOperand(2);
	F12END();
}

UINT32 v60_device::opMOVSHW() /* TRUSTED */
{
	F12DecodeFirstOperand(&v60_device::ReadAM, 1);
	m_modwritevalw = (INT16)(m_op1 & 0xFFFF);
	F12WriteSecondOperand(2);
	F12END();
}

UINT32 v60_device::opMOVTHB()
{
	F12DecodeFirstOperand(&v60_device::ReadAM, 1);
	m_modwritevalb = (UINT8)(m_op1 & 0xFF);

	// Check for overflow: the truncated bits must match the sign
	//  of the result, otherwise overflow
	if (((m_modwritevalb & 0x80) == 0x80 && ((m_op1 & 0xFF00) == 0xFF00)) ||
			((m_modwritevalb & 0x80) == 0 && ((m_op1 & 0xFF00) == 0x0000)))
		_OV = 0;
	else
		_OV = 1;

	F12WriteSecondOperand(0);
	F12END();
}

UINT32 v60_device::opMOVTWB()
{
	F12DecodeFirstOperand(&v60_device::ReadAM, 2);
	m_modwritevalb = (UINT8)(m_op1 & 0xFF);

	// Check for overflow: the truncated bits must match the sign
	//  of the result, otherwise overflow
	if (((m_modwritevalb & 0x80) == 0x80 && ((m_op1 & 0xFFFFFF00) == 0xFFFFFF00)) ||
			((m_modwritevalb & 0x80) == 0 && ((m_op1 & 0xFFFFFF00) == 0x00000000)))
		_OV = 0;
	else
		_OV = 1;

	F12WriteSecondOperand(0);
	F12END();
}

UINT32 v60_device::opMOVTWH()
{
	F12DecodeFirstOperand(&v60_device::ReadAM, 2);
	m_modwritevalh = (UINT16)(m_op1 & 0xFFFF);

	// Check for overflow: the truncated bits must match the sign
	//  of the result, otherwise overflow
	if (((m_modwritevalh & 0x8000) == 0x8000 && ((m_op1 & 0xFFFF0000) == 0xFFFF0000)) ||
			((m_modwritevalh & 0x8000) == 0 && ((m_op1 & 0xFFFF0000) == 0x00000000)))
		_OV = 0;
	else
		_OV = 1;

	F12WriteSecondOperand(1);
	F12END();
}


UINT32 v60_device::opMOVZBH() /* TRUSTED */
{
	F12DecodeFirstOperand(&v60_device::ReadAM, 0);
	m_modwritevalh = (UINT16)m_op1;
	F12WriteSecondOperand(1);
	F12END();
}

UINT32 v60_device::opMOVZBW() /* TRUSTED */
{
	F12DecodeFirstOperand(&v60_device::ReadAM, 0);
	m_modwritevalw = m_op1;
	F12WriteSecondOperand(2);
	F12END();
}

UINT32 v60_device::opMOVZHW() /* TRUSTED */
{
	F12DecodeFirstOperand(&v60_device::ReadAM, 1);
	m_modwritevalw = m_op1;
	F12WriteSecondOperand(2);
	F12END();
}

UINT32 v60_device::opMULB()
{
	UINT8 appb;
	UINT32 tmp;
	F12DecodeOperands(&v60_device::ReadAM, 0,&v60_device::ReadAMAddress, 0);

	F12LOADOP2BYTE();

	// @@@ OV not set!!
	tmp = (INT8)appb * (INT32)(INT8)m_op1;
	appb = tmp;
	_Z = (appb == 0);
	_S = ((appb & 0x80) != 0);
	_OV = ((tmp >> 8) != 0);

	F12STOREOP2BYTE();
	F12END();
}

UINT32 v60_device::opMULH()
{
	UINT16 apph;
	UINT32 tmp;
	F12DecodeOperands(&v60_device::ReadAM, 1,&v60_device::ReadAMAddress, 1);

	F12LOADOP2HALF();

	// @@@ OV not set!!
	tmp = (INT16)apph * (INT32)(INT16)m_op1;
	apph = tmp;
	_Z = (apph == 0);
	_S = ((apph & 0x8000) != 0);
	_OV = ((tmp >> 16) != 0);

	F12STOREOP2HALF();
	F12END();
}

UINT32 v60_device::opMULW()
{
	UINT32 appw;
	UINT64 tmp;
	F12DecodeOperands(&v60_device::ReadAM, 2,&v60_device::ReadAMAddress, 2);

	F12LOADOP2WORD();

	// @@@ OV not set!!
	tmp = (INT32)appw * (INT64)(INT32)m_op1;
	appw = tmp;
	_Z = (appw == 0);
	_S = ((appw & 0x80000000) != 0);
	_OV = ((tmp >> 32) != 0);

	F12STOREOP2WORD();
	F12END();
}

UINT32 v60_device::opMULUB()
{
	UINT8 appb;
	UINT32 tmp;
	F12DecodeOperands(&v60_device::ReadAM, 0,&v60_device::ReadAMAddress, 0);

	F12LOADOP2BYTE();

	// @@@ OV not set!!
	tmp = appb * (UINT8)m_op1;
	appb = tmp;
	_Z = (appb == 0);
	_S = ((appb & 0x80) != 0);
	_OV = ((tmp >> 8) != 0);

	F12STOREOP2BYTE();
	F12END();
}

UINT32 v60_device::opMULUH()
{
	UINT16 apph;
	UINT32 tmp;
	F12DecodeOperands(&v60_device::ReadAM, 1,&v60_device::ReadAMAddress, 1);

	F12LOADOP2HALF();

	// @@@ OV not set!!
	tmp = apph * (UINT16)m_op1;
	apph = tmp;
	_Z = (apph == 0);
	_S = ((apph & 0x8000) != 0);
	_OV = ((tmp >> 16) != 0);

	F12STOREOP2HALF();
	F12END();
}

UINT32 v60_device::opMULUW()
{
	UINT32 appw;
	UINT64 tmp;
	F12DecodeOperands(&v60_device::ReadAM, 2,&v60_device::ReadAMAddress, 2);

	F12LOADOP2WORD();

	// @@@ OV not set!!
	tmp = (UINT64)appw * (UINT64)m_op1;
	appw = tmp;
	_Z = (appw == 0);
	_S = ((appw & 0x80000000) != 0);
	_OV = ((tmp >> 32) != 0);

	F12STOREOP2WORD();
	F12END();
}

UINT32 v60_device::opNEGB() /* TRUSTED  (C too!)*/
{
	F12DecodeFirstOperand(&v60_device::ReadAM, 0);

	m_modwritevalb = 0;
	SUBB(m_modwritevalb, (INT8)m_op1);

	F12WriteSecondOperand(0);
	F12END();
}

UINT32 v60_device::opNEGH() /* TRUSTED  (C too!)*/
{
	F12DecodeFirstOperand(&v60_device::ReadAM, 1);

	m_modwritevalh = 0;
	SUBW(m_modwritevalh, (INT16)m_op1);

	F12WriteSecondOperand(1);
	F12END();
}

UINT32 v60_device::opNEGW() /* TRUSTED  (C too!)*/
{
	F12DecodeFirstOperand(&v60_device::ReadAM, 2);

	m_modwritevalw = 0;
	SUBL(m_modwritevalw, (INT32)m_op1);

	F12WriteSecondOperand(2);
	F12END();
}

UINT32 v60_device::opNOTB() /* TRUSTED */
{
	F12DecodeFirstOperand(&v60_device::ReadAM, 0);
	m_modwritevalb=~m_op1;

	_OV = 0;
	_S = ((m_modwritevalb & 0x80) != 0);
	_Z = (m_modwritevalb == 0);

	F12WriteSecondOperand(0);
	F12END();
}

UINT32 v60_device::opNOTH() /* TRUSTED */
{
	F12DecodeFirstOperand(&v60_device::ReadAM, 1);
	m_modwritevalh=~m_op1;

	_OV = 0;
	_S = ((m_modwritevalh & 0x8000) != 0);
	_Z = (m_modwritevalh == 0);

	F12WriteSecondOperand(1);
	F12END();
}

UINT32 v60_device::opNOTW() /* TRUSTED */
{
	F12DecodeFirstOperand(&v60_device::ReadAM, 2);
	m_modwritevalw=~m_op1;

	_OV = 0;
	_S = ((m_modwritevalw & 0x80000000) != 0);
	_Z = (m_modwritevalw == 0);

	F12WriteSecondOperand(2);
	F12END();
}

UINT32 v60_device::opNOT1() /* TRUSTED */
{
	UINT32 appw;
	F12DecodeOperands(&v60_device::ReadAM, 2,&v60_device::ReadAMAddress, 2);

	F12LOADOP2WORD();

	_CY = ((appw & (1 << m_op1)) != 0);
	_Z = !(_CY);

	if (_CY)
		appw &= ~(1 << m_op1);
	else
		appw |= (1 << m_op1);

	F12STOREOP2WORD();
	F12END();
}

UINT32 v60_device::opORB() /* TRUSTED  (C too!)*/
{
	UINT8 appb;
	F12DecodeOperands(&v60_device::ReadAM, 0,&v60_device::ReadAMAddress, 0);

	F12LOADOP2BYTE();

	ORB(appb, (UINT8)m_op1);

	F12STOREOP2BYTE();
	F12END();
}

UINT32 v60_device::opORH() /* TRUSTED (C too!)*/
{
	UINT16 apph;
	F12DecodeOperands(&v60_device::ReadAM, 1,&v60_device::ReadAMAddress, 1);

	F12LOADOP2HALF();

	ORW(apph, (UINT16)m_op1);

	F12STOREOP2HALF();
	F12END();
}

UINT32 v60_device::opORW() /* TRUSTED (C too!) */
{
	UINT32 appw;
	F12DecodeOperands(&v60_device::ReadAM, 2,&v60_device::ReadAMAddress, 2);

	F12LOADOP2WORD();

	ORL(appw, (UINT32)m_op1);

	F12STOREOP2WORD();
	F12END();
}

UINT32 v60_device::opOUTB()
{
	F12DecodeOperands(&v60_device::ReadAM, 0,&v60_device::ReadAMAddress, 2);
	m_io->write_byte(m_op2,(UINT8)m_op1);
	F12END();
}

UINT32 v60_device::opOUTH()
{
	F12DecodeOperands(&v60_device::ReadAM, 1,&v60_device::ReadAMAddress, 2);
	m_io->write_word_unaligned(m_op2,(UINT16)m_op1);
	F12END();
}

UINT32 v60_device::opOUTW()
{
	F12DecodeOperands(&v60_device::ReadAM, 2,&v60_device::ReadAMAddress, 2);
	m_io->write_dword_unaligned(m_op2, m_op1);
	F12END();
}

UINT32 v60_device::opREMB()
{
	UINT8 appb;
	F12DecodeOperands(&v60_device::ReadAM, 0,&v60_device::ReadAMAddress, 0);

	F12LOADOP2BYTE();

	_OV = 0;
	if (m_op1)
		appb= (INT8)appb % (INT8)m_op1;
	_Z = (appb == 0);
	_S = ((appb & 0x80) != 0);

	F12STOREOP2BYTE();
	F12END();
}

UINT32 v60_device::opREMH()
{
	UINT16 apph;
	F12DecodeOperands(&v60_device::ReadAM, 1,&v60_device::ReadAMAddress, 1);

	F12LOADOP2HALF();

	_OV = 0;
	if (m_op1)
		apph = (INT16)apph % (INT16)m_op1;
	_Z = (apph == 0);
	_S = ((apph & 0x8000) != 0);

	F12STOREOP2HALF();
	F12END();
}

UINT32 v60_device::opREMW()
{
	UINT32 appw;
	F12DecodeOperands(&v60_device::ReadAM, 2,&v60_device::ReadAMAddress, 2);

	F12LOADOP2WORD();

	_OV = 0;
	if (m_op1)
		appw = (INT32)appw % (INT32)m_op1;
	_Z = (appw == 0);
	_S = ((appw & 0x80000000) != 0);

	F12STOREOP2WORD();
	F12END();
}

UINT32 v60_device::opREMUB()
{
	UINT8 appb;
	F12DecodeOperands(&v60_device::ReadAM, 0,&v60_device::ReadAMAddress, 0);

	F12LOADOP2BYTE();

	_OV = 0;
	if (m_op1)
		appb %= (UINT8)m_op1;
	_Z = (appb == 0);
	_S = ((appb & 0x80) != 0);

	F12STOREOP2BYTE();
	F12END();
}

UINT32 v60_device::opREMUH()
{
	UINT16 apph;
	F12DecodeOperands(&v60_device::ReadAM, 1,&v60_device::ReadAMAddress, 1);

	F12LOADOP2HALF();

	_OV = 0;
	if (m_op1)
		apph %= (UINT16)m_op1;
	_Z = (apph == 0);
	_S = ((apph & 0x8000) != 0);

	F12STOREOP2HALF();
	F12END();
}

UINT32 v60_device::opREMUW()
{
	UINT32 appw;
	F12DecodeOperands(&v60_device::ReadAM, 2,&v60_device::ReadAMAddress, 2);

	F12LOADOP2WORD();

	_OV = 0;
	if (m_op1)
		appw %= m_op1;
	_Z = (appw == 0);
	_S = ((appw & 0x80000000) != 0);

	F12STOREOP2WORD();
	F12END();
}

UINT32 v60_device::opROTB() /* TRUSTED */
{
	UINT8 appb;
	INT8 i, count;

	F12DecodeOperands(&v60_device::ReadAM, 0,&v60_device::ReadAMAddress, 0);

	F12LOADOP2BYTE();

	count = (INT8)(m_op1 & 0xFF);
	if (count > 0)
	{
		for (i = 0;i < count;i++)
			appb = (appb << 1) | ((appb & 0x80) >> 7);

		_CY = (appb & 0x1) != 0;
	}
	else if (count < 0)
	{
		count=-count;
		for (i = 0;i < count;i++)
			appb = (appb >> 1) | ((appb & 0x1) << 7);

		_CY = (appb & 0x80) != 0;
	}
	else
		_CY = 0;

	_OV = 0;
	_S = (appb & 0x80) != 0;
	_Z = (appb == 0);

	F12STOREOP2BYTE();
	F12END();
}

UINT32 v60_device::opROTH() /* TRUSTED */
{
	UINT16 apph;
	INT8 i, count;

	F12DecodeOperands(&v60_device::ReadAM, 0,&v60_device::ReadAMAddress, 1);

	F12LOADOP2HALF();

	count = (INT8)(m_op1 & 0xFF);
	if (count > 0)
	{
		for (i = 0;i < count;i++)
			apph = (apph << 1) | ((apph & 0x8000) >> 15);

		_CY = (apph & 0x1) != 0;
	}
	else if (count < 0)
	{
		count=-count;
		for (i = 0;i < count;i++)
			apph = (apph >> 1) | ((apph & 0x1) << 15);

		_CY = (apph & 0x8000) != 0;
	}
	else
		_CY = 0;

	_OV = 0;
	_S = (apph & 0x8000) != 0;
	_Z = (apph == 0);

	F12STOREOP2HALF();
	F12END();
}

UINT32 v60_device::opROTW() /* TRUSTED */
{
	UINT32 appw;
	INT8 i, count;

	F12DecodeOperands(&v60_device::ReadAM, 0,&v60_device::ReadAMAddress, 2);

	F12LOADOP2WORD();

	count = (INT8)(m_op1 & 0xFF);
	if (count > 0)
	{
		for (i = 0;i < count;i++)
			appw = (appw << 1) | ((appw & 0x80000000) >> 31);

		_CY = (appw & 0x1) != 0;
	}
	else if (count < 0)
	{
		count=-count;
		for (i = 0;i < count;i++)
			appw = (appw >> 1) | ((appw & 0x1) << 31);

		_CY = (appw & 0x80000000) != 0;
	}
	else
		_CY = 0;

	_OV = 0;
	_S = (appw & 0x80000000) != 0;
	_Z = (appw == 0);

	F12STOREOP2WORD();
	F12END();
}

UINT32 v60_device::opROTCB() /* TRUSTED */
{
	UINT8 appb;
	INT8 i, cy, count;

	F12DecodeOperands(&v60_device::ReadAM, 0,&v60_device::ReadAMAddress, 0);

	F12LOADOP2BYTE();
	NORMALIZEFLAGS();

	count = (INT8)(m_op1 & 0xFF);
	if (count > 0)
	{
		for (i = 0;i < count;i++)
		{
			cy = _CY;
			_CY = (UINT8)((appb & 0x80) >> 7);
			appb = (appb << 1) | cy;
		}
	}
	else if (count < 0)
	{
		count=-count;
		for (i = 0;i < count;i++)
		{
			cy = _CY;
			_CY = (appb & 1);
			appb = (appb >> 1) | (cy << 7);
		}
	}
	else
		_CY = 0;

	_OV = 0;
	_S = (appb & 0x80) != 0;
	_Z = (appb == 0);

	F12STOREOP2BYTE();
	F12END();
}

UINT32 v60_device::opROTCH() /* TRUSTED */
{
	UINT16 apph;
	INT8 i, cy, count;

	F12DecodeOperands(&v60_device::ReadAM, 0,&v60_device::ReadAMAddress, 1);

	F12LOADOP2HALF();
	NORMALIZEFLAGS();

	count = (INT8)(m_op1 & 0xFF);
	if (count > 0)
	{
		for (i = 0;i < count;i++)
		{
			cy = _CY;
			_CY = (UINT8)((apph & 0x8000) >> 15);
			apph = (apph << 1) | cy;
		}
	}
	else if (count < 0)
	{
		count=-count;
		for (i = 0;i < count;i++)
		{
			cy = _CY;
			_CY = (UINT8)(apph & 1);
			apph = (apph >> 1) | ((UINT16)cy << 15);
		}
	}
	else
		_CY = 0;

	_OV = 0;
	_S = (apph & 0x8000) != 0;
	_Z = (apph == 0);

	F12STOREOP2HALF();
	F12END();
}

UINT32 v60_device::opROTCW() /* TRUSTED */
{
	UINT32 appw;
	INT8 i, cy, count;

	F12DecodeOperands(&v60_device::ReadAM, 0,&v60_device::ReadAMAddress, 2);

	F12LOADOP2WORD();
	NORMALIZEFLAGS();

	count = (INT8)(m_op1 & 0xFF);
	if (count > 0)
	{
		for (i = 0;i < count;i++)
		{
			cy = _CY;
			_CY = (UINT8)((appw & 0x80000000) >> 31);
			appw = (appw << 1) | cy;
		}
	}
	else if (count < 0)
	{
		count=-count;
		for (i = 0;i < count;i++)
		{
			cy = _CY;
			_CY = (UINT8)(appw & 1);
			appw = (appw >> 1) | ((UINT32)cy << 31);
		}
	}
	else
		_CY = 0;

	_OV = 0;
	_S = (appw & 0x80000000) != 0;
	_Z = (appw == 0);

	F12STOREOP2WORD();
	F12END();
}

UINT32 v60_device::opRVBIT()
{
	F12DecodeFirstOperand(&v60_device::ReadAM, 0);

	m_modwritevalb =(UINT8)
								(((m_op1 & (1 << 0)) << 7) |
									((m_op1 & (1 << 1)) << 5) |
									((m_op1 & (1 << 2)) << 3) |
									((m_op1 & (1 << 3)) << 1) |
									((m_op1 & (1 << 4)) >> 1) |
									((m_op1 & (1 << 5)) >> 3) |
									((m_op1 & (1 << 6)) >> 5) |
									((m_op1 & (1 << 7)) >> 7));

	F12WriteSecondOperand(0);
	F12END();
}

UINT32 v60_device::opRVBYT() /* TRUSTED */
{
	F12DecodeFirstOperand(&v60_device::ReadAM, 2);

	m_modwritevalw = ((m_op1 & 0x000000FF) << 24) |
									((m_op1 & 0x0000FF00) << 8)  |
									((m_op1 & 0x00FF0000) >> 8)  |
									((m_op1 & 0xFF000000) >> 24);

	F12WriteSecondOperand(2);
	F12END();
}

UINT32 v60_device::opSET1() /* TRUSTED */
{
	UINT32 appw;
	F12DecodeOperands(&v60_device::ReadAM, 2,&v60_device::ReadAMAddress, 2);

	F12LOADOP2WORD();

	_CY = ((appw & (1 << m_op1)) != 0);
	_Z = !(_CY);

	appw |= (1 << m_op1);

	F12STOREOP2WORD();
	F12END();
}


UINT32 v60_device::opSETF()
{
	F12DecodeFirstOperand(&v60_device::ReadAM, 0);

	// Normalize the flags
	NORMALIZEFLAGS();

	switch (m_op1 & 0xF)
	{
	case 0:
		if (!_OV) m_modwritevalb = 0;
		else m_modwritevalb = 1;
		break;
	case 1:
		if (_OV) m_modwritevalb = 0;
		else m_modwritevalb = 1;
		break;
	case 2:
		if (!_CY) m_modwritevalb = 0;
		else m_modwritevalb = 1;
		break;
	case 3:
		if (_CY) m_modwritevalb = 0;
		else m_modwritevalb = 1;
		break;
	case 4:
		if (!_Z) m_modwritevalb = 0;
		else m_modwritevalb = 1;
		break;
	case 5:
		if (_Z) m_modwritevalb = 0;
		else m_modwritevalb = 1;
		break;
	case 6:
		if (!(_CY | _Z)) m_modwritevalb = 0;
		else m_modwritevalb = 1;
		break;
	case 7:
		if ((_CY | _Z)) m_modwritevalb = 0;
		else m_modwritevalb = 1;
		break;
	case 8:
		if (!_S) m_modwritevalb = 0;
		else m_modwritevalb = 1;
		break;
	case 9:
		if (_S) m_modwritevalb = 0;
		else m_modwritevalb = 1;
		break;
	case 10:
		m_modwritevalb = 1;
		break;
	case 11:
		m_modwritevalb = 0;
		break;
	case 12:
		if (!(_S^_OV)) m_modwritevalb = 0;
		else m_modwritevalb = 1;
		break;
	case 13:
		if ((_S^_OV)) m_modwritevalb = 0;
		else m_modwritevalb = 1;
		break;
	case 14:
		if (!((_S^_OV)|_Z)) m_modwritevalb = 0;
		else m_modwritevalb = 1;
		break;
	case 15:
		if (((_S^_OV)|_Z)) m_modwritevalb = 0;
		else m_modwritevalb = 1;
		break;
	}

	F12WriteSecondOperand(0);

	F12END();
}

/*
#define SHIFTLEFT_OY(val, count, bitsize) \
{\
    UINT32 tmp = ((val) >> (bitsize - 1)) & 1; \
    tmp <<= count; \
    tmp -= 1; \
    tmp <<= (bitsize - (count)); \
    _OV = (((val) & tmp) != tmp); \
    _CY = (((val) & (1 << (count - 1))) != 0); \
}
*/

// During the shift, the overflow is set if the sign bit changes at any point during the shift
#define SHIFTLEFT_OV(val, count, bitsize) \
{\
	UINT32 tmp; \
	if (count == 32) \
		tmp = 0xFFFFFFFF; \
	else \
		tmp = ((1 << (count)) - 1); \
	tmp <<= (bitsize - (count)); \
	if (((val) >> (bitsize - 1)) & 1) \
		_OV = (((val) & tmp) != tmp); \
	else \
		_OV = (((val) & tmp) != 0); \
}

#define SHIFTLEFT_CY(val, count, bitsize) \
	_CY = (UINT8)(((val) >> (bitsize - count)) & 1);



#define SHIFTARITHMETICRIGHT_OV(val, count, bitsize) \
	_OV = 0;

#define SHIFTARITHMETICRIGHT_CY(val, count, bitsize) \
	_CY = (UINT8)(((val) >> (count - 1)) & 1);



UINT32 v60_device::opSHAB()
{
	UINT8 appb;
	INT8 count;

	F12DecodeOperands(&v60_device::ReadAM, 0,&v60_device::ReadAMAddress, 0);

	F12LOADOP2BYTE();

	count = (INT8)(m_op1 & 0xFF);

	// Special case: destination unchanged, flags set
	if (count == 0)
	{
		_CY = _OV = 0;
		SetSZPF_Byte(appb);
	}
	else if (count > 0)
	{
		SHIFTLEFT_OV(appb, count, 8);

		// @@@ Undefined what happens to CY when count >= bitsize
		SHIFTLEFT_CY(appb, count, 8);

		// do the actual shift...
		if (count >= 8)
			appb = 0;
		else
			appb <<= count;

		// and set zero and sign
		SetSZPF_Byte(appb);
	}
	else
	{
		count = -count;

		SHIFTARITHMETICRIGHT_OV(appb, count, 8);
		SHIFTARITHMETICRIGHT_CY(appb, count, 8);

		if (count >= 8)
			appb = (appb & 0x80) ? 0xFF : 0;
		else
			appb = ((INT8)appb) >> count;

		SetSZPF_Byte(appb);
	}

//  osd_printf_debug("SHAB: %x _CY: %d _Z: %d _OV: %d _S: %d\n", appb, _CY, _Z, _OV, _S);

	F12STOREOP2BYTE();
	F12END();
}

UINT32 v60_device::opSHAH()
{
	UINT16 apph;
	INT8 count;

	F12DecodeOperands(&v60_device::ReadAM, 0,&v60_device::ReadAMAddress, 1);

	F12LOADOP2HALF();

	count = (INT8)(m_op1 & 0xFF);

	// Special case: destination unchanged, flags set
	if (count == 0)
	{
		_CY = _OV = 0;
		SetSZPF_Word(apph);
	}
	else if (count > 0)
	{
		SHIFTLEFT_OV(apph, count, 16);

		// @@@ Undefined what happens to CY when count >= bitsize
		SHIFTLEFT_CY(apph, count, 16);

		// do the actual shift...
		if (count >= 16)
			apph = 0;
		else
			apph <<= count;

		// and set zero and sign
		SetSZPF_Word(apph);
	}
	else
	{
		count = -count;

		SHIFTARITHMETICRIGHT_OV(apph, count, 16);
		SHIFTARITHMETICRIGHT_CY(apph, count, 16);

		if (count >= 16)
			apph = (apph & 0x8000) ? 0xFFFF : 0;
		else
			apph = ((INT16)apph) >> count;

		SetSZPF_Word(apph);
	}

//  osd_printf_debug("SHAH: %x >> %d = %x _CY: %d _Z: %d _OV: %d _S: %d\n", oldval, count, apph, _CY, _Z, _OV, _S);

	F12STOREOP2HALF();
	F12END();
}

UINT32 v60_device::opSHAW()
{
	UINT32 appw;
	INT8 count;

	F12DecodeOperands(&v60_device::ReadAM, 0,&v60_device::ReadAMAddress, 2);

	F12LOADOP2WORD();

	count = (INT8)(m_op1 & 0xFF);

	// Special case: destination unchanged, flags set
	if (count == 0)
	{
		_CY = _OV = 0;
		SetSZPF_Long(appw);
	}
	else if (count > 0)
	{
		SHIFTLEFT_OV(appw, count, 32);

		// @@@ Undefined what happens to CY when count >= bitsize
		SHIFTLEFT_CY(appw, count, 32);

		// do the actual shift...
		if (count >= 32)
			appw = 0;
		else
			appw <<= count;

		// and set zero and sign
		SetSZPF_Long(appw);
	}
	else
	{
		count = -count;

		SHIFTARITHMETICRIGHT_OV(appw, count, 32);
		SHIFTARITHMETICRIGHT_CY(appw, count, 32);

		if (count >= 32)
			appw = (appw & 0x80000000) ? 0xFFFFFFFF : 0;
		else
			appw = ((INT32)appw) >> count;

		SetSZPF_Long(appw);
	}

//  osd_printf_debug("SHAW: %x >> %d = %x _CY: %d _Z: %d _OV: %d _S: %d\n", oldval, count, appw, _CY, _Z, _OV, _S);

	F12STOREOP2WORD();
	F12END();
}


UINT32 v60_device::opSHLB() /* TRUSTED */
{
	UINT8 appb;
	INT8 count;
	UINT32 tmp;

	F12DecodeOperands(&v60_device::ReadAM, 0,&v60_device::ReadAMAddress, 0);

	F12LOADOP2BYTE();

	count = (INT8)(m_op1 & 0xFF);
	if (count > 0)
	{
		// left shift flags:
		// carry gets the last bit shifted out,
		// overflow is always CLEARed

		_OV = 0;  // default to no overflow

		// now handle carry
		tmp = appb & 0xff;
		tmp <<= count;
		SetCFB(tmp);    // set carry properly

		// do the actual shift...
		appb <<= count;

		// and set zero and sign
		SetSZPF_Byte(appb);
	}
	else
	{
		if (count == 0)
		{
			// special case: clear carry and overflow, do nothing else
			_CY = _OV = 0;
			SetSZPF_Byte(appb); // doc. is unclear if this is true...
		}
		else
		{
			// right shift flags:
			// carry = last bit shifted out
			// overflow always cleared
			tmp = appb & 0xff;
			tmp >>= ((-count) - 1);
			_CY = (UINT8)(tmp & 0x1);
			_OV = 0;

			appb >>= -count;
			SetSZPF_Byte(appb);
		}
	}

//  osd_printf_debug("SHLB: %x _CY: %d _Z: %d _OV: %d _S: %d\n", appb, _CY, _Z, _OV, _S);

	F12STOREOP2BYTE();
	F12END();
}

UINT32 v60_device::opSHLH() /* TRUSTED */
{
	UINT16 apph;
	INT8 count;
	UINT32 tmp;

	F12DecodeOperands(&v60_device::ReadAM, 0,&v60_device::ReadAMAddress, 1);

	F12LOADOP2HALF();

	count = (INT8)(m_op1 & 0xFF);
//  osd_printf_debug("apph: %x count: %d  ", apph, count);
	if (count > 0)
	{
		// left shift flags:
		// carry gets the last bit shifted out,
		// overflow is always CLEARed

		_OV = 0;

		// now handle carry
		tmp = apph & 0xffff;
		tmp <<= count;
		SetCFW(tmp);    // set carry properly

		// do the actual shift...
		apph <<= count;

		// and set zero and sign
		SetSZPF_Word(apph);
	}
	else
	{
		if (count == 0)
		{
			// special case: clear carry and overflow, do nothing else
			_CY = _OV = 0;
			SetSZPF_Word(apph); // doc. is unclear if this is true...
		}
		else
		{
			// right shift flags:
			// carry = last bit shifted out
			// overflow always cleared
			tmp = apph & 0xffff;
			tmp >>= ((-count) - 1);
			_CY = (UINT8)(tmp & 0x1);
			_OV = 0;

			apph >>= -count;
			SetSZPF_Word(apph);
		}
	}

//  osd_printf_debug("SHLH: %x _CY: %d _Z: %d _OV: %d _S: %d\n", apph, _CY, _Z, _OV, _S);

	F12STOREOP2HALF();
	F12END();
}

UINT32 v60_device::opSHLW() /* TRUSTED */
{
	UINT32 appw;
	INT8 count;
	UINT64 tmp;

	F12DecodeOperands(&v60_device::ReadAM, 0,&v60_device::ReadAMAddress, 2);

	F12LOADOP2WORD();

	count = (INT8)(m_op1 & 0xFF);
	if (count > 0)
	{
		// left shift flags:
		// carry gets the last bit shifted out,
		// overflow is always CLEARed

		_OV = 0;

		// now handle carry
		tmp = appw & 0xffffffff;
		tmp <<= count;
		SetCFL(tmp);    // set carry properly

		// do the actual shift...
		appw <<= count;

		// and set zero and sign
		SetSZPF_Long(appw);
	}
	else
	{
		if (count == 0)
		{
			// special case: clear carry and overflow, do nothing else
			_CY = _OV = 0;
			SetSZPF_Long(appw); // doc. is unclear if this is true...
		}
		else
		{
			// right shift flags:
			// carry = last bit shifted out
			// overflow always cleared
			tmp = (UINT64)(appw & 0xffffffff);
			tmp >>= ((-count) - 1);
			_CY = (UINT8)(tmp & 0x1);
			_OV = 0;

			appw >>= -count;
			SetSZPF_Long(appw);
		}
	}

//  osd_printf_debug("SHLW: %x _CY: %d _Z: %d _OV: %d _S: %d\n", appw, _CY, _Z, _OV, _S);

	F12STOREOP2WORD();
	F12END();
}

UINT32 v60_device::opSTPR()
{
	F12DecodeFirstOperand(&v60_device::ReadAM, 2);
	if (m_op1 <= 28)
		m_modwritevalw = m_reg[m_op1 + 36];
	else
	{
		fatalerror("Invalid operand on STPR PC=%x\n", PC);
	}
	F12WriteSecondOperand(2);
	F12END();
}


UINT32 v60_device::opSUBB() /* TRUSTED (C too!) */
{
	UINT8 appb;
	F12DecodeOperands(&v60_device::ReadAM, 0,&v60_device::ReadAMAddress, 0);

	F12LOADOP2BYTE();

	SUBB(appb, (UINT8)m_op1);

	F12STOREOP2BYTE();
	F12END();
}

UINT32 v60_device::opSUBH() /* TRUSTED (C too!) */
{
	UINT16 apph;
	F12DecodeOperands(&v60_device::ReadAM, 1,&v60_device::ReadAMAddress, 1);

	F12LOADOP2HALF();

	SUBW(apph, (UINT16)m_op1);

	F12STOREOP2HALF();
	F12END();
}

UINT32 v60_device::opSUBW() /* TRUSTED (C too!) */
{
	UINT32 appw;
	F12DecodeOperands(&v60_device::ReadAM, 2,&v60_device::ReadAMAddress, 2);

	F12LOADOP2WORD();

	SUBL(appw, (UINT32)m_op1);

	F12STOREOP2WORD();
	F12END();
}


UINT32 v60_device::opSUBCB()
{
	UINT8 appb;
	UINT8 src;
	F12DecodeOperands(&v60_device::ReadAM, 0,&v60_device::ReadAMAddress, 0);

	F12LOADOP2BYTE();

	src = (UINT8)m_op1 + (_CY?1:0);
	SUBB(appb, src);

	F12STOREOP2BYTE();
	F12END();
}

UINT32 v60_device::opSUBCH()
{
	UINT16 apph;
	UINT16 src;

	F12DecodeOperands(&v60_device::ReadAM, 1,&v60_device::ReadAMAddress, 1);

	F12LOADOP2HALF();

	src = (UINT16)m_op1 + (_CY?1:0);
	SUBW(apph, src);

	F12STOREOP2HALF();
	F12END();
}

UINT32 v60_device::opSUBCW()
{
	UINT32 appw;
	UINT32 src;

	F12DecodeOperands(&v60_device::ReadAM, 2,&v60_device::ReadAMAddress, 2);

	F12LOADOP2WORD();

	src = (UINT32)m_op1 + (_CY?1:0);
	SUBL(appw, src);

	F12STOREOP2WORD();
	F12END();
}

UINT32 v60_device::opTEST1()
{
	F12DecodeOperands(&v60_device::ReadAM, 2,&v60_device::ReadAM, 2);

	_CY = ((m_op2 & (1 << m_op1)) != 0);
	_Z = !(_CY);

	F12END();
}

UINT32 v60_device::opUPDPSWW()
{
	F12DecodeOperands(&v60_device::ReadAM, 2,&v60_device::ReadAM, 2);

	/* can only modify condition code and control fields */
	m_op2 &= 0xFFFFFF;
	m_op1 &= 0xFFFFFF;
	v60WritePSW((v60ReadPSW() & (~m_op2)) | (m_op1 & m_op2));

	F12END();
}

UINT32 v60_device::opUPDPSWH()
{
	F12DecodeOperands(&v60_device::ReadAM, 2,&v60_device::ReadAM, 2);

	/* can only modify condition code fields */
	m_op2 &= 0xFFFF;
	m_op1 &= 0xFFFF;
	v60WritePSW((v60ReadPSW() & (~m_op2)) | (m_op1 & m_op2));

	F12END();
}

UINT32 v60_device::opXCHB() /* TRUSTED */
{
	UINT8 appb, temp;

	F12DecodeOperands(&v60_device::ReadAMAddress, 0,&v60_device::ReadAMAddress, 0);

	F12LOADOP1BYTE();
	temp = appb;
	F12LOADOP2BYTE();
	F12STOREOP1BYTE();
	appb = temp;
	F12STOREOP2BYTE();

	F12END()
}

UINT32 v60_device::opXCHH() /* TRUSTED */
{
	UINT16 apph, temp;

	F12DecodeOperands(&v60_device::ReadAMAddress, 1,&v60_device::ReadAMAddress, 1);

	F12LOADOP1HALF();
	temp = apph;
	F12LOADOP2HALF();
	F12STOREOP1HALF();
	apph = temp;
	F12STOREOP2HALF();

	F12END()
}

UINT32 v60_device::opXCHW() /* TRUSTED */
{
	UINT32 appw, temp;

	F12DecodeOperands(&v60_device::ReadAMAddress, 2,&v60_device::ReadAMAddress, 2);

	F12LOADOP1WORD();
	temp = appw;
	F12LOADOP2WORD();
	F12STOREOP1WORD();
	appw = temp;
	F12STOREOP2WORD();

	F12END()
}

UINT32 v60_device::opXORB() /* TRUSTED (C too!) */
{
	UINT8 appb;
	F12DecodeOperands(&v60_device::ReadAM, 0,&v60_device::ReadAMAddress, 0);

	F12LOADOP2BYTE();

	XORB(appb, (UINT8)m_op1);

	F12STOREOP2BYTE();
	F12END();
}

UINT32 v60_device::opXORH() /* TRUSTED (C too!) */
{
	UINT16 apph;
	F12DecodeOperands(&v60_device::ReadAM, 1,&v60_device::ReadAMAddress, 1);

	F12LOADOP2HALF();

	XORW(apph, (UINT16)m_op1);

	F12STOREOP2HALF();
	F12END();
}

UINT32 v60_device::opXORW() /* TRUSTED (C too!) */
{
	UINT32 appw;
	F12DecodeOperands(&v60_device::ReadAM, 2,&v60_device::ReadAMAddress, 2);

	F12LOADOP2WORD();

	XORL(appw, (UINT32)m_op1);

	F12STOREOP2WORD();
	F12END();
}

UINT32 v60_device::opMULX()
{
	INT32 a, b;
	INT64 res;

	F12DecodeOperands(&v60_device::ReadAM, 2,&v60_device::ReadAMAddress, 3);

	if (m_flag2)
	{
		a = m_reg[m_op2 & 0x1F];
	}
	else
	{
		a = m_program->read_dword_unaligned(m_op2);
	}

	res = (INT64)a * (INT64)(INT32)m_op1;

	b = (INT32)((res >> 32)&0xffffffff);
	a = (INT32)(res & 0xffffffff);

	_S = ((b & 0x80000000) != 0);
	_Z = (a == 0 && b == 0);

	if (m_flag2)
	{
		m_reg[m_op2 & 0x1F] = a;
		m_reg[(m_op2 & 0x1F) + 1] = b;
	}
	else
	{
		m_program->write_dword_unaligned(m_op2, a);
		m_program->write_dword_unaligned(m_op2 + 4, b);
	}

	F12END();
}

UINT32 v60_device::opMULUX()
{
	INT32 a, b;
	UINT64 res;

	F12DecodeOperands(&v60_device::ReadAM, 2,&v60_device::ReadAMAddress, 3);

	if (m_flag2)
	{
		a = m_reg[m_op2 & 0x1F];
	}
	else
	{
		a = m_program->read_dword_unaligned(m_op2);
	}

	res = (UINT64)a * (UINT64)m_op1;
	b = (INT32)((res >> 32)&0xffffffff);
	a = (INT32)(res & 0xffffffff);

	_S = ((b & 0x80000000) != 0);
	_Z = (a == 0 && b == 0);

	if (m_flag2)
	{
		m_reg[m_op2 & 0x1F] = a;
		m_reg[(m_op2 & 0x1F) + 1] = b;
	}
	else
	{
		m_program->write_dword_unaligned(m_op2, a);
		m_program->write_dword_unaligned(m_op2 + 4, b);
	}

	F12END();
}
