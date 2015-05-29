// license:BSD-3-Clause
// copyright-holders:Farfetch'd, R. Belmont
/*
 * CMPC: What happens to _S flag if the strings are identical?
 *   I suppose that it will be cleared. And is it set or cleared
 *   when the first one is a substring of the second? I suppose
 *   cleared (since _S should be (src > dst))
 * MOVC: Why MOVCS does not exist in downward version?
 * SHCHDB / SHCHDH: R27 is filled with the offset from the start or from the end?
 *
 * Strange stuff:
 *   SCHC opcodes does *not* modify _Z flag as stated in V60 manual:
 *   they do the opposite (set if not found, reset if found)
 */

#define F7AEND()  \
	return m_amlength1 + m_amlength2 + 4;

#define F7BEND()  \
	return m_amlength1 + m_amlength2 + 3;

#define F7CEND()  \
	return m_amlength1 + m_amlength2 + 3;

#define F7BCREATEBITMASK(x) \
	x = ((1 << (x)) - 1)

#define F7CCREATEBITMASK(x) \
	x = ((1 << (x)) - 1)

void v60_device::F7aDecodeOperands(am_func DecodeOp1, UINT8 dim1, am_func DecodeOp2, UINT8 dim2)
{
	UINT8 appb;
	// Decode first operand
	m_moddim = dim1;
	m_modm = m_subop & 0x40;
	m_modadd = PC + 2;
	m_amlength1 = (this->*DecodeOp1)();
	m_flag1 = m_amflag;
	m_op1 = m_amout;

	// Decode length
	appb = OpRead8(PC + 2 + m_amlength1);
	if (appb & 0x80)
		m_lenop1 = m_reg[appb & 0x1F];
	else
		m_lenop1 = appb;

	// Decode second operand
	m_moddim = dim2;
	m_modm = m_subop & 0x20;
	m_modadd = PC + 3 + m_amlength1;
	m_amlength2 = (this->*DecodeOp2)();
	m_flag2 = m_amflag;
	m_op2 = m_amout;

	// Decode length
	appb = OpRead8(PC + 3 + m_amlength1 + m_amlength2);
	if (appb & 0x80)
		m_lenop2 = m_reg[appb & 0x1F];
	else
		m_lenop2 = appb;
}

void v60_device::F7bDecodeFirstOperand(am_func DecodeOp1, UINT8 dim1)
{
	UINT8 appb;
	// Decode first operand
	m_moddim = dim1;
	m_modm = m_subop & 0x40;
	m_modadd = PC + 2;
	m_amlength1 = (this->*DecodeOp1)();
	m_flag1 = m_amflag;
	m_op1 = m_amout;

	// Decode ext
	appb = OpRead8(PC + 2 + m_amlength1);
	if (appb & 0x80)
		m_lenop1 = m_reg[appb & 0x1F];
	else
		m_lenop1 = appb;
}


void v60_device::F7bWriteSecondOperand(UINT8 dim2)
{
	m_moddim = dim2;
	m_modm = m_subop & 0x20;
	m_modadd = PC + 3 + m_amlength1;
	m_amlength2 = WriteAM();
}


void v60_device::F7bDecodeOperands(am_func DecodeOp1, UINT8 dim1, am_func DecodeOp2, UINT8 dim2)
{
	// Decode first operand
	F7bDecodeFirstOperand(DecodeOp1, dim1);
	m_bamoffset1 = m_bamoffset;

	// Decode second operand
	m_moddim = dim2;
	m_modm = m_subop & 0x20;
	m_modadd = PC + 3 + m_amlength1;
	m_amlength2 = (this->*DecodeOp2)();
	m_flag2 = m_amflag;
	m_op2 = m_amout;
	m_bamoffset2 = m_bamoffset;
}

void v60_device::F7cDecodeOperands(am_func DecodeOp1, UINT8 dim1, am_func DecodeOp2, UINT8 dim2)
{
	UINT8 appb;
	// Decode first operand
	m_moddim = dim1;
	m_modm = m_subop & 0x40;
	m_modadd = PC + 2;
	m_amlength1 = (this->*DecodeOp1)();
	m_flag1 = m_amflag;
	m_op1 = m_amout;

	// Decode second operand
	m_moddim = dim2;
	m_modm = m_subop & 0x20;
	m_modadd = PC + 2 + m_amlength1;
	m_amlength2 = (this->*DecodeOp2)();
	m_flag2 = m_amflag;
	m_op2 = m_amout;

	// Decode ext
	appb = OpRead8(PC + 2 + m_amlength1 + m_amlength2);
	if (appb & 0x80)
		m_lenop1 = m_reg[appb & 0x1F];
	else
		m_lenop1 = appb;
}

#define F7CLOADOP1BYTE(appb) \
	if (m_flag1) \
		appb = (UINT8)(m_reg[m_op1]&0xFF); \
	else \
		appb = m_program->read_byte(m_op1);

#define F7CLOADOP2BYTE(appb) \
	if (m_flag2) \
		appb = (UINT8)(m_reg[m_op2]&0xFF); \
	else \
		appb = m_program->read_byte(m_op2);


#define F7CSTOREOP2BYTE() \
	if (m_flag2) \
		SETREG8(m_reg[m_op2], appb); \
	else \
		m_program->write_byte(m_op2, appb);

#define F7CSTOREOP2HALF() \
	if (m_flag2) \
		SETREG16(m_reg[m_op2], apph); \
	else \
		m_program->write_word_unaligned(m_op2, apph);

UINT32 v60_device::opCMPSTRB(UINT8 bFill, UINT8 bStop)
{
	UINT32 i, dest;
	UINT8 c1, c2;

	F7aDecodeOperands(&v60_device::ReadAMAddress, 0,&v60_device::ReadAMAddress, 0);

	// Filling
	if (bFill)
	{
		if (m_lenop1 < m_lenop2)
		{
			for (i = m_lenop1; i < m_lenop2; i++)
				m_program->write_byte(m_op1 + i,(UINT8)R26);
		}
		else if (m_lenop2 < m_lenop1)
		{
			for (i = m_lenop2; i < m_lenop1; i++)
				m_program->write_byte(m_op2 + i,(UINT8)R26);
		}
	}

	dest = (m_lenop1 < m_lenop2 ? m_lenop1 : m_lenop2);

	_Z = 0;
	_S = 0;
	if (bStop) _CY = 1;

	for (i = 0; i < dest; i++)
	{
		c1 = m_program->read_byte(m_op1 + i);
		c2 = m_program->read_byte(m_op2 + i);

		if (c1 > c2)
		{
			_S = 1;   break;
		}
		else if (c2 > c1)
		{
			_S = 0;   break;
		}

		if (bStop)
			if (c1 == (UINT8)R26 || c2 == (UINT8)R26)
			{
				_CY = 0;
				break;
			}
	}

	R28 = m_lenop1 + i;
	R27 = m_lenop2 + i;

	if (i == dest)
	{
		if (m_lenop1 > m_lenop2)
			_S = 1;
		else if (m_lenop2 > m_lenop1)
			_S = 0;
		else
			_Z = 1;
	}

	F7AEND();
}

UINT32 v60_device::opCMPSTRH(UINT8 bFill, UINT8 bStop)
{
	UINT32 i, dest;
	UINT16 c1, c2;

	F7aDecodeOperands(&v60_device::ReadAMAddress, 0,&v60_device::ReadAMAddress, 0);

	// Filling
	if (bFill)
	{
		if (m_lenop1 < m_lenop2)
		{
			for (i = m_lenop1; i < m_lenop2; i++)
				m_program->write_word_unaligned(m_op1 + i * 2,(UINT16)R26);
		}
		else if (m_lenop2 < m_lenop1)
		{
			for (i = m_lenop2; i < m_lenop1; i++)
				m_program->write_word_unaligned(m_op2 + i * 2,(UINT16)R26);
		}
	}

	dest = (m_lenop1 < m_lenop2 ? m_lenop1 : m_lenop2);

	_Z = 0;
	_S = 0;
	if (bStop) _CY = 1;

	for (i = 0; i < dest; i++)
	{
		c1 = m_program->read_word_unaligned(m_op1 + i * 2);
		c2 = m_program->read_word_unaligned(m_op2 + i * 2);

		if (c1 > c2)
		{
			_S = 1;   break;
		}
		else if (c2 > c1)
		{
			_S = 0;   break;
		}

		if (bStop)
			if (c1 == (UINT16)R26 || c2 == (UINT16)R26)
			{
				_CY = 0;
				break;
			}
	}

	R28 = m_lenop1 + i * 2;
	R27 = m_lenop2 + i * 2;

	if (i == dest)
	{
		if (m_lenop1 > m_lenop2)
			_S = 1;
		else if (m_lenop2 > m_lenop1)
			_S = 0;
		else
			_Z = 1;
	}

	F7AEND();
}



UINT32 v60_device::opMOVSTRUB(UINT8 bFill, UINT8 bStop) /* TRUSTED (0, 0) (1, 0) */
{
	UINT32 i, dest;
	UINT8 c1;

//  if (bStop)
//  {
//      int a = 1;
//  }

	F7aDecodeOperands(&v60_device::ReadAMAddress, 0,&v60_device::ReadAMAddress, 0);

	dest = (m_lenop1 < m_lenop2 ? m_lenop1 : m_lenop2);

	for (i = 0; i < dest; i++)
	{
		m_program->write_byte(m_op2 + i,(c1 = m_program->read_byte(m_op1 + i)));

		if (bStop && c1 == (UINT8)R26)
			break;
	}

	R28 = m_op1 + i;
	R27 = m_op2 + i;

	if (bFill && m_lenop1 < m_lenop2)
	{
		for (;i < m_lenop2; i++)
			m_program->write_byte(m_op2 + i,(UINT8)R26);

		R27 = m_op2 + i;
	}


	F7AEND();
}

UINT32 v60_device::opMOVSTRDB(UINT8 bFill, UINT8 bStop)
{
	UINT32 i, dest;
	UINT8 c1;

	F7aDecodeOperands(&v60_device::ReadAMAddress, 0,&v60_device::ReadAMAddress, 0);

	dest = (m_lenop1 < m_lenop2 ? m_lenop1 : m_lenop2);

	for (i = 0; i < dest; i++)
	{
		m_program->write_byte(m_op2 + (dest - i - 1),(c1 = m_program->read_byte(m_op1 + (dest - i - 1))));

		if (bStop && c1 == (UINT8)R26)
			break;
	}

	R28 = m_op1 + (m_lenop1 - i - 1);
	R27 = m_op2 + (m_lenop2 - i - 1);

	if (bFill && m_lenop1 < m_lenop2)
	{
		for (;i < m_lenop2; i++)
			m_program->write_byte(m_op2 + dest + (m_lenop2 - i - 1),(UINT8)R26);

		R27 = m_op2 + (m_lenop2 - i - 1);
	}


	F7AEND();
}


UINT32 v60_device::opMOVSTRUH(UINT8 bFill, UINT8 bStop) /* TRUSTED (0, 0) (1, 0) */
{
	UINT32 i, dest;
	UINT16 c1;

//  if (bStop)
//  {   int a = 1; }

	F7aDecodeOperands(&v60_device::ReadAMAddress, 1,&v60_device::ReadAMAddress, 1);

	dest = (m_lenop1 < m_lenop2 ? m_lenop1 : m_lenop2);

	for (i = 0; i < dest; i++)
	{
		m_program->write_word_unaligned(m_op2 + i * 2,(c1 = m_program->read_word_unaligned(m_op1 + i * 2)));

		if (bStop && c1 == (UINT16)R26)
			break;
	}

	R28 = m_op1 + i * 2;
	R27 = m_op2 + i * 2;

	if (bFill && m_lenop1 < m_lenop2)
	{
		for (;i < m_lenop2; i++)
			m_program->write_word_unaligned(m_op2 + i * 2,(UINT16)R26);

		R27 = m_op2 + i * 2;
	}

	F7AEND();
}

UINT32 v60_device::opMOVSTRDH(UINT8 bFill, UINT8 bStop)
{
	UINT32 i, dest;
	UINT16 c1;

//  if (bFill | bStop)
//  { int a = 1; }

	F7aDecodeOperands(&v60_device::ReadAMAddress, 1,&v60_device::ReadAMAddress, 1);

//  if (m_lenop1 != m_lenop2)
//  { int a = 1; }

	dest = (m_lenop1 < m_lenop2 ? m_lenop1 : m_lenop2);

	for (i = 0; i < dest; i++)
	{
		m_program->write_word_unaligned(m_op2 + (dest - i - 1) * 2,(c1 = m_program->read_word_unaligned(m_op1 + (dest - i - 1) * 2)));

		if (bStop && c1 == (UINT16)R26)
			break;
	}

	R28 = m_op1 + (m_lenop1 - i - 1) * 2;
	R27 = m_op2 + (m_lenop2 - i - 1) * 2;

	if (bFill && m_lenop1 < m_lenop2)
	{
		for (;i < m_lenop2; i++)
			m_program->write_word_unaligned(m_op2 + (m_lenop2 - i - 1) * 2,(UINT16)R26);

		R27 = m_op2 + (m_lenop2 - i - 1) * 2;
	}

	F7AEND();
}

UINT32 v60_device::opSEARCHUB(UINT8 bSearch)
{
	UINT8 appb;
	UINT32 i;

	F7bDecodeOperands(&v60_device::ReadAMAddress, 0,&v60_device::ReadAM, 0);

	for (i = 0; i < m_lenop1; i++)
	{
		appb = (m_program->read_byte(m_op1 + i) == (UINT8)m_op2);
		if ((bSearch && appb) || (!bSearch && !appb))
			break;
	}

	R28 = m_op1 + i;
	R27 = i;

	// This is the opposite as stated in V60 manual...
	if (i != m_lenop1)
		_Z = 0;
	else
		_Z = 1;

	F7BEND();
}

UINT32 v60_device::opSEARCHUH(UINT8 bSearch)
{
	UINT8 appb;
	UINT32 i;

	F7bDecodeOperands(&v60_device::ReadAMAddress, 1,&v60_device::ReadAM, 1);

	for (i = 0; i < m_lenop1; i++)
	{
		appb = (m_program->read_word_unaligned(m_op1 + i * 2) == (UINT16)m_op2);
		if ((bSearch && appb) || (!bSearch && !appb))
			break;
	}

	R28 = m_op1 + i * 2;
	R27 = i;

	if (i != m_lenop1)
		_Z = 0;
	else
		_Z = 1;

	F7BEND();
}

UINT32 v60_device::opSEARCHDB(UINT8 bSearch)
{
	UINT8 appb;
	INT32 i;

	F7bDecodeOperands(&v60_device::ReadAMAddress, 0,&v60_device::ReadAM, 0);

	for (i = m_lenop1; i >= 0; i--)
	{
		appb = (m_program->read_byte(m_op1 + i) == (UINT8)m_op2);
		if ((bSearch && appb) || (!bSearch && !appb))
			break;
	}

	R28 = m_op1 + i;
	R27 = i;

	// This is the opposite as stated in V60 manual...
	if ((UINT32)i != m_lenop1)
		_Z = 0;
	else
		_Z = 1;

	F7BEND();
}

UINT32 v60_device::opSEARCHDH(UINT8 bSearch)
{
	UINT8 appb;
	INT32 i;

	F7bDecodeOperands(&v60_device::ReadAMAddress, 1,&v60_device::ReadAM, 1);

	for (i = m_lenop1 - 1; i >= 0; i--)
	{
		appb = (m_program->read_word_unaligned(m_op1 + i * 2) == (UINT16)m_op2);
		if ((bSearch && appb) || (!bSearch && !appb))
			break;
	}

	R28 = m_op1 + i * 2;
	R27 = i;

	if ((UINT32)i != m_lenop1)
		_Z = 0;
	else
		_Z = 1;

	F7BEND();
}


UINT32 v60_device::opSCHCUB() { return opSEARCHUB(1); }
UINT32 v60_device::opSCHCUH() { return opSEARCHUH(1); }
UINT32 v60_device::opSCHCDB() { return opSEARCHDB(1); }
UINT32 v60_device::opSCHCDH() { return opSEARCHDH(1); }
UINT32 v60_device::opSKPCUB() { return opSEARCHUB(0); }
UINT32 v60_device::opSKPCUH() { return opSEARCHUH(0); }
UINT32 v60_device::opSKPCDB() { return opSEARCHDB(0); }
UINT32 v60_device::opSKPCDH() { return opSEARCHDH(0); }

UINT32 v60_device::opCMPCB() { return opCMPSTRB(0, 0); }
UINT32 v60_device::opCMPCH() { return opCMPSTRH(0, 0); }
UINT32 v60_device::opCMPCFB() { return opCMPSTRB(1, 0); }
UINT32 v60_device::opCMPCFH() { return opCMPSTRH(1, 0); }
UINT32 v60_device::opCMPCSB() { return opCMPSTRB(0, 1); }
UINT32 v60_device::opCMPCSH() { return opCMPSTRH(0, 1); }

UINT32 v60_device::opMOVCUB() { return opMOVSTRUB(0, 0); }
UINT32 v60_device::opMOVCUH() { return opMOVSTRUH(0, 0); }
UINT32 v60_device::opMOVCFUB() { return opMOVSTRUB(1, 0); }
UINT32 v60_device::opMOVCFUH() { return opMOVSTRUH(1, 0); }
UINT32 v60_device::opMOVCSUB() { return opMOVSTRUB(0, 1); }
UINT32 v60_device::opMOVCSUH() { return opMOVSTRUH(0, 1); }

UINT32 v60_device::opMOVCDB() { return opMOVSTRDB(0, 0); }
UINT32 v60_device::opMOVCDH() { return opMOVSTRDH(0, 0); }
UINT32 v60_device::opMOVCFDB() { return opMOVSTRDB(1, 0); }
UINT32 v60_device::opMOVCFDH() { return opMOVSTRDH(1, 0); }

UINT32 v60_device::opEXTBFZ() /* TRUSTED */
{
	F7bDecodeFirstOperand(&v60_device::BitReadAM, 11);

	F7BCREATEBITMASK(m_lenop1);

	m_modwritevalw = (m_op1 >> m_bamoffset) & m_lenop1;

	F7bWriteSecondOperand(2);

	F7BEND();
}

UINT32 v60_device::opEXTBFS() /* TRUSTED */
{
	F7bDecodeFirstOperand(&v60_device::BitReadAM, 11);

	F7BCREATEBITMASK(m_lenop1);

	m_modwritevalw = (m_op1 >> m_bamoffset) & m_lenop1;
	if (m_modwritevalw & ((m_lenop1 + 1) >> 1))
		m_modwritevalw |= ~m_lenop1;

	F7bWriteSecondOperand(2);

	F7BEND();
}

UINT32 v60_device::opEXTBFL()
{
	UINT32 appw;

	F7bDecodeFirstOperand(&v60_device::BitReadAM, 11);

	appw = m_lenop1;
	F7BCREATEBITMASK(m_lenop1);

	m_modwritevalw = (m_op1 >> m_bamoffset) & m_lenop1;
	m_modwritevalw <<= 32 - appw;

	F7bWriteSecondOperand(2);

	F7BEND();
}

UINT32 v60_device::opSCHBS(UINT32 bSearch1)
{
	UINT32 i, data;
	UINT32 offset;

	F7bDecodeFirstOperand(&v60_device::BitReadAMAddress, 10);

	// Read first UINT8
	m_op1 += m_bamoffset / 8;
	data = m_program->read_byte(m_op1);
	offset = m_bamoffset & 7;

	// Scan bitstring
	for (i = 0; i < m_lenop1; i++)
	{
		// Update the work register
		R28 = m_op1;

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
			m_op1++;
			data = m_program->read_byte(m_op1);
		}
	}

	// Set zero if bit not found
	_Z = (i == m_lenop1);

	// Write to destination the final offset
	m_modwritevalw = i;
	F7bWriteSecondOperand(2);

	F7BEND();
}

UINT32 v60_device::opSCH0BSU() { return opSCHBS(0); }
UINT32 v60_device::opSCH1BSU() { return opSCHBS(1); }

UINT32 v60_device::opINSBFR()
{
	UINT32 appw;
	F7cDecodeOperands(&v60_device::ReadAM, 2,&v60_device::BitReadAMAddress, 11);

	F7CCREATEBITMASK(m_lenop1);

	m_op2 += m_bamoffset / 8;
	appw = m_program->read_dword_unaligned(m_op2);
	m_bamoffset &= 7;

	appw &= ~(m_lenop1 << m_bamoffset);
	appw |=  (m_lenop1 & m_op1) << m_bamoffset;

	m_program->write_dword_unaligned(m_op2, appw);

	F7CEND();
}

UINT32 v60_device::opINSBFL()
{
	UINT32 appw;
	F7cDecodeOperands(&v60_device::ReadAM, 2,&v60_device::BitReadAMAddress, 11);

	m_op1 >>= (32 - m_lenop1);

	F7CCREATEBITMASK(m_lenop1);

	m_op2 += m_bamoffset / 8;
	appw = m_program->read_dword_unaligned(m_op2);
	m_bamoffset &= 7;

	appw &= ~(m_lenop1 << m_bamoffset);
	appw |=  (m_lenop1 & m_op1) << m_bamoffset;

	m_program->write_dword_unaligned(m_op2, appw);

	F7CEND();
}

UINT32 v60_device::opMOVBSD()
{
	UINT32 i;
	UINT8 srcdata, dstdata;

	F7bDecodeOperands(&v60_device::BitReadAMAddress, 10, &v60_device::BitReadAMAddress, 10);

//  if (m_lenop1 != 1)
//  { int a = 1; }

	m_bamoffset1 += m_lenop1 - 1;
	m_bamoffset2 += m_lenop1 - 1;

	m_op1 += m_bamoffset1 / 8;
	m_op2 += m_bamoffset2 / 8;

	m_bamoffset1 &= 7;
	m_bamoffset2 &= 7;

	srcdata = m_program->read_byte(m_op1);
	dstdata = m_program->read_byte(m_op2);

	for (i = 0; i < m_lenop1; i++)
	{
		// Update work registers
		R28 = m_op1;
		R27 = m_op2;

		dstdata &= ~(1 << m_bamoffset2);
		dstdata |= ((srcdata >> m_bamoffset1) & 1) << m_bamoffset2;

		if (m_bamoffset1 == 0)
		{
			m_bamoffset1 = 8;
			m_op1--;
			srcdata = m_program->read_byte(m_op1);
		}
		if (m_bamoffset2 == 0)
		{
			m_program->write_byte(m_op2, dstdata);
			m_bamoffset2 = 8;
			m_op2--;
			dstdata = m_program->read_byte(m_op2);
		}

		m_bamoffset1--;
		m_bamoffset2--;
	}

	// Flush of the final data
	if (m_bamoffset2 != 7)
		m_program->write_byte(m_op2, dstdata);

	F7BEND();
}

UINT32 v60_device::opMOVBSU()
{
	UINT32 i;
	UINT8 srcdata, dstdata;

	F7bDecodeOperands(&v60_device::BitReadAMAddress, 10, &v60_device::BitReadAMAddress, 10);

	m_op1 += m_bamoffset1 / 8;
	m_op2 += m_bamoffset2 / 8;

	m_bamoffset1 &= 7;
	m_bamoffset2 &= 7;

	srcdata = m_program->read_byte(m_op1);
	dstdata = m_program->read_byte(m_op2);

	for (i = 0; i < m_lenop1; i++)
	{
		// Update work registers
		R28 = m_op1;
		R27 = m_op2;

		dstdata &= ~(1 << m_bamoffset2);
		dstdata |= ((srcdata >> m_bamoffset1) & 1) << m_bamoffset2;

		m_bamoffset1++;
		m_bamoffset2++;
		if (m_bamoffset1 == 8)
		{
			m_bamoffset1 = 0;
			m_op1++;
			srcdata = m_program->read_byte(m_op1);
		}
		if (m_bamoffset2 == 8)
		{
			m_program->write_byte(m_op2, dstdata);
			m_bamoffset2 = 0;
			m_op2++;
			dstdata = m_program->read_byte(m_op2);
		}
	}

	// Flush of the final data
	if (m_bamoffset2 != 0)
		m_program->write_byte(m_op2, dstdata);

	F7BEND();
}

// RADM 0x20f4b8 holds the time left

UINT32 v60_device::opADDDC()
{
	UINT8 appb;
	UINT8 src, dst;

	F7cDecodeOperands(&v60_device::ReadAM, 0, &v60_device::ReadAMAddress, 0);

	if (m_lenop1 != 0)
	{
		logerror("ADDDC %x (pat: %x)\n", m_op1, m_lenop1);
	}

	F7CLOADOP2BYTE(appb);

	src = (UINT8)(m_op1 >> 4) * 10 + (UINT8)(m_op1 & 0xF);
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

	appb = ((appb / 10) << 4) | (appb % 10);

	F7CSTOREOP2BYTE();
	F7CEND();
}

UINT32 v60_device::opSUBDC()
{
	INT8 appb;
	UINT32 src, dst;

	F7cDecodeOperands(&v60_device::ReadAM, 0, &v60_device::ReadAMAddress, 0);

	if (m_lenop1 != 0)
	{
		logerror("SUBDC %x (pat: %x)\n", m_op1, m_lenop1);
	}

	F7CLOADOP2BYTE(appb);

	src = (UINT32)(m_op1 >> 4) * 10 + (UINT32)(m_op1 & 0xF);
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

	appb = ((appb / 10) << 4) | (appb % 10);

	F7CSTOREOP2BYTE();
	F7CEND();
}

UINT32 v60_device::opSUBRDC()
{
	INT8 appb;
	UINT32 src, dst;

	F7cDecodeOperands(&v60_device::ReadAM, 0, &v60_device::ReadAMAddress, 0);

	if (m_lenop1 != 0)
	{
		logerror("SUBRDC %x (pat: %x)\n", m_op1, m_lenop1);
	}

	F7CLOADOP2BYTE(appb);

	src = (UINT32)(m_op1 >> 4) * 10 + (UINT32)(m_op1 & 0xF);
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

	appb = ((appb / 10) << 4) | (appb % 10);

	F7CSTOREOP2BYTE();
	F7CEND();
}

UINT32 v60_device::opCVTDPZ()
{
	UINT16 apph;

	F7cDecodeOperands(&v60_device::ReadAM, 0, &v60_device::ReadAMAddress, 1);

	apph = (UINT16)(((m_op1 >> 4) & 0xF) | ((m_op1 & 0xF) << 8));
	apph |= (m_lenop1);
	apph |= (m_lenop1 << 8);

	// Z flag is unchanged if src is zero, cleared otherwise
	if (m_op1 != 0) _Z = 0;

	F7CSTOREOP2HALF();
	F7CEND();
}

UINT32 v60_device::opCVTDZP()
{
	UINT8 appb;
	F7cDecodeOperands(&v60_device::ReadAM, 1, &v60_device::ReadAMAddress, 0);

	if ((m_op1 & 0xF0) != (m_lenop1 & 0xF0) || ((m_op1 >> 8) & 0xF0) != (m_lenop1 & 0xF0))
	{
		// Decimal exception
		logerror("CVTD.ZP Decimal exception #1!\n");
	}

	if ((m_op1 & 0xF) > 9 || ((m_op1 >> 8) & 0xF) > 9)
	{
		// Decimal exception
		logerror("CVTD.ZP Decimal exception #2!\n");
	}

	appb = (UINT8)(((m_op1 >> 8) & 0xF) | ((m_op1 & 0xF) << 4));
	if (appb != 0) _Z = 0;

	F7CSTOREOP2BYTE();
	F7CEND();
}

UINT32 v60_device::op58UNHANDLED()
{
	fatalerror("Unhandled 58 opcode at PC: /%06x\n", PC);
	return 0; /* never reached, fatalerror won't return */
}

UINT32 v60_device::op5AUNHANDLED()
{
	fatalerror("Unhandled 5A opcode at PC: /%06x\n", PC);
	return 0; /* never reached, fatalerror won't return */
}

UINT32 v60_device::op5BUNHANDLED()
{
	fatalerror("Unhandled 5B opcode at PC: /%06x\n", PC);
	return 0; /* never reached, fatalerror won't return */
}

UINT32 v60_device::op5DUNHANDLED()
{
	fatalerror("Unhandled 5D opcode at PC: /%06x\n", PC);
	return 0; /* never reached, fatalerror won't return */
}

UINT32 v60_device::op59UNHANDLED()
{
	fatalerror("Unhandled 59 opcode at PC: /%06x\n", PC);
	return 0; /* never reached, fatalerror won't return */
}

const v60_device::am_func v60_device::s_Op59Table[32] =
{
	&v60_device::opADDDC,
	&v60_device::opSUBDC,
	&v60_device::opSUBRDC,
	&v60_device::op59UNHANDLED,
	&v60_device::op59UNHANDLED,
	&v60_device::op59UNHANDLED,
	&v60_device::op59UNHANDLED,
	&v60_device::op59UNHANDLED,
	&v60_device::op59UNHANDLED,
	&v60_device::op59UNHANDLED,
	&v60_device::op59UNHANDLED,
	&v60_device::op59UNHANDLED,
	&v60_device::op59UNHANDLED,
	&v60_device::op59UNHANDLED,
	&v60_device::op59UNHANDLED,
	&v60_device::op59UNHANDLED,
	&v60_device::opCVTDPZ,
	&v60_device::op59UNHANDLED,
	&v60_device::op59UNHANDLED,
	&v60_device::op59UNHANDLED,
	&v60_device::op59UNHANDLED,
	&v60_device::op59UNHANDLED,
	&v60_device::op59UNHANDLED,
	&v60_device::op59UNHANDLED,
	&v60_device::opCVTDZP,
	&v60_device::op59UNHANDLED,
	&v60_device::op59UNHANDLED,
	&v60_device::op59UNHANDLED,
	&v60_device::op59UNHANDLED,
	&v60_device::op59UNHANDLED,
	&v60_device::op59UNHANDLED,
	&v60_device::op59UNHANDLED
};


const v60_device::am_func v60_device::s_Op5BTable[32] =
{
	&v60_device::opSCH0BSU,
	&v60_device::op5BUNHANDLED,
	&v60_device::opSCH1BSU,
		&v60_device::op5BUNHANDLED,
	&v60_device::op5BUNHANDLED,
	&v60_device::op5BUNHANDLED,
	&v60_device::op5BUNHANDLED,
	&v60_device::op5BUNHANDLED,
	&v60_device::opMOVBSU,
	&v60_device::opMOVBSD,
	&v60_device::op5BUNHANDLED,
	&v60_device::op5BUNHANDLED,
	&v60_device::op5BUNHANDLED,
	&v60_device::op5BUNHANDLED,
	&v60_device::op5BUNHANDLED,
	&v60_device::op5BUNHANDLED,
	&v60_device::op5BUNHANDLED,
	&v60_device::op5BUNHANDLED,
	&v60_device::op5BUNHANDLED,
	&v60_device::op5BUNHANDLED,
	&v60_device::op5BUNHANDLED,
	&v60_device::op5BUNHANDLED,
	&v60_device::op5BUNHANDLED,
	&v60_device::op5BUNHANDLED,
	&v60_device::op5BUNHANDLED,
	&v60_device::op5BUNHANDLED,
	&v60_device::op5BUNHANDLED,
	&v60_device::op5BUNHANDLED,
	&v60_device::op5BUNHANDLED,
	&v60_device::op5BUNHANDLED,
	&v60_device::op5BUNHANDLED,
	&v60_device::op5BUNHANDLED
};


const v60_device::am_func v60_device::s_Op5DTable[32] =
{
	&v60_device::op5DUNHANDLED,
	&v60_device::op5DUNHANDLED,
	&v60_device::op5DUNHANDLED,
	&v60_device::op5DUNHANDLED,
	&v60_device::op5DUNHANDLED,
	&v60_device::op5DUNHANDLED,
	&v60_device::op5DUNHANDLED,
	&v60_device::op5DUNHANDLED,
	&v60_device::opEXTBFS,
	&v60_device::opEXTBFZ,
	&v60_device::opEXTBFL,
	&v60_device::op5DUNHANDLED,
	&v60_device::op5DUNHANDLED,
	&v60_device::op5DUNHANDLED,
	&v60_device::op5DUNHANDLED,
	&v60_device::op5DUNHANDLED,
	&v60_device::op5DUNHANDLED,
	&v60_device::op5DUNHANDLED,
	&v60_device::op5DUNHANDLED,
	&v60_device::op5DUNHANDLED,
	&v60_device::op5DUNHANDLED,
	&v60_device::op5DUNHANDLED,
	&v60_device::op5DUNHANDLED,
	&v60_device::op5DUNHANDLED,
	&v60_device::opINSBFR,
	&v60_device::opINSBFL,
	&v60_device::op5DUNHANDLED,
	&v60_device::op5DUNHANDLED,
	&v60_device::op5DUNHANDLED,
	&v60_device::op5DUNHANDLED,
	&v60_device::op5DUNHANDLED,
	&v60_device::op5DUNHANDLED
};

const v60_device::am_func v60_device::s_Op58Table[32] =
{
	&v60_device::opCMPCB,
	&v60_device::opCMPCFB,
	&v60_device::opCMPCSB,
	&v60_device::op58UNHANDLED,
	&v60_device::op58UNHANDLED,
	&v60_device::op58UNHANDLED,
	&v60_device::op58UNHANDLED,
	&v60_device::op58UNHANDLED,
	&v60_device::opMOVCUB,
	&v60_device::opMOVCDB,
	&v60_device::opMOVCFUB,
	&v60_device::opMOVCFDB,
	&v60_device::opMOVCSUB,
	&v60_device::op58UNHANDLED,
	&v60_device::op58UNHANDLED,
	&v60_device::op58UNHANDLED,
	&v60_device::op58UNHANDLED,
	&v60_device::op58UNHANDLED,
	&v60_device::op58UNHANDLED,
	&v60_device::op58UNHANDLED,
	&v60_device::op58UNHANDLED,
	&v60_device::op58UNHANDLED,
	&v60_device::op58UNHANDLED,
	&v60_device::op58UNHANDLED,
	&v60_device::opSCHCUB,
	&v60_device::opSCHCDB,
	&v60_device::opSKPCUB,
	&v60_device::opSKPCDB,
	&v60_device::op58UNHANDLED,
	&v60_device::op58UNHANDLED,
	&v60_device::op58UNHANDLED,
	&v60_device::op58UNHANDLED
};

const v60_device::am_func v60_device::s_Op5ATable[32] =
{
	&v60_device::opCMPCH,
	&v60_device::opCMPCFH,
	&v60_device::opCMPCSH,
	&v60_device::op5AUNHANDLED,
	&v60_device::op5AUNHANDLED,
	&v60_device::op5AUNHANDLED,
	&v60_device::op5AUNHANDLED,
	&v60_device::op5AUNHANDLED,
	&v60_device::opMOVCUH,
	&v60_device::opMOVCDH,
	&v60_device::opMOVCFUH,
	&v60_device::opMOVCFDH,
	&v60_device::opMOVCSUH,
	&v60_device::op5AUNHANDLED,
	&v60_device::op5AUNHANDLED,
	&v60_device::op5AUNHANDLED,
	&v60_device::op5AUNHANDLED,
	&v60_device::op5AUNHANDLED,
	&v60_device::op5AUNHANDLED,
	&v60_device::op5AUNHANDLED,
	&v60_device::op5AUNHANDLED,
	&v60_device::op5AUNHANDLED,
	&v60_device::op5AUNHANDLED,
	&v60_device::op5AUNHANDLED,
	&v60_device::opSCHCUH,
	&v60_device::opSCHCDH,
	&v60_device::opSKPCUH,
	&v60_device::opSKPCDH,
	&v60_device::op5AUNHANDLED,
	&v60_device::op5AUNHANDLED,
	&v60_device::op5AUNHANDLED,
	&v60_device::op5AUNHANDLED
};

UINT32 v60_device::op58()
{
	m_subop = OpRead8(PC + 1);

	return (this->*s_Op58Table[m_subop & 0x1F])();
}

UINT32 v60_device::op5A()
{
	m_subop = OpRead8(PC + 1);

	return (this->*s_Op5ATable[m_subop & 0x1F])();
}

UINT32 v60_device::op5B()
{
	m_subop = OpRead8(PC + 1);

	return (this->*s_Op5BTable[m_subop & 0x1F])();
}

UINT32 v60_device::op5D()
{
	m_subop = OpRead8(PC + 1);

	return (this->*s_Op5DTable[m_subop & 0x1F])();
}

UINT32 v60_device::op59()
{
	m_subop = OpRead8(PC + 1);

	return (this->*s_Op59Table[m_subop & 0x1F])();
}
