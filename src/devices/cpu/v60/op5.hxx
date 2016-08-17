// license:BSD-3-Clause
// copyright-holders:Farfetch'd, R. Belmont
/*
 *  HALT: must add log
 */

UINT32 v60_device::opBRK()
{
/*
    UINT32 oldPSW = v60_update_psw_for_exception(0, 0);

    SP -=4;
    m_program->write_dword_unaligned(SP, EXCEPTION_CODE_AND_SIZE(0x0d00, 4));
    SP -=4;
    m_program->write_dword_unaligned(SP, oldPSW);
    SP -=4;
    m_program->write_dword_unaligned(SP, PC + 1);
    PC = GETINTVECT(13);
*/
	logerror("Skipping BRK opcode! PC=%x", PC);

	return 1;
}

UINT32 v60_device::opBRKV()
{
	UINT32 oldPSW = v60_update_psw_for_exception(0, 0);

	SP -=4;
	m_program->write_dword_unaligned(SP, PC);
	SP -=4;
	m_program->write_dword_unaligned(SP, EXCEPTION_CODE_AND_SIZE(0x1501, 4));
	SP -=4;
	m_program->write_dword_unaligned(SP, oldPSW);
	SP -=4;
	m_program->write_dword_unaligned(SP, PC + 1);
	PC = GETINTVECT(21);

	return 0;
}

UINT32 v60_device::opCLRTLBA()
{
	// @@@ TLB not yet supported
	logerror("Skipping CLRTLBA opcode! PC=%x\n", PC);
	return 1;
}

UINT32 v60_device::opDISPOSE()
{
	SP = FP;
	FP = m_program->read_dword_unaligned(SP);
	SP +=4;

	return 1;
}

UINT32 v60_device::opHALT()
{
	// @@@ It should wait for an interrupt to occur
	//logerror("HALT found: skipping");
	return 1;
}

UINT32 v60_device::opNOP() /* TRUSTED */
{
	return 1;
}

UINT32 v60_device::opRSR()
{
	PC = m_program->read_dword_unaligned(SP);
	SP +=4;

	return 0;
}

UINT32 v60_device::opTRAPFL()
{
	if ((TKCW & 0x1F0) & ((v60ReadPSW() & 0x1F00) >> 4))
	{
		// @@@ FPU exception
		fatalerror("Hit TRAPFL! PC=%x\n", PC);
	}

	return 1;
}
