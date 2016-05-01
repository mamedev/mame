// license:BSD-3-Clause
// copyright-holders:Farfetch'd, R. Belmont
UINT32 v60_device::opINCB() /* TRUSTED */
{
	UINT8 appb;
	m_modadd = PC + 1;
	m_moddim = 0;

	m_amlength1 = ReadAMAddress();

	if (m_amflag)
		appb = (UINT8)m_reg[m_amout];
	else
		appb = m_program->read_byte(m_amout);

	ADDB(appb, 1);

	if (m_amflag)
		SETREG8(m_reg[m_amout], appb);
	else
		m_program->write_byte(m_amout, appb);

	return m_amlength1 + 1;
}

UINT32 v60_device::opINCH() /* TRUSTED */
{
	UINT16 apph;
	m_modadd = PC + 1;
	m_moddim = 1;

	m_amlength1 = ReadAMAddress();

	if (m_amflag)
		apph = (UINT16)m_reg[m_amout];
	else
		apph = m_program->read_word_unaligned(m_amout);

	ADDW(apph, 1);

	if (m_amflag)
		SETREG16(m_reg[m_amout], apph);
	else
		m_program->write_word_unaligned(m_amout, apph);

	return m_amlength1 + 1;
}

UINT32 v60_device::opINCW() /* TRUSTED */
{
	UINT32 appw;
	m_modadd = PC + 1;
	m_moddim = 2;

	m_amlength1 = ReadAMAddress();

	if (m_amflag)
		appw = m_reg[m_amout];
	else
		appw = m_program->read_dword_unaligned(m_amout);

	ADDL(appw, 1);

	if (m_amflag)
		m_reg[m_amout] = appw;
	else
		m_program->write_dword_unaligned(m_amout, appw);

	return m_amlength1 + 1;
}

UINT32 v60_device::opDECB() /* TRUSTED */
{
	UINT8 appb;
	m_modadd = PC + 1;
	m_moddim = 0;

	m_amlength1 = ReadAMAddress();

	if (m_amflag)
		appb = (UINT8)m_reg[m_amout];
	else
		appb = m_program->read_byte(m_amout);

	SUBB(appb, 1);

	if (m_amflag)
		SETREG8(m_reg[m_amout], appb);
	else
		m_program->write_byte(m_amout, appb);

	return m_amlength1 + 1;
}

UINT32 v60_device::opDECH() /* TRUSTED */
{
	UINT16 apph;
	m_modadd = PC + 1;
	m_moddim = 1;

	m_amlength1 = ReadAMAddress();

	if (m_amflag)
		apph = (UINT16)m_reg[m_amout];
	else
		apph = m_program->read_word_unaligned(m_amout);

	SUBW(apph, 1);

	if (m_amflag)
		SETREG16(m_reg[m_amout], apph);
	else
		m_program->write_word_unaligned(m_amout, apph);

	return m_amlength1 + 1;
}

UINT32 v60_device::opDECW() /* TRUSTED */
{
	UINT32 appw;
	m_modadd = PC + 1;
	m_moddim = 2;

	m_amlength1 = ReadAMAddress();

	if (m_amflag)
		appw = m_reg[m_amout];
	else
		appw = m_program->read_dword_unaligned(m_amout);

	SUBL(appw, 1);

	if (m_amflag)
		m_reg[m_amout] = appw;
	else
		m_program->write_dword_unaligned(m_amout, appw);

	return m_amlength1 + 1;
}

UINT32 v60_device::opJMP() /* TRUSTED */
{
	m_modadd = PC + 1;
	m_moddim = 0;

	// Read the address of the operand
	ReadAMAddress();

	// It cannot be a register!!
	assert(m_amflag == 0);

	// Jump there
	PC = m_amout;

	return 0;
}

UINT32 v60_device::opJSR() /* TRUSTED */
{
	m_modadd = PC + 1;
	m_moddim = 0;

	// Read the address of the operand
	m_amlength1 = ReadAMAddress();

	// It cannot be a register!!
	assert(m_amflag == 0);

	// Save NextPC into the stack
	SP -= 4;
	m_program->write_dword_unaligned(SP, PC + m_amlength1 + 1);

	// Jump there
	PC = m_amout;

	return 0;
}

UINT32 v60_device::opPREPARE()    /* somewhat TRUSTED */
{
	m_modadd = PC + 1;
	m_moddim = 2;

	// Read the operand
	m_amlength1 = ReadAM();

	// step 1: save frame pointer on the stack
	SP -= 4;
	m_program->write_dword_unaligned(SP, FP);

	// step 2: FP = new SP
	FP = SP;

	// step 3: SP -= operand
	SP -= m_amout;

	return m_amlength1 + 1;
}

UINT32 v60_device::opRET() /* TRUSTED */
{
	m_modadd = PC + 1;
	m_moddim = 2;

	// Read the operand
	ReadAM();

	// Read return address from stack
	PC = m_program->read_dword_unaligned(SP);
	SP +=4;

	// Restore AP from stack
	AP = m_program->read_dword_unaligned(SP);
	SP +=4;

	// Skip stack frame
	SP += m_amout;

	return 0;
}

UINT32 v60_device::opTRAP()
{
	UINT32 oldPSW;

	m_modadd = PC + 1;
	m_moddim = 0;

	// Read the operand
	m_amlength1 = ReadAM();

	// Normalize the flags
	NORMALIZEFLAGS();

	switch ((m_amout >> 4) & 0xF)
	{
	case 0:
		if (!_OV) return m_amlength1 + 1;
		else break;
	case 1:
		if (_OV) return m_amlength1 + 1;
		else break;
	case 2:
		if (!_CY) return m_amlength1 + 1;
		else break;
	case 3:
		if (_CY) return m_amlength1 + 1;
		else break;
	case 4:
		if (!_Z) return m_amlength1 + 1;
		else break;
	case 5:
		if (_Z) return m_amlength1 + 1;
		else break;
	case 6:
		if (!(_CY | _Z)) return m_amlength1 + 1;
		else break;
	case 7:
		if ((_CY | _Z)) return m_amlength1 + 1;
		else break;
	case 8:
		if (!_S) return m_amlength1 + 1;
		else break;
	case 9:
		if (_S) return m_amlength1 + 1;
		else break;
	case 10:
		break;
	case 11:
		return m_amlength1 + 1;
	case 12:
		if (!(_S^_OV)) return m_amlength1 + 1;
		else break;
	case 13:
		if ((_S^_OV)) return m_amlength1 + 1;
		else break;
	case 14:
		if (!((_S^_OV)|_Z)) return m_amlength1 + 1;
		else break;
	case 15:
		if (((_S^_OV)|_Z)) return m_amlength1 + 1;
		else break;
	}

	oldPSW = v60_update_psw_for_exception(0, 0);

	// Issue the software trap with interrupts
	SP -= 4;
	m_program->write_dword_unaligned(SP, EXCEPTION_CODE_AND_SIZE(0x3000 + 0x100 * (m_amout & 0xF), 4));

	SP -= 4;
	m_program->write_dword_unaligned(SP, oldPSW);

	SP -= 4;
	m_program->write_dword_unaligned(SP, PC + m_amlength1 + 1);

	PC = GETINTVECT(48 + (m_amout & 0xF));

	return 0;
}

UINT32 v60_device::opRETIU() /* TRUSTED */
{
	UINT32 newPSW;
	m_modadd = PC + 1;
	m_moddim = 1;

	// Read the operand
	ReadAM();

	// Restore PC and PSW from stack
	PC = m_program->read_dword_unaligned(SP);
	SP += 4;

	newPSW = m_program->read_dword_unaligned(SP);
	SP += 4;

	// Destroy stack frame
	SP += m_amout;

	v60WritePSW(newPSW);

	return 0;
}

UINT32 v60_device::opRETIS()
{
	UINT32 newPSW;

	m_modadd = PC + 1;
	m_moddim = 1;

	// Read the operand
	ReadAM();

	// Restore PC and PSW from stack
	PC = m_program->read_dword_unaligned(SP);
	SP += 4;

	newPSW = m_program->read_dword_unaligned(SP);
	SP += 4;

	// Destroy stack frame
	SP += m_amout;

	v60WritePSW(newPSW);

	return 0;
}

UINT32 v60_device::opSTTASK()
{
	int i;
	UINT32 adr;

	m_modadd = PC + 1;
	m_moddim = 2;

	m_amlength1 = ReadAM();

	adr = TR;

	v60WritePSW(v60ReadPSW() | 0x10000000);
	v60SaveStack();

	m_program->write_dword_unaligned(adr, TKCW);
	adr += 4;
	if(SYCW & 0x100) {
		m_program->write_dword_unaligned(adr, L0SP);
		adr += 4;
	}
	if(SYCW & 0x200) {
		m_program->write_dword_unaligned(adr, L1SP);
		adr += 4;
	}
	if(SYCW & 0x400) {
		m_program->write_dword_unaligned(adr, L2SP);
		adr += 4;
	}
	if(SYCW & 0x800) {
		m_program->write_dword_unaligned(adr, L3SP);
		adr += 4;
	}

	// 31 registers supported, _not_ 32
	for(i = 0; i < 31; i++)
		if(m_amout & (1 << i)) {
			m_program->write_dword_unaligned(adr, m_reg[i]);
			adr += 4;
		}

	// #### Ignore the virtual addressing crap.

	return m_amlength1 + 1;
}

UINT32 v60_device::opGETPSW()
{
	m_modadd = PC + 1;
	m_moddim = 2;
	m_modwritevalw = v60ReadPSW();

	// Write PSW to the operand
	m_amlength1 = WriteAM();

	return m_amlength1 + 1;
}

UINT32 v60_device::opTASI()
{
	UINT8 appb;
	m_modadd = PC + 1;
	m_moddim = 0;

	// Load the address of the operand
	m_amlength1 = ReadAMAddress();

	// Load UINT8 from the address
	if (m_amflag)
		appb = (UINT8)m_reg[m_amout & 0x1F];
	else
		appb = m_program->read_byte(m_amout);

	// Set the flags for SUB appb, FF
	SUBB(appb, 0xff);

	// Write FF in the operand
	if (m_amflag)
		SETREG8(m_reg[m_amout & 0x1F], 0xFF);
	else
		m_program->write_byte(m_amout, 0xFF);

	return m_amlength1 + 1;
}

UINT32 v60_device::opCLRTLB()
{
	m_modadd = PC + 1;
	m_moddim = 2;

	// Read the operand
	m_amlength1 = ReadAM();

	// @@@ TLB not yet emulated

	return m_amlength1 + 1;
}

UINT32 v60_device::opPOPM()
{
	int i;

	m_modadd = PC + 1;
	m_moddim = 2;

	// Read the bit register list
	m_amlength1 = ReadAM();

	for (i = 0;i < 31;i++)
		if (m_amout & (1 << i))
		{
			m_reg[i] = m_program->read_dword_unaligned(SP);
			SP += 4;
		}

	if (m_amout & (1 << 31))
	{
		v60WritePSW((v60ReadPSW() & 0xffff0000) | m_program->read_word_unaligned(SP));
		SP += 4;
	}

	return m_amlength1 + 1;
}

UINT32 v60_device::opPUSHM()
{
	int i;

	m_modadd = PC + 1;
	m_moddim = 2;

	// Read the bit register list
	m_amlength1 = ReadAM();

	if (m_amout & (1 << 31))
	{
		SP -= 4;
		m_program->write_dword_unaligned(SP, v60ReadPSW());
	}

	for (i = 0;i < 31;i++)
		if (m_amout & (1 << (30 - i)))
		{
			SP -= 4;
			m_program->write_dword_unaligned(SP, m_reg[(30 - i)]);
		}


	return m_amlength1 + 1;
}

UINT32 v60_device::opTESTB() /* TRUSTED */
{
	m_modadd = PC + 1;
	m_moddim = 0;

	// Read the operand
	m_amlength1 = ReadAM();

	_Z = (m_amout == 0);
	_S = ((m_amout & 0x80) != 0);
	_CY = 0;
	_OV = 0;

	return m_amlength1 + 1;
}

UINT32 v60_device::opTESTH() /* TRUSTED */
{
	m_modadd = PC + 1;
	m_moddim = 1;

	// Read the operand
	m_amlength1 = ReadAM();

	_Z = (m_amout == 0);
	_S = ((m_amout & 0x8000) != 0);
	_CY = 0;
	_OV = 0;

	return m_amlength1 + 1;
}

UINT32 v60_device::opTESTW() /* TRUSTED */
{
	m_modadd = PC + 1;
	m_moddim = 2;

	// Read the operand
	m_amlength1 = ReadAM();

	_Z = (m_amout == 0);
	_S = ((m_amout & 0x80000000) != 0);
	_CY = 0;
	_OV = 0;

	return m_amlength1 + 1;
}

UINT32 v60_device::opPUSH()
{
	m_modadd = PC + 1;
	m_moddim = 2;

	m_amlength1 = ReadAM();

	SP-=4;
	m_program->write_dword_unaligned(SP, m_amout);

	return m_amlength1 + 1;
}

UINT32 v60_device::opPOP()
{
	m_modadd = PC + 1;
	m_moddim = 2;
	m_modwritevalw = m_program->read_dword_unaligned(SP);
	SP +=4;
	m_amlength1 = WriteAM();

	return m_amlength1 + 1;
}


UINT32 v60_device::opINCB_0() { m_modm = 0; return opINCB(); }
UINT32 v60_device::opINCB_1() { m_modm = 1; return opINCB(); }
UINT32 v60_device::opINCH_0() { m_modm = 0; return opINCH(); }
UINT32 v60_device::opINCH_1() { m_modm = 1; return opINCH(); }
UINT32 v60_device::opINCW_0() { m_modm = 0; return opINCW(); }
UINT32 v60_device::opINCW_1() { m_modm = 1; return opINCW(); }

UINT32 v60_device::opDECB_0() { m_modm = 0; return opDECB(); }
UINT32 v60_device::opDECB_1() { m_modm = 1; return opDECB(); }
UINT32 v60_device::opDECH_0() { m_modm = 0; return opDECH(); }
UINT32 v60_device::opDECH_1() { m_modm = 1; return opDECH(); }
UINT32 v60_device::opDECW_0() { m_modm = 0; return opDECW(); }
UINT32 v60_device::opDECW_1() { m_modm = 1; return opDECW(); }

UINT32 v60_device::opJMP_0() { m_modm = 0; return opJMP(); }
UINT32 v60_device::opJMP_1() { m_modm = 1; return opJMP(); }

UINT32 v60_device::opJSR_0() { m_modm = 0; return opJSR(); }
UINT32 v60_device::opJSR_1() { m_modm = 1; return opJSR(); }

UINT32 v60_device::opPREPARE_0() { m_modm = 0; return opPREPARE(); }
UINT32 v60_device::opPREPARE_1() { m_modm = 1; return opPREPARE(); }

UINT32 v60_device::opRET_0() { m_modm = 0; return opRET(); }
UINT32 v60_device::opRET_1() { m_modm = 1; return opRET(); }

UINT32 v60_device::opTRAP_0() { m_modm = 0; return opTRAP(); }
UINT32 v60_device::opTRAP_1() { m_modm = 1; return opTRAP(); }

UINT32 v60_device::opRETIU_0() { m_modm = 0; return opRETIU(); }
UINT32 v60_device::opRETIU_1() { m_modm = 1; return opRETIU(); }

UINT32 v60_device::opRETIS_0() { m_modm = 0; return opRETIS(); }
UINT32 v60_device::opRETIS_1() { m_modm = 1; return opRETIS(); }

UINT32 v60_device::opGETPSW_0() { m_modm = 0; return opGETPSW(); }
UINT32 v60_device::opGETPSW_1() { m_modm = 1; return opGETPSW(); }

UINT32 v60_device::opTASI_0() { m_modm = 0; return opTASI(); }
UINT32 v60_device::opTASI_1() { m_modm = 1; return opTASI(); }

UINT32 v60_device::opCLRTLB_0() { m_modm = 0; return opCLRTLB(); }
UINT32 v60_device::opCLRTLB_1() { m_modm = 1; return opCLRTLB(); }

UINT32 v60_device::opPOPM_0() { m_modm = 0; return opPOPM(); }
UINT32 v60_device::opPOPM_1() { m_modm = 1; return opPOPM(); }

UINT32 v60_device::opPUSHM_0() { m_modm = 0; return opPUSHM(); }
UINT32 v60_device::opPUSHM_1() { m_modm = 1; return opPUSHM(); }

UINT32 v60_device::opTESTB_0() { m_modm = 0; return opTESTB(); }
UINT32 v60_device::opTESTB_1() { m_modm = 1; return opTESTB(); }

UINT32 v60_device::opTESTH_0() { m_modm = 0; return opTESTH(); }
UINT32 v60_device::opTESTH_1() { m_modm = 1; return opTESTH(); }

UINT32 v60_device::opTESTW_0() { m_modm = 0; return opTESTW(); }
UINT32 v60_device::opTESTW_1() { m_modm = 1; return opTESTW(); }

UINT32 v60_device::opPUSH_0() { m_modm = 0; return opPUSH(); }
UINT32 v60_device::opPUSH_1() { m_modm = 1; return opPUSH(); }

UINT32 v60_device::opPOP_0() { m_modm = 0; return opPOP(); }
UINT32 v60_device::opPOP_1() { m_modm = 1; return opPOP(); }

UINT32 v60_device::opSTTASK_0() { m_modm = 0; return opSTTASK(); }
UINT32 v60_device::opSTTASK_1() { m_modm = 1; return opSTTASK(); }
