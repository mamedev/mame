
// AM2 Functions (for ReadAMAddress)
// *********************************

static UINT32 am2Register(v60_state *cpustate)
{
	cpustate->amflag = 1;
	cpustate->amout = cpustate->modval & 0x1F;
	return 1;
}

static UINT32 am2RegisterIndirect(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->reg[cpustate->modval & 0x1F];
	return 1;
}

static UINT32 bam2RegisterIndirect(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->reg[cpustate->modval & 0x1F];
	cpustate->bamoffset = 0;
	return 1;
}

static UINT32 am2RegisterIndirectIndexed(v60_state *cpustate)
{
	cpustate->amflag = 0;

	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->reg[cpustate->modval2 & 0x1F] + cpustate->reg[cpustate->modval & 0x1F];
		break;
	case 1:
		cpustate->amout = cpustate->reg[cpustate->modval2 & 0x1F] + cpustate->reg[cpustate->modval & 0x1F] * 2;
		break;
	case 2:
		cpustate->amout = cpustate->reg[cpustate->modval2 & 0x1F] + cpustate->reg[cpustate->modval & 0x1F] * 4;
		break;
	case 3:
		cpustate->amout = cpustate->reg[cpustate->modval2 & 0x1F] + cpustate->reg[cpustate->modval & 0x1F] * 8;
		break;
	}

	return 2;
}

static UINT32 bam2RegisterIndirectIndexed(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->reg[cpustate->modval2 & 0x1F];
	cpustate->bamoffset = cpustate->reg[cpustate->modval & 0x1F];
	return 2;
}

static UINT32 am2Autoincrement(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->reg[cpustate->modval & 0x1F];

	switch (cpustate->moddim)
	{
	case 0:
		cpustate->reg[cpustate->modval & 0x1F] += 1;
		break;
	case 1:
		cpustate->reg[cpustate->modval & 0x1F] += 2;
		break;
	case 2:
		cpustate->reg[cpustate->modval & 0x1F] += 4;
		break;
	case 3:
		cpustate->reg[cpustate->modval & 0x1F] += 8;
		break;
	}

	return 1;
}

static UINT32 bam2Autoincrement(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->reg[cpustate->modval & 0x1F];
	cpustate->bamoffset = 0;

	switch (cpustate->moddim)
	{
	case 10:
		cpustate->reg[cpustate->modval & 0x1F] +=1;
		break;
	case 11:
		cpustate->reg[cpustate->modval & 0x1F] +=4;
		break;
	default:
		fatalerror("CPU - AM2 - 7 (t0 cpustate->PC=%x)\n", cpustate->PC);
		break;
	}

	return 1;
}

static UINT32 am2Autodecrement(v60_state *cpustate)
{
	cpustate->amflag = 0;

	switch (cpustate->moddim)
	{
	case 0:
		cpustate->reg[cpustate->modval & 0x1F] -= 1;
		break;
	case 1:
		cpustate->reg[cpustate->modval & 0x1F] -= 2;
		break;
	case 2:
		cpustate->reg[cpustate->modval & 0x1F] -= 4;
		break;
	case 3:
		cpustate->reg[cpustate->modval & 0x1F] -= 8;
		break;
	}

	cpustate->amout = cpustate->reg[cpustate->modval & 0x1F];
	return 1;
}

static UINT32 bam2Autodecrement(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->bamoffset = 0;

	switch (cpustate->moddim)
	{
	case 10:
		cpustate->reg[cpustate->modval & 0x1F]-=1;
		break;
	case 11:
		cpustate->reg[cpustate->modval & 0x1F]-=4;
		break;
	default:
		fatalerror("CPU - BAM2 - 7 (cpustate->PC=%06x)\n", cpustate->PC);
		break;
	}

	cpustate->amout = cpustate->reg[cpustate->modval & 0x1F];
	return 1;
}


static UINT32 am2Displacement8(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->reg[cpustate->modval & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 1);

	return 2;
}

static UINT32 bam2Displacement8(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->reg[cpustate->modval & 0x1F];
	cpustate->bamoffset = (INT8)OpRead8(cpustate, cpustate->modadd + 1);

	return 2;
}

static UINT32 am2Displacement16(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->reg[cpustate->modval & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 1);

	return 3;
}

static UINT32 bam2Displacement16(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->reg[cpustate->modval & 0x1F];
	cpustate->bamoffset = (INT16)OpRead16(cpustate, cpustate->modadd + 1);

	return 3;
}

static UINT32 am2Displacement32(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->reg[cpustate->modval & 0x1F] + OpRead32(cpustate, cpustate->modadd + 1);

	return 5;
}

static UINT32 bam2Displacement32(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->reg[cpustate->modval & 0x1F];
	cpustate->bamoffset = OpRead32(cpustate, cpustate->modadd + 1);

	return 5;
}

static UINT32 am2DisplacementIndexed8(v60_state *cpustate)
{
	cpustate->amflag = 0;

	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->reg[cpustate->modval2 & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F];
		break;
	case 1:
		cpustate->amout = cpustate->reg[cpustate->modval2 & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 2;
		break;
	case 2:
		cpustate->amout = cpustate->reg[cpustate->modval2 & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 4;
		break;
	case 3:
		cpustate->amout = cpustate->reg[cpustate->modval2 & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 8;
		break;
	}

	return 3;
}

static UINT32 bam2DisplacementIndexed8(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->reg[cpustate->modval2 & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 2);
	cpustate->bamoffset = cpustate->reg[cpustate->modval & 0x1F];

	return 3;
}

static UINT32 am2DisplacementIndexed16(v60_state *cpustate)
{
	cpustate->amflag = 0;

	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->reg[cpustate->modval2 & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F];
		break;
	case 1:
		cpustate->amout = cpustate->reg[cpustate->modval2 & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 2;
		break;
	case 2:
		cpustate->amout = cpustate->reg[cpustate->modval2 & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 4;
		break;
	case 3:
		cpustate->amout = cpustate->reg[cpustate->modval2 & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 8;
		break;
	}

	return 4;
}

static UINT32 bam2DisplacementIndexed16(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->reg[cpustate->modval2 & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 2);
	cpustate->bamoffset = cpustate->reg[cpustate->modval & 0x1F];

	return 4;
}

static UINT32 am2DisplacementIndexed32(v60_state *cpustate)
{
	cpustate->amflag = 0;

	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->reg[cpustate->modval2 & 0x1F] + OpRead32(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F];
		break;
	case 1:
		cpustate->amout = cpustate->reg[cpustate->modval2 & 0x1F] + OpRead32(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 2;
		break;
	case 2:
		cpustate->amout = cpustate->reg[cpustate->modval2 & 0x1F] + OpRead32(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 4;
		break;
	case 3:
		cpustate->amout = cpustate->reg[cpustate->modval2 & 0x1F] + OpRead32(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 8;
		break;
	}

	return 6;
}

static UINT32 bam2DisplacementIndexed32(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->reg[cpustate->modval2 & 0x1F] + OpRead32(cpustate, cpustate->modadd + 2);
	cpustate->bamoffset = cpustate->reg[cpustate->modval & 0x1F];

	return 6;
}

static UINT32 am2PCDisplacement8(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 1);

	return 2;
}

static UINT32 bam2PCDisplacement8(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->PC;
	cpustate->bamoffset = (INT8)OpRead8(cpustate, cpustate->modadd + 1);

	return 2;
}

static UINT32 am2PCDisplacement16(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 1);

	return 3;
}

static UINT32 bam2PCDisplacement16(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->PC;
	cpustate->bamoffset = (INT16)OpRead16(cpustate, cpustate->modadd + 1);

	return 3;
}

static UINT32 am2PCDisplacement32(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->PC + OpRead32(cpustate, cpustate->modadd + 1);

	return 5;
}

static UINT32 bam2PCDisplacement32(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->PC;
	cpustate->bamoffset = OpRead32(cpustate, cpustate->modadd + 1);

	return 5;
}


static UINT32 am2PCDisplacementIndexed8(v60_state *cpustate)
{
	cpustate->amflag = 0;

	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F];
		break;
	case 1:
		cpustate->amout = cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 2;
		break;
	case 2:
		cpustate->amout = cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 4;
		break;
	case 3:
		cpustate->amout = cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 8;
		break;
	}

	return 3;
}

static UINT32 bam2PCDisplacementIndexed8(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 2);
	cpustate->bamoffset = cpustate->reg[cpustate->modval & 0x1F];

	return 3;
}

static UINT32 am2PCDisplacementIndexed16(v60_state *cpustate)
{
	cpustate->amflag = 0;

	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F];
		break;
	case 1:
		cpustate->amout = cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 2;
		break;
	case 2:
		cpustate->amout = cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 4;
		break;
	case 3:
		cpustate->amout = cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 8;
		break;
	}

	return 4;
}

static UINT32 bam2PCDisplacementIndexed16(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 2);
	cpustate->bamoffset = cpustate->reg[cpustate->modval & 0x1F];

	return 4;
}

static UINT32 am2PCDisplacementIndexed32(v60_state *cpustate)
{
	cpustate->amflag = 0;

	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->PC + OpRead32(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F];
		break;
	case 1:
		cpustate->amout = cpustate->PC + OpRead32(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 2;
		break;
	case 2:
		cpustate->amout = cpustate->PC + OpRead32(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 4;
		break;
	case 3:
		cpustate->amout = cpustate->PC + OpRead32(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 8;
		break;
	}

	return 6;
}

static UINT32 bam2PCDisplacementIndexed32(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->PC + OpRead32(cpustate, cpustate->modadd + 2);
	cpustate->bamoffset = cpustate->reg[cpustate->modval & 0x1F];

	return 6;
}

static UINT32 am2DisplacementIndirect8(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 1));

	return 2;
}

static UINT32 bam2DisplacementIndirect8(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 1));
	cpustate->bamoffset = 0;
	return 2;
}

static UINT32 am2DisplacementIndirect16(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 1));

	return 3;
}

static UINT32 bam2DisplacementIndirect16(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 1));
	cpustate->bamoffset = 0;
	return 3;
}

static UINT32 am2DisplacementIndirect32(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + OpRead32(cpustate, cpustate->modadd + 1));

	return 5;
}

static UINT32 bam2DisplacementIndirect32(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + OpRead32(cpustate, cpustate->modadd + 1));
	cpustate->bamoffset = 0;

	return 5;
}

static UINT32 am2DisplacementIndirectIndexed8(v60_state *cpustate)
{
	cpustate->amflag = 0;

	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F];
		break;
	case 1:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 2;
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 4;
		break;
	case 3:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 8;
		break;
	}

	return 3;
}

static UINT32 bam2DisplacementIndirectIndexed8(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 2));
	cpustate->bamoffset = cpustate->reg[cpustate->modval & 0x1F];

	return 3;
}

static UINT32 am2DisplacementIndirectIndexed16(v60_state *cpustate)
{
	cpustate->amflag = 0;

	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F];
		break;
	case 1:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 2;
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 4;
		break;
	case 3:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 8;
		break;
	}

	return 4;
}

static UINT32 bam2DisplacementIndirectIndexed16(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 2));
	cpustate->bamoffset = cpustate->reg[cpustate->modval & 0x1F];

	return 4;
}

static UINT32 am2DisplacementIndirectIndexed32(v60_state *cpustate)
{
	cpustate->amflag = 0;

	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + OpRead32(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F];
		break;
	case 1:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + OpRead32(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 2;
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + OpRead32(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 4;
		break;
	case 3:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + OpRead32(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 8;
		break;
	}

	return 6;
}

static UINT32 bam2DisplacementIndirectIndexed32(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + OpRead32(cpustate, cpustate->modadd + 2));
	cpustate->bamoffset = cpustate->reg[cpustate->modval & 0x1F];

	return 6;
}

static UINT32 am2PCDisplacementIndirect8(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 1));

	return 2;
}

static UINT32 bam2PCDisplacementIndirect8(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 1));
	cpustate->bamoffset = 0;

	return 2;
}

static UINT32 am2PCDisplacementIndirect16(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 1));

	return 3;
}

static UINT32 bam2PCDisplacementIndirect16(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 1));
	cpustate->bamoffset = 0;

	return 3;
}

static UINT32 am2PCDisplacementIndirect32(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 1));

	return 5;
}

static UINT32 bam2PCDisplacementIndirect32(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 1));
	cpustate->bamoffset = 0;

	return 5;
}

static UINT32 am2PCDisplacementIndirectIndexed8(v60_state *cpustate)
{
	cpustate->amflag = 0;

	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F];
		break;
	case 1:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 2;
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 4;
		break;
	case 3:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 8;
		break;
	}

	return 3;
}

static UINT32 bam2PCDisplacementIndirectIndexed8(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 2));
	cpustate->bamoffset = cpustate->reg[cpustate->modval & 0x1F];

	return 3;
}

static UINT32 am2PCDisplacementIndirectIndexed16(v60_state *cpustate)
{
	cpustate->amflag = 0;

	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F];
		break;
	case 1:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 2;
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 4;
		break;
	case 3:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 8;
		break;
	}

	return 4;
}


static UINT32 bam2PCDisplacementIndirectIndexed16(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 2));
	cpustate->bamoffset = cpustate->reg[cpustate->modval & 0x1F];

	return 4;
}


static UINT32 am2PCDisplacementIndirectIndexed32(v60_state *cpustate)
{
	cpustate->amflag = 0;

	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F];
		break;
	case 1:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 2;
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 4;
		break;
	case 3:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 8;
		break;
	}

	return 6;
}

static UINT32 bam2PCDisplacementIndirectIndexed32(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 2));
	cpustate->bamoffset = cpustate->reg[cpustate->modval & 0x1F];

	return 6;
}

static UINT32 am2DoubleDisplacement8(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 1)) + (INT8)OpRead8(cpustate, cpustate->modadd + 2);

	return 3;
}

static UINT32 bam2DoubleDisplacement8(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 1));
	cpustate->bamoffset = (INT8)OpRead8(cpustate, cpustate->modadd + 2);

	return 3;
}

static UINT32 am2DoubleDisplacement16(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 1)) + (INT16)OpRead16(cpustate, cpustate->modadd + 3);

	return 5;
}

static UINT32 bam2DoubleDisplacement16(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 1));
	cpustate->bamoffset = (INT8)OpRead8(cpustate, cpustate->modadd + 3);

	return 5;
}

static UINT32 am2DoubleDisplacement32(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + OpRead32(cpustate, cpustate->modadd + 1)) + OpRead32(cpustate, cpustate->modadd + 5);

	return 9;
}

static UINT32 bam2DoubleDisplacement32(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + OpRead32(cpustate, cpustate->modadd + 1));
	cpustate->bamoffset = OpRead32(cpustate, cpustate->modadd + 5);

	return 9;
}


static UINT32 am2PCDoubleDisplacement8(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 1)) + (INT8)OpRead8(cpustate, cpustate->modadd + 2);

	return 3;
}

static UINT32 bam2PCDoubleDisplacement8(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 1));
	cpustate->bamoffset = (INT8)OpRead8(cpustate, cpustate->modadd + 2);

	return 3;
}

static UINT32 am2PCDoubleDisplacement16(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 1)) + (INT16)OpRead16(cpustate, cpustate->modadd + 3);

	return 5;
}

static UINT32 bam2PCDoubleDisplacement16(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 1));
	cpustate->bamoffset = (INT8)OpRead8(cpustate, cpustate->modadd + 3);

	return 5;
}

static UINT32 am2PCDoubleDisplacement32(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 1)) + OpRead32(cpustate, cpustate->modadd + 5);

	return 9;
}

static UINT32 bam2PCDoubleDisplacement32(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 1));
	cpustate->bamoffset = OpRead32(cpustate, cpustate->modadd + 5);

	return 9;
}

static UINT32 am2DirectAddress(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = OpRead32(cpustate, cpustate->modadd + 1);

	return 5;
}

static UINT32 bam2DirectAddress(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = OpRead32(cpustate, cpustate->modadd + 1);
	cpustate->bamoffset = 0;

	return 5;
}

static UINT32 am2DirectAddressIndexed(v60_state *cpustate)
{
	cpustate->amflag = 0;

	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = OpRead32(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F];
		break;
	case 1:
		cpustate->amout = OpRead32(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 2;
		break;
	case 2:
		cpustate->amout = OpRead32(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 4;
		break;
	case 3:
		cpustate->amout = OpRead32(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 8;
		break;
	}

	return 6;
}

static UINT32 bam2DirectAddressIndexed(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = OpRead32(cpustate, cpustate->modadd + 2);
	cpustate->bamoffset = cpustate->reg[cpustate->modval & 0x1F];

	return 6;
}

static UINT32 am2DirectAddressDeferred(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(OpRead32(cpustate, cpustate->modadd + 1));

	return 5;
}

static UINT32 bam2DirectAddressDeferred(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(OpRead32(cpustate, cpustate->modadd + 1));
	cpustate->bamoffset = 0;

	return 5;
}

static UINT32 am2DirectAddressDeferredIndexed(v60_state *cpustate)
{
	cpustate->amflag = 0;

	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_dword_unaligned(OpRead32(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F];
		break;
	case 1:
		cpustate->amout = cpustate->program->read_dword_unaligned(OpRead32(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 2;
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(OpRead32(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 4;
		break;
	case 3:
		cpustate->amout = cpustate->program->read_dword_unaligned(OpRead32(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 8;
		break;
	}

	return 6;
}

static UINT32 bam2DirectAddressDeferredIndexed(v60_state *cpustate)
{
	cpustate->amflag = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(OpRead32(cpustate, cpustate->modadd + 2));
	cpustate->bamoffset = cpustate->reg[cpustate->modval & 0x1F];

	return 6;
}

static UINT32 am2Immediate(v60_state *cpustate)
{
	// ignore LDPR
	return am1Immediate(cpustate);
}

static UINT32 am2ImmediateQuick(v60_state *cpustate)
{
	// ignore LDPR
	return am1ImmediateQuick(cpustate);
}


// AM2 Tables (for ReadAMAddress)
// ******************************

static UINT32 am2Error1(v60_state *cpustate)
{
	// f1lap trips this, why?
	logerror("CPU - AM2 - 1 (cpustate->PC=%06x)", cpustate->PC);
	return 0;
}

static UINT32 am2Error2(v60_state *cpustate)
{
	fatalerror("CPU - AM2 - 2 (cpustate->PC=%06x)\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

#ifdef UNUSED_FUNCTION
static UINT32 am2Error3(v60_state *cpustate)
{
	fatalerror("CPU - AM2 - 3 (cpustate->PC=%06x)\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}
#endif

static UINT32 am2Error4(v60_state *cpustate)
{
	fatalerror("CPU - AM2 - 4 (cpustate->PC=%06x)\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 am2Error5(v60_state *cpustate)
{
	fatalerror("CPU - AM2 - 5 (cpustate->PC=%06x)\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 bam2Error1(v60_state *cpustate)
{
	fatalerror("CPU - BAM2 - 1 (cpustate->PC=%06x)\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 bam2Error2(v60_state *cpustate)
{
	fatalerror("CPU - BAM2 - 2 (cpustate->PC=%06x)\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

#ifdef UNUSED_FUNCTION
static UINT32 bam2Error3(v60_state *cpustate)
{
	fatalerror("CPU - BAM2 - 3 (cpustate->PC=%06x)\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}
#endif

static UINT32 bam2Error4(v60_state *cpustate)
{
	fatalerror("CPU - BAM2 - 4 (cpustate->PC=%06x)\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 bam2Error5(v60_state *cpustate)
{
	fatalerror("CPU - BAM2 - 5 (cpustate->PC=%06x)\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 bam2Error6(v60_state *cpustate)
{
	fatalerror("CPU - BAM2 - 6 (cpustate->PC=%06x)\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}


static UINT32 (*const AMTable2_G7a[16])(v60_state *) =
{
	am2PCDisplacementIndexed8,
	am2PCDisplacementIndexed16,
	am2PCDisplacementIndexed32,
	am2DirectAddressIndexed,
	am2Error5,
	am2Error5,
	am2Error5,
	am2Error5,
	am2PCDisplacementIndirectIndexed8,
	am2PCDisplacementIndirectIndexed16,
	am2PCDisplacementIndirectIndexed32,
	am2DirectAddressDeferredIndexed,
	am2Error5,
	am2Error5,
	am2Error5,
	am2Error5
};

static UINT32 (*const BAMTable2_G7a[16])(v60_state *) =
{
	bam2PCDisplacementIndexed8,
	bam2PCDisplacementIndexed16,
	bam2PCDisplacementIndexed32,
	bam2DirectAddressIndexed,
	bam2Error5,
	bam2Error5,
	bam2Error5,
	bam2Error5,
	bam2PCDisplacementIndirectIndexed8,
	bam2PCDisplacementIndirectIndexed16,
	bam2PCDisplacementIndirectIndexed32,
	bam2DirectAddressDeferredIndexed,
	bam2Error5,
	bam2Error5,
	bam2Error5,
	bam2Error5
};

static UINT32 am2Group7a(v60_state *cpustate)
{
	if (!(cpustate->modval2 & 0x10))
		return am2Error4(cpustate);

	return AMTable2_G7a[cpustate->modval2 & 0xF](cpustate);
}

static UINT32 bam2Group7a(v60_state *cpustate)
{
	if (!(cpustate->modval2 & 0x10))
		return bam2Error4(cpustate);

	return BAMTable2_G7a[cpustate->modval2 & 0xF](cpustate);
}

static UINT32 (*const AMTable2_G7[32])(v60_state *) =
{
	am2ImmediateQuick,
	am2ImmediateQuick,
	am2ImmediateQuick,
	am2ImmediateQuick,
	am2ImmediateQuick,
	am2ImmediateQuick,
	am2ImmediateQuick,
	am2ImmediateQuick,
	am2ImmediateQuick,
	am2ImmediateQuick,
	am2ImmediateQuick,
	am2ImmediateQuick,
	am2ImmediateQuick,
	am2ImmediateQuick,
	am2ImmediateQuick,
	am2ImmediateQuick,
	am2PCDisplacement8,
	am2PCDisplacement16,
	am2PCDisplacement32,
	am2DirectAddress,
	am2Immediate,
	am2Error2,
	am2Error2,
	am2Error2,
	am2PCDisplacementIndirect8,
	am2PCDisplacementIndirect16,
	am2PCDisplacementIndirect32,
	am2DirectAddressDeferred,
	am2PCDoubleDisplacement8,
	am2PCDoubleDisplacement16,
	am2PCDoubleDisplacement32,
	am2Error2
};

static UINT32 (*const BAMTable2_G7[32])(v60_state *) =
{
	bam2Error6,
	bam2Error6,
	bam2Error6,
	bam2Error6,
	bam2Error6,
	bam2Error6,
	bam2Error6,
	bam2Error6,
	bam2Error6,
	bam2Error6,
	bam2Error6,
	bam2Error6,
	bam2Error6,
	bam2Error6,
	bam2Error6,
	bam2Error6,
	bam2PCDisplacement8,
	bam2PCDisplacement16,
	bam2PCDisplacement32,
	bam2DirectAddress,
	bam2Error6,
	bam2Error2,
	bam2Error2,
	bam2Error2,
	bam2PCDisplacementIndirect8,
	bam2PCDisplacementIndirect16,
	bam2PCDisplacementIndirect32,
	bam2DirectAddressDeferred,
	bam2PCDoubleDisplacement8,
	bam2PCDoubleDisplacement16,
	bam2PCDoubleDisplacement32,
	bam2Error2
};

static UINT32 (*const AMTable2_G6[8])(v60_state *) =
{
	am2DisplacementIndexed8,
	am2DisplacementIndexed16,
	am2DisplacementIndexed32,
	am2RegisterIndirectIndexed,
	am2DisplacementIndirectIndexed8,
	am2DisplacementIndirectIndexed16,
	am2DisplacementIndirectIndexed32,
	am2Group7a
};

static UINT32 (*const BAMTable2_G6[8])(v60_state *) =
{
	bam2DisplacementIndexed8,
	bam2DisplacementIndexed16,
	bam2DisplacementIndexed32,
	bam2RegisterIndirectIndexed,
	bam2DisplacementIndirectIndexed8,
	bam2DisplacementIndirectIndexed16,
	bam2DisplacementIndirectIndexed32,
	bam2Group7a
};




static UINT32 am2Group6(v60_state *cpustate)
{
	cpustate->modval2 = OpRead8(cpustate, cpustate->modadd + 1);
	return AMTable2_G6[cpustate->modval2 >> 5](cpustate);
}
static UINT32 bam2Group6(v60_state *cpustate)
{
	cpustate->modval2 = OpRead8(cpustate, cpustate->modadd + 1);
	return BAMTable2_G6[cpustate->modval2 >> 5](cpustate);
}

static UINT32 am2Group7(v60_state *cpustate)
{
	return AMTable2_G7[cpustate->modval & 0x1F](cpustate);
}
static UINT32 bam2Group7(v60_state *cpustate)
{
	return BAMTable2_G7[cpustate->modval & 0x1F](cpustate);
}


static UINT32 (*const AMTable2[2][8])(v60_state *) =
{
	{
		am2Displacement8,
		am2Displacement16,
		am2Displacement32,
		am2RegisterIndirect,
		am2DisplacementIndirect8,
		am2DisplacementIndirect16,
		am2DisplacementIndirect32,
		am2Group7
	},

	{
		am2DoubleDisplacement8,
		am2DoubleDisplacement16,
		am2DoubleDisplacement32,
		am2Register,
		am2Autoincrement,
		am2Autodecrement,
		am2Group6,
		am2Error1
	}
};

static UINT32 (*const BAMTable2[2][8])(v60_state *) =
{
	{
		bam2Displacement8,
		bam2Displacement16,
		bam2Displacement32,
		bam2RegisterIndirect,
		bam2DisplacementIndirect8,
		bam2DisplacementIndirect16,
		bam2DisplacementIndirect32,
		bam2Group7
	},

	{
		bam2DoubleDisplacement8,
		bam2DoubleDisplacement16,
		bam2DoubleDisplacement32,
		bam2Error6,
		bam2Autoincrement,
		bam2Autodecrement,
		bam2Group6,
		bam2Error1
	}
};




