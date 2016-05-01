// license:BSD-3-Clause
// copyright-holders:Farfetch'd, R. Belmont
// AM2 Functions (for ReadAMAddress)
// *********************************

UINT32 v60_device::am2Register()
{
	m_amflag = 1;
	m_amout = m_modval & 0x1F;
	return 1;
}

UINT32 v60_device::am2RegisterIndirect()
{
	m_amflag = 0;
	m_amout = m_reg[m_modval & 0x1F];
	return 1;
}

UINT32 v60_device::bam2RegisterIndirect()
{
	m_amflag = 0;
	m_amout = m_reg[m_modval & 0x1F];
	m_bamoffset = 0;
	return 1;
}

UINT32 v60_device::am2RegisterIndirectIndexed()
{
	m_amflag = 0;

	switch (m_moddim)
	{
	case 0:
		m_amout = m_reg[m_modval2 & 0x1F] + m_reg[m_modval & 0x1F];
		break;
	case 1:
		m_amout = m_reg[m_modval2 & 0x1F] + m_reg[m_modval & 0x1F] * 2;
		break;
	case 2:
		m_amout = m_reg[m_modval2 & 0x1F] + m_reg[m_modval & 0x1F] * 4;
		break;
	case 3:
		m_amout = m_reg[m_modval2 & 0x1F] + m_reg[m_modval & 0x1F] * 8;
		break;
	}

	return 2;
}

UINT32 v60_device::bam2RegisterIndirectIndexed()
{
	m_amflag = 0;
	m_amout = m_reg[m_modval2 & 0x1F];
	m_bamoffset = m_reg[m_modval & 0x1F];
	return 2;
}

UINT32 v60_device::am2Autoincrement()
{
	m_amflag = 0;
	m_amout = m_reg[m_modval & 0x1F];

	switch (m_moddim)
	{
	case 0:
		m_reg[m_modval & 0x1F] += 1;
		break;
	case 1:
		m_reg[m_modval & 0x1F] += 2;
		break;
	case 2:
		m_reg[m_modval & 0x1F] += 4;
		break;
	case 3:
		m_reg[m_modval & 0x1F] += 8;
		break;
	}

	return 1;
}

UINT32 v60_device::bam2Autoincrement()
{
	m_amflag = 0;
	m_amout = m_reg[m_modval & 0x1F];
	m_bamoffset = 0;

	switch (m_moddim)
	{
	case 10:
		m_reg[m_modval & 0x1F] +=1;
		break;
	case 11:
		m_reg[m_modval & 0x1F] +=4;
		break;
	default:
		fatalerror("CPU - AM2 - 7 (t0 PC=%x)\n", PC);
		break;
	}

	return 1;
}

UINT32 v60_device::am2Autodecrement()
{
	m_amflag = 0;

	switch (m_moddim)
	{
	case 0:
		m_reg[m_modval & 0x1F] -= 1;
		break;
	case 1:
		m_reg[m_modval & 0x1F] -= 2;
		break;
	case 2:
		m_reg[m_modval & 0x1F] -= 4;
		break;
	case 3:
		m_reg[m_modval & 0x1F] -= 8;
		break;
	}

	m_amout = m_reg[m_modval & 0x1F];
	return 1;
}

UINT32 v60_device::bam2Autodecrement()
{
	m_amflag = 0;
	m_bamoffset = 0;

	switch (m_moddim)
	{
	case 10:
		m_reg[m_modval & 0x1F]-=1;
		break;
	case 11:
		m_reg[m_modval & 0x1F]-=4;
		break;
	default:
		fatalerror("CPU - BAM2 - 7 (PC=%06x)\n", PC);
		break;
	}

	m_amout = m_reg[m_modval & 0x1F];
	return 1;
}


UINT32 v60_device::am2Displacement8()
{
	m_amflag = 0;
	m_amout = m_reg[m_modval & 0x1F] + (INT8)OpRead8(m_modadd + 1);

	return 2;
}

UINT32 v60_device::bam2Displacement8()
{
	m_amflag = 0;
	m_amout = m_reg[m_modval & 0x1F];
	m_bamoffset = (INT8)OpRead8(m_modadd + 1);

	return 2;
}

UINT32 v60_device::am2Displacement16()
{
	m_amflag = 0;
	m_amout = m_reg[m_modval & 0x1F] + (INT16)OpRead16(m_modadd + 1);

	return 3;
}

UINT32 v60_device::bam2Displacement16()
{
	m_amflag = 0;
	m_amout = m_reg[m_modval & 0x1F];
	m_bamoffset = (INT16)OpRead16(m_modadd + 1);

	return 3;
}

UINT32 v60_device::am2Displacement32()
{
	m_amflag = 0;
	m_amout = m_reg[m_modval & 0x1F] + OpRead32(m_modadd + 1);

	return 5;
}

UINT32 v60_device::bam2Displacement32()
{
	m_amflag = 0;
	m_amout = m_reg[m_modval & 0x1F];
	m_bamoffset = OpRead32(m_modadd + 1);

	return 5;
}

UINT32 v60_device::am2DisplacementIndexed8()
{
	m_amflag = 0;

	switch (m_moddim)
	{
	case 0:
		m_amout = m_reg[m_modval2 & 0x1F] + (INT8)OpRead8(m_modadd + 2) + m_reg[m_modval & 0x1F];
		break;
	case 1:
		m_amout = m_reg[m_modval2 & 0x1F] + (INT8)OpRead8(m_modadd + 2) + m_reg[m_modval & 0x1F] * 2;
		break;
	case 2:
		m_amout = m_reg[m_modval2 & 0x1F] + (INT8)OpRead8(m_modadd + 2) + m_reg[m_modval & 0x1F] * 4;
		break;
	case 3:
		m_amout = m_reg[m_modval2 & 0x1F] + (INT8)OpRead8(m_modadd + 2) + m_reg[m_modval & 0x1F] * 8;
		break;
	}

	return 3;
}

UINT32 v60_device::bam2DisplacementIndexed8()
{
	m_amflag = 0;
	m_amout = m_reg[m_modval2 & 0x1F] + (INT8)OpRead8(m_modadd + 2);
	m_bamoffset = m_reg[m_modval & 0x1F];

	return 3;
}

UINT32 v60_device::am2DisplacementIndexed16()
{
	m_amflag = 0;

	switch (m_moddim)
	{
	case 0:
		m_amout = m_reg[m_modval2 & 0x1F] + (INT16)OpRead16(m_modadd + 2) + m_reg[m_modval & 0x1F];
		break;
	case 1:
		m_amout = m_reg[m_modval2 & 0x1F] + (INT16)OpRead16(m_modadd + 2) + m_reg[m_modval & 0x1F] * 2;
		break;
	case 2:
		m_amout = m_reg[m_modval2 & 0x1F] + (INT16)OpRead16(m_modadd + 2) + m_reg[m_modval & 0x1F] * 4;
		break;
	case 3:
		m_amout = m_reg[m_modval2 & 0x1F] + (INT16)OpRead16(m_modadd + 2) + m_reg[m_modval & 0x1F] * 8;
		break;
	}

	return 4;
}

UINT32 v60_device::bam2DisplacementIndexed16()
{
	m_amflag = 0;
	m_amout = m_reg[m_modval2 & 0x1F] + (INT16)OpRead16(m_modadd + 2);
	m_bamoffset = m_reg[m_modval & 0x1F];

	return 4;
}

UINT32 v60_device::am2DisplacementIndexed32()
{
	m_amflag = 0;

	switch (m_moddim)
	{
	case 0:
		m_amout = m_reg[m_modval2 & 0x1F] + OpRead32(m_modadd + 2) + m_reg[m_modval & 0x1F];
		break;
	case 1:
		m_amout = m_reg[m_modval2 & 0x1F] + OpRead32(m_modadd + 2) + m_reg[m_modval & 0x1F] * 2;
		break;
	case 2:
		m_amout = m_reg[m_modval2 & 0x1F] + OpRead32(m_modadd + 2) + m_reg[m_modval & 0x1F] * 4;
		break;
	case 3:
		m_amout = m_reg[m_modval2 & 0x1F] + OpRead32(m_modadd + 2) + m_reg[m_modval & 0x1F] * 8;
		break;
	}

	return 6;
}

UINT32 v60_device::bam2DisplacementIndexed32()
{
	m_amflag = 0;
	m_amout = m_reg[m_modval2 & 0x1F] + OpRead32(m_modadd + 2);
	m_bamoffset = m_reg[m_modval & 0x1F];

	return 6;
}

UINT32 v60_device::am2PCDisplacement8()
{
	m_amflag = 0;
	m_amout = PC + (INT8)OpRead8(m_modadd + 1);

	return 2;
}

UINT32 v60_device::bam2PCDisplacement8()
{
	m_amflag = 0;
	m_amout = PC;
	m_bamoffset = (INT8)OpRead8(m_modadd + 1);

	return 2;
}

UINT32 v60_device::am2PCDisplacement16()
{
	m_amflag = 0;
	m_amout = PC + (INT16)OpRead16(m_modadd + 1);

	return 3;
}

UINT32 v60_device::bam2PCDisplacement16()
{
	m_amflag = 0;
	m_amout = PC;
	m_bamoffset = (INT16)OpRead16(m_modadd + 1);

	return 3;
}

UINT32 v60_device::am2PCDisplacement32()
{
	m_amflag = 0;
	m_amout = PC + OpRead32(m_modadd + 1);

	return 5;
}

UINT32 v60_device::bam2PCDisplacement32()
{
	m_amflag = 0;
	m_amout = PC;
	m_bamoffset = OpRead32(m_modadd + 1);

	return 5;
}


UINT32 v60_device::am2PCDisplacementIndexed8()
{
	m_amflag = 0;

	switch (m_moddim)
	{
	case 0:
		m_amout = PC + (INT8)OpRead8(m_modadd + 2) + m_reg[m_modval & 0x1F];
		break;
	case 1:
		m_amout = PC + (INT8)OpRead8(m_modadd + 2) + m_reg[m_modval & 0x1F] * 2;
		break;
	case 2:
		m_amout = PC + (INT8)OpRead8(m_modadd + 2) + m_reg[m_modval & 0x1F] * 4;
		break;
	case 3:
		m_amout = PC + (INT8)OpRead8(m_modadd + 2) + m_reg[m_modval & 0x1F] * 8;
		break;
	}

	return 3;
}

UINT32 v60_device::bam2PCDisplacementIndexed8()
{
	m_amflag = 0;
	m_amout = PC + (INT8)OpRead8(m_modadd + 2);
	m_bamoffset = m_reg[m_modval & 0x1F];

	return 3;
}

UINT32 v60_device::am2PCDisplacementIndexed16()
{
	m_amflag = 0;

	switch (m_moddim)
	{
	case 0:
		m_amout = PC + (INT16)OpRead16(m_modadd + 2) + m_reg[m_modval & 0x1F];
		break;
	case 1:
		m_amout = PC + (INT16)OpRead16(m_modadd + 2) + m_reg[m_modval & 0x1F] * 2;
		break;
	case 2:
		m_amout = PC + (INT16)OpRead16(m_modadd + 2) + m_reg[m_modval & 0x1F] * 4;
		break;
	case 3:
		m_amout = PC + (INT16)OpRead16(m_modadd + 2) + m_reg[m_modval & 0x1F] * 8;
		break;
	}

	return 4;
}

UINT32 v60_device::bam2PCDisplacementIndexed16()
{
	m_amflag = 0;
	m_amout = PC + (INT16)OpRead16(m_modadd + 2);
	m_bamoffset = m_reg[m_modval & 0x1F];

	return 4;
}

UINT32 v60_device::am2PCDisplacementIndexed32()
{
	m_amflag = 0;

	switch (m_moddim)
	{
	case 0:
		m_amout = PC + OpRead32(m_modadd + 2) + m_reg[m_modval & 0x1F];
		break;
	case 1:
		m_amout = PC + OpRead32(m_modadd + 2) + m_reg[m_modval & 0x1F] * 2;
		break;
	case 2:
		m_amout = PC + OpRead32(m_modadd + 2) + m_reg[m_modval & 0x1F] * 4;
		break;
	case 3:
		m_amout = PC + OpRead32(m_modadd + 2) + m_reg[m_modval & 0x1F] * 8;
		break;
	}

	return 6;
}

UINT32 v60_device::bam2PCDisplacementIndexed32()
{
	m_amflag = 0;
	m_amout = PC + OpRead32(m_modadd + 2);
	m_bamoffset = m_reg[m_modval & 0x1F];

	return 6;
}

UINT32 v60_device::am2DisplacementIndirect8()
{
	m_amflag = 0;
	m_amout = m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT8)OpRead8(m_modadd + 1));

	return 2;
}

UINT32 v60_device::bam2DisplacementIndirect8()
{
	m_amflag = 0;
	m_amout = m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT8)OpRead8(m_modadd + 1));
	m_bamoffset = 0;
	return 2;
}

UINT32 v60_device::am2DisplacementIndirect16()
{
	m_amflag = 0;
	m_amout = m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT16)OpRead16(m_modadd + 1));

	return 3;
}

UINT32 v60_device::bam2DisplacementIndirect16()
{
	m_amflag = 0;
	m_amout = m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT16)OpRead16(m_modadd + 1));
	m_bamoffset = 0;
	return 3;
}

UINT32 v60_device::am2DisplacementIndirect32()
{
	m_amflag = 0;
	m_amout = m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + OpRead32(m_modadd + 1));

	return 5;
}

UINT32 v60_device::bam2DisplacementIndirect32()
{
	m_amflag = 0;
	m_amout = m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + OpRead32(m_modadd + 1));
	m_bamoffset = 0;

	return 5;
}

UINT32 v60_device::am2DisplacementIndirectIndexed8()
{
	m_amflag = 0;

	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + (INT8)OpRead8(m_modadd + 2)) + m_reg[m_modval & 0x1F];
		break;
	case 1:
		m_amout = m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + (INT8)OpRead8(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 2;
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + (INT8)OpRead8(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 4;
		break;
	case 3:
		m_amout = m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + (INT8)OpRead8(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 8;
		break;
	}

	return 3;
}

UINT32 v60_device::bam2DisplacementIndirectIndexed8()
{
	m_amflag = 0;
	m_amout = m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + (INT8)OpRead8(m_modadd + 2));
	m_bamoffset = m_reg[m_modval & 0x1F];

	return 3;
}

UINT32 v60_device::am2DisplacementIndirectIndexed16()
{
	m_amflag = 0;

	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + (INT16)OpRead16(m_modadd + 2)) + m_reg[m_modval & 0x1F];
		break;
	case 1:
		m_amout = m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + (INT16)OpRead16(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 2;
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + (INT16)OpRead16(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 4;
		break;
	case 3:
		m_amout = m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + (INT16)OpRead16(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 8;
		break;
	}

	return 4;
}

UINT32 v60_device::bam2DisplacementIndirectIndexed16()
{
	m_amflag = 0;
	m_amout = m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + (INT16)OpRead16(m_modadd + 2));
	m_bamoffset = m_reg[m_modval & 0x1F];

	return 4;
}

UINT32 v60_device::am2DisplacementIndirectIndexed32()
{
	m_amflag = 0;

	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + OpRead32(m_modadd + 2)) + m_reg[m_modval & 0x1F];
		break;
	case 1:
		m_amout = m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + OpRead32(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 2;
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + OpRead32(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 4;
		break;
	case 3:
		m_amout = m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + OpRead32(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 8;
		break;
	}

	return 6;
}

UINT32 v60_device::bam2DisplacementIndirectIndexed32()
{
	m_amflag = 0;
	m_amout = m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + OpRead32(m_modadd + 2));
	m_bamoffset = m_reg[m_modval & 0x1F];

	return 6;
}

UINT32 v60_device::am2PCDisplacementIndirect8()
{
	m_amflag = 0;
	m_amout = m_program->read_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 1));

	return 2;
}

UINT32 v60_device::bam2PCDisplacementIndirect8()
{
	m_amflag = 0;
	m_amout = m_program->read_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 1));
	m_bamoffset = 0;

	return 2;
}

UINT32 v60_device::am2PCDisplacementIndirect16()
{
	m_amflag = 0;
	m_amout = m_program->read_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 1));

	return 3;
}

UINT32 v60_device::bam2PCDisplacementIndirect16()
{
	m_amflag = 0;
	m_amout = m_program->read_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 1));
	m_bamoffset = 0;

	return 3;
}

UINT32 v60_device::am2PCDisplacementIndirect32()
{
	m_amflag = 0;
	m_amout = m_program->read_dword_unaligned(PC + OpRead32(m_modadd + 1));

	return 5;
}

UINT32 v60_device::bam2PCDisplacementIndirect32()
{
	m_amflag = 0;
	m_amout = m_program->read_dword_unaligned(PC + OpRead32(m_modadd + 1));
	m_bamoffset = 0;

	return 5;
}

UINT32 v60_device::am2PCDisplacementIndirectIndexed8()
{
	m_amflag = 0;

	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 2)) + m_reg[m_modval & 0x1F];
		break;
	case 1:
		m_amout = m_program->read_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 2;
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 4;
		break;
	case 3:
		m_amout = m_program->read_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 8;
		break;
	}

	return 3;
}

UINT32 v60_device::bam2PCDisplacementIndirectIndexed8()
{
	m_amflag = 0;
	m_amout = m_program->read_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 2));
	m_bamoffset = m_reg[m_modval & 0x1F];

	return 3;
}

UINT32 v60_device::am2PCDisplacementIndirectIndexed16()
{
	m_amflag = 0;

	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 2)) + m_reg[m_modval & 0x1F];
		break;
	case 1:
		m_amout = m_program->read_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 2;
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 4;
		break;
	case 3:
		m_amout = m_program->read_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 8;
		break;
	}

	return 4;
}


UINT32 v60_device::bam2PCDisplacementIndirectIndexed16()
{
	m_amflag = 0;
	m_amout = m_program->read_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 2));
	m_bamoffset = m_reg[m_modval & 0x1F];

	return 4;
}


UINT32 v60_device::am2PCDisplacementIndirectIndexed32()
{
	m_amflag = 0;

	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_dword_unaligned(PC + OpRead32(m_modadd + 2)) + m_reg[m_modval & 0x1F];
		break;
	case 1:
		m_amout = m_program->read_dword_unaligned(PC + OpRead32(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 2;
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(PC + OpRead32(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 4;
		break;
	case 3:
		m_amout = m_program->read_dword_unaligned(PC + OpRead32(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 8;
		break;
	}

	return 6;
}

UINT32 v60_device::bam2PCDisplacementIndirectIndexed32()
{
	m_amflag = 0;
	m_amout = m_program->read_dword_unaligned(PC + OpRead32(m_modadd + 2));
	m_bamoffset = m_reg[m_modval & 0x1F];

	return 6;
}

UINT32 v60_device::am2DoubleDisplacement8()
{
	m_amflag = 0;
	m_amout = m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT8)OpRead8(m_modadd + 1)) + (INT8)OpRead8(m_modadd + 2);

	return 3;
}

UINT32 v60_device::bam2DoubleDisplacement8()
{
	m_amflag = 0;
	m_amout = m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT8)OpRead8(m_modadd + 1));
	m_bamoffset = (INT8)OpRead8(m_modadd + 2);

	return 3;
}

UINT32 v60_device::am2DoubleDisplacement16()
{
	m_amflag = 0;
	m_amout = m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT16)OpRead16(m_modadd + 1)) + (INT16)OpRead16(m_modadd + 3);

	return 5;
}

UINT32 v60_device::bam2DoubleDisplacement16()
{
	m_amflag = 0;
	m_amout = m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT16)OpRead16(m_modadd + 1));
	m_bamoffset = (INT8)OpRead8(m_modadd + 3);

	return 5;
}

UINT32 v60_device::am2DoubleDisplacement32()
{
	m_amflag = 0;
	m_amout = m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + OpRead32(m_modadd + 1)) + OpRead32(m_modadd + 5);

	return 9;
}

UINT32 v60_device::bam2DoubleDisplacement32()
{
	m_amflag = 0;
	m_amout = m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + OpRead32(m_modadd + 1));
	m_bamoffset = OpRead32(m_modadd + 5);

	return 9;
}


UINT32 v60_device::am2PCDoubleDisplacement8()
{
	m_amflag = 0;
	m_amout = m_program->read_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 1)) + (INT8)OpRead8(m_modadd + 2);

	return 3;
}

UINT32 v60_device::bam2PCDoubleDisplacement8()
{
	m_amflag = 0;
	m_amout = m_program->read_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 1));
	m_bamoffset = (INT8)OpRead8(m_modadd + 2);

	return 3;
}

UINT32 v60_device::am2PCDoubleDisplacement16()
{
	m_amflag = 0;
	m_amout = m_program->read_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 1)) + (INT16)OpRead16(m_modadd + 3);

	return 5;
}

UINT32 v60_device::bam2PCDoubleDisplacement16()
{
	m_amflag = 0;
	m_amout = m_program->read_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 1));
	m_bamoffset = (INT8)OpRead8(m_modadd + 3);

	return 5;
}

UINT32 v60_device::am2PCDoubleDisplacement32()
{
	m_amflag = 0;
	m_amout = m_program->read_dword_unaligned(PC + OpRead32(m_modadd + 1)) + OpRead32(m_modadd + 5);

	return 9;
}

UINT32 v60_device::bam2PCDoubleDisplacement32()
{
	m_amflag = 0;
	m_amout = m_program->read_dword_unaligned(PC + OpRead32(m_modadd + 1));
	m_bamoffset = OpRead32(m_modadd + 5);

	return 9;
}

UINT32 v60_device::am2DirectAddress()
{
	m_amflag = 0;
	m_amout = OpRead32(m_modadd + 1);

	return 5;
}

UINT32 v60_device::bam2DirectAddress()
{
	m_amflag = 0;
	m_amout = OpRead32(m_modadd + 1);
	m_bamoffset = 0;

	return 5;
}

UINT32 v60_device::am2DirectAddressIndexed()
{
	m_amflag = 0;

	switch (m_moddim)
	{
	case 0:
		m_amout = OpRead32(m_modadd + 2) + m_reg[m_modval & 0x1F];
		break;
	case 1:
		m_amout = OpRead32(m_modadd + 2) + m_reg[m_modval & 0x1F] * 2;
		break;
	case 2:
		m_amout = OpRead32(m_modadd + 2) + m_reg[m_modval & 0x1F] * 4;
		break;
	case 3:
		m_amout = OpRead32(m_modadd + 2) + m_reg[m_modval & 0x1F] * 8;
		break;
	}

	return 6;
}

UINT32 v60_device::bam2DirectAddressIndexed()
{
	m_amflag = 0;
	m_amout = OpRead32(m_modadd + 2);
	m_bamoffset = m_reg[m_modval & 0x1F];

	return 6;
}

UINT32 v60_device::am2DirectAddressDeferred()
{
	m_amflag = 0;
	m_amout = m_program->read_dword_unaligned(OpRead32(m_modadd + 1));

	return 5;
}

UINT32 v60_device::bam2DirectAddressDeferred()
{
	m_amflag = 0;
	m_amout = m_program->read_dword_unaligned(OpRead32(m_modadd + 1));
	m_bamoffset = 0;

	return 5;
}

UINT32 v60_device::am2DirectAddressDeferredIndexed()
{
	m_amflag = 0;

	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_dword_unaligned(OpRead32(m_modadd + 2)) + m_reg[m_modval & 0x1F];
		break;
	case 1:
		m_amout = m_program->read_dword_unaligned(OpRead32(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 2;
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(OpRead32(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 4;
		break;
	case 3:
		m_amout = m_program->read_dword_unaligned(OpRead32(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 8;
		break;
	}

	return 6;
}

UINT32 v60_device::bam2DirectAddressDeferredIndexed()
{
	m_amflag = 0;
	m_amout = m_program->read_dword_unaligned(OpRead32(m_modadd + 2));
	m_bamoffset = m_reg[m_modval & 0x1F];

	return 6;
}

UINT32 v60_device::am2Immediate()
{
	// ignore LDPR
	return am1Immediate();
}

UINT32 v60_device::am2ImmediateQuick()
{
	// ignore LDPR
	return am1ImmediateQuick();
}


// AM2 Tables (for ReadAMAddress)
// ******************************

UINT32 v60_device::am2Error1()
{
	// f1lap trips this, why?
	logerror("CPU - AM2 - 1 (PC=%06x)", PC);
	return 0;
}

UINT32 v60_device::am2Error2()
{
	fatalerror("CPU - AM2 - 2 (PC=%06x)\n", PC);
	return 0; /* never reached, fatalerror won't return */
}

#ifdef UNUSED_FUNCTION
UINT32 v60_device::am2Error3()
{
	fatalerror("CPU - AM2 - 3 (PC=%06x)\n", PC);
	return 0; /* never reached, fatalerror won't return */
}
#endif

UINT32 v60_device::am2Error4()
{
	fatalerror("CPU - AM2 - 4 (PC=%06x)\n", PC);
	return 0; /* never reached, fatalerror won't return */
}

UINT32 v60_device::am2Error5()
{
	fatalerror("CPU - AM2 - 5 (PC=%06x)\n", PC);
	return 0; /* never reached, fatalerror won't return */
}

UINT32 v60_device::bam2Error1()
{
	fatalerror("CPU - BAM2 - 1 (PC=%06x)\n", PC);
	return 0; /* never reached, fatalerror won't return */
}

UINT32 v60_device::bam2Error2()
{
	fatalerror("CPU - BAM2 - 2 (PC=%06x)\n", PC);
	return 0; /* never reached, fatalerror won't return */
}

#ifdef UNUSED_FUNCTION
UINT32 v60_device::bam2Error3()
{
	fatalerror("CPU - BAM2 - 3 (PC=%06x)\n", PC);
	return 0; /* never reached, fatalerror won't return */
}
#endif

UINT32 v60_device::bam2Error4()
{
	fatalerror("CPU - BAM2 - 4 (PC=%06x)\n", PC);
	return 0; /* never reached, fatalerror won't return */
}

UINT32 v60_device::bam2Error5()
{
	fatalerror("CPU - BAM2 - 5 (PC=%06x)\n", PC);
	return 0; /* never reached, fatalerror won't return */
}

UINT32 v60_device::bam2Error6()
{
	fatalerror("CPU - BAM2 - 6 (PC=%06x)\n", PC);
	return 0; /* never reached, fatalerror won't return */
}


const v60_device::am_func v60_device::s_AMTable2_G7a[16] =
{
	&v60_device::am2PCDisplacementIndexed8,
	&v60_device::am2PCDisplacementIndexed16,
	&v60_device::am2PCDisplacementIndexed32,
	&v60_device::am2DirectAddressIndexed,
	&v60_device::am2Error5,
	&v60_device::am2Error5,
	&v60_device::am2Error5,
	&v60_device::am2Error5,
	&v60_device::am2PCDisplacementIndirectIndexed8,
	&v60_device::am2PCDisplacementIndirectIndexed16,
	&v60_device::am2PCDisplacementIndirectIndexed32,
	&v60_device::am2DirectAddressDeferredIndexed,
	&v60_device::am2Error5,
	&v60_device::am2Error5,
	&v60_device::am2Error5,
	&v60_device::am2Error5
};

const v60_device::am_func v60_device::s_BAMTable2_G7a[16] =
{
	&v60_device::bam2PCDisplacementIndexed8,
	&v60_device::bam2PCDisplacementIndexed16,
	&v60_device::bam2PCDisplacementIndexed32,
	&v60_device::bam2DirectAddressIndexed,
	&v60_device::bam2Error5,
	&v60_device::bam2Error5,
	&v60_device::bam2Error5,
	&v60_device::bam2Error5,
	&v60_device::bam2PCDisplacementIndirectIndexed8,
	&v60_device::bam2PCDisplacementIndirectIndexed16,
	&v60_device::bam2PCDisplacementIndirectIndexed32,
	&v60_device::bam2DirectAddressDeferredIndexed,
	&v60_device::bam2Error5,
	&v60_device::bam2Error5,
	&v60_device::bam2Error5,
	&v60_device::bam2Error5
};

UINT32 v60_device::am2Group7a()
{
	if (!(m_modval2 & 0x10))
		return am2Error4();

	return (this->*s_AMTable2_G7a[m_modval2 & 0xF])();
}

UINT32 v60_device::bam2Group7a()
{
	if (!(m_modval2 & 0x10))
		return bam2Error4();

	return (this->*s_BAMTable2_G7a[m_modval2 & 0xF])();
}

const v60_device::am_func v60_device::s_AMTable2_G7[32] =
{
	&v60_device::am2ImmediateQuick,
	&v60_device::am2ImmediateQuick,
	&v60_device::am2ImmediateQuick,
	&v60_device::am2ImmediateQuick,
	&v60_device::am2ImmediateQuick,
	&v60_device::am2ImmediateQuick,
	&v60_device::am2ImmediateQuick,
	&v60_device::am2ImmediateQuick,
	&v60_device::am2ImmediateQuick,
	&v60_device::am2ImmediateQuick,
	&v60_device::am2ImmediateQuick,
	&v60_device::am2ImmediateQuick,
	&v60_device::am2ImmediateQuick,
	&v60_device::am2ImmediateQuick,
	&v60_device::am2ImmediateQuick,
	&v60_device::am2ImmediateQuick,
	&v60_device::am2PCDisplacement8,
	&v60_device::am2PCDisplacement16,
	&v60_device::am2PCDisplacement32,
	&v60_device::am2DirectAddress,
	&v60_device::am2Immediate,
	&v60_device::am2Error2,
	&v60_device::am2Error2,
	&v60_device::am2Error2,
	&v60_device::am2PCDisplacementIndirect8,
	&v60_device::am2PCDisplacementIndirect16,
	&v60_device::am2PCDisplacementIndirect32,
	&v60_device::am2DirectAddressDeferred,
	&v60_device::am2PCDoubleDisplacement8,
	&v60_device::am2PCDoubleDisplacement16,
	&v60_device::am2PCDoubleDisplacement32,
	&v60_device::am2Error2
};

const v60_device::am_func v60_device::s_BAMTable2_G7[32] =
{
	&v60_device::bam2Error6,
	&v60_device::bam2Error6,
	&v60_device::bam2Error6,
	&v60_device::bam2Error6,
	&v60_device::bam2Error6,
	&v60_device::bam2Error6,
	&v60_device::bam2Error6,
	&v60_device::bam2Error6,
	&v60_device::bam2Error6,
	&v60_device::bam2Error6,
	&v60_device::bam2Error6,
	&v60_device::bam2Error6,
	&v60_device::bam2Error6,
	&v60_device::bam2Error6,
	&v60_device::bam2Error6,
	&v60_device::bam2Error6,
	&v60_device::bam2PCDisplacement8,
	&v60_device::bam2PCDisplacement16,
	&v60_device::bam2PCDisplacement32,
	&v60_device::bam2DirectAddress,
	&v60_device::bam2Error6,
	&v60_device::bam2Error2,
	&v60_device::bam2Error2,
	&v60_device::bam2Error2,
	&v60_device::bam2PCDisplacementIndirect8,
	&v60_device::bam2PCDisplacementIndirect16,
	&v60_device::bam2PCDisplacementIndirect32,
	&v60_device::bam2DirectAddressDeferred,
	&v60_device::bam2PCDoubleDisplacement8,
	&v60_device::bam2PCDoubleDisplacement16,
	&v60_device::bam2PCDoubleDisplacement32,
	&v60_device::bam2Error2
};

const v60_device::am_func v60_device::s_AMTable2_G6[8] =
{
	&v60_device::am2DisplacementIndexed8,
	&v60_device::am2DisplacementIndexed16,
	&v60_device::am2DisplacementIndexed32,
	&v60_device::am2RegisterIndirectIndexed,
	&v60_device::am2DisplacementIndirectIndexed8,
	&v60_device::am2DisplacementIndirectIndexed16,
	&v60_device::am2DisplacementIndirectIndexed32,
	&v60_device::am2Group7a
};

const v60_device::am_func v60_device::s_BAMTable2_G6[8] =
{
	&v60_device::bam2DisplacementIndexed8,
	&v60_device::bam2DisplacementIndexed16,
	&v60_device::bam2DisplacementIndexed32,
	&v60_device::bam2RegisterIndirectIndexed,
	&v60_device::bam2DisplacementIndirectIndexed8,
	&v60_device::bam2DisplacementIndirectIndexed16,
	&v60_device::bam2DisplacementIndirectIndexed32,
	&v60_device::bam2Group7a
};




UINT32 v60_device::am2Group6()
{
	m_modval2 = OpRead8(m_modadd + 1);
	return (this->*s_AMTable2_G6[m_modval2 >> 5])();
}
UINT32 v60_device::bam2Group6()
{
	m_modval2 = OpRead8(m_modadd + 1);
	return (this->*s_BAMTable2_G6[m_modval2 >> 5])();
}

UINT32 v60_device::am2Group7()
{
	return (this->*s_AMTable2_G7[m_modval & 0x1F])();
}
UINT32 v60_device::bam2Group7()
{
	return (this->*s_BAMTable2_G7[m_modval & 0x1F])();
}


const v60_device::am_func v60_device::s_AMTable2[2][8] =
{
	{
		&v60_device::am2Displacement8,
		&v60_device::am2Displacement16,
		&v60_device::am2Displacement32,
		&v60_device::am2RegisterIndirect,
		&v60_device::am2DisplacementIndirect8,
		&v60_device::am2DisplacementIndirect16,
		&v60_device::am2DisplacementIndirect32,
		&v60_device::am2Group7
	},

	{
		&v60_device::am2DoubleDisplacement8,
		&v60_device::am2DoubleDisplacement16,
		&v60_device::am2DoubleDisplacement32,
		&v60_device::am2Register,
		&v60_device::am2Autoincrement,
		&v60_device::am2Autodecrement,
		&v60_device::am2Group6,
		&v60_device::am2Error1
	}
};

const v60_device::am_func v60_device::s_BAMTable2[2][8] =
{
	{
		&v60_device::bam2Displacement8,
		&v60_device::bam2Displacement16,
		&v60_device::bam2Displacement32,
		&v60_device::bam2RegisterIndirect,
		&v60_device::bam2DisplacementIndirect8,
		&v60_device::bam2DisplacementIndirect16,
		&v60_device::bam2DisplacementIndirect32,
		&v60_device::bam2Group7
	},

	{
		&v60_device::bam2DoubleDisplacement8,
		&v60_device::bam2DoubleDisplacement16,
		&v60_device::bam2DoubleDisplacement32,
		&v60_device::bam2Error6,
		&v60_device::bam2Autoincrement,
		&v60_device::bam2Autodecrement,
		&v60_device::bam2Group6,
		&v60_device::bam2Error1
	}
};
