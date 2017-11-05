// license:BSD-3-Clause
// copyright-holders:Farfetch'd, R. Belmont
/*
    FULLY TRUSTED
*/

uint32_t v60_device::opBGT8() /* TRUSTED */
{
	NORMALIZEFLAGS();

	if (!((_S ^ _OV) | _Z))
	{
		PC += (int8_t)OpRead8(PC + 1);
		return 0;
	}

	return 2;
}

uint32_t v60_device::opBGT16() /* TRUSTED */
{
	NORMALIZEFLAGS();

	if (!((_S ^ _OV) | _Z))
	{
		PC += (int16_t)OpRead16(PC + 1);
		return 0;
	}

	return 3;
}


uint32_t v60_device::opBGE8() /* TRUSTED */
{
	NORMALIZEFLAGS();

	if (!(_S ^ _OV))
	{
		PC += (int8_t)OpRead8(PC + 1);
		return 0;
	}

	return 2;
}

uint32_t v60_device::opBGE16() /* TRUSTED */
{
	NORMALIZEFLAGS();

	if (!(_S ^ _OV))
	{
		PC += (int16_t)OpRead16(PC + 1);
		return 0;
	}

	return 3;
}

uint32_t v60_device::opBLT8() /* TRUSTED */
{
	NORMALIZEFLAGS();

	if ((_S ^ _OV))
	{
		PC += (int8_t)OpRead8(PC + 1);
		return 0;
	}

	return 2;
}

uint32_t v60_device::opBLT16() /* TRUSTED */
{
	NORMALIZEFLAGS();

	if ((_S ^ _OV))
	{
		PC += (int16_t)OpRead16(PC + 1);
		return 0;
	}

	return 3;
}


uint32_t v60_device::opBLE8() /* TRUSTED */
{
	NORMALIZEFLAGS();

	if (((_S ^ _OV) | _Z))
	{
		PC += (int8_t)OpRead8(PC + 1);
		return 0;
	}

	return 2;
}

uint32_t v60_device::opBLE16() /* TRUSTED */
{
	NORMALIZEFLAGS();

	if (((_S ^ _OV) | _Z))
	{
		PC += (int16_t)OpRead16(PC + 1);
		return 0;
	}

	return 3;
}

uint32_t v60_device::opBH8() /* TRUSTED */
{
	if (!(_CY | _Z))
	{
		PC += (int8_t)OpRead8(PC + 1);
		return 0;
	}

	return 2;
}

uint32_t v60_device::opBH16() /* TRUSTED */
{
	if (!(_CY | _Z))
	{
		PC += (int16_t)OpRead16(PC + 1);
		return 0;
	}

	return 3;
}

uint32_t v60_device::opBNH8() /* TRUSTED */
{
	if ((_CY | _Z))
	{
		PC += (int8_t)OpRead8(PC + 1);
		return 0;
	}

	return 2;
}

uint32_t v60_device::opBNH16() /* TRUSTED */
{
	if ((_CY | _Z))
	{
		PC += (int16_t)OpRead16(PC + 1);
		return 0;
	}

	return 3;
}

uint32_t v60_device::opBNL8() /* TRUSTED */
{
	if (!(_CY))
	{
		PC += (int8_t)OpRead8(PC + 1);
		return 0;
	}

	return 2;
}

uint32_t v60_device::opBNL16() /* TRUSTED */
{
	if (!(_CY))
	{
		PC += (int16_t)OpRead16(PC + 1);
		return 0;
	}

	return 3;
}

uint32_t v60_device::opBL8() /* TRUSTED */
{
	if ((_CY))
	{
		PC += (int8_t)OpRead8(PC + 1);
		return 0;
	}

	return 2;
}

uint32_t v60_device::opBL16() /* TRUSTED */
{
	if ((_CY))
	{
		PC += (int16_t)OpRead16(PC + 1);
		return 0;
	}

	return 3;
}

uint32_t v60_device::opBNE8() /* TRUSTED */
{
	if (!(_Z))
	{
		PC += (int8_t)OpRead8(PC + 1);
		return 0;
	}

	return 2;
}

uint32_t v60_device::opBNE16() /* TRUSTED */
{
	if (!(_Z))
	{
		PC += (int16_t)OpRead16(PC + 1);
		return 0;
	}

	return 3;
}

uint32_t v60_device::opBE8() /* TRUSTED */
{
	if ((_Z))
	{
		PC += (int8_t)OpRead8(PC + 1);
		return 0;
	}

	return 2;
}

uint32_t v60_device::opBE16() /* TRUSTED */
{
	if ((_Z))
	{
		PC += (int16_t)OpRead16(PC + 1);
		return 0;
	}

	return 3;
}

uint32_t v60_device::opBNV8() /* TRUSTED */
{
	if (!(_OV))
	{
		PC += (int8_t)OpRead8(PC + 1);
		return 0;
	}

	return 2;
}

uint32_t v60_device::opBNV16() /* TRUSTED */
{
	if (!(_OV))
	{
		PC += (int16_t)OpRead16(PC + 1);
		return 0;
	}

	return 3;
}

uint32_t v60_device::opBV8() /* TRUSTED */
{
	if ((_OV))
	{
		PC += (int8_t)OpRead8(PC + 1);
		return 0;
	}

	return 2;
}

uint32_t v60_device::opBV16() /* TRUSTED */
{
	if ((_OV))
	{
		PC += (int16_t)OpRead16(PC + 1);
		return 0;
	}

	return 3;
}

uint32_t v60_device::opBP8() /* TRUSTED */
{
	if (!(_S))
	{
		PC += (int8_t)OpRead8(PC + 1);
		return 0;
	}

	return 2;
}

uint32_t v60_device::opBP16() /* TRUSTED */
{
	if (!(_S))
	{
		PC += (int16_t)OpRead16(PC + 1);
		return 0;
	}

	return 3;
}

uint32_t v60_device::opBN8() /* TRUSTED */
{
	if ((_S))
	{
		PC += (int8_t)OpRead8(PC + 1);
		return 0;
	}

	return 2;
}

uint32_t v60_device::opBN16() /* TRUSTED */
{
	if ((_S))
	{
		PC += (int16_t)OpRead16(PC + 1);
		return 0;
	}

	return 3;
}

uint32_t v60_device::opBR8() /* TRUSTED */
{
	PC += (int8_t)OpRead8(PC + 1);
	return 0;
}

uint32_t v60_device::opBR16() /* TRUSTED */
{
	PC += (int16_t)OpRead16(PC + 1);
	return 0;
}

uint32_t v60_device::opBSR() /* TRUSTED */
{
	// Save Next PC onto the stack
	SP -= 4;
	m_program->write_dword_unaligned(SP, PC + 3);

	// Jump to subroutine
	PC += (int16_t)OpRead16(PC + 1);
	return 0;
}
