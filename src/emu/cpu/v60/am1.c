
// AM1 Functions (for ReadAM)
// **************************

static UINT32 am1Register(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = (UINT8)cpustate->reg[cpustate->modval & 0x1F];
		break;
	case 1:
		cpustate->amout = (UINT16)cpustate->reg[cpustate->modval & 0x1F];
		break;
	case 2:
		cpustate->amout = cpustate->reg[cpustate->modval & 0x1F];
		break;
	}

	return 1;
}

static UINT32 am1RegisterIndirect(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->reg[cpustate->modval & 0x1F]);
		break;
	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->reg[cpustate->modval & 0x1F]);
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F]);
		break;
	}

	return 1;
}

static UINT32 bam1RegisterIndirect(v60_state *cpustate)
{
	cpustate->bamoffset = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F]);
	return 1;
}

static UINT32 am1RegisterIndirectIndexed(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->reg[cpustate->modval2 & 0x1F] + cpustate->reg[cpustate->modval & 0x1F]);
		break;
	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + cpustate->reg[cpustate->modval & 0x1F] * 2);
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + cpustate->reg[cpustate->modval & 0x1F] * 4);
		break;
	}

	return 2;
}

static UINT32 bam1RegisterIndirectIndexed(v60_state *cpustate)
{
	cpustate->bamoffset = cpustate->reg[cpustate->modval & 0x1F];
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + cpustate->bamoffset / 8);
	cpustate->bamoffset&=7;
	return 2;
}

static UINT32 am1Autoincrement(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->reg[cpustate->modval & 0x1F]);
		cpustate->reg[cpustate->modval & 0x1F]++;
		break;
	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->reg[cpustate->modval & 0x1F]);
		cpustate->reg[cpustate->modval & 0x1F] +=2;
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F]);
		cpustate->reg[cpustate->modval & 0x1F] +=4;
		break;
	}

	return 1;
}

static UINT32 bam1Autoincrement(v60_state *cpustate)
{
	cpustate->bamoffset = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F]);
	switch (cpustate->moddim)
	{
	case 10:
		cpustate->reg[cpustate->modval & 0x1F] +=1;
		break;
	case 11:
		cpustate->reg[cpustate->modval & 0x1F] +=4;
		break;
	default:
		fatalerror("CPU - BAM1 - 7\n");
		break;
	}
	return 1;
}

static UINT32 am1Autodecrement(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->reg[cpustate->modval & 0x1F]--;
		cpustate->amout = cpustate->program->read_byte(cpustate->reg[cpustate->modval & 0x1F]);
		break;
	case 1:
		cpustate->reg[cpustate->modval & 0x1F]-=2;
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->reg[cpustate->modval & 0x1F]);
		break;
	case 2:
		cpustate->reg[cpustate->modval & 0x1F]-=4;
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F]);
		break;
	}

	return 1;
}

static UINT32 bam1Autodecrement(v60_state *cpustate)
{
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
		fatalerror("CPU - BAM1 - 7\n");
		break;
	}
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F]);
	return 1;
}

static UINT32 am1Displacement8(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->reg[cpustate->modval & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 1));
		break;
	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 1));
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 1));
		break;
	}

	return 2;
}

static UINT32 bam1Displacement8(v60_state *cpustate)
{
	cpustate->bamoffset = cpustate->program->read_byte(cpustate->modadd + 1);
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + cpustate->bamoffset / 8);
	cpustate->bamoffset&=7;
	return 2;
}


static UINT32 am1Displacement16(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->reg[cpustate->modval & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 1));
		break;
	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 1));
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 1));
		break;
	}

	return 3;
}

static UINT32 bam1Displacement16(v60_state *cpustate)
{
	cpustate->bamoffset = OpRead16(cpustate, cpustate->modadd + 1);
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + cpustate->bamoffset / 8);
	cpustate->bamoffset&=7;
	return 3;
}

static UINT32 am1Displacement32(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->reg[cpustate->modval & 0x1F] + OpRead32(cpustate, cpustate->modadd + 1));
		break;
	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->reg[cpustate->modval & 0x1F] + OpRead32(cpustate, cpustate->modadd + 1));
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + OpRead32(cpustate, cpustate->modadd + 1));
		break;
	}

	return 5;
}

static UINT32 bam1Displacement32(v60_state *cpustate)
{
	cpustate->bamoffset = OpRead32(cpustate, cpustate->modadd + 1);
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + cpustate->bamoffset / 8);
	cpustate->bamoffset&=7;
	return 5;
}

static UINT32 am1DisplacementIndexed8(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->reg[cpustate->modval2 & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F]);
		break;
	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 2);
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 4);
		break;
	}

	return 3;
}

static UINT32 bam1DisplacementIndexed8(v60_state *cpustate)
{
	cpustate->bamoffset = cpustate->reg[cpustate->modval & 0x1F];
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 2) + cpustate->bamoffset / 8);
	cpustate->bamoffset&=7;
	return 3;
}

static UINT32 am1DisplacementIndexed16(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->reg[cpustate->modval2 & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F]);
		break;
	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 2);
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 4);
		break;
	}

	return 4;
}

static UINT32 bam1DisplacementIndexed16(v60_state *cpustate)
{
	cpustate->bamoffset = cpustate->reg[cpustate->modval & 0x1F];
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 2) + cpustate->bamoffset / 8);
	cpustate->bamoffset&=7;
	return 4;
}

static UINT32 am1DisplacementIndexed32(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->reg[cpustate->modval2 & 0x1F] + OpRead32(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F]);
		break;
	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + OpRead32(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 2);
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + OpRead32(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 4);
		break;
	}

	return 6;
}

static UINT32 bam1DisplacementIndexed32(v60_state *cpustate)
{
	cpustate->bamoffset = cpustate->reg[cpustate->modval & 0x1F];
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + OpRead32(cpustate, cpustate->modadd + 2) + cpustate->bamoffset / 8);
	cpustate->bamoffset&=7;
	return 6;
}


static UINT32 am1PCDisplacement8(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 1));
		break;
	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 1));
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 1));
		break;
	}

	return 2;
}

static UINT32 bam1PCDisplacement8(v60_state *cpustate)
{
	cpustate->bamoffset = OpRead8(cpustate, cpustate->modadd + 1);
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + cpustate->bamoffset / 8);
	cpustate->bamoffset&=7;
	return 2;
}

static UINT32 am1PCDisplacement16(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 1));
		break;
	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 1));
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 1));
		break;
	}

	return 3;
}

static UINT32 bam1PCDisplacement16(v60_state *cpustate)
{
	cpustate->bamoffset = OpRead16(cpustate, cpustate->modadd + 1);
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + cpustate->bamoffset / 8);
	cpustate->bamoffset&=7;
	return 3;
}

static UINT32 am1PCDisplacement32(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 1));
		break;
	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 1));
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 1));
		break;
	}

	return 5;
}

static UINT32 bam1PCDisplacement32(v60_state *cpustate)
{
	cpustate->bamoffset = OpRead32(cpustate, cpustate->modadd + 1);
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + cpustate->bamoffset / 8);
	cpustate->bamoffset&=7;
	return 5;
}

static UINT32 am1PCDisplacementIndexed8(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F]);
		break;
	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 2);
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 4);
		break;
	}

	return 3;
}

static UINT32 bam1PCDisplacementIndexed8(v60_state *cpustate)
{
	cpustate->bamoffset = cpustate->reg[cpustate->modval & 0x1F];
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 2) + cpustate->bamoffset / 8);
	cpustate->bamoffset&=7;
	return 3;
}


static UINT32 am1PCDisplacementIndexed16(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F]);
		break;
	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 2);
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 4);
		break;
	}

	return 4;
}

static UINT32 bam1PCDisplacementIndexed16(v60_state *cpustate)
{
	cpustate->bamoffset = cpustate->reg[cpustate->modval & 0x1F];
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 2) + cpustate->bamoffset / 8);
	cpustate->bamoffset&=7;
	return 4;
}

static UINT32 am1PCDisplacementIndexed32(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F]);
		break;
	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 2);
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 4);
		break;
	}

	return 6;
}

static UINT32 bam1PCDisplacementIndexed32(v60_state *cpustate)
{
	cpustate->bamoffset = cpustate->reg[cpustate->modval & 0x1F];
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 2) + cpustate->bamoffset / 8);
	cpustate->bamoffset&=7;
	return 6;
}

static UINT32 am1DisplacementIndirect8(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 1)));
		break;
	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 1)));
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 1)));
		break;
	}

	return 2;
}

static UINT32 bam1DisplacementIndirect8(v60_state *cpustate)
{
	cpustate->bamoffset = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 1)));
	return 2;
}

static UINT32 am1DisplacementIndirect16(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 1)));
		break;
	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 1)));
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 1)));
		break;
	}

	return 3;
}

static UINT32 bam1DisplacementIndirect16(v60_state *cpustate)
{
	cpustate->bamoffset = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 1)));
	return 3;
}

static UINT32 am1DisplacementIndirect32(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + OpRead32(cpustate, cpustate->modadd + 1)));
		break;
	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + OpRead32(cpustate, cpustate->modadd + 1)));
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + OpRead32(cpustate, cpustate->modadd + 1)));
		break;
	}

	return 5;
}

static UINT32 bam1DisplacementIndirect32(v60_state *cpustate)
{
	cpustate->bamoffset = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + OpRead32(cpustate, cpustate->modadd + 1)));
	return 5;
}

static UINT32 am1DisplacementIndirectIndexed8(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F]);
		break;
	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 2);
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 4);
		break;
	}

	return 3;
}

static UINT32 bam1DisplacementIndirectIndexed8(v60_state *cpustate)
{
	cpustate->bamoffset = cpustate->reg[cpustate->modval & 0x1F];
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 2)) + cpustate->bamoffset / 8);
	cpustate->bamoffset&=7;
	return 3;
}

static UINT32 am1DisplacementIndirectIndexed16(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F]);
		break;
	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 2);
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 4);
		break;
	}

	return 4;
}

static UINT32 bam1DisplacementIndirectIndexed16(v60_state *cpustate)
{
	cpustate->bamoffset = cpustate->reg[cpustate->modval & 0x1F];
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 2)) + cpustate->bamoffset / 8);
	cpustate->bamoffset&=7;
	return 4;
}

static UINT32 am1DisplacementIndirectIndexed32(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + OpRead32(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F]);
		break;
	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + OpRead32(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 2);
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + OpRead32(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 4);
		break;
	}

	return 6;
}

static UINT32 bam1DisplacementIndirectIndexed32(v60_state *cpustate)
{
	cpustate->bamoffset = cpustate->reg[cpustate->modval & 0x1F];
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval2 & 0x1F] + OpRead32(cpustate, cpustate->modadd + 2)) + cpustate->bamoffset / 8);
	cpustate->bamoffset&=7;
	return 6;
}

static UINT32 am1PCDisplacementIndirect8(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->program->read_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 1)));
		break;
	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 1)));
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 1)));
		break;
	}

	return 2;
}

static UINT32 bam1PCDisplacementIndirect8(v60_state *cpustate)
{
	cpustate->bamoffset = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 1)));
	return 2;
}

static UINT32 am1PCDisplacementIndirect16(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->program->read_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 1)));
		break;
	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 1)));
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 1)));
		break;
	}

	return 3;
}

static UINT32 bam1PCDisplacementIndirect16(v60_state *cpustate)
{
	cpustate->bamoffset = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 1)));
	return 3;
}

static UINT32 am1PCDisplacementIndirect32(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->program->read_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 1)));
		break;
	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 1)));
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 1)));
		break;
	}

	return 5;
}

static UINT32 bam1PCDisplacementIndirect32(v60_state *cpustate)
{
	cpustate->bamoffset = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 1)));
	return 5;
}

static UINT32 am1PCDisplacementIndirectIndexed8(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->program->read_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F]);
		break;
	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 2);
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 4);
		break;
	}

	return 3;
}

static UINT32 bam1PCDisplacementIndirectIndexed8(v60_state *cpustate)
{
	cpustate->bamoffset = cpustate->reg[cpustate->modval & 0x1F];
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 2)) + cpustate->bamoffset / 8);
	cpustate->bamoffset&=7;
	return 3;
}

static UINT32 am1PCDisplacementIndirectIndexed16(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->program->read_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F]);
		break;
	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 2);
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 4);
		break;
	}

	return 4;
}

static UINT32 bam1PCDisplacementIndirectIndexed16(v60_state *cpustate)
{
	cpustate->bamoffset = cpustate->reg[cpustate->modval & 0x1F];
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 2)) + cpustate->bamoffset / 8);
	cpustate->bamoffset&=7;
	return 4;
}

static UINT32 am1PCDisplacementIndirectIndexed32(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->program->read_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F]);
		break;
	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 2);
		break;
	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 4);
		break;
	}

	return 6;
}

static UINT32 bam1PCDisplacementIndirectIndexed32(v60_state *cpustate)
{
	cpustate->bamoffset = cpustate->reg[cpustate->modval & 0x1F];
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 2)) + cpustate->bamoffset / 8);
	cpustate->bamoffset&=7;
	return 6;
}

static UINT32 am1DoubleDisplacement8(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 1)) + (INT8)OpRead8(cpustate, cpustate->modadd + 2));
		break;

	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 1)) + (INT8)OpRead8(cpustate, cpustate->modadd + 2));
		break;

	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 1)) + (INT8)OpRead8(cpustate, cpustate->modadd + 2));
		break;
	}

	return 3;
}

static UINT32 bam1DoubleDisplacement8(v60_state *cpustate)
{
	cpustate->bamoffset = OpRead8(cpustate, cpustate->modadd + 2);
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT8)OpRead8(cpustate, cpustate->modadd + 1)) + cpustate->bamoffset / 8);
	cpustate->bamoffset&=7;
	return 3;
}

static UINT32 am1DoubleDisplacement16(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 1)) + (INT16)OpRead16(cpustate, cpustate->modadd + 3));
		break;

	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 1)) + (INT16)OpRead16(cpustate, cpustate->modadd + 3));
		break;

	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 1)) + (INT16)OpRead16(cpustate, cpustate->modadd + 3));
		break;
	}

	return 5;
}

static UINT32 bam1DoubleDisplacement16(v60_state *cpustate)
{
	cpustate->bamoffset = OpRead16(cpustate, cpustate->modadd + 3);
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + (INT16)OpRead16(cpustate, cpustate->modadd + 1)) + cpustate->bamoffset / 8);
	cpustate->bamoffset&=7;
	return 5;
}

static UINT32 am1DoubleDisplacement32(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + OpRead32(cpustate, cpustate->modadd + 1)) + OpRead32(cpustate, cpustate->modadd + 5));
		break;

	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + OpRead32(cpustate, cpustate->modadd + 1)) + OpRead32(cpustate, cpustate->modadd + 5));
		break;

	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + OpRead32(cpustate, cpustate->modadd + 1)) + OpRead32(cpustate, cpustate->modadd + 5));
		break;
	}

	return 9;
}

static UINT32 bam1DoubleDisplacement32(v60_state *cpustate)
{
	cpustate->bamoffset = OpRead32(cpustate, cpustate->modadd + 5);
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->reg[cpustate->modval & 0x1F] + OpRead32(cpustate, cpustate->modadd + 1)) + cpustate->bamoffset / 8);
	cpustate->bamoffset&=7;
	return 9;
}

static UINT32 am1PCDoubleDisplacement8(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->program->read_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 1)) + (INT8)OpRead8(cpustate, cpustate->modadd + 2));
		break;

	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 1)) + (INT8)OpRead8(cpustate, cpustate->modadd + 2));
		break;

	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 1)) + (INT8)OpRead8(cpustate, cpustate->modadd + 2));
		break;
	}

	return 3;
}

static UINT32 bam1PCDoubleDisplacement8(v60_state *cpustate)
{
	cpustate->bamoffset = OpRead8(cpustate, cpustate->modadd + 2);
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + (INT8)OpRead8(cpustate, cpustate->modadd + 1)) + cpustate->bamoffset / 8);
	cpustate->bamoffset&=7;
	return 3;
}

static UINT32 am1PCDoubleDisplacement16(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->program->read_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 1)) + (INT16)OpRead16(cpustate, cpustate->modadd + 3));
		break;

	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 1)) + (INT16)OpRead16(cpustate, cpustate->modadd + 3));
		break;

	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 1)) + (INT16)OpRead16(cpustate, cpustate->modadd + 3));
		break;
	}

	return 5;
}

static UINT32 bam1PCDoubleDisplacement16(v60_state *cpustate)
{
	cpustate->bamoffset = OpRead16(cpustate, cpustate->modadd + 3);
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + (INT16)OpRead16(cpustate, cpustate->modadd + 1)) + cpustate->bamoffset / 8);
	cpustate->bamoffset&=7;
	return 5;
}


static UINT32 am1PCDoubleDisplacement32(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->program->read_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 1)) + OpRead32(cpustate, cpustate->modadd + 5));
		break;

	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 1)) + OpRead32(cpustate, cpustate->modadd + 5));
		break;

	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 1)) + OpRead32(cpustate, cpustate->modadd + 5));
		break;
	}

	return 9;
}

static UINT32 bam1PCDoubleDisplacement32(v60_state *cpustate)
{
	cpustate->bamoffset = OpRead32(cpustate, cpustate->modadd + 5);
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(cpustate->PC + OpRead32(cpustate, cpustate->modadd + 1)) + cpustate->bamoffset / 8);
	cpustate->bamoffset&=7;
	return 9;
}

static UINT32 am1DirectAddress(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(OpRead32(cpustate, cpustate->modadd + 1));
		break;

	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(OpRead32(cpustate, cpustate->modadd + 1));
		break;

	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(OpRead32(cpustate, cpustate->modadd + 1));
		break;
	}

	return 5;
}

static UINT32 bam1DirectAddress(v60_state *cpustate)
{
	cpustate->bamoffset = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(OpRead32(cpustate, cpustate->modadd + 1));
	return 5;
}

static UINT32 am1DirectAddressIndexed(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(OpRead32(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F]);
		break;

	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(OpRead32(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 2);
		break;

	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(OpRead32(cpustate, cpustate->modadd + 2) + cpustate->reg[cpustate->modval & 0x1F] * 4);
		break;
	}

	return 6;
}

static UINT32 bam1DirectAddressIndexed(v60_state *cpustate)
{
	cpustate->bamoffset = cpustate->reg[cpustate->modval & 0x1F];
	cpustate->amout = cpustate->program->read_dword_unaligned(OpRead32(cpustate, cpustate->modadd + 2) + cpustate->bamoffset / 8);
	cpustate->bamoffset&=7;
	return 6;
}

static UINT32 am1DirectAddressDeferred(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->program->read_dword_unaligned(OpRead32(cpustate, cpustate->modadd + 1)));
		break;

	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->program->read_dword_unaligned(OpRead32(cpustate, cpustate->modadd + 1)));
		break;

	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(OpRead32(cpustate, cpustate->modadd + 1)));
		break;
	}

	return 5;
}

static UINT32 bam1DirectAddressDeferred(v60_state *cpustate)
{
	cpustate->bamoffset = 0;
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(OpRead32(cpustate, cpustate->modadd + 1)));
	return 5;
}

static UINT32 am1DirectAddressDeferredIndexed(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = cpustate->program->read_byte(cpustate->program->read_dword_unaligned(OpRead32(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F]);
		break;

	case 1:
		cpustate->amout = cpustate->program->read_word_unaligned(cpustate->program->read_dword_unaligned(OpRead32(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 2);
		break;

	case 2:
		cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(OpRead32(cpustate, cpustate->modadd + 2)) + cpustate->reg[cpustate->modval & 0x1F] * 4);
		break;
	}

	return 6;
}

static UINT32 bam1DirectAddressDeferredIndexed(v60_state *cpustate)
{
	cpustate->bamoffset = cpustate->reg[cpustate->modval & 0x1F];
	cpustate->amout = cpustate->program->read_dword_unaligned(cpustate->program->read_dword_unaligned(OpRead32(cpustate, cpustate->modadd + 2)) + cpustate->bamoffset / 8);
	cpustate->bamoffset&=7;
	return 6;
}

static UINT32 am1Immediate(v60_state *cpustate)
{
	switch (cpustate->moddim)
	{
	case 0:
		cpustate->amout = OpRead8(cpustate, cpustate->modadd + 1);
		return 2;

	case 1:
		cpustate->amout = OpRead16(cpustate, cpustate->modadd + 1);
		return 3;

	case 2:
		cpustate->amout = OpRead32(cpustate, cpustate->modadd + 1);
		return 5;
	}

	// It should not be here!  Written to avoid warning
	assert(0);
	return 1;
}

static UINT32 am1ImmediateQuick(v60_state *cpustate)
{
	cpustate->amout = cpustate->modval & 0xF;
	return 1;
}




// AM1 Tables (for ReadAM)
// ***********************

static UINT32 am1Error1(v60_state *cpustate)
{
	fatalerror("CPU - AM1 - 1 (cpustate->PC=%06x)\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 bam1Error1(v60_state *cpustate)
{
	fatalerror("CPU - BAM1 - 1 (cpustate->PC=%06x)\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 am1Error2(v60_state *cpustate)
{
	fatalerror("CPU - AM1 - 2 (cpustate->PC=%06x)\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 bam1Error2(v60_state *cpustate)
{
	fatalerror("CPU - BAM1 - 2 (cpustate->PC=%06x)\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

#ifdef UNUSED_FUNCTION
static UINT32 am1Error3(v60_state *cpustate)
{
	fatalerror("CPU - AM1 - 3 (cpustate->PC=%06x)\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 bam1Error3(v60_state *cpustate)
{
	fatalerror("CPU - BAM1 - 3 (cpustate->PC=%06x)\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}
#endif

static UINT32 am1Error4(v60_state *cpustate)
{
	fatalerror("CPU - AM1 - 4 (cpustate->PC=%06x)\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 bam1Error4(v60_state *cpustate)
{
	fatalerror("CPU - BAM1 - 4 (cpustate->PC=%06x)\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 am1Error5(v60_state *cpustate)
{
	fatalerror("CPU - AM1 - 5 (cpustate->PC=%06x)\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 bam1Error5(v60_state *cpustate)
{
	fatalerror("CPU - BAM1 - 5 (cpustate->PC=%06x)\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 bam1Error6(v60_state *cpustate)
{
	fatalerror("CPU - BAM1 - 6 (cpustate->PC=%06x)\n", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 (*const AMTable1_G7a[16])(v60_state *) =
{
	am1PCDisplacementIndexed8,
	am1PCDisplacementIndexed16,
	am1PCDisplacementIndexed32,
	am1DirectAddressIndexed,
	am1Error5,
	am1Error5,
	am1Error5,
	am1Error5,
	am1PCDisplacementIndirectIndexed8,
	am1PCDisplacementIndirectIndexed16,
	am1PCDisplacementIndirectIndexed32,
	am1DirectAddressDeferredIndexed,
	am1Error5,
	am1Error5,
	am1Error5,
	am1Error5
};

static UINT32 (*const BAMTable1_G7a[16])(v60_state *) =
{
	bam1PCDisplacementIndexed8,
	bam1PCDisplacementIndexed16,
	bam1PCDisplacementIndexed32,
	bam1DirectAddressIndexed,
	bam1Error5,
	bam1Error5,
	bam1Error5,
	bam1Error5,
	bam1PCDisplacementIndirectIndexed8,
	bam1PCDisplacementIndirectIndexed16,
	bam1PCDisplacementIndirectIndexed32,
	bam1DirectAddressDeferredIndexed,
	bam1Error5,
	bam1Error5,
	bam1Error5,
	bam1Error5
};


static UINT32 am1Group7a(v60_state *cpustate)
{
	if (!(cpustate->modval2 & 0x10))
		return am1Error4(cpustate);

	return AMTable1_G7a[cpustate->modval2 & 0xF](cpustate);
}

static UINT32 bam1Group7a(v60_state *cpustate)
{
	if (!(cpustate->modval2 & 0x10))
		return bam1Error4(cpustate);

	return BAMTable1_G7a[cpustate->modval2 & 0xF](cpustate);
}

static UINT32 (*const AMTable1_G7[32])(v60_state *) =
{
	am1ImmediateQuick,
	am1ImmediateQuick,
	am1ImmediateQuick,
	am1ImmediateQuick,
	am1ImmediateQuick,
	am1ImmediateQuick,
	am1ImmediateQuick,
	am1ImmediateQuick,
	am1ImmediateQuick,
	am1ImmediateQuick,
	am1ImmediateQuick,
	am1ImmediateQuick,
	am1ImmediateQuick,
	am1ImmediateQuick,
	am1ImmediateQuick,
	am1ImmediateQuick,
	am1PCDisplacement8,
	am1PCDisplacement16,
	am1PCDisplacement32,
	am1DirectAddress,
	am1Immediate,
	am1Error2,
	am1Error2,
	am1Error2,
	am1PCDisplacementIndirect8,
	am1PCDisplacementIndirect16,
	am1PCDisplacementIndirect32,
	am1DirectAddressDeferred,
	am1PCDoubleDisplacement8,
	am1PCDoubleDisplacement16,
	am1PCDoubleDisplacement32,
	am1Error2
};

static UINT32 (*const BAMTable1_G7[32])(v60_state *) =
{
	bam1Error6,
	bam1Error6,
	bam1Error6,
	bam1Error6,
	bam1Error6,
	bam1Error6,
	bam1Error6,
	bam1Error6,
	bam1Error6,
	bam1Error6,
	bam1Error6,
	bam1Error6,
	bam1Error6,
	bam1Error6,
	bam1Error6,
	bam1Error6,
	bam1PCDisplacement8,
	bam1PCDisplacement16,
	bam1PCDisplacement32,
	bam1DirectAddress,
	bam1Error6,
	bam1Error2,
	bam1Error2,
	bam1Error2,
	bam1PCDisplacementIndirect8,
	bam1PCDisplacementIndirect16,
	bam1PCDisplacementIndirect32,
	bam1DirectAddressDeferred,
	bam1PCDoubleDisplacement8,
	bam1PCDoubleDisplacement16,
	bam1PCDoubleDisplacement32,
	bam1Error2
};



static UINT32 (*const AMTable1_G6[8])(v60_state *) =
{
	am1DisplacementIndexed8,
	am1DisplacementIndexed16,
	am1DisplacementIndexed32,
	am1RegisterIndirectIndexed,
	am1DisplacementIndirectIndexed8,
	am1DisplacementIndirectIndexed16,
	am1DisplacementIndirectIndexed32,
	am1Group7a
};

static UINT32 (*const BAMTable1_G6[8])(v60_state *) =
{
	bam1DisplacementIndexed8,
	bam1DisplacementIndexed16,
	bam1DisplacementIndexed32,
	bam1RegisterIndirectIndexed,
	bam1DisplacementIndirectIndexed8,
	bam1DisplacementIndirectIndexed16,
	bam1DisplacementIndirectIndexed32,
	bam1Group7a
};


static UINT32 am1Group6(v60_state *cpustate)
{
	cpustate->modval2 = OpRead8(cpustate, cpustate->modadd + 1);
	return AMTable1_G6[cpustate->modval2 >> 5](cpustate);
}

static UINT32 bam1Group6(v60_state *cpustate)
{
	cpustate->modval2 = OpRead8(cpustate, cpustate->modadd + 1);
	return BAMTable1_G6[cpustate->modval2 >> 5](cpustate);
}


static UINT32 am1Group7(v60_state *cpustate)
{
	return AMTable1_G7[cpustate->modval & 0x1F](cpustate);
}

static UINT32 bam1Group7(v60_state *cpustate)
{
	return BAMTable1_G7[cpustate->modval & 0x1F](cpustate);
}

static UINT32 (*const AMTable1[2][8])(v60_state *) =
{
	{
		am1Displacement8,
		am1Displacement16,
		am1Displacement32,
		am1RegisterIndirect,
		am1DisplacementIndirect8,
		am1DisplacementIndirect16,
		am1DisplacementIndirect32,
		am1Group7
	},

	{
		am1DoubleDisplacement8,
		am1DoubleDisplacement16,
		am1DoubleDisplacement32,
		am1Register,
		am1Autoincrement,
		am1Autodecrement,
		am1Group6,
		am1Error1
	}
};


static UINT32 (*const BAMTable1[2][8])(v60_state *) =
{
	{
		bam1Displacement8,
		bam1Displacement16,
		bam1Displacement32,
		bam1RegisterIndirect,
		bam1DisplacementIndirect8,
		bam1DisplacementIndirect16,
		bam1DisplacementIndirect32,
		bam1Group7
	},

	{
		bam1DoubleDisplacement8,
		bam1DoubleDisplacement16,
		bam1DoubleDisplacement32,
		bam1Error6,
		bam1Autoincrement,
		bam1Autodecrement,
		bam1Group6,
		bam1Error1
	}
};
