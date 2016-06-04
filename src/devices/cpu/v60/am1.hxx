// license:BSD-3-Clause
// copyright-holders:Farfetch'd, R. Belmont
// AM1 Functions (for ReadAM)
// **************************

UINT32 v60_device::am1Register()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = (UINT8)m_reg[m_modval & 0x1F];
		break;
	case 1:
		m_amout = (UINT16)m_reg[m_modval & 0x1F];
		break;
	case 2:
		m_amout = m_reg[m_modval & 0x1F];
		break;
	}

	return 1;
}

UINT32 v60_device::am1RegisterIndirect()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(m_reg[m_modval & 0x1F]);
		break;
	case 1:
		m_amout = m_program->read_word_unaligned(m_reg[m_modval & 0x1F]);
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(m_reg[m_modval & 0x1F]);
		break;
	}

	return 1;
}

UINT32 v60_device::bam1RegisterIndirect()
{
	m_bamoffset = 0;
	m_amout = m_program->read_dword_unaligned(m_reg[m_modval & 0x1F]);
	return 1;
}

UINT32 v60_device::am1RegisterIndirectIndexed()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(m_reg[m_modval2 & 0x1F] + m_reg[m_modval & 0x1F]);
		break;
	case 1:
		m_amout = m_program->read_word_unaligned(m_reg[m_modval2 & 0x1F] + m_reg[m_modval & 0x1F] * 2);
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + m_reg[m_modval & 0x1F] * 4);
		break;
	}

	return 2;
}

UINT32 v60_device::bam1RegisterIndirectIndexed()
{
	m_bamoffset = m_reg[m_modval & 0x1F];
	m_amout = m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + m_bamoffset / 8);
	m_bamoffset&=7;
	return 2;
}

UINT32 v60_device::am1Autoincrement()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(m_reg[m_modval & 0x1F]);
		m_reg[m_modval & 0x1F]++;
		break;
	case 1:
		m_amout = m_program->read_word_unaligned(m_reg[m_modval & 0x1F]);
		m_reg[m_modval & 0x1F] +=2;
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(m_reg[m_modval & 0x1F]);
		m_reg[m_modval & 0x1F] +=4;
		break;
	}

	return 1;
}

UINT32 v60_device::bam1Autoincrement()
{
	m_bamoffset = 0;
	m_amout = m_program->read_dword_unaligned(m_reg[m_modval & 0x1F]);
	switch (m_moddim)
	{
	case 10:
		m_reg[m_modval & 0x1F] +=1;
		break;
	case 11:
		m_reg[m_modval & 0x1F] +=4;
		break;
	default:
		fatalerror("CPU - BAM1 - 7\n");
		break;
	}
	return 1;
}

UINT32 v60_device::am1Autodecrement()
{
	switch (m_moddim)
	{
	case 0:
		m_reg[m_modval & 0x1F]--;
		m_amout = m_program->read_byte(m_reg[m_modval & 0x1F]);
		break;
	case 1:
		m_reg[m_modval & 0x1F]-=2;
		m_amout = m_program->read_word_unaligned(m_reg[m_modval & 0x1F]);
		break;
	case 2:
		m_reg[m_modval & 0x1F]-=4;
		m_amout = m_program->read_dword_unaligned(m_reg[m_modval & 0x1F]);
		break;
	}

	return 1;
}

UINT32 v60_device::bam1Autodecrement()
{
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
		fatalerror("CPU - BAM1 - 7\n");
		break;
	}
	m_amout = m_program->read_dword_unaligned(m_reg[m_modval & 0x1F]);
	return 1;
}

UINT32 v60_device::am1Displacement8()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(m_reg[m_modval & 0x1F] + (INT8)OpRead8(m_modadd + 1));
		break;
	case 1:
		m_amout = m_program->read_word_unaligned(m_reg[m_modval & 0x1F] + (INT8)OpRead8(m_modadd + 1));
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT8)OpRead8(m_modadd + 1));
		break;
	}

	return 2;
}

UINT32 v60_device::bam1Displacement8()
{
	m_bamoffset = m_program->read_byte(m_modadd + 1);
	m_amout = m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + m_bamoffset / 8);
	m_bamoffset&=7;
	return 2;
}


UINT32 v60_device::am1Displacement16()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(m_reg[m_modval & 0x1F] + (INT16)OpRead16(m_modadd + 1));
		break;
	case 1:
		m_amout = m_program->read_word_unaligned(m_reg[m_modval & 0x1F] + (INT16)OpRead16(m_modadd + 1));
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT16)OpRead16(m_modadd + 1));
		break;
	}

	return 3;
}

UINT32 v60_device::bam1Displacement16()
{
	m_bamoffset = OpRead16(m_modadd + 1);
	m_amout = m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + m_bamoffset / 8);
	m_bamoffset&=7;
	return 3;
}

UINT32 v60_device::am1Displacement32()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(m_reg[m_modval & 0x1F] + OpRead32(m_modadd + 1));
		break;
	case 1:
		m_amout = m_program->read_word_unaligned(m_reg[m_modval & 0x1F] + OpRead32(m_modadd + 1));
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + OpRead32(m_modadd + 1));
		break;
	}

	return 5;
}

UINT32 v60_device::bam1Displacement32()
{
	m_bamoffset = OpRead32(m_modadd + 1);
	m_amout = m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + m_bamoffset / 8);
	m_bamoffset&=7;
	return 5;
}

UINT32 v60_device::am1DisplacementIndexed8()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(m_reg[m_modval2 & 0x1F] + (INT8)OpRead8(m_modadd + 2) + m_reg[m_modval & 0x1F]);
		break;
	case 1:
		m_amout = m_program->read_word_unaligned(m_reg[m_modval2 & 0x1F] + (INT8)OpRead8(m_modadd + 2) + m_reg[m_modval & 0x1F] * 2);
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + (INT8)OpRead8(m_modadd + 2) + m_reg[m_modval & 0x1F] * 4);
		break;
	}

	return 3;
}

UINT32 v60_device::bam1DisplacementIndexed8()
{
	m_bamoffset = m_reg[m_modval & 0x1F];
	m_amout = m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + (INT8)OpRead8(m_modadd + 2) + m_bamoffset / 8);
	m_bamoffset&=7;
	return 3;
}

UINT32 v60_device::am1DisplacementIndexed16()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(m_reg[m_modval2 & 0x1F] + (INT16)OpRead16(m_modadd + 2) + m_reg[m_modval & 0x1F]);
		break;
	case 1:
		m_amout = m_program->read_word_unaligned(m_reg[m_modval2 & 0x1F] + (INT16)OpRead16(m_modadd + 2) + m_reg[m_modval & 0x1F] * 2);
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + (INT16)OpRead16(m_modadd + 2) + m_reg[m_modval & 0x1F] * 4);
		break;
	}

	return 4;
}

UINT32 v60_device::bam1DisplacementIndexed16()
{
	m_bamoffset = m_reg[m_modval & 0x1F];
	m_amout = m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + (INT16)OpRead16(m_modadd + 2) + m_bamoffset / 8);
	m_bamoffset&=7;
	return 4;
}

UINT32 v60_device::am1DisplacementIndexed32()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(m_reg[m_modval2 & 0x1F] + OpRead32(m_modadd + 2) + m_reg[m_modval & 0x1F]);
		break;
	case 1:
		m_amout = m_program->read_word_unaligned(m_reg[m_modval2 & 0x1F] + OpRead32(m_modadd + 2) + m_reg[m_modval & 0x1F] * 2);
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + OpRead32(m_modadd + 2) + m_reg[m_modval & 0x1F] * 4);
		break;
	}

	return 6;
}

UINT32 v60_device::bam1DisplacementIndexed32()
{
	m_bamoffset = m_reg[m_modval & 0x1F];
	m_amout = m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + OpRead32(m_modadd + 2) + m_bamoffset / 8);
	m_bamoffset&=7;
	return 6;
}


UINT32 v60_device::am1PCDisplacement8()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(PC + (INT8)OpRead8(m_modadd + 1));
		break;
	case 1:
		m_amout = m_program->read_word_unaligned(PC + (INT8)OpRead8(m_modadd + 1));
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 1));
		break;
	}

	return 2;
}

UINT32 v60_device::bam1PCDisplacement8()
{
	m_bamoffset = OpRead8(m_modadd + 1);
	m_amout = m_program->read_dword_unaligned(PC + m_bamoffset / 8);
	m_bamoffset&=7;
	return 2;
}

UINT32 v60_device::am1PCDisplacement16()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(PC + (INT16)OpRead16(m_modadd + 1));
		break;
	case 1:
		m_amout = m_program->read_word_unaligned(PC + (INT16)OpRead16(m_modadd + 1));
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 1));
		break;
	}

	return 3;
}

UINT32 v60_device::bam1PCDisplacement16()
{
	m_bamoffset = OpRead16(m_modadd + 1);
	m_amout = m_program->read_dword_unaligned(PC + m_bamoffset / 8);
	m_bamoffset&=7;
	return 3;
}

UINT32 v60_device::am1PCDisplacement32()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(PC + OpRead32(m_modadd + 1));
		break;
	case 1:
		m_amout = m_program->read_word_unaligned(PC + OpRead32(m_modadd + 1));
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(PC + OpRead32(m_modadd + 1));
		break;
	}

	return 5;
}

UINT32 v60_device::bam1PCDisplacement32()
{
	m_bamoffset = OpRead32(m_modadd + 1);
	m_amout = m_program->read_dword_unaligned(PC + m_bamoffset / 8);
	m_bamoffset&=7;
	return 5;
}

UINT32 v60_device::am1PCDisplacementIndexed8()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(PC + (INT8)OpRead8(m_modadd + 2) + m_reg[m_modval & 0x1F]);
		break;
	case 1:
		m_amout = m_program->read_word_unaligned(PC + (INT8)OpRead8(m_modadd + 2) + m_reg[m_modval & 0x1F] * 2);
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 2) + m_reg[m_modval & 0x1F] * 4);
		break;
	}

	return 3;
}

UINT32 v60_device::bam1PCDisplacementIndexed8()
{
	m_bamoffset = m_reg[m_modval & 0x1F];
	m_amout = m_program->read_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 2) + m_bamoffset / 8);
	m_bamoffset&=7;
	return 3;
}


UINT32 v60_device::am1PCDisplacementIndexed16()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(PC + (INT16)OpRead16(m_modadd + 2) + m_reg[m_modval & 0x1F]);
		break;
	case 1:
		m_amout = m_program->read_word_unaligned(PC + (INT16)OpRead16(m_modadd + 2) + m_reg[m_modval & 0x1F] * 2);
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 2) + m_reg[m_modval & 0x1F] * 4);
		break;
	}

	return 4;
}

UINT32 v60_device::bam1PCDisplacementIndexed16()
{
	m_bamoffset = m_reg[m_modval & 0x1F];
	m_amout = m_program->read_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 2) + m_bamoffset / 8);
	m_bamoffset&=7;
	return 4;
}

UINT32 v60_device::am1PCDisplacementIndexed32()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(PC + OpRead32(m_modadd + 2) + m_reg[m_modval & 0x1F]);
		break;
	case 1:
		m_amout = m_program->read_word_unaligned(PC + OpRead32(m_modadd + 2) + m_reg[m_modval & 0x1F] * 2);
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(PC + OpRead32(m_modadd + 2) + m_reg[m_modval & 0x1F] * 4);
		break;
	}

	return 6;
}

UINT32 v60_device::bam1PCDisplacementIndexed32()
{
	m_bamoffset = m_reg[m_modval & 0x1F];
	m_amout = m_program->read_dword_unaligned(PC + OpRead32(m_modadd + 2) + m_bamoffset / 8);
	m_bamoffset&=7;
	return 6;
}

UINT32 v60_device::am1DisplacementIndirect8()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT8)OpRead8(m_modadd + 1)));
		break;
	case 1:
		m_amout = m_program->read_word_unaligned(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT8)OpRead8(m_modadd + 1)));
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT8)OpRead8(m_modadd + 1)));
		break;
	}

	return 2;
}

UINT32 v60_device::bam1DisplacementIndirect8()
{
	m_bamoffset = 0;
	m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT8)OpRead8(m_modadd + 1)));
	return 2;
}

UINT32 v60_device::am1DisplacementIndirect16()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT16)OpRead16(m_modadd + 1)));
		break;
	case 1:
		m_amout = m_program->read_word_unaligned(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT16)OpRead16(m_modadd + 1)));
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT16)OpRead16(m_modadd + 1)));
		break;
	}

	return 3;
}

UINT32 v60_device::bam1DisplacementIndirect16()
{
	m_bamoffset = 0;
	m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT16)OpRead16(m_modadd + 1)));
	return 3;
}

UINT32 v60_device::am1DisplacementIndirect32()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + OpRead32(m_modadd + 1)));
		break;
	case 1:
		m_amout = m_program->read_word_unaligned(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + OpRead32(m_modadd + 1)));
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + OpRead32(m_modadd + 1)));
		break;
	}

	return 5;
}

UINT32 v60_device::bam1DisplacementIndirect32()
{
	m_bamoffset = 0;
	m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + OpRead32(m_modadd + 1)));
	return 5;
}

UINT32 v60_device::am1DisplacementIndirectIndexed8()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + (INT8)OpRead8(m_modadd + 2)) + m_reg[m_modval & 0x1F]);
		break;
	case 1:
		m_amout = m_program->read_word_unaligned(m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + (INT8)OpRead8(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 2);
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + (INT8)OpRead8(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 4);
		break;
	}

	return 3;
}

UINT32 v60_device::bam1DisplacementIndirectIndexed8()
{
	m_bamoffset = m_reg[m_modval & 0x1F];
	m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + (INT8)OpRead8(m_modadd + 2)) + m_bamoffset / 8);
	m_bamoffset&=7;
	return 3;
}

UINT32 v60_device::am1DisplacementIndirectIndexed16()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + (INT16)OpRead16(m_modadd + 2)) + m_reg[m_modval & 0x1F]);
		break;
	case 1:
		m_amout = m_program->read_word_unaligned(m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + (INT16)OpRead16(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 2);
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + (INT16)OpRead16(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 4);
		break;
	}

	return 4;
}

UINT32 v60_device::bam1DisplacementIndirectIndexed16()
{
	m_bamoffset = m_reg[m_modval & 0x1F];
	m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + (INT16)OpRead16(m_modadd + 2)) + m_bamoffset / 8);
	m_bamoffset&=7;
	return 4;
}

UINT32 v60_device::am1DisplacementIndirectIndexed32()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + OpRead32(m_modadd + 2)) + m_reg[m_modval & 0x1F]);
		break;
	case 1:
		m_amout = m_program->read_word_unaligned(m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + OpRead32(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 2);
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + OpRead32(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 4);
		break;
	}

	return 6;
}

UINT32 v60_device::bam1DisplacementIndirectIndexed32()
{
	m_bamoffset = m_reg[m_modval & 0x1F];
	m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + OpRead32(m_modadd + 2)) + m_bamoffset / 8);
	m_bamoffset&=7;
	return 6;
}

UINT32 v60_device::am1PCDisplacementIndirect8()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(m_program->read_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 1)));
		break;
	case 1:
		m_amout = m_program->read_word_unaligned(m_program->read_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 1)));
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 1)));
		break;
	}

	return 2;
}

UINT32 v60_device::bam1PCDisplacementIndirect8()
{
	m_bamoffset = 0;
	m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 1)));
	return 2;
}

UINT32 v60_device::am1PCDisplacementIndirect16()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(m_program->read_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 1)));
		break;
	case 1:
		m_amout = m_program->read_word_unaligned(m_program->read_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 1)));
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 1)));
		break;
	}

	return 3;
}

UINT32 v60_device::bam1PCDisplacementIndirect16()
{
	m_bamoffset = 0;
	m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 1)));
	return 3;
}

UINT32 v60_device::am1PCDisplacementIndirect32()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(m_program->read_dword_unaligned(PC + OpRead32(m_modadd + 1)));
		break;
	case 1:
		m_amout = m_program->read_word_unaligned(m_program->read_dword_unaligned(PC + OpRead32(m_modadd + 1)));
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(PC + OpRead32(m_modadd + 1)));
		break;
	}

	return 5;
}

UINT32 v60_device::bam1PCDisplacementIndirect32()
{
	m_bamoffset = 0;
	m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(PC + OpRead32(m_modadd + 1)));
	return 5;
}

UINT32 v60_device::am1PCDisplacementIndirectIndexed8()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(m_program->read_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 2)) + m_reg[m_modval & 0x1F]);
		break;
	case 1:
		m_amout = m_program->read_word_unaligned(m_program->read_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 2);
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 4);
		break;
	}

	return 3;
}

UINT32 v60_device::bam1PCDisplacementIndirectIndexed8()
{
	m_bamoffset = m_reg[m_modval & 0x1F];
	m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 2)) + m_bamoffset / 8);
	m_bamoffset&=7;
	return 3;
}

UINT32 v60_device::am1PCDisplacementIndirectIndexed16()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(m_program->read_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 2)) + m_reg[m_modval & 0x1F]);
		break;
	case 1:
		m_amout = m_program->read_word_unaligned(m_program->read_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 2);
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 4);
		break;
	}

	return 4;
}

UINT32 v60_device::bam1PCDisplacementIndirectIndexed16()
{
	m_bamoffset = m_reg[m_modval & 0x1F];
	m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 2)) + m_bamoffset / 8);
	m_bamoffset&=7;
	return 4;
}

UINT32 v60_device::am1PCDisplacementIndirectIndexed32()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(m_program->read_dword_unaligned(PC + OpRead32(m_modadd + 2)) + m_reg[m_modval & 0x1F]);
		break;
	case 1:
		m_amout = m_program->read_word_unaligned(m_program->read_dword_unaligned(PC + OpRead32(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 2);
		break;
	case 2:
		m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(PC + OpRead32(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 4);
		break;
	}

	return 6;
}

UINT32 v60_device::bam1PCDisplacementIndirectIndexed32()
{
	m_bamoffset = m_reg[m_modval & 0x1F];
	m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(PC + OpRead32(m_modadd + 2)) + m_bamoffset / 8);
	m_bamoffset&=7;
	return 6;
}

UINT32 v60_device::am1DoubleDisplacement8()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT8)OpRead8(m_modadd + 1)) + (INT8)OpRead8(m_modadd + 2));
		break;

	case 1:
		m_amout = m_program->read_word_unaligned(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT8)OpRead8(m_modadd + 1)) + (INT8)OpRead8(m_modadd + 2));
		break;

	case 2:
		m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT8)OpRead8(m_modadd + 1)) + (INT8)OpRead8(m_modadd + 2));
		break;
	}

	return 3;
}

UINT32 v60_device::bam1DoubleDisplacement8()
{
	m_bamoffset = OpRead8(m_modadd + 2);
	m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT8)OpRead8(m_modadd + 1)) + m_bamoffset / 8);
	m_bamoffset&=7;
	return 3;
}

UINT32 v60_device::am1DoubleDisplacement16()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT16)OpRead16(m_modadd + 1)) + (INT16)OpRead16(m_modadd + 3));
		break;

	case 1:
		m_amout = m_program->read_word_unaligned(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT16)OpRead16(m_modadd + 1)) + (INT16)OpRead16(m_modadd + 3));
		break;

	case 2:
		m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT16)OpRead16(m_modadd + 1)) + (INT16)OpRead16(m_modadd + 3));
		break;
	}

	return 5;
}

UINT32 v60_device::bam1DoubleDisplacement16()
{
	m_bamoffset = OpRead16(m_modadd + 3);
	m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT16)OpRead16(m_modadd + 1)) + m_bamoffset / 8);
	m_bamoffset&=7;
	return 5;
}

UINT32 v60_device::am1DoubleDisplacement32()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + OpRead32(m_modadd + 1)) + OpRead32(m_modadd + 5));
		break;

	case 1:
		m_amout = m_program->read_word_unaligned(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + OpRead32(m_modadd + 1)) + OpRead32(m_modadd + 5));
		break;

	case 2:
		m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + OpRead32(m_modadd + 1)) + OpRead32(m_modadd + 5));
		break;
	}

	return 9;
}

UINT32 v60_device::bam1DoubleDisplacement32()
{
	m_bamoffset = OpRead32(m_modadd + 5);
	m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + OpRead32(m_modadd + 1)) + m_bamoffset / 8);
	m_bamoffset&=7;
	return 9;
}

UINT32 v60_device::am1PCDoubleDisplacement8()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(m_program->read_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 1)) + (INT8)OpRead8(m_modadd + 2));
		break;

	case 1:
		m_amout = m_program->read_word_unaligned(m_program->read_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 1)) + (INT8)OpRead8(m_modadd + 2));
		break;

	case 2:
		m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 1)) + (INT8)OpRead8(m_modadd + 2));
		break;
	}

	return 3;
}

UINT32 v60_device::bam1PCDoubleDisplacement8()
{
	m_bamoffset = OpRead8(m_modadd + 2);
	m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 1)) + m_bamoffset / 8);
	m_bamoffset&=7;
	return 3;
}

UINT32 v60_device::am1PCDoubleDisplacement16()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(m_program->read_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 1)) + (INT16)OpRead16(m_modadd + 3));
		break;

	case 1:
		m_amout = m_program->read_word_unaligned(m_program->read_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 1)) + (INT16)OpRead16(m_modadd + 3));
		break;

	case 2:
		m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 1)) + (INT16)OpRead16(m_modadd + 3));
		break;
	}

	return 5;
}

UINT32 v60_device::bam1PCDoubleDisplacement16()
{
	m_bamoffset = OpRead16(m_modadd + 3);
	m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 1)) + m_bamoffset / 8);
	m_bamoffset&=7;
	return 5;
}


UINT32 v60_device::am1PCDoubleDisplacement32()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(m_program->read_dword_unaligned(PC + OpRead32(m_modadd + 1)) + OpRead32(m_modadd + 5));
		break;

	case 1:
		m_amout = m_program->read_word_unaligned(m_program->read_dword_unaligned(PC + OpRead32(m_modadd + 1)) + OpRead32(m_modadd + 5));
		break;

	case 2:
		m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(PC + OpRead32(m_modadd + 1)) + OpRead32(m_modadd + 5));
		break;
	}

	return 9;
}

UINT32 v60_device::bam1PCDoubleDisplacement32()
{
	m_bamoffset = OpRead32(m_modadd + 5);
	m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(PC + OpRead32(m_modadd + 1)) + m_bamoffset / 8);
	m_bamoffset&=7;
	return 9;
}

UINT32 v60_device::am1DirectAddress()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(OpRead32(m_modadd + 1));
		break;

	case 1:
		m_amout = m_program->read_word_unaligned(OpRead32(m_modadd + 1));
		break;

	case 2:
		m_amout = m_program->read_dword_unaligned(OpRead32(m_modadd + 1));
		break;
	}

	return 5;
}

UINT32 v60_device::bam1DirectAddress()
{
	m_bamoffset = 0;
	m_amout = m_program->read_dword_unaligned(OpRead32(m_modadd + 1));
	return 5;
}

UINT32 v60_device::am1DirectAddressIndexed()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(OpRead32(m_modadd + 2) + m_reg[m_modval & 0x1F]);
		break;

	case 1:
		m_amout = m_program->read_word_unaligned(OpRead32(m_modadd + 2) + m_reg[m_modval & 0x1F] * 2);
		break;

	case 2:
		m_amout = m_program->read_dword_unaligned(OpRead32(m_modadd + 2) + m_reg[m_modval & 0x1F] * 4);
		break;
	}

	return 6;
}

UINT32 v60_device::bam1DirectAddressIndexed()
{
	m_bamoffset = m_reg[m_modval & 0x1F];
	m_amout = m_program->read_dword_unaligned(OpRead32(m_modadd + 2) + m_bamoffset / 8);
	m_bamoffset&=7;
	return 6;
}

UINT32 v60_device::am1DirectAddressDeferred()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(m_program->read_dword_unaligned(OpRead32(m_modadd + 1)));
		break;

	case 1:
		m_amout = m_program->read_word_unaligned(m_program->read_dword_unaligned(OpRead32(m_modadd + 1)));
		break;

	case 2:
		m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(OpRead32(m_modadd + 1)));
		break;
	}

	return 5;
}

UINT32 v60_device::bam1DirectAddressDeferred()
{
	m_bamoffset = 0;
	m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(OpRead32(m_modadd + 1)));
	return 5;
}

UINT32 v60_device::am1DirectAddressDeferredIndexed()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = m_program->read_byte(m_program->read_dword_unaligned(OpRead32(m_modadd + 2)) + m_reg[m_modval & 0x1F]);
		break;

	case 1:
		m_amout = m_program->read_word_unaligned(m_program->read_dword_unaligned(OpRead32(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 2);
		break;

	case 2:
		m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(OpRead32(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 4);
		break;
	}

	return 6;
}

UINT32 v60_device::bam1DirectAddressDeferredIndexed()
{
	m_bamoffset = m_reg[m_modval & 0x1F];
	m_amout = m_program->read_dword_unaligned(m_program->read_dword_unaligned(OpRead32(m_modadd + 2)) + m_bamoffset / 8);
	m_bamoffset&=7;
	return 6;
}

UINT32 v60_device::am1Immediate()
{
	switch (m_moddim)
	{
	case 0:
		m_amout = OpRead8(m_modadd + 1);
		return 2;

	case 1:
		m_amout = OpRead16(m_modadd + 1);
		return 3;

	case 2:
		m_amout = OpRead32(m_modadd + 1);
		return 5;
	}

	// It should not be here!  Written to avoid warning
	assert(0);
	return 1;
}

UINT32 v60_device::am1ImmediateQuick()
{
	m_amout = m_modval & 0xF;
	return 1;
}




// AM1 Tables (for ReadAM)
// ***********************

UINT32 v60_device::am1Error1()
{
	fatalerror("CPU - AM1 - 1 (PC=%06x)\n", PC);
	return 0; /* never reached, fatalerror won't return */
}

UINT32 v60_device::bam1Error1()
{
	fatalerror("CPU - BAM1 - 1 (PC=%06x)\n", PC);
	return 0; /* never reached, fatalerror won't return */
}

UINT32 v60_device::am1Error2()
{
	fatalerror("CPU - AM1 - 2 (PC=%06x)\n", PC);
	return 0; /* never reached, fatalerror won't return */
}

UINT32 v60_device::bam1Error2()
{
	fatalerror("CPU - BAM1 - 2 (PC=%06x)\n", PC);
	return 0; /* never reached, fatalerror won't return */
}

#ifdef UNUSED_FUNCTION
UINT32 v60_device::am1Error3()
{
	fatalerror("CPU - AM1 - 3 (PC=%06x)\n", PC);
	return 0; /* never reached, fatalerror won't return */
}

UINT32 v60_device::bam1Error3()
{
	fatalerror("CPU - BAM1 - 3 (PC=%06x)\n", PC);
	return 0; /* never reached, fatalerror won't return */
}
#endif

UINT32 v60_device::am1Error4()
{
	fatalerror("CPU - AM1 - 4 (PC=%06x)\n", PC);
	return 0; /* never reached, fatalerror won't return */
}

UINT32 v60_device::bam1Error4()
{
	fatalerror("CPU - BAM1 - 4 (PC=%06x)\n", PC);
	return 0; /* never reached, fatalerror won't return */
}

UINT32 v60_device::am1Error5()
{
	fatalerror("CPU - AM1 - 5 (PC=%06x)\n", PC);
	return 0; /* never reached, fatalerror won't return */
}

UINT32 v60_device::bam1Error5()
{
	fatalerror("CPU - BAM1 - 5 (PC=%06x)\n", PC);
	return 0; /* never reached, fatalerror won't return */
}

UINT32 v60_device::bam1Error6()
{
	fatalerror("CPU - BAM1 - 6 (PC=%06x)\n", PC);
	return 0; /* never reached, fatalerror won't return */
}

const v60_device::am_func v60_device::s_AMTable1_G7a[16] =
{
	&v60_device::am1PCDisplacementIndexed8,
	&v60_device::am1PCDisplacementIndexed16,
	&v60_device::am1PCDisplacementIndexed32,
	&v60_device::am1DirectAddressIndexed,
	&v60_device::am1Error5,
	&v60_device::am1Error5,
	&v60_device::am1Error5,
	&v60_device::am1Error5,
	&v60_device::am1PCDisplacementIndirectIndexed8,
	&v60_device::am1PCDisplacementIndirectIndexed16,
	&v60_device::am1PCDisplacementIndirectIndexed32,
	&v60_device::am1DirectAddressDeferredIndexed,
	&v60_device::am1Error5,
	&v60_device::am1Error5,
	&v60_device::am1Error5,
	&v60_device::am1Error5
};

const v60_device::am_func v60_device::s_BAMTable1_G7a[16] =
{
	&v60_device::bam1PCDisplacementIndexed8,
	&v60_device::bam1PCDisplacementIndexed16,
	&v60_device::bam1PCDisplacementIndexed32,
	&v60_device::bam1DirectAddressIndexed,
	&v60_device::bam1Error5,
	&v60_device::bam1Error5,
	&v60_device::bam1Error5,
	&v60_device::bam1Error5,
	&v60_device::bam1PCDisplacementIndirectIndexed8,
	&v60_device::bam1PCDisplacementIndirectIndexed16,
	&v60_device::bam1PCDisplacementIndirectIndexed32,
	&v60_device::bam1DirectAddressDeferredIndexed,
	&v60_device::bam1Error5,
	&v60_device::bam1Error5,
	&v60_device::bam1Error5,
	&v60_device::bam1Error5
};


UINT32 v60_device::am1Group7a()
{
	if (!(m_modval2 & 0x10))
		return am1Error4();

	return (this->*s_AMTable1_G7a[m_modval2 & 0xF])();
}

UINT32 v60_device::bam1Group7a()
{
	if (!(m_modval2 & 0x10))
		return bam1Error4();

	return (this->*s_BAMTable1_G7a[m_modval2 & 0xF])();
}

const v60_device::am_func v60_device::s_AMTable1_G7[32] =
{
	&v60_device::am1ImmediateQuick,
	&v60_device::am1ImmediateQuick,
	&v60_device::am1ImmediateQuick,
	&v60_device::am1ImmediateQuick,
	&v60_device::am1ImmediateQuick,
	&v60_device::am1ImmediateQuick,
	&v60_device::am1ImmediateQuick,
	&v60_device::am1ImmediateQuick,
	&v60_device::am1ImmediateQuick,
	&v60_device::am1ImmediateQuick,
	&v60_device::am1ImmediateQuick,
	&v60_device::am1ImmediateQuick,
	&v60_device::am1ImmediateQuick,
	&v60_device::am1ImmediateQuick,
	&v60_device::am1ImmediateQuick,
	&v60_device::am1ImmediateQuick,
	&v60_device::am1PCDisplacement8,
	&v60_device::am1PCDisplacement16,
	&v60_device::am1PCDisplacement32,
	&v60_device::am1DirectAddress,
	&v60_device::am1Immediate,
	&v60_device::am1Error2,
	&v60_device::am1Error2,
	&v60_device::am1Error2,
	&v60_device::am1PCDisplacementIndirect8,
	&v60_device::am1PCDisplacementIndirect16,
	&v60_device::am1PCDisplacementIndirect32,
	&v60_device::am1DirectAddressDeferred,
	&v60_device::am1PCDoubleDisplacement8,
	&v60_device::am1PCDoubleDisplacement16,
	&v60_device::am1PCDoubleDisplacement32,
	&v60_device::am1Error2
};

const v60_device::am_func v60_device::s_BAMTable1_G7[32] =
{
	&v60_device::bam1Error6,
	&v60_device::bam1Error6,
	&v60_device::bam1Error6,
	&v60_device::bam1Error6,
	&v60_device::bam1Error6,
	&v60_device::bam1Error6,
	&v60_device::bam1Error6,
	&v60_device::bam1Error6,
	&v60_device::bam1Error6,
	&v60_device::bam1Error6,
	&v60_device::bam1Error6,
	&v60_device::bam1Error6,
	&v60_device::bam1Error6,
	&v60_device::bam1Error6,
	&v60_device::bam1Error6,
	&v60_device::bam1Error6,
	&v60_device::bam1PCDisplacement8,
	&v60_device::bam1PCDisplacement16,
	&v60_device::bam1PCDisplacement32,
	&v60_device::bam1DirectAddress,
	&v60_device::bam1Error6,
	&v60_device::bam1Error2,
	&v60_device::bam1Error2,
	&v60_device::bam1Error2,
	&v60_device::bam1PCDisplacementIndirect8,
	&v60_device::bam1PCDisplacementIndirect16,
	&v60_device::bam1PCDisplacementIndirect32,
	&v60_device::bam1DirectAddressDeferred,
	&v60_device::bam1PCDoubleDisplacement8,
	&v60_device::bam1PCDoubleDisplacement16,
	&v60_device::bam1PCDoubleDisplacement32,
	&v60_device::bam1Error2
};



const v60_device::am_func v60_device::s_AMTable1_G6[8] =
{
	&v60_device::am1DisplacementIndexed8,
	&v60_device::am1DisplacementIndexed16,
	&v60_device::am1DisplacementIndexed32,
	&v60_device::am1RegisterIndirectIndexed,
	&v60_device::am1DisplacementIndirectIndexed8,
	&v60_device::am1DisplacementIndirectIndexed16,
	&v60_device::am1DisplacementIndirectIndexed32,
	&v60_device::am1Group7a
};

const v60_device::am_func v60_device::s_BAMTable1_G6[8] =
{
	&v60_device::bam1DisplacementIndexed8,
	&v60_device::bam1DisplacementIndexed16,
	&v60_device::bam1DisplacementIndexed32,
	&v60_device::bam1RegisterIndirectIndexed,
	&v60_device::bam1DisplacementIndirectIndexed8,
	&v60_device::bam1DisplacementIndirectIndexed16,
	&v60_device::bam1DisplacementIndirectIndexed32,
	&v60_device::bam1Group7a
};


UINT32 v60_device::am1Group6()
{
	m_modval2 = OpRead8(m_modadd + 1);
	return (this->*s_AMTable1_G6[m_modval2 >> 5])();
}

UINT32 v60_device::bam1Group6()
{
	m_modval2 = OpRead8(m_modadd + 1);
	return (this->*s_BAMTable1_G6[m_modval2 >> 5])();
}


UINT32 v60_device::am1Group7()
{
	return (this->*s_AMTable1_G7[m_modval & 0x1F])();
}

UINT32 v60_device::bam1Group7()
{
	return (this->*s_BAMTable1_G7[m_modval & 0x1F])();
}

const v60_device::am_func v60_device::s_AMTable1[2][8] =
{
	{
		&v60_device::am1Displacement8,
		&v60_device::am1Displacement16,
		&v60_device::am1Displacement32,
		&v60_device::am1RegisterIndirect,
		&v60_device::am1DisplacementIndirect8,
		&v60_device::am1DisplacementIndirect16,
		&v60_device::am1DisplacementIndirect32,
		&v60_device::am1Group7
	},

	{
		&v60_device::am1DoubleDisplacement8,
		&v60_device::am1DoubleDisplacement16,
		&v60_device::am1DoubleDisplacement32,
		&v60_device::am1Register,
		&v60_device::am1Autoincrement,
		&v60_device::am1Autodecrement,
		&v60_device::am1Group6,
		&v60_device::am1Error1
	}
};


const v60_device::am_func v60_device::s_BAMTable1[2][8] =
{
	{
		&v60_device::bam1Displacement8,
		&v60_device::bam1Displacement16,
		&v60_device::bam1Displacement32,
		&v60_device::bam1RegisterIndirect,
		&v60_device::bam1DisplacementIndirect8,
		&v60_device::bam1DisplacementIndirect16,
		&v60_device::bam1DisplacementIndirect32,
		&v60_device::bam1Group7
	},

	{
		&v60_device::bam1DoubleDisplacement8,
		&v60_device::bam1DoubleDisplacement16,
		&v60_device::bam1DoubleDisplacement32,
		&v60_device::bam1Error6,
		&v60_device::bam1Autoincrement,
		&v60_device::bam1Autodecrement,
		&v60_device::bam1Group6,
		&v60_device::bam1Error1
	}
};
