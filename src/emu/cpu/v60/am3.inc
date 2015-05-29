// license:BSD-3-Clause
// copyright-holders:Farfetch'd, R. Belmont
// AM3 Functions (for ReadAM)
// **************************

UINT32 v60_device::am3Register()
{
	switch (m_moddim)
	{
	case 0:
		SETREG8(m_reg[m_modval & 0x1F], m_modwritevalb);
		break;
	case 1:
		SETREG16(m_reg[m_modval & 0x1F], m_modwritevalh);
		break;
	case 2:
		m_reg[m_modval & 0x1F] = m_modwritevalw;
		break;
	}

	return 1;
}

UINT32 v60_device::am3RegisterIndirect()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(m_reg[m_modval & 0x1F], m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(m_reg[m_modval & 0x1F], m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(m_reg[m_modval & 0x1F], m_modwritevalw);
		break;
	}

	return 1;
}

UINT32 v60_device::am3RegisterIndirectIndexed()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(m_reg[m_modval2 & 0x1F] + m_reg[m_modval & 0x1F], m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(m_reg[m_modval2 & 0x1F] + m_reg[m_modval & 0x1F] * 2, m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(m_reg[m_modval2 & 0x1F] + m_reg[m_modval & 0x1F] * 4, m_modwritevalw);
		break;
	}

	return 2;
}

UINT32 v60_device::am3Autoincrement()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(m_reg[m_modval & 0x1F], m_modwritevalb);
		m_reg[m_modval & 0x1F] += 1;
		break;
	case 1:
		m_program->write_word_unaligned(m_reg[m_modval & 0x1F], m_modwritevalh);
		m_reg[m_modval & 0x1F] += 2;
		break;
	case 2:
		m_program->write_dword_unaligned(m_reg[m_modval & 0x1F], m_modwritevalw);
		m_reg[m_modval & 0x1F] += 4;
		break;
	}

	return 1;
}

UINT32 v60_device::am3Autodecrement()
{
	switch (m_moddim)
	{
	case 0:
		m_reg[m_modval & 0x1F] -= 1;
		m_program->write_byte(m_reg[m_modval & 0x1F], m_modwritevalb);
		break;
	case 1:
		m_reg[m_modval & 0x1F] -= 2;
		m_program->write_word_unaligned(m_reg[m_modval & 0x1F], m_modwritevalh);
		break;
	case 2:
		m_reg[m_modval & 0x1F] -= 4;
		m_program->write_dword_unaligned(m_reg[m_modval & 0x1F], m_modwritevalw);
		break;
	}

	return 1;
}

UINT32 v60_device::am3Displacement8()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(m_reg[m_modval & 0x1F] + (INT8)OpRead8(m_modadd + 1), m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(m_reg[m_modval & 0x1F] + (INT8)OpRead8(m_modadd + 1), m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(m_reg[m_modval & 0x1F] + (INT8)OpRead8(m_modadd + 1), m_modwritevalw);
		break;
	}

	return 2;
}

UINT32 v60_device::am3Displacement16()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(m_reg[m_modval & 0x1F] + (INT16)OpRead16(m_modadd + 1), m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(m_reg[m_modval & 0x1F] + (INT16)OpRead16(m_modadd + 1), m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(m_reg[m_modval & 0x1F] + (INT16)OpRead16(m_modadd + 1), m_modwritevalw);
		break;
	}

	return 3;
}

UINT32 v60_device::am3Displacement32()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(m_reg[m_modval & 0x1F] + OpRead32(m_modadd + 1), m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(m_reg[m_modval & 0x1F] + OpRead32(m_modadd + 1), m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(m_reg[m_modval & 0x1F] + OpRead32(m_modadd + 1), m_modwritevalw);
		break;
	}

	return 5;
}


UINT32 v60_device::am3DisplacementIndexed8()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(m_reg[m_modval2 & 0x1F] + (INT8)OpRead8(m_modadd + 2) + m_reg[m_modval & 0x1F], m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(m_reg[m_modval2 & 0x1F] + (INT8)OpRead8(m_modadd + 2) + m_reg[m_modval & 0x1F] * 2, m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(m_reg[m_modval2 & 0x1F] + (INT8)OpRead8(m_modadd + 2) + m_reg[m_modval & 0x1F] * 4, m_modwritevalw);
		break;
	}

	return 3;
}

UINT32 v60_device::am3DisplacementIndexed16()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(m_reg[m_modval2 & 0x1F] + (INT16)OpRead16(m_modadd + 2) + m_reg[m_modval & 0x1F], m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(m_reg[m_modval2 & 0x1F] + (INT16)OpRead16(m_modadd + 2) + m_reg[m_modval & 0x1F] * 2, m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(m_reg[m_modval2 & 0x1F] + (INT16)OpRead16(m_modadd + 2) + m_reg[m_modval & 0x1F] * 4, m_modwritevalw);
		break;
	}

	return 4;
}

UINT32 v60_device::am3DisplacementIndexed32()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(m_reg[m_modval2 & 0x1F] + OpRead32(m_modadd + 2) + m_reg[m_modval & 0x1F], m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(m_reg[m_modval2 & 0x1F] + OpRead32(m_modadd + 2) + m_reg[m_modval & 0x1F] * 2, m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(m_reg[m_modval2 & 0x1F] + OpRead32(m_modadd + 2) + m_reg[m_modval & 0x1F] * 4, m_modwritevalw);
		break;
	}

	return 6;
}


UINT32 v60_device::am3PCDisplacement8()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(PC + (INT8)OpRead8(m_modadd + 1), m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(PC + (INT8)OpRead8(m_modadd + 1), m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 1), m_modwritevalw);
		break;
	}

	return 2;
}

UINT32 v60_device::am3PCDisplacement16()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(PC + (INT16)OpRead16(m_modadd + 1), m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(PC + (INT16)OpRead16(m_modadd + 1), m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 1), m_modwritevalw);
		break;
	}

	return 3;
}

UINT32 v60_device::am3PCDisplacement32()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(PC + OpRead32(m_modadd + 1), m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(PC + OpRead32(m_modadd + 1), m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(PC + OpRead32(m_modadd + 1), m_modwritevalw);
		break;
	}

	return 5;
}

UINT32 v60_device::am3PCDisplacementIndexed8()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(PC + (INT8)OpRead8(m_modadd + 2) + m_reg[m_modval & 0x1F], m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(PC + (INT8)OpRead8(m_modadd + 2) + m_reg[m_modval & 0x1F] * 2, m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 2) + m_reg[m_modval & 0x1F] * 4, m_modwritevalw);
		break;
	}

	return 3;
}

UINT32 v60_device::am3PCDisplacementIndexed16()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(PC + (INT16)OpRead16(m_modadd + 2) + m_reg[m_modval & 0x1F], m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(PC + (INT16)OpRead16(m_modadd + 2) + m_reg[m_modval & 0x1F] * 2, m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 2) + m_reg[m_modval & 0x1F] * 4, m_modwritevalw);
		break;
	}

	return 4;
}

UINT32 v60_device::am3PCDisplacementIndexed32()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(PC + OpRead32(m_modadd + 2) + m_reg[m_modval & 0x1F], m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(PC + OpRead32(m_modadd + 2) + m_reg[m_modval & 0x1F] * 2, m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(PC + OpRead32(m_modadd + 2) + m_reg[m_modval & 0x1F] * 4, m_modwritevalw);
		break;
	}

	return 6;
}

UINT32 v60_device::am3DisplacementIndirect8()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT8)OpRead8(m_modadd + 1)), m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT8)OpRead8(m_modadd + 1)), m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT8)OpRead8(m_modadd + 1)), m_modwritevalw);
		break;
	}

	return 2;
}

UINT32 v60_device::am3DisplacementIndirect16()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT16)OpRead16(m_modadd + 1)), m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT16)OpRead16(m_modadd + 1)), m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT16)OpRead16(m_modadd + 1)), m_modwritevalw);
		break;
	}

	return 3;
}

UINT32 v60_device::am3DisplacementIndirect32()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + OpRead32(m_modadd + 1)), m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + OpRead32(m_modadd + 1)), m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + OpRead32(m_modadd + 1)), m_modwritevalw);
		break;
	}

	return 5;
}


UINT32 v60_device::am3DisplacementIndirectIndexed8()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + (INT8)OpRead8(m_modadd + 2)) + m_reg[m_modval & 0x1F], m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + (INT8)OpRead8(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 2, m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + (INT8)OpRead8(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 4, m_modwritevalw);
		break;
	}

	return 3;
}

UINT32 v60_device::am3DisplacementIndirectIndexed16()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + (INT16)OpRead16(m_modadd + 2)) + m_reg[m_modval & 0x1F], m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + (INT16)OpRead16(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 2, m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + (INT16)OpRead16(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 4, m_modwritevalw);
		break;
	}

	return 4;
}

UINT32 v60_device::am3DisplacementIndirectIndexed32()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + OpRead32(m_modadd + 2)) + m_reg[m_modval & 0x1F], m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + OpRead32(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 2, m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(m_program->read_dword_unaligned(m_reg[m_modval2 & 0x1F] + OpRead32(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 4, m_modwritevalw);
		break;
	}

	return 6;
}

UINT32 v60_device::am3PCDisplacementIndirect8()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(m_program->read_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 1)), m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(m_program->read_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 1)), m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(m_program->read_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 1)), m_modwritevalw);
		break;
	}

	return 2;
}

UINT32 v60_device::am3PCDisplacementIndirect16()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(m_program->read_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 1)), m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(m_program->read_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 1)), m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(m_program->read_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 1)), m_modwritevalw);
		break;
	}

	return 3;
}

UINT32 v60_device::am3PCDisplacementIndirect32()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(m_program->read_dword_unaligned(PC + OpRead32(m_modadd + 1)), m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(m_program->read_dword_unaligned(PC + OpRead32(m_modadd + 1)), m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(m_program->read_dword_unaligned(PC + OpRead32(m_modadd + 1)), m_modwritevalw);
		break;
	}

	return 5;
}


UINT32 v60_device::am3PCDisplacementIndirectIndexed8()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(m_program->read_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 2)) + m_reg[m_modval & 0x1F], m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(m_program->read_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 2, m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(m_program->read_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 4, m_modwritevalw);
		break;
	}

	return 3;
}

UINT32 v60_device::am3PCDisplacementIndirectIndexed16()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(m_program->read_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 2)) + m_reg[m_modval & 0x1F], m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(m_program->read_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 2, m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(m_program->read_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 4, m_modwritevalw);
		break;
	}

	return 4;
}

UINT32 v60_device::am3PCDisplacementIndirectIndexed32()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(m_program->read_dword_unaligned(PC + OpRead32(m_modadd + 2)) + m_reg[m_modval & 0x1F], m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(m_program->read_dword_unaligned(PC + OpRead32(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 2, m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(m_program->read_dword_unaligned(PC + OpRead32(m_modadd + 2)) + m_reg[m_modval & 0x1F] * 4, m_modwritevalw);
		break;
	}

	return 6;
}


UINT32 v60_device::am3DoubleDisplacement8()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT8)OpRead8(m_modadd + 1)) + (INT8)OpRead8(m_modadd + 2), m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT8)OpRead8(m_modadd + 1)) + (INT8)OpRead8(m_modadd + 2), m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT8)OpRead8(m_modadd + 1)) + (INT8)OpRead8(m_modadd + 2), m_modwritevalw);
		break;
	}

	return 3;
}

UINT32 v60_device::am3DoubleDisplacement16()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT16)OpRead16(m_modadd + 1)) + (INT16)OpRead16(m_modadd + 3), m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT16)OpRead16(m_modadd + 1)) + (INT16)OpRead16(m_modadd + 3), m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + (INT16)OpRead16(m_modadd + 1)) + (INT16)OpRead16(m_modadd + 3), m_modwritevalw);
		break;
	}

	return 5;
}

UINT32 v60_device::am3DoubleDisplacement32()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + OpRead32(m_modadd + 1)) + OpRead32(m_modadd + 5), m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + OpRead32(m_modadd + 1)) + OpRead32(m_modadd + 5), m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(m_program->read_dword_unaligned(m_reg[m_modval & 0x1F] + OpRead32(m_modadd + 1)) + OpRead32(m_modadd + 5), m_modwritevalw);
		break;
	}

	return 9;
}


UINT32 v60_device::am3PCDoubleDisplacement8()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(m_program->read_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 1)) + (INT8)OpRead8(m_modadd + 2), m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(m_program->read_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 1)) + (INT8)OpRead8(m_modadd + 2), m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(m_program->read_dword_unaligned(PC + (INT8)OpRead8(m_modadd + 1)) + (INT8)OpRead8(m_modadd + 2), m_modwritevalw);
		break;
	}

	return 3;
}

UINT32 v60_device::am3PCDoubleDisplacement16()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(m_program->read_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 1)) + (INT16)OpRead16(m_modadd + 3), m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(m_program->read_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 1)) + (INT16)OpRead16(m_modadd + 3), m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(m_program->read_dword_unaligned(PC + (INT16)OpRead16(m_modadd + 1)) + (INT16)OpRead16(m_modadd + 3), m_modwritevalw);
		break;
	}

	return 5;
}

UINT32 v60_device::am3PCDoubleDisplacement32()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(m_program->read_dword_unaligned(PC + OpRead32(m_modadd + 1)) + OpRead32(m_modadd + 5), m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(m_program->read_dword_unaligned(PC + OpRead32(m_modadd + 1)) + OpRead32(m_modadd + 5), m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(m_program->read_dword_unaligned(PC + OpRead32(m_modadd + 1)) + OpRead32(m_modadd + 5), m_modwritevalw);
		break;
	}

	return 9;
}

UINT32 v60_device::am3DirectAddress()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(OpRead32(m_modadd + 1), m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(OpRead32(m_modadd + 1), m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(OpRead32(m_modadd + 1), m_modwritevalw);
		break;
	}

	return 5;
}

UINT32 v60_device::am3DirectAddressIndexed()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(OpRead32(m_modadd + 2) + m_reg[m_modval & 0x1F], m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(OpRead32(m_modadd + 2) + m_reg[m_modval & 0x1F] * 2, m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(OpRead32(m_modadd + 2) + m_reg[m_modval & 0x1F] * 4, m_modwritevalw);
		break;
	}

	return 6;
}

UINT32 v60_device::am3DirectAddressDeferred()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(m_program->read_dword_unaligned(OpRead32(m_modadd + 1)), m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(m_program->read_dword_unaligned(OpRead32(m_modadd + 1)), m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(m_program->read_dword_unaligned(OpRead32(m_modadd + 1)), m_modwritevalw);
		break;
	}

	return 5;
}

UINT32 v60_device::am3DirectAddressDeferredIndexed()
{
	switch (m_moddim)
	{
	case 0:
		m_program->write_byte(m_program->read_dword_unaligned(OpRead32(m_modadd + 2)) + m_reg[m_modval & 0x1F], m_modwritevalb);
		break;
	case 1:
		m_program->write_word_unaligned(m_program->read_dword_unaligned(OpRead32(m_modadd + 2)) + m_reg[m_modval & 0x1F], m_modwritevalh);
		break;
	case 2:
		m_program->write_dword_unaligned(m_program->read_dword_unaligned(OpRead32(m_modadd + 2)) + m_reg[m_modval & 0x1F], m_modwritevalw);
		break;
	}

	return 6;
}

UINT32 v60_device::am3Immediate()
{
	fatalerror("CPU - AM3 - IMM (PC=%06x)\n", PC);
	return 0; /* never reached, fatalerror won't return */
}

UINT32 v60_device::am3ImmediateQuick()
{
	fatalerror("CPU - AM3 - IMMQ (PC=%06x)\n", PC);
	return 0; /* never reached, fatalerror won't return */
}



// AM3 Tables (for ReadAMAddress)
// ******************************

UINT32 v60_device::am3Error1()
{
	fatalerror("CPU - AM3 - 1 (PC=%06x)\n", PC);
	return 0; /* never reached, fatalerror won't return */
}

UINT32 v60_device::am3Error2()
{
	fatalerror("CPU - AM3 - 2 (PC=%06x)\n", PC);
	return 0; /* never reached, fatalerror won't return */
}

#ifdef UNUSED_FUNCTION
UINT32 v60_device::am3Error3()
{
	fatalerror("CPU - AM3 - 3 (PC=%06x)\n", PC);
	return 0; /* never reached, fatalerror won't return */
}
#endif

UINT32 v60_device::am3Error4()
{
	fatalerror("CPU - AM3 - 4 (PC=%06x)\n", PC);
	return 0; /* never reached, fatalerror won't return */
}

UINT32 v60_device::am3Error5()
{
	fatalerror("CPU - AM3 - 5 (PC=%06x)\n", PC);
	return 0; /* never reached, fatalerror won't return */
}

const v60_device::am_func v60_device::s_AMTable3_G7a[16] =
{
	&v60_device::am3PCDisplacementIndexed8,
	&v60_device::am3PCDisplacementIndexed16,
	&v60_device::am3PCDisplacementIndexed32,
	&v60_device::am3DirectAddressIndexed,
	&v60_device::am3Error5,
	&v60_device::am3Error5,
	&v60_device::am3Error5,
	&v60_device::am3Error5,
	&v60_device::am3PCDisplacementIndirectIndexed8,
	&v60_device::am3PCDisplacementIndirectIndexed16,
	&v60_device::am3PCDisplacementIndirectIndexed32,
	&v60_device::am3DirectAddressDeferredIndexed,
	&v60_device::am3Error5,
	&v60_device::am3Error5,
	&v60_device::am3Error5,
	&v60_device::am3Error5
};

UINT32 v60_device::am3Group7a()
{
	if (!(m_modval2 & 0x10))
		return am3Error4();

	return (this->*s_AMTable3_G7a[m_modval2 & 0xF])();
}

const v60_device::am_func v60_device::s_AMTable3_G7[32] =
{
	&v60_device::am3ImmediateQuick,
	&v60_device::am3ImmediateQuick,
	&v60_device::am3ImmediateQuick,
	&v60_device::am3ImmediateQuick,
	&v60_device::am3ImmediateQuick,
	&v60_device::am3ImmediateQuick,
	&v60_device::am3ImmediateQuick,
	&v60_device::am3ImmediateQuick,
	&v60_device::am3ImmediateQuick,
	&v60_device::am3ImmediateQuick,
	&v60_device::am3ImmediateQuick,
	&v60_device::am3ImmediateQuick,
	&v60_device::am3ImmediateQuick,
	&v60_device::am3ImmediateQuick,
	&v60_device::am3ImmediateQuick,
	&v60_device::am3ImmediateQuick,
	&v60_device::am3PCDisplacement8,
	&v60_device::am3PCDisplacement16,
	&v60_device::am3PCDisplacement32,
	&v60_device::am3DirectAddress,
	&v60_device::am3Immediate,
	&v60_device::am3Error2,
	&v60_device::am3Error2,
	&v60_device::am3Error2,
	&v60_device::am3PCDisplacementIndirect8,
	&v60_device::am3PCDisplacementIndirect16,
	&v60_device::am3PCDisplacementIndirect32,
	&v60_device::am3DirectAddressDeferred,
	&v60_device::am3PCDoubleDisplacement8,
	&v60_device::am3PCDoubleDisplacement16,
	&v60_device::am3PCDoubleDisplacement32,
	&v60_device::am3Error2
};

const v60_device::am_func v60_device::s_AMTable3_G6[8] =
{
	&v60_device::am3DisplacementIndexed8,
	&v60_device::am3DisplacementIndexed16,
	&v60_device::am3DisplacementIndexed32,
	&v60_device::am3RegisterIndirectIndexed,
	&v60_device::am3DisplacementIndirectIndexed8,
	&v60_device::am3DisplacementIndirectIndexed16,
	&v60_device::am3DisplacementIndirectIndexed32,
	&v60_device::am3Group7a
};




UINT32 v60_device::am3Group6()
{
	m_modval2 = OpRead8(m_modadd + 1);
	return (this->*s_AMTable3_G6[m_modval2 >> 5])();
}


UINT32 v60_device::am3Group7()
{
	return (this->*s_AMTable3_G7[m_modval & 0x1F])();
}



const v60_device::am_func v60_device::s_AMTable3[2][8] =
{
	{
		&v60_device::am3Displacement8,
		&v60_device::am3Displacement16,
		&v60_device::am3Displacement32,
		&v60_device::am3RegisterIndirect,
		&v60_device::am3DisplacementIndirect8,
		&v60_device::am3DisplacementIndirect16,
		&v60_device::am3DisplacementIndirect32,
		&v60_device::am3Group7
	},

	{
		&v60_device::am3DoubleDisplacement8,
		&v60_device::am3DoubleDisplacement16,
		&v60_device::am3DoubleDisplacement32,
		&v60_device::am3Register,
		&v60_device::am3Autoincrement,
		&v60_device::am3Autodecrement,
		&v60_device::am3Group6,
		&v60_device::am3Error1
	}
};
