
/*
    FULLY TRUSTED
*/

static UINT32 opBGT8(v60_state *cpustate) /* TRUSTED */
{
	NORMALIZEFLAGS(cpustate);

	if (!((cpustate->_S ^ cpustate->_OV) | cpustate->_Z))
	{
		cpustate->PC += (INT8)OpRead8(cpustate, cpustate->PC + 1);
		return 0;
	}

	return 2;
}

static UINT32 opBGT16(v60_state *cpustate) /* TRUSTED */
{
	NORMALIZEFLAGS(cpustate);

	if (!((cpustate->_S ^ cpustate->_OV) | cpustate->_Z))
	{
		cpustate->PC += (INT16)OpRead16(cpustate, cpustate->PC + 1);
		return 0;
	}

	return 3;
}


static UINT32 opBGE8(v60_state *cpustate) /* TRUSTED */
{
	NORMALIZEFLAGS(cpustate);

	if (!(cpustate->_S ^ cpustate->_OV))
	{
		cpustate->PC += (INT8)OpRead8(cpustate, cpustate->PC + 1);
		return 0;
	}

	return 2;
}

static UINT32 opBGE16(v60_state *cpustate) /* TRUSTED */
{
	NORMALIZEFLAGS(cpustate);

	if (!(cpustate->_S ^ cpustate->_OV))
	{
		cpustate->PC += (INT16)OpRead16(cpustate, cpustate->PC + 1);
		return 0;
	}

	return 3;
}

static UINT32 opBLT8(v60_state *cpustate) /* TRUSTED */
{
	NORMALIZEFLAGS(cpustate);

	if ((cpustate->_S ^ cpustate->_OV))
	{
		cpustate->PC += (INT8)OpRead8(cpustate, cpustate->PC + 1);
		return 0;
	}

	return 2;
}

static UINT32 opBLT16(v60_state *cpustate) /* TRUSTED */
{
	NORMALIZEFLAGS(cpustate);

	if ((cpustate->_S ^ cpustate->_OV))
	{
		cpustate->PC += (INT16)OpRead16(cpustate, cpustate->PC + 1);
		return 0;
	}

	return 3;
}


static UINT32 opBLE8(v60_state *cpustate) /* TRUSTED */
{
	NORMALIZEFLAGS(cpustate);

	if (((cpustate->_S ^ cpustate->_OV) | cpustate->_Z))
	{
		cpustate->PC += (INT8)OpRead8(cpustate, cpustate->PC + 1);
		return 0;
	}

	return 2;
}

static UINT32 opBLE16(v60_state *cpustate) /* TRUSTED */
{
	NORMALIZEFLAGS(cpustate);

	if (((cpustate->_S ^ cpustate->_OV) | cpustate->_Z))
	{
		cpustate->PC += (INT16)OpRead16(cpustate, cpustate->PC + 1);
		return 0;
	}

	return 3;
}

static UINT32 opBH8(v60_state *cpustate) /* TRUSTED */
{
	if (!(cpustate->_CY | cpustate->_Z))
	{
		cpustate->PC += (INT8)OpRead8(cpustate, cpustate->PC + 1);
		return 0;
	}

	return 2;
}

static UINT32 opBH16(v60_state *cpustate) /* TRUSTED */
{
	if (!(cpustate->_CY | cpustate->_Z))
	{
		cpustate->PC += (INT16)OpRead16(cpustate, cpustate->PC + 1);
		return 0;
	}

	return 3;
}

static UINT32 opBNH8(v60_state *cpustate) /* TRUSTED */
{
	if ((cpustate->_CY | cpustate->_Z))
	{
		cpustate->PC += (INT8)OpRead8(cpustate, cpustate->PC + 1);
		return 0;
	}

	return 2;
}

static UINT32 opBNH16(v60_state *cpustate) /* TRUSTED */
{
	if ((cpustate->_CY | cpustate->_Z))
	{
		cpustate->PC += (INT16)OpRead16(cpustate, cpustate->PC + 1);
		return 0;
	}

	return 3;
}

static UINT32 opBNL8(v60_state *cpustate) /* TRUSTED */
{
	if (!(cpustate->_CY))
	{
		cpustate->PC += (INT8)OpRead8(cpustate, cpustate->PC + 1);
		return 0;
	}

	return 2;
}

static UINT32 opBNL16(v60_state *cpustate) /* TRUSTED */
{
	if (!(cpustate->_CY))
	{
		cpustate->PC += (INT16)OpRead16(cpustate, cpustate->PC + 1);
		return 0;
	}

	return 3;
}

static UINT32 opBL8(v60_state *cpustate) /* TRUSTED */
{
	if ((cpustate->_CY))
	{
		cpustate->PC += (INT8)OpRead8(cpustate, cpustate->PC + 1);
		return 0;
	}

	return 2;
}

static UINT32 opBL16(v60_state *cpustate) /* TRUSTED */
{
	if ((cpustate->_CY))
	{
		cpustate->PC += (INT16)OpRead16(cpustate, cpustate->PC + 1);
		return 0;
	}

	return 3;
}

static UINT32 opBNE8(v60_state *cpustate) /* TRUSTED */
{
	if (!(cpustate->_Z))
	{
		cpustate->PC += (INT8)OpRead8(cpustate, cpustate->PC + 1);
		return 0;
	}

	return 2;
}

static UINT32 opBNE16(v60_state *cpustate) /* TRUSTED */
{
	if (!(cpustate->_Z))
	{
		cpustate->PC += (INT16)OpRead16(cpustate, cpustate->PC + 1);
		return 0;
	}

	return 3;
}

static UINT32 opBE8(v60_state *cpustate) /* TRUSTED */
{
	if ((cpustate->_Z))
	{
		cpustate->PC += (INT8)OpRead8(cpustate, cpustate->PC + 1);
		return 0;
	}

	return 2;
}

static UINT32 opBE16(v60_state *cpustate) /* TRUSTED */
{
	if ((cpustate->_Z))
	{
		cpustate->PC += (INT16)OpRead16(cpustate, cpustate->PC + 1);
		return 0;
	}

	return 3;
}

static UINT32 opBNV8(v60_state *cpustate) /* TRUSTED */
{
	if (!(cpustate->_OV))
	{
		cpustate->PC += (INT8)OpRead8(cpustate, cpustate->PC + 1);
		return 0;
	}

	return 2;
}

static UINT32 opBNV16(v60_state *cpustate) /* TRUSTED */
{
	if (!(cpustate->_OV))
	{
		cpustate->PC += (INT16)OpRead16(cpustate, cpustate->PC + 1);
		return 0;
	}

	return 3;
}

static UINT32 opBV8(v60_state *cpustate) /* TRUSTED */
{
	if ((cpustate->_OV))
	{
		cpustate->PC += (INT8)OpRead8(cpustate, cpustate->PC + 1);
		return 0;
	}

	return 2;
}

static UINT32 opBV16(v60_state *cpustate) /* TRUSTED */
{
	if ((cpustate->_OV))
	{
		cpustate->PC += (INT16)OpRead16(cpustate, cpustate->PC + 1);
		return 0;
	}

	return 3;
}

static UINT32 opBP8(v60_state *cpustate) /* TRUSTED */
{
	if (!(cpustate->_S))
	{
		cpustate->PC += (INT8)OpRead8(cpustate, cpustate->PC + 1);
		return 0;
	}

	return 2;
}

static UINT32 opBP16(v60_state *cpustate) /* TRUSTED */
{
	if (!(cpustate->_S))
	{
		cpustate->PC += (INT16)OpRead16(cpustate, cpustate->PC + 1);
		return 0;
	}

	return 3;
}

static UINT32 opBN8(v60_state *cpustate) /* TRUSTED */
{
	if ((cpustate->_S))
	{
		cpustate->PC += (INT8)OpRead8(cpustate, cpustate->PC + 1);
		return 0;
	}

	return 2;
}

static UINT32 opBN16(v60_state *cpustate) /* TRUSTED */
{
	if ((cpustate->_S))
	{
		cpustate->PC += (INT16)OpRead16(cpustate, cpustate->PC + 1);
		return 0;
	}

	return 3;
}

static UINT32 opBR8(v60_state *cpustate) /* TRUSTED */
{
	cpustate->PC += (INT8)OpRead8(cpustate, cpustate->PC + 1);
	return 0;
}

static UINT32 opBR16(v60_state *cpustate) /* TRUSTED */
{
	cpustate->PC += (INT16)OpRead16(cpustate, cpustate->PC + 1);
	return 0;
}

static UINT32 opBSR(v60_state *cpustate) /* TRUSTED */
{
	// Save Next cpustate->PC onto the stack
	cpustate->SP -= 4;
	cpustate->program->write_dword_unaligned(cpustate->SP, cpustate->PC + 3);

	// Jump to subroutine
	cpustate->PC += (INT16)OpRead16(cpustate, cpustate->PC + 1);
	return 0;
}
