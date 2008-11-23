
// AM3 Functions (for ReadAM)
// **************************

static UINT32 am3Register(void)
{
	switch (modDim)
	{
	case 0:
		SETREG8(v60.reg[modVal&0x1F], modWriteValB);
		break;
	case 1:
		SETREG16(v60.reg[modVal&0x1F], modWriteValH);
		break;
	case 2:
		v60.reg[modVal&0x1F] = modWriteValW;
		break;
	}

	return 1;
}

static UINT32 am3RegisterIndirect(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, v60.reg[modVal&0x1F], modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, v60.reg[modVal&0x1F], modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, v60.reg[modVal&0x1F], modWriteValW);
		break;
	}

	return 1;
}

static UINT32 am3RegisterIndirectIndexed(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, v60.reg[modVal2&0x1F] + v60.reg[modVal&0x1F], modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, v60.reg[modVal2&0x1F] + v60.reg[modVal&0x1F] * 2, modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, v60.reg[modVal2&0x1F] + v60.reg[modVal&0x1F] * 4, modWriteValW);
		break;
	}

	return 2;
}

static UINT32 am3Autoincrement(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, v60.reg[modVal&0x1F], modWriteValB);
		v60.reg[modVal&0x1F] += 1;
		break;
	case 1:
		MemWrite16(v60.program, v60.reg[modVal&0x1F], modWriteValH);
		v60.reg[modVal&0x1F] += 2;
		break;
	case 2:
		MemWrite32(v60.program, v60.reg[modVal&0x1F], modWriteValW);
		v60.reg[modVal&0x1F] += 4;
		break;
	}

	return 1;
}

static UINT32 am3Autodecrement(void)
{
	switch (modDim)
	{
	case 0:
		v60.reg[modVal&0x1F] -= 1;
		MemWrite8(v60.program, v60.reg[modVal&0x1F], modWriteValB);
		break;
	case 1:
		v60.reg[modVal&0x1F] -= 2;
		MemWrite16(v60.program, v60.reg[modVal&0x1F], modWriteValH);
		break;
	case 2:
		v60.reg[modVal&0x1F] -= 4;
		MemWrite32(v60.program, v60.reg[modVal&0x1F], modWriteValW);
		break;
	}

	return 1;
}

static UINT32 am3Displacement8(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, v60.reg[modVal&0x1F] + (INT8)OpRead8(v60.program,modAdd+1), modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, v60.reg[modVal&0x1F] + (INT8)OpRead8(v60.program,modAdd+1), modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, v60.reg[modVal&0x1F] + (INT8)OpRead8(v60.program,modAdd+1), modWriteValW);
		break;
	}

	return 2;
}

static UINT32 am3Displacement16(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, v60.reg[modVal&0x1F] + (INT16)OpRead16(v60.program,modAdd+1), modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, v60.reg[modVal&0x1F] + (INT16)OpRead16(v60.program,modAdd+1), modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, v60.reg[modVal&0x1F] + (INT16)OpRead16(v60.program,modAdd+1), modWriteValW);
		break;
	}

	return 3;
}

static UINT32 am3Displacement32(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, v60.reg[modVal&0x1F] + OpRead32(v60.program,modAdd+1), modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, v60.reg[modVal&0x1F] + OpRead32(v60.program,modAdd+1), modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, v60.reg[modVal&0x1F] + OpRead32(v60.program,modAdd+1), modWriteValW);
		break;
	}

	return 5;
}


static UINT32 am3DisplacementIndexed8(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, v60.reg[modVal2&0x1F] + (INT8)OpRead8(v60.program,modAdd+2) + v60.reg[modVal&0x1F], modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, v60.reg[modVal2&0x1F] + (INT8)OpRead8(v60.program,modAdd+2) + v60.reg[modVal&0x1F] * 2, modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, v60.reg[modVal2&0x1F] + (INT8)OpRead8(v60.program,modAdd+2) + v60.reg[modVal&0x1F] * 4, modWriteValW);
		break;
	}

	return 3;
}

static UINT32 am3DisplacementIndexed16(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, v60.reg[modVal2&0x1F] + (INT16)OpRead16(v60.program,modAdd+2) + v60.reg[modVal&0x1F], modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, v60.reg[modVal2&0x1F] + (INT16)OpRead16(v60.program,modAdd+2) + v60.reg[modVal&0x1F] * 2, modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, v60.reg[modVal2&0x1F] + (INT16)OpRead16(v60.program,modAdd+2) + v60.reg[modVal&0x1F] * 4, modWriteValW);
		break;
	}

	return 4;
}

static UINT32 am3DisplacementIndexed32(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, v60.reg[modVal2&0x1F] + OpRead32(v60.program,modAdd+2) + v60.reg[modVal&0x1F], modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, v60.reg[modVal2&0x1F] + OpRead32(v60.program,modAdd+2) + v60.reg[modVal&0x1F] * 2, modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, v60.reg[modVal2&0x1F] + OpRead32(v60.program,modAdd+2) + v60.reg[modVal&0x1F] * 4, modWriteValW);
		break;
	}

	return 6;
}


static UINT32 am3PCDisplacement8(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, PC + (INT8)OpRead8(v60.program,modAdd+1), modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, PC + (INT8)OpRead8(v60.program,modAdd+1), modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, PC + (INT8)OpRead8(v60.program,modAdd+1), modWriteValW);
		break;
	}

	return 2;
}

static UINT32 am3PCDisplacement16(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, PC + (INT16)OpRead16(v60.program,modAdd+1), modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, PC + (INT16)OpRead16(v60.program,modAdd+1), modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, PC + (INT16)OpRead16(v60.program,modAdd+1), modWriteValW);
		break;
	}

	return 3;
}

static UINT32 am3PCDisplacement32(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, PC + OpRead32(v60.program,modAdd+1), modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, PC + OpRead32(v60.program,modAdd+1), modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, PC + OpRead32(v60.program,modAdd+1), modWriteValW);
		break;
	}

	return 5;
}

static UINT32 am3PCDisplacementIndexed8(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, PC + (INT8)OpRead8(v60.program,modAdd+2) + v60.reg[modVal&0x1F], modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, PC + (INT8)OpRead8(v60.program,modAdd+2) + v60.reg[modVal&0x1F] * 2, modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, PC + (INT8)OpRead8(v60.program,modAdd+2) + v60.reg[modVal&0x1F] * 4, modWriteValW);
		break;
	}

	return 3;
}

static UINT32 am3PCDisplacementIndexed16(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, PC + (INT16)OpRead16(v60.program,modAdd+2) + v60.reg[modVal&0x1F], modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, PC + (INT16)OpRead16(v60.program,modAdd+2) + v60.reg[modVal&0x1F] * 2, modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, PC + (INT16)OpRead16(v60.program,modAdd+2) + v60.reg[modVal&0x1F] * 4, modWriteValW);
		break;
	}

	return 4;
}

static UINT32 am3PCDisplacementIndexed32(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, PC + OpRead32(v60.program,modAdd+2) + v60.reg[modVal&0x1F], modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, PC + OpRead32(v60.program,modAdd+2) + v60.reg[modVal&0x1F] * 2, modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, PC + OpRead32(v60.program,modAdd+2) + v60.reg[modVal&0x1F] * 4, modWriteValW);
		break;
	}

	return 6;
}

static UINT32 am3DisplacementIndirect8(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, MemRead32(v60.program,v60.reg[modVal&0x1F] + (INT8)OpRead8(v60.program,modAdd+1)), modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, MemRead32(v60.program,v60.reg[modVal&0x1F] + (INT8)OpRead8(v60.program,modAdd+1)), modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, MemRead32(v60.program,v60.reg[modVal&0x1F] + (INT8)OpRead8(v60.program,modAdd+1)), modWriteValW);
		break;
	}

	return 2;
}

static UINT32 am3DisplacementIndirect16(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, MemRead32(v60.program,v60.reg[modVal&0x1F] + (INT16)OpRead16(v60.program,modAdd+1)), modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, MemRead32(v60.program,v60.reg[modVal&0x1F] + (INT16)OpRead16(v60.program,modAdd+1)), modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, MemRead32(v60.program,v60.reg[modVal&0x1F] + (INT16)OpRead16(v60.program,modAdd+1)), modWriteValW);
		break;
	}

	return 3;
}

static UINT32 am3DisplacementIndirect32(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, MemRead32(v60.program,v60.reg[modVal&0x1F] + OpRead32(v60.program,modAdd+1)), modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, MemRead32(v60.program,v60.reg[modVal&0x1F] + OpRead32(v60.program,modAdd+1)), modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, MemRead32(v60.program,v60.reg[modVal&0x1F] + OpRead32(v60.program,modAdd+1)), modWriteValW);
		break;
	}

	return 5;
}


static UINT32 am3DisplacementIndirectIndexed8(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, MemRead32(v60.program,v60.reg[modVal2&0x1F] + (INT8)OpRead8(v60.program,modAdd+2)) + v60.reg[modVal&0x1F], modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, MemRead32(v60.program,v60.reg[modVal2&0x1F] + (INT8)OpRead8(v60.program,modAdd+2)) + v60.reg[modVal&0x1F] * 2, modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, MemRead32(v60.program,v60.reg[modVal2&0x1F] + (INT8)OpRead8(v60.program,modAdd+2)) + v60.reg[modVal&0x1F] * 4, modWriteValW);
		break;
	}

	return 3;
}

static UINT32 am3DisplacementIndirectIndexed16(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, MemRead32(v60.program,v60.reg[modVal2&0x1F] + (INT16)OpRead16(v60.program,modAdd+2)) + v60.reg[modVal&0x1F], modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, MemRead32(v60.program,v60.reg[modVal2&0x1F] + (INT16)OpRead16(v60.program,modAdd+2)) + v60.reg[modVal&0x1F] * 2, modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, MemRead32(v60.program,v60.reg[modVal2&0x1F] + (INT16)OpRead16(v60.program,modAdd+2)) + v60.reg[modVal&0x1F] * 4, modWriteValW);
		break;
	}

	return 4;
}

static UINT32 am3DisplacementIndirectIndexed32(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, MemRead32(v60.program,v60.reg[modVal2&0x1F] + OpRead32(v60.program,modAdd+2)) + v60.reg[modVal&0x1F], modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, MemRead32(v60.program,v60.reg[modVal2&0x1F] + OpRead32(v60.program,modAdd+2)) + v60.reg[modVal&0x1F] * 2, modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, MemRead32(v60.program,v60.reg[modVal2&0x1F] + OpRead32(v60.program,modAdd+2)) + v60.reg[modVal&0x1F] * 4, modWriteValW);
		break;
	}

	return 6;
}

static UINT32 am3PCDisplacementIndirect8(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, MemRead32(v60.program,PC + (INT8)OpRead8(v60.program,modAdd+1)), modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, MemRead32(v60.program,PC + (INT8)OpRead8(v60.program,modAdd+1)), modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, MemRead32(v60.program,PC + (INT8)OpRead8(v60.program,modAdd+1)), modWriteValW);
		break;
	}

	return 2;
}

static UINT32 am3PCDisplacementIndirect16(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, MemRead32(v60.program,PC + (INT16)OpRead16(v60.program,modAdd+1)), modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, MemRead32(v60.program,PC + (INT16)OpRead16(v60.program,modAdd+1)), modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, MemRead32(v60.program,PC + (INT16)OpRead16(v60.program,modAdd+1)), modWriteValW);
		break;
	}

	return 3;
}

static UINT32 am3PCDisplacementIndirect32(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, MemRead32(v60.program,PC + OpRead32(v60.program,modAdd+1)), modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, MemRead32(v60.program,PC + OpRead32(v60.program,modAdd+1)), modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, MemRead32(v60.program,PC + OpRead32(v60.program,modAdd+1)), modWriteValW);
		break;
	}

	return 5;
}


static UINT32 am3PCDisplacementIndirectIndexed8(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, MemRead32(v60.program,PC + (INT8)OpRead8(v60.program,modAdd+2)) + v60.reg[modVal&0x1F], modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, MemRead32(v60.program,PC + (INT8)OpRead8(v60.program,modAdd+2)) + v60.reg[modVal&0x1F] * 2, modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, MemRead32(v60.program,PC + (INT8)OpRead8(v60.program,modAdd+2)) + v60.reg[modVal&0x1F] * 4, modWriteValW);
		break;
	}

	return 3;
}

static UINT32 am3PCDisplacementIndirectIndexed16(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, MemRead32(v60.program,PC + (INT16)OpRead16(v60.program,modAdd+2)) + v60.reg[modVal&0x1F], modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, MemRead32(v60.program,PC + (INT16)OpRead16(v60.program,modAdd+2)) + v60.reg[modVal&0x1F] * 2, modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, MemRead32(v60.program,PC + (INT16)OpRead16(v60.program,modAdd+2)) + v60.reg[modVal&0x1F] * 4, modWriteValW);
		break;
	}

	return 4;
}

static UINT32 am3PCDisplacementIndirectIndexed32(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, MemRead32(v60.program,PC + OpRead32(v60.program,modAdd+2)) + v60.reg[modVal&0x1F], modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, MemRead32(v60.program,PC + OpRead32(v60.program,modAdd+2)) + v60.reg[modVal&0x1F] * 2, modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, MemRead32(v60.program,PC + OpRead32(v60.program,modAdd+2)) + v60.reg[modVal&0x1F] * 4, modWriteValW);
		break;
	}

	return 6;
}


static UINT32 am3DoubleDisplacement8(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, MemRead32(v60.program,v60.reg[modVal&0x1F] + (INT8)OpRead8(v60.program,modAdd+1)) + (INT8)OpRead8(v60.program,modAdd+2), modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, MemRead32(v60.program,v60.reg[modVal&0x1F] + (INT8)OpRead8(v60.program,modAdd+1)) + (INT8)OpRead8(v60.program,modAdd+2), modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, MemRead32(v60.program,v60.reg[modVal&0x1F] + (INT8)OpRead8(v60.program,modAdd+1)) + (INT8)OpRead8(v60.program,modAdd+2), modWriteValW);
		break;
	}

	return 3;
}

static UINT32 am3DoubleDisplacement16(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, MemRead32(v60.program,v60.reg[modVal&0x1F] + (INT16)OpRead16(v60.program,modAdd+1)) + (INT16)OpRead16(v60.program,modAdd+3), modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, MemRead32(v60.program,v60.reg[modVal&0x1F] + (INT16)OpRead16(v60.program,modAdd+1)) + (INT16)OpRead16(v60.program,modAdd+3), modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, MemRead32(v60.program,v60.reg[modVal&0x1F] + (INT16)OpRead16(v60.program,modAdd+1)) + (INT16)OpRead16(v60.program,modAdd+3), modWriteValW);
		break;
	}

	return 5;
}

static UINT32 am3DoubleDisplacement32(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, MemRead32(v60.program,v60.reg[modVal&0x1F] + OpRead32(v60.program,modAdd+1)) + OpRead32(v60.program,modAdd+5), modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, MemRead32(v60.program,v60.reg[modVal&0x1F] + OpRead32(v60.program,modAdd+1)) + OpRead32(v60.program,modAdd+5), modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, MemRead32(v60.program,v60.reg[modVal&0x1F] + OpRead32(v60.program,modAdd+1)) + OpRead32(v60.program,modAdd+5), modWriteValW);
		break;
	}

	return 9;
}


static UINT32 am3PCDoubleDisplacement8(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, MemRead32(v60.program,PC + (INT8)OpRead8(v60.program,modAdd+1)) + (INT8)OpRead8(v60.program,modAdd+2), modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, MemRead32(v60.program,PC + (INT8)OpRead8(v60.program,modAdd+1)) + (INT8)OpRead8(v60.program,modAdd+2), modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, MemRead32(v60.program,PC + (INT8)OpRead8(v60.program,modAdd+1)) + (INT8)OpRead8(v60.program,modAdd+2), modWriteValW);
		break;
	}

	return 3;
}

static UINT32 am3PCDoubleDisplacement16(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, MemRead32(v60.program,PC + (INT16)OpRead16(v60.program,modAdd+1)) + (INT16)OpRead16(v60.program,modAdd+3), modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, MemRead32(v60.program,PC + (INT16)OpRead16(v60.program,modAdd+1)) + (INT16)OpRead16(v60.program,modAdd+3), modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, MemRead32(v60.program,PC + (INT16)OpRead16(v60.program,modAdd+1)) + (INT16)OpRead16(v60.program,modAdd+3), modWriteValW);
		break;
	}

	return 5;
}

static UINT32 am3PCDoubleDisplacement32(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, MemRead32(v60.program,PC + OpRead32(v60.program,modAdd+1)) + OpRead32(v60.program,modAdd+5), modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, MemRead32(v60.program,PC + OpRead32(v60.program,modAdd+1)) + OpRead32(v60.program,modAdd+5), modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, MemRead32(v60.program,PC + OpRead32(v60.program,modAdd+1)) + OpRead32(v60.program,modAdd+5), modWriteValW);
		break;
	}

	return 9;
}

static UINT32 am3DirectAddress(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, OpRead32(v60.program,modAdd+1), modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, OpRead32(v60.program,modAdd+1), modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, OpRead32(v60.program,modAdd+1), modWriteValW);
		break;
	}

	return 5;
}

static UINT32 am3DirectAddressIndexed(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, OpRead32(v60.program,modAdd+2) + v60.reg[modVal&0x1F], modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, OpRead32(v60.program,modAdd+2) + v60.reg[modVal&0x1F] * 2, modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, OpRead32(v60.program,modAdd+2) + v60.reg[modVal&0x1F] * 4, modWriteValW);
		break;
	}

	return 6;
}

static UINT32 am3DirectAddressDeferred(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, MemRead32(v60.program,OpRead32(v60.program,modAdd+1)), modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, MemRead32(v60.program,OpRead32(v60.program,modAdd+1)), modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, MemRead32(v60.program,OpRead32(v60.program,modAdd+1)), modWriteValW);
		break;
	}

	return 5;
}

static UINT32 am3DirectAddressDeferredIndexed(void)
{
	switch (modDim)
	{
	case 0:
		MemWrite8(v60.program, MemRead32(v60.program,OpRead32(v60.program,modAdd+2)) + v60.reg[modVal&0x1F], modWriteValB);
		break;
	case 1:
		MemWrite16(v60.program, MemRead32(v60.program,OpRead32(v60.program,modAdd+2)) + v60.reg[modVal&0x1F], modWriteValH);
		break;
	case 2:
		MemWrite32(v60.program, MemRead32(v60.program,OpRead32(v60.program,modAdd+2)) + v60.reg[modVal&0x1F], modWriteValW);
		break;
	}

	return 6;
}

static UINT32 am3Immediate(void)
{
	fatalerror("CPU - AM3 - IMM (PC=%06x)", PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 am3ImmediateQuick(void)
{
	fatalerror("CPU - AM3 - IMMQ (PC=%06x)", PC);
	return 0; /* never reached, fatalerror won't return */
}



// AM3 Tables (for ReadAMAddress)
// ******************************

static UINT32 am3Error1(void)
{
	fatalerror("CPU - AM3 - 1 (PC=%06x)", PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 am3Error2(void)
{
	fatalerror("CPU - AM3 - 2 (PC=%06x)", PC);
	return 0; /* never reached, fatalerror won't return */
}

#ifdef UNUSED_FUNCTION
static UINT32 am3Error3(void)
{
	fatalerror("CPU - AM3 - 3 (PC=%06x)", PC);
	return 0; /* never reached, fatalerror won't return */
}
#endif

static UINT32 am3Error4(void)
{
	fatalerror("CPU - AM3 - 4 (PC=%06x)", PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 am3Error5(void)
{
	fatalerror("CPU - AM3 - 5 (PC=%06x)", PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 (*const AMTable3_G7a[16])(void) =
{
	am3PCDisplacementIndexed8,
	am3PCDisplacementIndexed16,
	am3PCDisplacementIndexed32,
	am3DirectAddressIndexed,
	am3Error5,
	am3Error5,
	am3Error5,
	am3Error5,
	am3PCDisplacementIndirectIndexed8,
	am3PCDisplacementIndirectIndexed16,
	am3PCDisplacementIndirectIndexed32,
	am3DirectAddressDeferredIndexed,
	am3Error5,
	am3Error5,
	am3Error5,
	am3Error5
};

static UINT32 am3Group7a(void)
{
	if (!(modVal2&0x10))
		return am3Error4();

	return AMTable3_G7a[modVal2&0xF]();
}

static UINT32 (*const AMTable3_G7[32])(void) =
{
	am3ImmediateQuick,
	am3ImmediateQuick,
	am3ImmediateQuick,
	am3ImmediateQuick,
	am3ImmediateQuick,
	am3ImmediateQuick,
	am3ImmediateQuick,
	am3ImmediateQuick,
	am3ImmediateQuick,
	am3ImmediateQuick,
	am3ImmediateQuick,
	am3ImmediateQuick,
	am3ImmediateQuick,
	am3ImmediateQuick,
	am3ImmediateQuick,
	am3ImmediateQuick,
	am3PCDisplacement8,
	am3PCDisplacement16,
	am3PCDisplacement32,
  am3DirectAddress,
	am3Immediate,
	am3Error2,
	am3Error2,
	am3Error2,
  am3PCDisplacementIndirect8,
  am3PCDisplacementIndirect16,
  am3PCDisplacementIndirect32,
	am3DirectAddressDeferred,
	am3PCDoubleDisplacement8,
	am3PCDoubleDisplacement16,
	am3PCDoubleDisplacement32,
  am3Error2
};

static UINT32 (*const AMTable3_G6[8])(void) =
{
	am3DisplacementIndexed8,
	am3DisplacementIndexed16,
	am3DisplacementIndexed32,
	am3RegisterIndirectIndexed,
	am3DisplacementIndirectIndexed8,
	am3DisplacementIndirectIndexed16,
	am3DisplacementIndirectIndexed32,
	am3Group7a
};




static UINT32 am3Group6(void)
{
	modVal2=OpRead8(v60.program,modAdd+1);
	return AMTable3_G6[modVal2>>5]();
}


static UINT32 am3Group7(void)
{
	return AMTable3_G7[modVal&0x1F]();
}



static UINT32 (*const AMTable3[2][8])(void) =
{
	{
		am3Displacement8,
		am3Displacement16,
		am3Displacement32,
		am3RegisterIndirect,
		am3DisplacementIndirect8,
		am3DisplacementIndirect16,
		am3DisplacementIndirect32,
		am3Group7
	},

	{
		am3DoubleDisplacement8,
		am3DoubleDisplacement16,
		am3DoubleDisplacement32,
		am3Register,
		am3Autoincrement,
		am3Autodecrement,
		am3Group6,
		am3Error1
	}
};



