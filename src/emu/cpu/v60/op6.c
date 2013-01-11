
/*
    FULLY TRUSTED
*/

static UINT32 opTB(v60_state *cpustate, int reg) /* TRUSTED */
{
	if (cpustate->reg[reg] == 0)
	{
		cpustate->PC += (INT16)OpRead16(cpustate, cpustate->PC + 2);
		return 0;
	}

	return 4;
}

static UINT32 opDBGT(v60_state *cpustate, int reg) /* TRUSTED */
{
	cpustate->reg[reg]--;

	NORMALIZEFLAGS(cpustate);
	if ((cpustate->reg[reg] != 0) && !((cpustate->_S ^ cpustate->_OV) | cpustate->_Z))
	{
		cpustate->PC += (INT16)OpRead16(cpustate, cpustate->PC + 2);
		return 0;
	}

	return 4;
}

static UINT32 opDBLE(v60_state *cpustate, int reg) /* TRUSTED */
{
	cpustate->reg[reg]--;

	NORMALIZEFLAGS(cpustate);
	if ((cpustate->reg[reg] != 0) && ((cpustate->_S ^ cpustate->_OV) | cpustate->_Z))
	{
		cpustate->PC += (INT16)OpRead16(cpustate, cpustate->PC + 2);
		return 0;
	}

	return 4;
}


static UINT32 opDBGE(v60_state *cpustate, int reg) /* TRUSTED */
{
	cpustate->reg[reg]--;

	NORMALIZEFLAGS(cpustate);
	if ((cpustate->reg[reg] != 0) && !(cpustate->_S ^ cpustate->_OV))
	{
		cpustate->PC += (INT16)OpRead16(cpustate, cpustate->PC + 2);
		return 0;
	}

	return 4;
}

static UINT32 opDBLT(v60_state *cpustate, int reg) /* TRUSTED */
{
	cpustate->reg[reg]--;

	NORMALIZEFLAGS(cpustate);
	if ((cpustate->reg[reg] != 0) && (cpustate->_S ^ cpustate->_OV))
	{
		cpustate->PC += (INT16)OpRead16(cpustate, cpustate->PC + 2);
		return 0;
	}

	return 4;
}

static UINT32 opDBH(v60_state *cpustate, int reg) /* TRUSTED */
{
	cpustate->reg[reg]--;

	if ((cpustate->reg[reg] != 0) && !(cpustate->_CY | cpustate->_Z))
	{
		cpustate->PC += (INT16)OpRead16(cpustate, cpustate->PC + 2);
		return 0;
	}

	return 4;
}

static UINT32 opDBNH(v60_state *cpustate, int reg) /* TRUSTED */
{
	cpustate->reg[reg]--;

	if ((cpustate->reg[reg] != 0) && (cpustate->_CY | cpustate->_Z))
	{
		cpustate->PC += (INT16)OpRead16(cpustate, cpustate->PC + 2);
		return 0;
	}

	return 4;
}


static UINT32 opDBL(v60_state *cpustate, int reg) /* TRUSTED */
{
	cpustate->reg[reg]--;

	if ((cpustate->reg[reg] != 0) && (cpustate->_CY))
	{
		cpustate->PC += (INT16)OpRead16(cpustate, cpustate->PC + 2);
		return 0;
	}

	return 4;
}

static UINT32 opDBNL(v60_state *cpustate, int reg) /* TRUSTED */
{
	cpustate->reg[reg]--;

	if ((cpustate->reg[reg] != 0) && !(cpustate->_CY))
	{
		cpustate->PC += (INT16)OpRead16(cpustate, cpustate->PC + 2);
		return 0;
	}

	return 4;
}

static UINT32 opDBE(v60_state *cpustate, int reg) /* TRUSTED */
{
	cpustate->reg[reg]--;

	if ((cpustate->reg[reg] != 0) && (cpustate->_Z))
	{
		cpustate->PC += (INT16)OpRead16(cpustate, cpustate->PC + 2);
		return 0;
	}

	return 4;
}

static UINT32 opDBNE(v60_state *cpustate, int reg) /* TRUSTED */
{
	cpustate->reg[reg]--;

	if ((cpustate->reg[reg] != 0) && !(cpustate->_Z))
	{
		cpustate->PC += (INT16)OpRead16(cpustate, cpustate->PC + 2);
		return 0;
	}

	return 4;
}

static UINT32 opDBV(v60_state *cpustate, int reg) /* TRUSTED */
{
	cpustate->reg[reg]--;

	if ((cpustate->reg[reg] != 0) && (cpustate->_OV))
	{
		cpustate->PC += (INT16)OpRead16(cpustate, cpustate->PC + 2);
		return 0;
	}

	return 4;
}

static UINT32 opDBNV(v60_state *cpustate, int reg) /* TRUSTED */
{
	cpustate->reg[reg]--;

	if ((cpustate->reg[reg] != 0) && !(cpustate->_OV))
	{
		cpustate->PC += (INT16)OpRead16(cpustate, cpustate->PC + 2);
		return 0;
	}

	return 4;
}

static UINT32 opDBN(v60_state *cpustate, int reg) /* TRUSTED */
{
	cpustate->reg[reg]--;

	if ((cpustate->reg[reg] != 0) && (cpustate->_S))
	{
		cpustate->PC += (INT16)OpRead16(cpustate, cpustate->PC + 2);
		return 0;
	}

	return 4;
}

static UINT32 opDBP(v60_state *cpustate, int reg) /* TRUSTED */
{
	cpustate->reg[reg]--;

	if ((cpustate->reg[reg] != 0) && !(cpustate->_S))
	{
		cpustate->PC += (INT16)OpRead16(cpustate, cpustate->PC + 2);
		return 0;
	}

	return 4;
}

static UINT32 opDBR(v60_state *cpustate, int reg) /* TRUSTED */
{
	cpustate->reg[reg]--;

	if (cpustate->reg[reg] != 0)
	{
		cpustate->PC += (INT16)OpRead16(cpustate, cpustate->PC + 2);
		return 0;
	}

	return 4;
}

static UINT32 (*const OpC6Table[8])(v60_state *, int reg) = /* TRUSTED */
{
	opDBV,
	opDBL,
	opDBE,
	opDBNH,
	opDBN,
	opDBR,
	opDBLT,
	opDBLE
};

static UINT32 (*const OpC7Table[8])(v60_state *, int reg) = /* TRUSTED */
{
	opDBNV,
	opDBNL,
	opDBNE,
	opDBH,
	opDBP,
	opTB,
	opDBGE,
	opDBGT
};


static UINT32 opC6(v60_state *cpustate) /* TRUSTED */
{
	UINT8 appb = OpRead8(cpustate, cpustate->PC + 1);
	return OpC6Table[appb >> 5](cpustate, appb & 0x1f);
}

static UINT32 opC7(v60_state *cpustate) /* TRUSTED */
{
	UINT8 appb = OpRead8(cpustate, cpustate->PC + 1);
	return OpC7Table[appb >> 5](cpustate, appb & 0x1f);
}
