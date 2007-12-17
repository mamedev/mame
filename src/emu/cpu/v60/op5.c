
/*
 *  HALT: must add log
 */

static UINT32 opBRK(void)
{
/*
    UINT32 oldPSW = v60_update_psw_for_exception(0, 0);

    SP -=4;
    MemWrite32(SP, EXCEPTION_CODE_AND_SIZE(0x0d00, 4));
    SP -=4;
    MemWrite32(SP, oldPSW);
    SP -=4;
    MemWrite32(SP, PC + 1);
    PC = GETINTVECT(13);
    ChangePC(PC);
*/
	logerror("Skipping BRK opcode! PC=%x", PC);

	return 1;
}

static UINT32 opBRKV(void)
{
	UINT32 oldPSW = v60_update_psw_for_exception(0, 0);

	SP -=4;
	MemWrite32(SP, PC);
	SP -=4;
	MemWrite32(SP, EXCEPTION_CODE_AND_SIZE(0x1501, 4));
	SP -=4;
	MemWrite32(SP, oldPSW);
	SP -=4;
	MemWrite32(SP, PC + 1);
	PC = GETINTVECT(21);
	ChangePC(PC);

	return 0;
}

static UINT32 opCLRTLBA(void)
{
	// @@@ TLB not yet supported
	logerror("Skipping CLRTLBA opcode! PC=%x\n", PC);
	return 1;
}

static UINT32 opDISPOSE(void)
{
	SP = FP;
	FP = MemRead32(SP);
	SP +=4;

	return 1;
}

static UINT32 opHALT(void)
{
	// @@@ It should wait for an interrupt to occur
	//logerror("HALT found: skipping");
	return 1;
}

static UINT32 opNOP(void) /* TRUSTED */
{
	return 1;
}

static UINT32 opRSR(void)
{
	PC = MemRead32(SP);
	SP +=4;
	ChangePC(PC);

	return 0;
}

static UINT32 opTRAPFL(void)
{
	if ((TKCW & 0x1F0) & ((v60ReadPSW() & 0x1F00) >> 4))
	{
		// @@@ FPU exception
		fatalerror("Hit TRAPFL! PC=%x", PC);
	}

	return 1;
}







