
// AM3 Functions (for ReadAM)
// **************************

static UINT32 am3Register(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		SETREG8(cpustate->reg[cpustate->modval & 0x1F], cpustate->modwritevalb);
		break;
	case 1:
		SETREG16(cpustate->reg[cpustate->modval & 0x1F], cpustate->modwritevalh);
		break;
	case 2:
		cpustate->reg[cpustate->modval & 0x1F] = cpustate->modwritevalw;
		break;
	}

	return 1;
}

static UINT32 am3RegisterIndirect(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->reg[cpustate->modval & 0x1F], cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->reg[cpustate->modval & 0x1F], cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F], cpustate->modwritevalw);
		break;
	}

	return 1;
}

static UINT32 am3RegisterIndirectIndexed(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->reg[cpustate->modval2 & 0x1F] + cpustate->reg[cpustate->modval & 0x1F], cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + cpustate->reg[cpustate->modval & 0x1F] * 2, cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + cpustate->reg[cpustate->modval & 0x1F] * 4, cpustate->modwritevalw);
		break;
	}

	return 2;
}

static UINT32 am3Autoincrement(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->reg[cpustate->modval & 0x1F], cpustate->modwritevalb);
		cpustate->reg[cpustate->modval & 0x1F] += 1;
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->reg[cpustate->modval & 0x1F], cpustate->modwritevalh);
		cpustate->reg[cpustate->modval & 0x1F] += 2;
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F], cpustate->modwritevalw);
		cpustate->reg[cpustate->modval & 0x1F] += 4;
		break;
	}

	return 1;
}

static UINT32 am3Autodecrement(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->reg[cpustate->modval & 0x1F] -= 1;
		cpustate->program->write_byte(cpustate->reg[cpustate->modval & 0x1F], cpustate->modwritevalb);
		break;
	case 1:
		cpustate->reg[cpustate->modval & 0x1F] -= 2;
		cpustate->program->write_word_unaligned(cpustate->reg[cpustate->modval & 0x1F], cpustate->modwritevalh);
		break;
	case 2:
		cpustate->reg[cpustate->modval & 0x1F] -= 4;
		cpustate->program->write_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F], cpustate->modwritevalw);
		break;
	}

	return 1;
}

static UINT32 am3Displacement8(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->reg[cpustate->modval & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 1), cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 1), cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 1), cpustate->modwritevalw);
		break;
	}

	return 2;
}

static UINT32 am3Displacement16(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->reg[cpustate->modval & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 1), cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 1), cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 1), cpustate->modwritevalw);
		break;
	}

	return 3;
}

static UINT32 am3Displacement32(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->reg[cpustate->modval & 0x1F] + OpRead32(cpustate, cpustate->modadd + 1), cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->reg[cpustate->modval & 0x1F] + OpRead32(cpustate, cpustate->modadd + 1), cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + OpRead32(cpustate, cpustate->modadd + 1), cpustate->modwritevalw);
		break;
	}

	return 5;
}


static UINT32 am3DisplacementIndexed8(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->reg[cpustate->modval2 & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F], cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 2, cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 4, cpustate->modwritevalw);
		break;
	}

	return 3;
}

static UINT32 am3DisplacementIndexed16(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->reg[cpustate->modval2 & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F], cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 2, cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 4, cpustate->modwritevalw);
		break;
	}

	return 4;
}

static UINT32 am3DisplacementIndexed32(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->reg[cpustate->modval2 & 0x1F] + OpRead32(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F], cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + OpRead32(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 2, cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + OpRead32(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 4, cpustate->modwritevalw);
		break;
	}

	return 6;
}


static UINT32 am3PCDisplacement8(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 1), cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 1), cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 1), cpustate->modwritevalw);
		break;
	}

	return 2;
}

static UINT32 am3PCDisplacement16(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 1), cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 1), cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 1), cpustate->modwritevalw);
		break;
	}

	return 3;
}

static UINT32 am3PCDisplacement32(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 1), cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 1), cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 1), cpustate->modwritevalw);
		break;
	}

	return 5;
}

static UINT32 am3PCDisplacementIndexed8(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F], cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 2, cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 4, cpustate->modwritevalw);
		break;
	}

	return 3;
}

static UINT32 am3PCDisplacementIndexed16(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F], cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 2, cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 4, cpustate->modwritevalw);
		break;
	}

	return 4;
}

static UINT32 am3PCDisplacementIndexed32(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F], cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 2, cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 4, cpustate->modwritevalw);
		break;
	}

	return 6;
}

static UINT32 am3DisplacementIndirect8(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 1)), cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 1)), cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 1)), cpustate->modwritevalw);
		break;
	}

	return 2;
}

static UINT32 am3DisplacementIndirect16(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 1)), cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 1)), cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 1)), cpustate->modwritevalw);
		break;
	}

	return 3;
}

static UINT32 am3DisplacementIndirect32(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + OpRead32(cpustate, cpustate->modadd + 1)), cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + OpRead32(cpustate, cpustate->modadd + 1)), cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + OpRead32(cpustate, cpustate->modadd + 1)), cpustate->modwritevalw);
		break;
	}

	return 5;
}


static UINT32 am3DisplacementIndirectIndexed8(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F], cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 2, cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 4, cpustate->modwritevalw);
		break;
	}

	return 3;
}

static UINT32 am3DisplacementIndirectIndexed16(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F], cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 2, cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 4, cpustate->modwritevalw);
		break;
	}

	return 4;
}

static UINT32 am3DisplacementIndirectIndexed32(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + OpRead32(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F], cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + OpRead32(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 2, cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + OpRead32(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 4, cpustate->modwritevalw);
		break;
	}

	return 6;
}

static UINT32 am3PCDisplacementIndirect8(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->program->read_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 1)), cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 1)), cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 1)), cpustate->modwritevalw);
		break;
	}

	return 2;
}

static UINT32 am3PCDisplacementIndirect16(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->program->read_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 1)), cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 1)), cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 1)), cpustate->modwritevalw);
		break;
	}

	return 3;
}

static UINT32 am3PCDisplacementIndirect32(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->program->read_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 1)), cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 1)), cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 1)), cpustate->modwritevalw);
		break;
	}

	return 5;
}


static UINT32 am3PCDisplacementIndirectIndexed8(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->program->read_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F], cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 2, cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 4, cpustate->modwritevalw);
		break;
	}

	return 3;
}

static UINT32 am3PCDisplacementIndirectIndexed16(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->program->read_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F], cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 2, cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 4, cpustate->modwritevalw);
		break;
	}

	return 4;
}

static UINT32 am3PCDisplacementIndirectIndexed32(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->program->read_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F], cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 2, cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 4, cpustate->modwritevalw);
		break;
	}

	return 6;
}


static UINT32 am3DoubleDisplacement8(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 1)) + (INT8)OpRead8(cpustate, cpustate->modadd + 2), cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 1)) + (INT8)OpRead8(cpustate, cpustate->modadd + 2), cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 1)) + (INT8)OpRead8(cpustate, cpustate->modadd + 2), cpustate->modwritevalw);
		break;
	}

	return 3;
}

static UINT32 am3DoubleDisplacement16(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 1)) + (INT16)OpRead16(cpustate, cpustate->modadd + 3), cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 1)) + (INT16)OpRead16(cpustate, cpustate->modadd + 3), cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 1)) + (INT16)OpRead16(cpustate, cpustate->modadd + 3), cpustate->modwritevalw);
		break;
	}

	return 5;
}

static UINT32 am3DoubleDisplacement32(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + OpRead32(cpustate, cpustate->modadd + 1)) + OpRead32(cpustate, cpustate->modadd + 5), cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + OpRead32(cpustate, cpustate->modadd + 1)) + OpRead32(cpustate, cpustate->modadd + 5), cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + OpRead32(cpustate, cpustate->modadd + 1)) + OpRead32(cpustate, cpustate->modadd + 5), cpustate->modwritevalw);
		break;
	}

	return 9;
}


static UINT32 am3PCDoubleDisplacement8(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->program->read_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 1)) + (INT8)OpRead8(cpustate, cpustate->modadd + 2), cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 1)) + (INT8)OpRead8(cpustate, cpustate->modadd + 2), cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 1)) + (INT8)OpRead8(cpustate, cpustate->modadd + 2), cpustate->modwritevalw);
		break;
	}

	return 3;
}

static UINT32 am3PCDoubleDisplacement16(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->program->read_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 1)) + (INT16)OpRead16(cpustate, cpustate->modadd + 3), cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 1)) + (INT16)OpRead16(cpustate, cpustate->modadd + 3), cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 1)) + (INT16)OpRead16(cpustate, cpustate->modadd + 3), cpustate->modwritevalw);
		break;
	}

	return 5;
}

static UINT32 am3PCDoubleDisplacement32(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->program->read_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 1)) + OpRead32(cpustate, cpustate->modadd + 5), cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 1)) + OpRead32(cpustate, cpustate->modadd + 5), cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 1)) + OpRead32(cpustate, cpustate->modadd + 5), cpustate->modwritevalw);
		break;
	}

	return 9;
}

static UINT32 am3DirectAddress(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(OpRead32(cpustate, cpustate->modadd + 1), cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(OpRead32(cpustate, cpustate->modadd + 1), cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(OpRead32(cpustate, cpustate->modadd + 1), cpustate->modwritevalw);
		break;
	}

	return 5;
}

static UINT32 am3DirectAddressIndexed(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(OpRead32(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F], cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(OpRead32(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 2, cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(OpRead32(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 4, cpustate->modwritevalw);
		break;
	}

	return 6;
}

static UINT32 am3DirectAddressDeferred(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->program->read_dword_unaligned(OpRead32(cpustate, cpustate->modadd + 1)), cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->program->read_dword_unaligned(OpRead32(cpustate, cpustate->modadd + 1)), cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->program->read_dword_unaligned(OpRead32(cpustate, cpustate->modadd + 1)), cpustate->modwritevalw);
		break;
	}

	return 5;
}

static UINT32 am3DirectAddressDeferredIndexed(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->program->write_byte(cpustate->program->read_dword_unaligned(OpRead32(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F], cpustate->modwritevalb);
		break;
	case 1:
		cpustate->program->write_word_unaligned(cpustate->program->read_dword_unaligned(OpRead32(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F], cpustate->modwritevalh);
		break;
	case 2:
		cpustate->program->write_dword_unaligned(cpustate->program->read_dword_unaligned(OpRead32(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F], cpustate->modwritevalw);
		break;
	}

	return 6;
}

static UINT32 am3Immediate(v60_state *cpustate)
{
	fatalerror("CPU - AM3 - IMM (cpustate->PC=%06x)\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 am3ImmediateQuick(v60_state *cpustate)
{
	fatalerror("CPU - AM3 - IMMQ (cpustate->PC=%06x)\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}



// AM3 Tables (for ReadAMAddress)
// ******************************

static UINT32 am3Error1(v60_state *cpustate)
{
	fatalerror("CPU - AM3 - 1 (cpustate->PC=%06x)\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 am3Error2(v60_state *cpustate)
{
	fatalerror("CPU - AM3 - 2 (cpustate->PC=%06x)\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

#ifdef UNUSED_FUNCTION
static UINT32 am3Error3(v60_state *cpustate)
{
	fatalerror("CPU - AM3 - 3 (cpustate->PC=%06x)\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}
#endif

static UINT32 am3Error4(v60_state *cpustate)
{
	fatalerror("CPU - AM3 - 4 (cpustate->PC=%06x)\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 am3Error5(v60_state *cpustate)
{
	fatalerror("CPU - AM3 - 5 (cpustate->PC=%06x)\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 (*const AMTable3_G7a[16])(v60_state *) =
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

static UINT32 am3Group7a(v60_state *cpustate)
{
	if (!(cpustate->modval2 & 0x10))
		return am3Error4(cpustate);

	return AMTable3_G7a[cpustate->modval2 & 0xF](cpustate);
}

static UINT32 (*const AMTable3_G7[32])(v60_state *) =
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

static UINT32 (*const AMTable3_G6[8])(v60_state *) =
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




static UINT32 am3Group6(v60_state *cpustate)
{
	cpustate->modval2 = OpRead8(cpustate, cpustate->modadd + 1);
	return AMTable3_G6[cpustate->modval2 >> 5](cpustate);
}


static UINT32 am3Group7(v60_state *cpustate)
{
	return AMTable3_G7[cpustate->modval & 0x1F](cpustate);
}



static UINT32 (*const AMTable3[2][8])(v60_state *) =
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
