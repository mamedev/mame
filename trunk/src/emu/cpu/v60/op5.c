
/*
 *  HALT: must add log
 */

static UINT32 opBRK(v60_state *cpustate)
{
/*
    UINT32 oldPSW = v60_update_psw_for_exception(cpustate, 0, 0);

    cpustate->SP -=4;
    cpustate->program->write_dword_unaligned(cpustate->SP, EXCEPTION_CODE_AND_SIZE(0x0d00, 4));
    cpustate->SP -=4;
    cpustate->program->write_dword_unaligned(cpustate->SP, oldPSW);
    cpustate->SP -=4;
    cpustate->program->write_dword_unaligned(cpustate->SP, cpustate->PC + 1);
    cpustate->PC = GETINTVECT(cpustate, 13);
*/
	logerror("Skipping BRK opcode! cpustate->PC=%x", cpustate->PC);

	return 1;
}

static UINT32 opBRKV(v60_state *cpustate)
{
	UINT32 oldPSW = v60_update_psw_for_exception(cpustate, 0, 0);

	cpustate->SP -=4;
	cpustate->program->write_dword_unaligned(cpustate->SP, cpustate->PC);
	cpustate->SP -=4;
	cpustate->program->write_dword_unaligned(cpustate->SP, EXCEPTION_CODE_AND_SIZE(0x1501, 4));
	cpustate->SP -=4;
	cpustate->program->write_dword_unaligned(cpustate->SP, oldPSW);
	cpustate->SP -=4;
	cpustate->program->write_dword_unaligned(cpustate->SP, cpustate->PC + 1);
	cpustate->PC = GETINTVECT(cpustate, 21);

	return 0;
}

static UINT32 opCLRTLBA(v60_state *cpustate)
{
	// @@@ TLB not yet supported
	logerror("Skipping CLRTLBA opcode! cpustate->PC=%x\n", cpustate->PC);
	return 1;
}

static UINT32 opDISPOSE(v60_state *cpustate)
{
	cpustate->SP = cpustate->FP;
	cpustate->FP = cpustate->program->read_dword_unaligned(cpustate->SP);
	cpustate->SP +=4;

	return 1;
}

static UINT32 opHALT(v60_state *cpustate)
{
	// @@@ It should wait for an interrupt to occur
	//logerror("HALT found: skipping");
	return 1;
}

static UINT32 opNOP(v60_state *cpustate) /* TRUSTED */
{
	return 1;
}

static UINT32 opRSR(v60_state *cpustate)
{
	cpustate->PC = cpustate->program->read_dword_unaligned(cpustate->SP);
	cpustate->SP +=4;

	return 0;
}

static UINT32 opTRAPFL(v60_state *cpustate)
{
	if ((cpustate->TKCW & 0x1F0) & ((v60ReadPSW(cpustate) & 0x1F00) >> 4))
	{
		// @@@ FPU exception
		fatalerror("Hit TRAPFL! cpustate->PC=%x\n", cpustate->PC);
	}

	return 1;
}
