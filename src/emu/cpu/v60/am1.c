
// AM1 Functions (for ReadAM)
// **************************

static UINT32 am1Register(void)
{
	switch (modDim)
	{
	case 0:
		amOut=(UINT8)v60.reg[modVal&0x1F];
		break;
	case 1:
		amOut=(UINT16)v60.reg[modVal&0x1F];
		break;
	case 2:
		amOut=v60.reg[modVal&0x1F];
		break;
	}

	return 1;
}

static UINT32 am1RegisterIndirect(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,v60.reg[modVal&0x1F]);
		break;
	case 1:
		amOut=MemRead16(v60.program,v60.reg[modVal&0x1F]);
		break;
	case 2:
		amOut=MemRead32(v60.program,v60.reg[modVal&0x1F]);
		break;
	}

	return 1;
}

static UINT32 bam1RegisterIndirect(void)
{
	bamOffset=0;
	amOut=MemRead32(v60.program,v60.reg[modVal&0x1F]);
	return 1;
}

static UINT32 am1RegisterIndirectIndexed(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,v60.reg[modVal2&0x1F]+v60.reg[modVal&0x1F]);
		break;
	case 1:
		amOut=MemRead16(v60.program,v60.reg[modVal2&0x1F]+v60.reg[modVal&0x1F]*2);
		break;
	case 2:
		amOut=MemRead32(v60.program,v60.reg[modVal2&0x1F]+v60.reg[modVal&0x1F]*4);
		break;
	}

	return 2;
}

static UINT32 bam1RegisterIndirectIndexed(void)
{
	bamOffset=v60.reg[modVal&0x1F];
	amOut=MemRead32(v60.program,v60.reg[modVal2&0x1F]+bamOffset/8);
	bamOffset&=7;
	return 2;
}

static UINT32 am1Autoincrement(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,v60.reg[modVal&0x1F]);
		v60.reg[modVal&0x1F]++;
		break;
	case 1:
		amOut=MemRead16(v60.program,v60.reg[modVal&0x1F]);
		v60.reg[modVal&0x1F]+=2;
		break;
	case 2:
		amOut=MemRead32(v60.program,v60.reg[modVal&0x1F]);
		v60.reg[modVal&0x1F]+=4;
		break;
	}

	return 1;
}

static UINT32 bam1Autoincrement(void)
{
	bamOffset=0;
	amOut=MemRead32(v60.program,v60.reg[modVal&0x1F]);
	switch (modDim)
	{
	case 10:
		v60.reg[modVal&0x1F]+=1;
		break;
	case 11:
		v60.reg[modVal&0x1F]+=4;
		break;
	default:
		fatalerror("CPU - BAM1 - 7");
		break;
	}
	return 1;
}

static UINT32 am1Autodecrement(void)
{
	switch (modDim)
	{
	case 0:
		v60.reg[modVal&0x1F]--;
		amOut=MemRead8(v60.program,v60.reg[modVal&0x1F]);
		break;
	case 1:
		v60.reg[modVal&0x1F]-=2;
		amOut=MemRead16(v60.program,v60.reg[modVal&0x1F]);
		break;
	case 2:
		v60.reg[modVal&0x1F]-=4;
		amOut=MemRead32(v60.program,v60.reg[modVal&0x1F]);
		break;
	}

	return 1;
}

static UINT32 bam1Autodecrement(void)
{
	bamOffset=0;
	switch (modDim)
	{
	case 10:
		v60.reg[modVal&0x1F]-=1;
		break;
	case 11:
		v60.reg[modVal&0x1F]-=4;
		break;
	default:
		fatalerror("CPU - BAM1 - 7");
		break;
	}
	amOut=MemRead32(v60.program,v60.reg[modVal&0x1F]);
	return 1;
}

static UINT32 am1Displacement8(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,v60.reg[modVal&0x1F] + (INT8)OpRead8(v60.program,modAdd+1));
		break;
	case 1:
		amOut=MemRead16(v60.program,v60.reg[modVal&0x1F] + (INT8)OpRead8(v60.program,modAdd+1));
		break;
	case 2:
		amOut=MemRead32(v60.program,v60.reg[modVal&0x1F] + (INT8)OpRead8(v60.program,modAdd+1));
		break;
	}

	return 2;
}

static UINT32 bam1Displacement8(void)
{
	bamOffset=MemRead8(v60.program,modAdd+1);
	amOut=MemRead32(v60.program,v60.reg[modVal&0x1F]+bamOffset/8);
	bamOffset&=7;
	return 2;
}


static UINT32 am1Displacement16(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,v60.reg[modVal&0x1F] + (INT16)OpRead16(v60.program,modAdd+1));
		break;
	case 1:
		amOut=MemRead16(v60.program,v60.reg[modVal&0x1F] + (INT16)OpRead16(v60.program,modAdd+1));
		break;
	case 2:
		amOut=MemRead32(v60.program,v60.reg[modVal&0x1F] + (INT16)OpRead16(v60.program,modAdd+1));
		break;
	}

	return 3;
}

static UINT32 bam1Displacement16(void)
{
	bamOffset=OpRead16(v60.program,modAdd+1);
	amOut=MemRead32(v60.program,v60.reg[modVal&0x1F]+bamOffset/8);
	bamOffset&=7;
	return 3;
}

static UINT32 am1Displacement32(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,v60.reg[modVal&0x1F] + OpRead32(v60.program,modAdd+1));
		break;
	case 1:
		amOut=MemRead16(v60.program,v60.reg[modVal&0x1F] + OpRead32(v60.program,modAdd+1));
		break;
	case 2:
		amOut=MemRead32(v60.program,v60.reg[modVal&0x1F] + OpRead32(v60.program,modAdd+1));
		break;
	}

	return 5;
}

static UINT32 bam1Displacement32(void)
{
	bamOffset=OpRead32(v60.program,modAdd+1);
	amOut=MemRead32(v60.program,v60.reg[modVal&0x1F]+bamOffset/8);
	bamOffset&=7;
	return 5;
}

static UINT32 am1DisplacementIndexed8(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,v60.reg[modVal2&0x1F] + (INT8)OpRead8(v60.program,modAdd+2) + v60.reg[modVal&0x1F]);
		break;
	case 1:
		amOut=MemRead16(v60.program,v60.reg[modVal2&0x1F] + (INT8)OpRead8(v60.program,modAdd+2) + v60.reg[modVal&0x1F]*2);
		break;
	case 2:
		amOut=MemRead32(v60.program,v60.reg[modVal2&0x1F] + (INT8)OpRead8(v60.program,modAdd+2) + v60.reg[modVal&0x1F]*4);
		break;
	}

	return 3;
}

static UINT32 bam1DisplacementIndexed8(void)
{
	bamOffset=v60.reg[modVal&0x1F];
	amOut=MemRead32(v60.program,v60.reg[modVal2&0x1F] + (INT8)OpRead8(v60.program,modAdd+2)+bamOffset/8);
	bamOffset&=7;
	return 3;
}

static UINT32 am1DisplacementIndexed16(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,v60.reg[modVal2&0x1F] + (INT16)OpRead16(v60.program,modAdd+2) + v60.reg[modVal&0x1F]);
		break;
	case 1:
		amOut=MemRead16(v60.program,v60.reg[modVal2&0x1F] + (INT16)OpRead16(v60.program,modAdd+2) + v60.reg[modVal&0x1F]*2);
		break;
	case 2:
		amOut=MemRead32(v60.program,v60.reg[modVal2&0x1F] + (INT16)OpRead16(v60.program,modAdd+2) + v60.reg[modVal&0x1F]*4);
		break;
	}

	return 4;
}

static UINT32 bam1DisplacementIndexed16(void)
{
	bamOffset=v60.reg[modVal&0x1F];
	amOut=MemRead32(v60.program,v60.reg[modVal2&0x1F] + (INT16)OpRead16(v60.program,modAdd+2)+bamOffset/8);
	bamOffset&=7;
	return 4;
}

static UINT32 am1DisplacementIndexed32(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,v60.reg[modVal2&0x1F] + OpRead32(v60.program,modAdd+2) + v60.reg[modVal&0x1F]);
		break;
	case 1:
		amOut=MemRead16(v60.program,v60.reg[modVal2&0x1F] + OpRead32(v60.program,modAdd+2) + v60.reg[modVal&0x1F]*2);
		break;
	case 2:
		amOut=MemRead32(v60.program,v60.reg[modVal2&0x1F] + OpRead32(v60.program,modAdd+2) + v60.reg[modVal&0x1F]*4);
		break;
	}

	return 6;
}

static UINT32 bam1DisplacementIndexed32(void)
{
	bamOffset=v60.reg[modVal&0x1F];
	amOut=MemRead32(v60.program,v60.reg[modVal2&0x1F] + OpRead32(v60.program,modAdd+2)+bamOffset/8);
	bamOffset&=7;
	return 6;
}


static UINT32 am1PCDisplacement8(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,PC + (INT8)OpRead8(v60.program,modAdd+1));
		break;
	case 1:
		amOut=MemRead16(v60.program,PC + (INT8)OpRead8(v60.program,modAdd+1));
		break;
	case 2:
		amOut=MemRead32(v60.program,PC + (INT8)OpRead8(v60.program,modAdd+1));
		break;
	}

	return 2;
}

static UINT32 bam1PCDisplacement8(void)
{
	bamOffset=OpRead8(v60.program,modAdd+1);
	amOut=MemRead32(v60.program,PC+bamOffset/8);
	bamOffset&=7;
	return 2;
}

static UINT32 am1PCDisplacement16(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,PC + (INT16)OpRead16(v60.program,modAdd+1));
		break;
	case 1:
		amOut=MemRead16(v60.program,PC + (INT16)OpRead16(v60.program,modAdd+1));
		break;
	case 2:
		amOut=MemRead32(v60.program,PC + (INT16)OpRead16(v60.program,modAdd+1));
		break;
	}

	return 3;
}

static UINT32 bam1PCDisplacement16(void)
{
	bamOffset=OpRead16(v60.program,modAdd+1);
	amOut=MemRead32(v60.program,PC+bamOffset/8);
	bamOffset&=7;
	return 3;
}

static UINT32 am1PCDisplacement32(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,PC + OpRead32(v60.program,modAdd+1));
		break;
	case 1:
		amOut=MemRead16(v60.program,PC + OpRead32(v60.program,modAdd+1));
		break;
	case 2:
		amOut=MemRead32(v60.program,PC + OpRead32(v60.program,modAdd+1));
		break;
	}

	return 5;
}

static UINT32 bam1PCDisplacement32(void)
{
	bamOffset=OpRead32(v60.program,modAdd+1);
	amOut=MemRead32(v60.program,PC+bamOffset/8);
	bamOffset&=7;
	return 5;
}

static UINT32 am1PCDisplacementIndexed8(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,PC + (INT8)OpRead8(v60.program,modAdd+2) + v60.reg[modVal&0x1F]);
		break;
	case 1:
		amOut=MemRead16(v60.program,PC + (INT8)OpRead8(v60.program,modAdd+2) + v60.reg[modVal&0x1F]*2);
		break;
	case 2:
		amOut=MemRead32(v60.program,PC + (INT8)OpRead8(v60.program,modAdd+2) + v60.reg[modVal&0x1F]*4);
		break;
	}

	return 3;
}

static UINT32 bam1PCDisplacementIndexed8(void)
{
	bamOffset=v60.reg[modVal&0x1F];
	amOut=MemRead32(v60.program,PC + (INT8)OpRead8(v60.program,modAdd+2)+bamOffset/8);
	bamOffset&=7;
	return 3;
}


static UINT32 am1PCDisplacementIndexed16(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,PC + (INT16)OpRead16(v60.program,modAdd+2) + v60.reg[modVal&0x1F]);
		break;
	case 1:
		amOut=MemRead16(v60.program,PC + (INT16)OpRead16(v60.program,modAdd+2) + v60.reg[modVal&0x1F]*2);
		break;
	case 2:
		amOut=MemRead32(v60.program,PC + (INT16)OpRead16(v60.program,modAdd+2) + v60.reg[modVal&0x1F]*4);
		break;
	}

	return 4;
}

static UINT32 bam1PCDisplacementIndexed16(void)
{
	bamOffset=v60.reg[modVal&0x1F];
	amOut=MemRead32(v60.program,PC + (INT16)OpRead16(v60.program,modAdd+2)+bamOffset/8);
	bamOffset&=7;
	return 4;
}

static UINT32 am1PCDisplacementIndexed32(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,PC + OpRead32(v60.program,modAdd+2) + v60.reg[modVal&0x1F]);
		break;
	case 1:
		amOut=MemRead16(v60.program,PC + OpRead32(v60.program,modAdd+2) + v60.reg[modVal&0x1F]*2);
		break;
	case 2:
		amOut=MemRead32(v60.program,PC + OpRead32(v60.program,modAdd+2) + v60.reg[modVal&0x1F]*4);
		break;
	}

	return 6;
}

static UINT32 bam1PCDisplacementIndexed32(void)
{
	bamOffset=v60.reg[modVal&0x1F];
	amOut=MemRead32(v60.program,PC + OpRead32(v60.program,modAdd+2)+bamOffset/8);
	bamOffset&=7;
	return 6;
}

static UINT32 am1DisplacementIndirect8(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,MemRead32(v60.program,v60.reg[modVal&0x1F] + (INT8)OpRead8(v60.program,modAdd+1)));
		break;
	case 1:
		amOut=MemRead16(v60.program,MemRead32(v60.program,v60.reg[modVal&0x1F] + (INT8)OpRead8(v60.program,modAdd+1)));
		break;
	case 2:
		amOut=MemRead32(v60.program,MemRead32(v60.program,v60.reg[modVal&0x1F] + (INT8)OpRead8(v60.program,modAdd+1)));
		break;
	}

	return 2;
}

static UINT32 bam1DisplacementIndirect8(void)
{
	bamOffset=0;
	amOut=MemRead32(v60.program,MemRead32(v60.program,v60.reg[modVal&0x1F] + (INT8)OpRead8(v60.program,modAdd+1)));
	return 2;
}

static UINT32 am1DisplacementIndirect16(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,MemRead32(v60.program,v60.reg[modVal&0x1F] + (INT16)OpRead16(v60.program,modAdd+1)));
		break;
	case 1:
		amOut=MemRead16(v60.program,MemRead32(v60.program,v60.reg[modVal&0x1F] + (INT16)OpRead16(v60.program,modAdd+1)));
		break;
	case 2:
		amOut=MemRead32(v60.program,MemRead32(v60.program,v60.reg[modVal&0x1F] + (INT16)OpRead16(v60.program,modAdd+1)));
		break;
	}

	return 3;
}

static UINT32 bam1DisplacementIndirect16(void)
{
	bamOffset=0;
	amOut=MemRead32(v60.program,MemRead32(v60.program,v60.reg[modVal&0x1F] + (INT16)OpRead16(v60.program,modAdd+1)));
	return 3;
}

static UINT32 am1DisplacementIndirect32(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,MemRead32(v60.program,v60.reg[modVal&0x1F] + OpRead32(v60.program,modAdd+1)));
		break;
	case 1:
		amOut=MemRead16(v60.program,MemRead32(v60.program,v60.reg[modVal&0x1F] + OpRead32(v60.program,modAdd+1)));
		break;
	case 2:
		amOut=MemRead32(v60.program,MemRead32(v60.program,v60.reg[modVal&0x1F] + OpRead32(v60.program,modAdd+1)));
		break;
	}

	return 5;
}

static UINT32 bam1DisplacementIndirect32(void)
{
	bamOffset=0;
	amOut=MemRead32(v60.program,MemRead32(v60.program,v60.reg[modVal&0x1F] + OpRead32(v60.program,modAdd+1)));
	return 5;
}

static UINT32 am1DisplacementIndirectIndexed8(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,MemRead32(v60.program,v60.reg[modVal2&0x1F] + (INT8)OpRead8(v60.program,modAdd+2)) + v60.reg[modVal&0x1F]);
		break;
	case 1:
		amOut=MemRead16(v60.program,MemRead32(v60.program,v60.reg[modVal2&0x1F] + (INT8)OpRead8(v60.program,modAdd+2)) + v60.reg[modVal&0x1F]*2);
		break;
	case 2:
		amOut=MemRead32(v60.program,MemRead32(v60.program,v60.reg[modVal2&0x1F] + (INT8)OpRead8(v60.program,modAdd+2)) + v60.reg[modVal&0x1F]*4);
		break;
	}

	return 3;
}

static UINT32 bam1DisplacementIndirectIndexed8(void)
{
	bamOffset=v60.reg[modVal&0x1F];
	amOut=MemRead32(v60.program,MemRead32(v60.program,v60.reg[modVal2&0x1F] + (INT8)OpRead8(v60.program,modAdd+2))+bamOffset/8);
	bamOffset&=7;
	return 3;
}

static UINT32 am1DisplacementIndirectIndexed16(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,MemRead32(v60.program,v60.reg[modVal2&0x1F] + (INT16)OpRead16(v60.program,modAdd+2)) + v60.reg[modVal&0x1F]);
		break;
	case 1:
		amOut=MemRead16(v60.program,MemRead32(v60.program,v60.reg[modVal2&0x1F] + (INT16)OpRead16(v60.program,modAdd+2)) + v60.reg[modVal&0x1F]*2);
		break;
	case 2:
		amOut=MemRead32(v60.program,MemRead32(v60.program,v60.reg[modVal2&0x1F] + (INT16)OpRead16(v60.program,modAdd+2)) + v60.reg[modVal&0x1F]*4);
		break;
	}

	return 4;
}

static UINT32 bam1DisplacementIndirectIndexed16(void)
{
	bamOffset=v60.reg[modVal&0x1F];
	amOut=MemRead32(v60.program,MemRead32(v60.program,v60.reg[modVal2&0x1F] + (INT16)OpRead16(v60.program,modAdd+2))+bamOffset/8);
	bamOffset&=7;
	return 4;
}

static UINT32 am1DisplacementIndirectIndexed32(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,MemRead32(v60.program,v60.reg[modVal2&0x1F] + OpRead32(v60.program,modAdd+2)) + v60.reg[modVal&0x1F]);
		break;
	case 1:
		amOut=MemRead16(v60.program,MemRead32(v60.program,v60.reg[modVal2&0x1F] + OpRead32(v60.program,modAdd+2)) + v60.reg[modVal&0x1F]*2);
		break;
	case 2:
		amOut=MemRead32(v60.program,MemRead32(v60.program,v60.reg[modVal2&0x1F] + OpRead32(v60.program,modAdd+2)) + v60.reg[modVal&0x1F]*4);
		break;
	}

	return 6;
}

static UINT32 bam1DisplacementIndirectIndexed32(void)
{
	bamOffset=v60.reg[modVal&0x1F];
	amOut=MemRead32(v60.program,MemRead32(v60.program,v60.reg[modVal2&0x1F] + OpRead32(v60.program,modAdd+2))+bamOffset/8);
	bamOffset&=7;
	return 6;
}

static UINT32 am1PCDisplacementIndirect8(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,MemRead32(v60.program,PC + (INT8)OpRead8(v60.program,modAdd+1)));
		break;
	case 1:
		amOut=MemRead16(v60.program,MemRead32(v60.program,PC + (INT8)OpRead8(v60.program,modAdd+1)));
		break;
	case 2:
		amOut=MemRead32(v60.program,MemRead32(v60.program,PC + (INT8)OpRead8(v60.program,modAdd+1)));
		break;
	}

	return 2;
}

static UINT32 bam1PCDisplacementIndirect8(void)
{
	bamOffset=0;
	amOut=MemRead32(v60.program,MemRead32(v60.program,PC + (INT8)OpRead8(v60.program,modAdd+1)));
	return 2;
}

static UINT32 am1PCDisplacementIndirect16(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,MemRead32(v60.program,PC + (INT16)OpRead16(v60.program,modAdd+1)));
		break;
	case 1:
		amOut=MemRead16(v60.program,MemRead32(v60.program,PC + (INT16)OpRead16(v60.program,modAdd+1)));
		break;
	case 2:
		amOut=MemRead32(v60.program,MemRead32(v60.program,PC + (INT16)OpRead16(v60.program,modAdd+1)));
		break;
	}

	return 3;
}

static UINT32 bam1PCDisplacementIndirect16(void)
{
	bamOffset=0;
	amOut=MemRead32(v60.program,MemRead32(v60.program,PC + (INT16)OpRead16(v60.program,modAdd+1)));
	return 3;
}

static UINT32 am1PCDisplacementIndirect32(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,MemRead32(v60.program,PC + OpRead32(v60.program,modAdd+1)));
		break;
	case 1:
		amOut=MemRead16(v60.program,MemRead32(v60.program,PC + OpRead32(v60.program,modAdd+1)));
		break;
	case 2:
		amOut=MemRead32(v60.program,MemRead32(v60.program,PC + OpRead32(v60.program,modAdd+1)));
		break;
	}

	return 5;
}

static UINT32 bam1PCDisplacementIndirect32(void)
{
	bamOffset=0;
	amOut=MemRead32(v60.program,MemRead32(v60.program,PC + OpRead32(v60.program,modAdd+1)));
	return 5;
}

static UINT32 am1PCDisplacementIndirectIndexed8(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,MemRead32(v60.program,PC + (INT8)OpRead8(v60.program,modAdd+2)) + v60.reg[modVal&0x1F]);
		break;
	case 1:
		amOut=MemRead16(v60.program,MemRead32(v60.program,PC + (INT8)OpRead8(v60.program,modAdd+2)) + v60.reg[modVal&0x1F]*2);
		break;
	case 2:
		amOut=MemRead32(v60.program,MemRead32(v60.program,PC + (INT8)OpRead8(v60.program,modAdd+2)) + v60.reg[modVal&0x1F]*4);
		break;
	}

	return 3;
}

static UINT32 bam1PCDisplacementIndirectIndexed8(void)
{
	bamOffset=v60.reg[modVal&0x1F];
	amOut=MemRead32(v60.program,MemRead32(v60.program,PC + (INT8)OpRead8(v60.program,modAdd+2))+bamOffset/8);
	bamOffset&=7;
	return 3;
}

static UINT32 am1PCDisplacementIndirectIndexed16(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,MemRead32(v60.program,PC + (INT16)OpRead16(v60.program,modAdd+2)) + v60.reg[modVal&0x1F]);
		break;
	case 1:
		amOut=MemRead16(v60.program,MemRead32(v60.program,PC + (INT16)OpRead16(v60.program,modAdd+2)) + v60.reg[modVal&0x1F]*2);
		break;
	case 2:
		amOut=MemRead32(v60.program,MemRead32(v60.program,PC + (INT16)OpRead16(v60.program,modAdd+2)) + v60.reg[modVal&0x1F]*4);
		break;
	}

	return 4;
}

static UINT32 bam1PCDisplacementIndirectIndexed16(void)
{
	bamOffset=v60.reg[modVal&0x1F];
	amOut=MemRead32(v60.program,MemRead32(v60.program,PC + (INT16)OpRead16(v60.program,modAdd+2))+bamOffset/8);
	bamOffset&=7;
	return 4;
}

static UINT32 am1PCDisplacementIndirectIndexed32(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,MemRead32(v60.program,PC + OpRead32(v60.program,modAdd+2)) + v60.reg[modVal&0x1F]);
		break;
	case 1:
		amOut=MemRead16(v60.program,MemRead32(v60.program,PC + OpRead32(v60.program,modAdd+2)) + v60.reg[modVal&0x1F]*2);
		break;
	case 2:
		amOut=MemRead32(v60.program,MemRead32(v60.program,PC + OpRead32(v60.program,modAdd+2)) + v60.reg[modVal&0x1F]*4);
		break;
	}

	return 6;
}

static UINT32 bam1PCDisplacementIndirectIndexed32(void)
{
	bamOffset=v60.reg[modVal&0x1F];
	amOut=MemRead32(v60.program,MemRead32(v60.program,PC + OpRead32(v60.program,modAdd+2))+bamOffset/8);
	bamOffset&=7;
	return 6;
}

static UINT32 am1DoubleDisplacement8(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,MemRead32(v60.program,v60.reg[modVal&0x1F] + (INT8)OpRead8(v60.program,modAdd+1)) + (INT8)OpRead8(v60.program,modAdd+2));
		break;

	case 1:
		amOut=MemRead16(v60.program,MemRead32(v60.program,v60.reg[modVal&0x1F] + (INT8)OpRead8(v60.program,modAdd+1)) + (INT8)OpRead8(v60.program,modAdd+2));
		break;

	case 2:
		amOut=MemRead32(v60.program,MemRead32(v60.program,v60.reg[modVal&0x1F] + (INT8)OpRead8(v60.program,modAdd+1)) + (INT8)OpRead8(v60.program,modAdd+2));
		break;
	}

	return 3;
}

static UINT32 bam1DoubleDisplacement8(void)
{
	bamOffset=OpRead8(v60.program,modAdd+2);
	amOut=MemRead32(v60.program,MemRead32(v60.program,v60.reg[modVal&0x1F] + (INT8)OpRead8(v60.program,modAdd+1))+bamOffset/8);
	bamOffset&=7;
	return 3;
}

static UINT32 am1DoubleDisplacement16(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,MemRead32(v60.program,v60.reg[modVal&0x1F] + (INT16)OpRead16(v60.program,modAdd+1)) + (INT16)OpRead16(v60.program,modAdd+3));
		break;

	case 1:
		amOut=MemRead16(v60.program,MemRead32(v60.program,v60.reg[modVal&0x1F] + (INT16)OpRead16(v60.program,modAdd+1)) + (INT16)OpRead16(v60.program,modAdd+3));
		break;

	case 2:
		amOut=MemRead32(v60.program,MemRead32(v60.program,v60.reg[modVal&0x1F] + (INT16)OpRead16(v60.program,modAdd+1)) + (INT16)OpRead16(v60.program,modAdd+3));
		break;
	}

	return 5;
}

static UINT32 bam1DoubleDisplacement16(void)
{
	bamOffset=OpRead16(v60.program,modAdd+3);
	amOut=MemRead32(v60.program,MemRead32(v60.program,v60.reg[modVal&0x1F] + (INT16)OpRead16(v60.program,modAdd+1))+bamOffset/8);
	bamOffset&=7;
	return 5;
}

static UINT32 am1DoubleDisplacement32(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,MemRead32(v60.program,v60.reg[modVal&0x1F] + OpRead32(v60.program,modAdd+1)) + OpRead32(v60.program,modAdd+5));
		break;

	case 1:
		amOut=MemRead16(v60.program,MemRead32(v60.program,v60.reg[modVal&0x1F] + OpRead32(v60.program,modAdd+1)) + OpRead32(v60.program,modAdd+5));
		break;

	case 2:
		amOut=MemRead32(v60.program,MemRead32(v60.program,v60.reg[modVal&0x1F] + OpRead32(v60.program,modAdd+1)) + OpRead32(v60.program,modAdd+5));
		break;
	}

	return 9;
}

static UINT32 bam1DoubleDisplacement32(void)
{
	bamOffset=OpRead32(v60.program,modAdd+5);
	amOut=MemRead32(v60.program,MemRead32(v60.program,v60.reg[modVal&0x1F] + OpRead32(v60.program,modAdd+1))+bamOffset/8);
	bamOffset&=7;
	return 9;
}

static UINT32 am1PCDoubleDisplacement8(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,MemRead32(v60.program,PC + (INT8)OpRead8(v60.program,modAdd+1)) + (INT8)OpRead8(v60.program,modAdd+2));
		break;

	case 1:
		amOut=MemRead16(v60.program,MemRead32(v60.program,PC + (INT8)OpRead8(v60.program,modAdd+1)) + (INT8)OpRead8(v60.program,modAdd+2));
		break;

	case 2:
		amOut=MemRead32(v60.program,MemRead32(v60.program,PC + (INT8)OpRead8(v60.program,modAdd+1)) + (INT8)OpRead8(v60.program,modAdd+2));
		break;
	}

	return 3;
}

static UINT32 bam1PCDoubleDisplacement8(void)
{
	bamOffset=OpRead8(v60.program,modAdd+2);
	amOut=MemRead32(v60.program,MemRead32(v60.program,PC + (INT8)OpRead8(v60.program,modAdd+1))+bamOffset/8);
	bamOffset&=7;
	return 3;
}

static UINT32 am1PCDoubleDisplacement16(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,MemRead32(v60.program,PC + (INT16)OpRead16(v60.program,modAdd+1)) + (INT16)OpRead16(v60.program,modAdd+3));
		break;

	case 1:
		amOut=MemRead16(v60.program,MemRead32(v60.program,PC + (INT16)OpRead16(v60.program,modAdd+1)) + (INT16)OpRead16(v60.program,modAdd+3));
		break;

	case 2:
		amOut=MemRead32(v60.program,MemRead32(v60.program,PC + (INT16)OpRead16(v60.program,modAdd+1)) + (INT16)OpRead16(v60.program,modAdd+3));
		break;
	}

	return 5;
}

static UINT32 bam1PCDoubleDisplacement16(void)
{
	bamOffset=OpRead16(v60.program,modAdd+3);
	amOut=MemRead32(v60.program,MemRead32(v60.program,PC + (INT16)OpRead16(v60.program,modAdd+1))+bamOffset/8);
	bamOffset&=7;
	return 5;
}


static UINT32 am1PCDoubleDisplacement32(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,MemRead32(v60.program,PC + OpRead32(v60.program,modAdd+1)) + OpRead32(v60.program,modAdd+5));
		break;

	case 1:
		amOut=MemRead16(v60.program,MemRead32(v60.program,PC + OpRead32(v60.program,modAdd+1)) + OpRead32(v60.program,modAdd+5));
		break;

	case 2:
		amOut=MemRead32(v60.program,MemRead32(v60.program,PC + OpRead32(v60.program,modAdd+1)) + OpRead32(v60.program,modAdd+5));
		break;
	}

	return 9;
}

static UINT32 bam1PCDoubleDisplacement32(void)
{
	bamOffset=OpRead32(v60.program,modAdd+5);
	amOut=MemRead32(v60.program,MemRead32(v60.program,PC + OpRead32(v60.program,modAdd+1))+bamOffset/8);
	bamOffset&=7;
	return 9;
}

static UINT32 am1DirectAddress(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,OpRead32(v60.program,modAdd+1));
		break;

	case 1:
		amOut=MemRead16(v60.program,OpRead32(v60.program,modAdd+1));
		break;

	case 2:
		amOut=MemRead32(v60.program,OpRead32(v60.program,modAdd+1));
		break;
	}

	return 5;
}

static UINT32 bam1DirectAddress(void)
{
	bamOffset=0;
	amOut=MemRead32(v60.program,OpRead32(v60.program,modAdd+1));
	return 5;
}

static UINT32 am1DirectAddressIndexed(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,OpRead32(v60.program,modAdd+2) + v60.reg[modVal&0x1F]);
		break;

	case 1:
		amOut=MemRead16(v60.program,OpRead32(v60.program,modAdd+2) + v60.reg[modVal&0x1F]*2);
		break;

	case 2:
		amOut=MemRead32(v60.program,OpRead32(v60.program,modAdd+2) + v60.reg[modVal&0x1F]*4);
		break;
	}

	return 6;
}

static UINT32 bam1DirectAddressIndexed(void)
{
	bamOffset=v60.reg[modVal&0x1F];
	amOut=MemRead32(v60.program,OpRead32(v60.program,modAdd+2)+bamOffset/8);
	bamOffset&=7;
	return 6;
}

static UINT32 am1DirectAddressDeferred(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,MemRead32(v60.program,OpRead32(v60.program,modAdd+1)));
		break;

	case 1:
		amOut=MemRead16(v60.program,MemRead32(v60.program,OpRead32(v60.program,modAdd+1)));
		break;

	case 2:
		amOut=MemRead32(v60.program,MemRead32(v60.program,OpRead32(v60.program,modAdd+1)));
		break;
	}

	return 5;
}

static UINT32 bam1DirectAddressDeferred(void)
{
	bamOffset=0;
	amOut=MemRead32(v60.program,MemRead32(v60.program,OpRead32(v60.program,modAdd+1)));
	return 5;
}

static UINT32 am1DirectAddressDeferredIndexed(void)
{
	switch (modDim)
	{
	case 0:
		amOut=MemRead8(v60.program,MemRead32(v60.program,OpRead32(v60.program,modAdd+2)) + v60.reg[modVal&0x1F]);
		break;

	case 1:
		amOut=MemRead16(v60.program,MemRead32(v60.program,OpRead32(v60.program,modAdd+2)) + v60.reg[modVal&0x1F]*2);
		break;

	case 2:
		amOut=MemRead32(v60.program,MemRead32(v60.program,OpRead32(v60.program,modAdd+2)) + v60.reg[modVal&0x1F]*4);
		break;
	}

	return 6;
}

static UINT32 bam1DirectAddressDeferredIndexed(void)
{
	bamOffset=v60.reg[modVal&0x1F];
	amOut=MemRead32(v60.program,MemRead32(v60.program,OpRead32(v60.program,modAdd+2))+bamOffset/8);
	bamOffset&=7;
	return 6;
}

static UINT32 am1Immediate(void)
{
	switch (modDim)
	{
	case 0:
		amOut=OpRead8(v60.program,modAdd+1);
		return 2;
		break;

	case 1:
		amOut=OpRead16(v60.program,modAdd+1);
		return 3;
		break;

	case 2:
		amOut=OpRead32(v60.program,modAdd+1);
		return 5;
		break;
	}

	// It should not be here!  Written to avoid warning
	assert(0);
	return 1;
}

static UINT32 am1ImmediateQuick(void)
{
	amOut=modVal&0xF;
	return 1;
}




// AM1 Tables (for ReadAM)
// ***********************

static UINT32 am1Error1(void)
{
	fatalerror("CPU - AM1 - 1 (PC=%06x)", PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 bam1Error1(void)
{
	fatalerror("CPU - BAM1 - 1 (PC=%06x)", PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 am1Error2(void)
{
	fatalerror("CPU - AM1 - 2 (PC=%06x)", PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 bam1Error2(void)
{
	fatalerror("CPU - BAM1 - 2 (PC=%06x)", PC);
	return 0; /* never reached, fatalerror won't return */
}

#ifdef UNUSED_FUNCTION
static UINT32 am1Error3(void)
{
	fatalerror("CPU - AM1 - 3 (PC=%06x)", PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 bam1Error3(void)
{
	fatalerror("CPU - BAM1 - 3 (PC=%06x)", PC);
	return 0; /* never reached, fatalerror won't return */
}
#endif

static UINT32 am1Error4(void)
{
	fatalerror("CPU - AM1 - 4 (PC=%06x)", PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 bam1Error4(void)
{
	fatalerror("CPU - BAM1 - 4 (PC=%06x)", PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 am1Error5(void)
{
	fatalerror("CPU - AM1 - 5 (PC=%06x)", PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 bam1Error5(void)
{
	fatalerror("CPU - BAM1 - 5 (PC=%06x)", PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 bam1Error6(void)
{
	fatalerror("CPU - BAM1 - 6 (PC=%06x)", PC);
	return 0; /* never reached, fatalerror won't return */
}

static UINT32 (*const AMTable1_G7a[16])(void) =
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

static UINT32 (*const BAMTable1_G7a[16])(void) =
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


static UINT32 am1Group7a(void)
{
	if (!(modVal2&0x10))
		return am1Error4();

	return AMTable1_G7a[modVal2&0xF]();
}

static UINT32 bam1Group7a(void)
{
	if (!(modVal2&0x10))
		return bam1Error4();

	return BAMTable1_G7a[modVal2&0xF]();
}

static UINT32 (*const AMTable1_G7[32])(void) =
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

static UINT32 (*const BAMTable1_G7[32])(void) =
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



static UINT32 (*const AMTable1_G6[8])(void) =
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

static UINT32 (*const BAMTable1_G6[8])(void) =
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


static UINT32 am1Group6(void)
{
	modVal2=OpRead8(v60.program,modAdd+1);
	return AMTable1_G6[modVal2>>5]();
}

static UINT32 bam1Group6(void)
{
	modVal2=OpRead8(v60.program,modAdd+1);
	return BAMTable1_G6[modVal2>>5]();
}


static UINT32 am1Group7(void)
{
	return AMTable1_G7[modVal&0x1F]();
}

static UINT32 bam1Group7(void)
{
	return BAMTable1_G7[modVal&0x1F]();
}

static UINT32 (*const AMTable1[2][8])(void) =
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


static UINT32 (*const BAMTable1[2][8])(void) =
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




