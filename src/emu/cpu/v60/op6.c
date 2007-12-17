
/*
    FULLY TRUSTED
*/

static UINT32 opTB(int reg) /* TRUSTED */
{
	if (v60.reg[reg] == 0)
	{
		PC += (INT16)OpRead16(PC + 2);
		return 0;
	}

	return 4;
}

static UINT32 opDBGT(int reg) /* TRUSTED */
{
	v60.reg[reg]--;

	NORMALIZEFLAGS();
	if ((v60.reg[reg] != 0) && !((_S ^ _OV) | _Z))
	{
		PC += (INT16)OpRead16(PC + 2);
		return 0;
	}

	return 4;
}

static UINT32 opDBLE(int reg) /* TRUSTED */
{
	v60.reg[reg]--;

	NORMALIZEFLAGS();
	if ((v60.reg[reg] != 0) && ((_S ^ _OV) | _Z))
	{
		PC += (INT16)OpRead16(PC + 2);
		return 0;
	}

	return 4;
}


static UINT32 opDBGE(int reg) /* TRUSTED */
{
	v60.reg[reg]--;

	NORMALIZEFLAGS();
	if ((v60.reg[reg] != 0) && !(_S ^ _OV))
	{
		PC += (INT16)OpRead16(PC + 2);
		return 0;
	}

	return 4;
}

static UINT32 opDBLT(int reg) /* TRUSTED */
{
	v60.reg[reg]--;

	NORMALIZEFLAGS();
	if ((v60.reg[reg] != 0) && (_S ^ _OV))
	{
		PC += (INT16)OpRead16(PC + 2);
		return 0;
	}

	return 4;
}

static UINT32 opDBH(int reg) /* TRUSTED */
{
	v60.reg[reg]--;

	if ((v60.reg[reg] != 0) && !(_CY | _Z))
	{
		PC += (INT16)OpRead16(PC + 2);
		return 0;
	}

	return 4;
}

static UINT32 opDBNH(int reg) /* TRUSTED */
{
	v60.reg[reg]--;

	if ((v60.reg[reg] != 0) && (_CY | _Z))
	{
		PC += (INT16)OpRead16(PC + 2);
		return 0;
	}

	return 4;
}


static UINT32 opDBL(int reg) /* TRUSTED */
{
	v60.reg[reg]--;

	if ((v60.reg[reg] != 0) && (_CY))
	{
		PC += (INT16)OpRead16(PC + 2);
		return 0;
	}

	return 4;
}

static UINT32 opDBNL(int reg) /* TRUSTED */
{
	v60.reg[reg]--;

	if ((v60.reg[reg] != 0) && !(_CY))
	{
		PC += (INT16)OpRead16(PC + 2);
		return 0;
	}

	return 4;
}

static UINT32 opDBE(int reg) /* TRUSTED */
{
	v60.reg[reg]--;

	if ((v60.reg[reg] != 0) && (_Z))
	{
		PC += (INT16)OpRead16(PC + 2);
		return 0;
	}

	return 4;
}

static UINT32 opDBNE(int reg) /* TRUSTED */
{
	v60.reg[reg]--;

	if ((v60.reg[reg] != 0) && !(_Z))
	{
		PC += (INT16)OpRead16(PC + 2);
		return 0;
	}

	return 4;
}

static UINT32 opDBV(int reg) /* TRUSTED */
{
	v60.reg[reg]--;

	if ((v60.reg[reg] != 0) && (_OV))
	{
		PC += (INT16)OpRead16(PC + 2);
		return 0;
	}

	return 4;
}

static UINT32 opDBNV(int reg) /* TRUSTED */
{
	v60.reg[reg]--;

	if ((v60.reg[reg] != 0) && !(_OV))
	{
		PC += (INT16)OpRead16(PC + 2);
		return 0;
	}

	return 4;
}

static UINT32 opDBN(int reg) /* TRUSTED */
{
	v60.reg[reg]--;

	if ((v60.reg[reg] != 0) && (_S))
	{
		PC += (INT16)OpRead16(PC + 2);
		return 0;
	}

	return 4;
}

static UINT32 opDBP(int reg) /* TRUSTED */
{
	v60.reg[reg]--;

	if ((v60.reg[reg] != 0) && !(_S))
	{
		PC += (INT16)OpRead16(PC + 2);
		return 0;
	}

	return 4;
}

static UINT32 opDBR(int reg) /* TRUSTED */
{
	v60.reg[reg]--;

	if (v60.reg[reg] != 0)
	{
		PC += (INT16)OpRead16(PC + 2);
		return 0;
	}

	return 4;
}

static UINT32 (*const OpC6Table[8])(int reg) = /* TRUSTED */
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

static UINT32 (*const OpC7Table[8])(int reg) = /* TRUSTED */
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


static UINT32 opC6(void) /* TRUSTED */
{
	UINT8 appb=OpRead8(PC + 1);
	return OpC6Table[appb>>5](appb&0x1f);
}

static UINT32 opC7(void) /* TRUSTED */
{
	UINT8 appb=OpRead8(PC + 1);
	return OpC7Table[appb>>5](appb&0x1f);
}

