
/*
    FULLY TRUSTED
*/

static UINT32 opBGT8(void) /* TRUSTED */
{
	NORMALIZEFLAGS();

	if (!((_S ^ _OV) | _Z))
	{
		PC += (INT8)OpRead8(PC + 1);
		return 0;
	}

	return 2;
}

static UINT32 opBGT16(void) /* TRUSTED */
{
	NORMALIZEFLAGS();

	if (!((_S ^ _OV) | _Z))
	{
		PC += (INT16)OpRead16(PC + 1);
		return 0;
	}

	return 3;
}


static UINT32 opBGE8(void) /* TRUSTED */
{
	NORMALIZEFLAGS();

	if (!(_S ^ _OV))
	{
		PC += (INT8)OpRead8(PC + 1);
		return 0;
	}

	return 2;
}

static UINT32 opBGE16(void) /* TRUSTED */
{
	NORMALIZEFLAGS();

	if (!(_S ^ _OV))
	{
		PC += (INT16)OpRead16(PC + 1);
		return 0;
	}

	return 3;
}

static UINT32 opBLT8(void) /* TRUSTED */
{
	NORMALIZEFLAGS();

	if ((_S ^ _OV))
	{
		PC += (INT8)OpRead8(PC + 1);
		return 0;
	}

	return 2;
}

static UINT32 opBLT16(void) /* TRUSTED */
{
	NORMALIZEFLAGS();

	if ((_S ^ _OV))
	{
		PC += (INT16)OpRead16(PC + 1);
		return 0;
	}

	return 3;
}


static UINT32 opBLE8(void) /* TRUSTED */
{
	NORMALIZEFLAGS();

	if (((_S ^ _OV) | _Z))
	{
		PC += (INT8)OpRead8(PC + 1);
		return 0;
	}

	return 2;
}

static UINT32 opBLE16(void) /* TRUSTED */
{
	NORMALIZEFLAGS();

	if (((_S ^ _OV) | _Z))
	{
		PC += (INT16)OpRead16(PC + 1);
		return 0;
	}

	return 3;
}

static UINT32 opBH8(void) /* TRUSTED */
{
	if (!(_CY | _Z))
	{
		PC += (INT8)OpRead8(PC + 1);
		return 0;
	}

	return 2;
}

static UINT32 opBH16(void) /* TRUSTED */
{
	if (!(_CY | _Z))
	{
		PC += (INT16)OpRead16(PC + 1);
		return 0;
	}

	return 3;
}

static UINT32 opBNH8(void) /* TRUSTED */
{
	if ((_CY | _Z))
	{
		PC += (INT8)OpRead8(PC + 1);
		return 0;
	}

	return 2;
}

static UINT32 opBNH16(void) /* TRUSTED */
{
	if ((_CY | _Z))
	{
		PC += (INT16)OpRead16(PC + 1);
		return 0;
	}

	return 3;
}

static UINT32 opBNL8(void) /* TRUSTED */
{
	if (!(_CY))
	{
		PC += (INT8)OpRead8(PC + 1);
		return 0;
	}

	return 2;
}

static UINT32 opBNL16(void) /* TRUSTED */
{
	if (!(_CY))
	{
		PC += (INT16)OpRead16(PC + 1);
		return 0;
	}

	return 3;
}

static UINT32 opBL8(void) /* TRUSTED */
{
	if ((_CY))
	{
		PC += (INT8)OpRead8(PC + 1);
		return 0;
	}

	return 2;
}

static UINT32 opBL16(void) /* TRUSTED */
{
	if ((_CY))
	{
		PC += (INT16)OpRead16(PC + 1);
		return 0;
	}

	return 3;
}

static UINT32 opBNE8(void) /* TRUSTED */
{
	if (!(_Z))
	{
		PC += (INT8)OpRead8(PC + 1);
		return 0;
	}

	return 2;
}

static UINT32 opBNE16(void) /* TRUSTED */
{
	if (!(_Z))
	{
		PC += (INT16)OpRead16(PC + 1);
		return 0;
	}

	return 3;
}

static UINT32 opBE8(void) /* TRUSTED */
{
	if ((_Z))
	{
		PC += (INT8)OpRead8(PC + 1);
		return 0;
	}

	return 2;
}

static UINT32 opBE16(void) /* TRUSTED */
{
	if ((_Z))
	{
		PC += (INT16)OpRead16(PC + 1);
		return 0;
	}

	return 3;
}

static UINT32 opBNV8(void) /* TRUSTED */
{
	if (!(_OV))
	{
		PC += (INT8)OpRead8(PC + 1);
		return 0;
	}

	return 2;
}

static UINT32 opBNV16(void) /* TRUSTED */
{
	if (!(_OV))
	{
		PC += (INT16)OpRead16(PC + 1);
		return 0;
	}

	return 3;
}

static UINT32 opBV8(void) /* TRUSTED */
{
	if ((_OV))
	{
		PC += (INT8)OpRead8(PC + 1);
		return 0;
	}

	return 2;
}

static UINT32 opBV16(void) /* TRUSTED */
{
	if ((_OV))
	{
		PC += (INT16)OpRead16(PC + 1);
		return 0;
	}

	return 3;
}

static UINT32 opBP8(void) /* TRUSTED */
{
	if (!(_S))
	{
		PC += (INT8)OpRead8(PC + 1);
		return 0;
	}

	return 2;
}

static UINT32 opBP16(void) /* TRUSTED */
{
	if (!(_S))
	{
		PC += (INT16)OpRead16(PC + 1);
		return 0;
	}

	return 3;
}

static UINT32 opBN8(void) /* TRUSTED */
{
	if ((_S))
	{
		PC += (INT8)OpRead8(PC + 1);
		return 0;
	}

	return 2;
}

static UINT32 opBN16(void) /* TRUSTED */
{
	if ((_S))
	{
		PC += (INT16)OpRead16(PC + 1);
		return 0;
	}

	return 3;
}

static UINT32 opBR8(void) /* TRUSTED */
{
	PC += (INT8)OpRead8(PC + 1);
	return 0;
}

static UINT32 opBR16(void) /* TRUSTED */
{
	PC += (INT16)OpRead16(PC + 1);
	return 0;
}

static UINT32 opBSR(void) /* TRUSTED */
{
	// Save Next PC onto the stack
	SP -= 4;
	MemWrite32(SP, PC+3);

	// Jump to subroutine
	PC += (INT16)OpRead16(PC + 1);
	return 0;
}

