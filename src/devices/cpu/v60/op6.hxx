// license:BSD-3-Clause
// copyright-holders:Farfetch'd, R. Belmont
/*
    FULLY TRUSTED
*/

uint32_t v60_device::opTB(int reg) /* TRUSTED */
{
	if (m_reg[reg] == 0)
	{
		PC += (int16_t)OpRead16(PC + 2);
		return 0;
	}

	return 4;
}

uint32_t v60_device::opDBGT(int reg) /* TRUSTED */
{
	m_reg[reg]--;

	NORMALIZEFLAGS();
	if ((m_reg[reg] != 0) && !((_S ^ _OV) | _Z))
	{
		PC += (int16_t)OpRead16(PC + 2);
		return 0;
	}

	return 4;
}

uint32_t v60_device::opDBLE(int reg) /* TRUSTED */
{
	m_reg[reg]--;

	NORMALIZEFLAGS();
	if ((m_reg[reg] != 0) && ((_S ^ _OV) | _Z))
	{
		PC += (int16_t)OpRead16(PC + 2);
		return 0;
	}

	return 4;
}


uint32_t v60_device::opDBGE(int reg) /* TRUSTED */
{
	m_reg[reg]--;

	NORMALIZEFLAGS();
	if ((m_reg[reg] != 0) && !(_S ^ _OV))
	{
		PC += (int16_t)OpRead16(PC + 2);
		return 0;
	}

	return 4;
}

uint32_t v60_device::opDBLT(int reg) /* TRUSTED */
{
	m_reg[reg]--;

	NORMALIZEFLAGS();
	if ((m_reg[reg] != 0) && (_S ^ _OV))
	{
		PC += (int16_t)OpRead16(PC + 2);
		return 0;
	}

	return 4;
}

uint32_t v60_device::opDBH(int reg) /* TRUSTED */
{
	m_reg[reg]--;

	if ((m_reg[reg] != 0) && !(_CY | _Z))
	{
		PC += (int16_t)OpRead16(PC + 2);
		return 0;
	}

	return 4;
}

uint32_t v60_device::opDBNH(int reg) /* TRUSTED */
{
	m_reg[reg]--;

	if ((m_reg[reg] != 0) && (_CY | _Z))
	{
		PC += (int16_t)OpRead16(PC + 2);
		return 0;
	}

	return 4;
}


uint32_t v60_device::opDBL(int reg) /* TRUSTED */
{
	m_reg[reg]--;

	if ((m_reg[reg] != 0) && (_CY))
	{
		PC += (int16_t)OpRead16(PC + 2);
		return 0;
	}

	return 4;
}

uint32_t v60_device::opDBNL(int reg) /* TRUSTED */
{
	m_reg[reg]--;

	if ((m_reg[reg] != 0) && !(_CY))
	{
		PC += (int16_t)OpRead16(PC + 2);
		return 0;
	}

	return 4;
}

uint32_t v60_device::opDBE(int reg) /* TRUSTED */
{
	m_reg[reg]--;

	if ((m_reg[reg] != 0) && (_Z))
	{
		PC += (int16_t)OpRead16(PC + 2);
		return 0;
	}

	return 4;
}

uint32_t v60_device::opDBNE(int reg) /* TRUSTED */
{
	m_reg[reg]--;

	if ((m_reg[reg] != 0) && !(_Z))
	{
		PC += (int16_t)OpRead16(PC + 2);
		return 0;
	}

	return 4;
}

uint32_t v60_device::opDBV(int reg) /* TRUSTED */
{
	m_reg[reg]--;

	if ((m_reg[reg] != 0) && (_OV))
	{
		PC += (int16_t)OpRead16(PC + 2);
		return 0;
	}

	return 4;
}

uint32_t v60_device::opDBNV(int reg) /* TRUSTED */
{
	m_reg[reg]--;

	if ((m_reg[reg] != 0) && !(_OV))
	{
		PC += (int16_t)OpRead16(PC + 2);
		return 0;
	}

	return 4;
}

uint32_t v60_device::opDBN(int reg) /* TRUSTED */
{
	m_reg[reg]--;

	if ((m_reg[reg] != 0) && (_S))
	{
		PC += (int16_t)OpRead16(PC + 2);
		return 0;
	}

	return 4;
}

uint32_t v60_device::opDBP(int reg) /* TRUSTED */
{
	m_reg[reg]--;

	if ((m_reg[reg] != 0) && !(_S))
	{
		PC += (int16_t)OpRead16(PC + 2);
		return 0;
	}

	return 4;
}

uint32_t v60_device::opDBR(int reg) /* TRUSTED */
{
	m_reg[reg]--;

	if (m_reg[reg] != 0)
	{
		PC += (int16_t)OpRead16(PC + 2);
		return 0;
	}

	return 4;
}

const v60_device::op6_func v60_device::s_OpC6Table[8] = /* TRUSTED */
{
	&v60_device::opDBV,
	&v60_device::opDBL,
	&v60_device::opDBE,
	&v60_device::opDBNH,
	&v60_device::opDBN,
	&v60_device::opDBR,
	&v60_device::opDBLT,
	&v60_device::opDBLE
};

const v60_device::op6_func v60_device::s_OpC7Table[8] = /* TRUSTED */
{
	&v60_device::opDBNV,
	&v60_device::opDBNL,
	&v60_device::opDBNE,
	&v60_device::opDBH,
	&v60_device::opDBP,
	&v60_device::opTB,
	&v60_device::opDBGE,
	&v60_device::opDBGT
};


uint32_t v60_device::opC6() /* TRUSTED */
{
	uint8_t appb = OpRead8(PC + 1);
	return (this->*s_OpC6Table[appb >> 5])(appb & 0x1f);
}

uint32_t v60_device::opC7() /* TRUSTED */
{
	uint8_t appb = OpRead8(PC + 1);
	return (this->*s_OpC7Table[appb >> 5])(appb & 0x1f);
}
