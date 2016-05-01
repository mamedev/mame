// license:BSD-3-Clause
// copyright-holders:Ville Linde
/* SHARC memory operations */

UINT32 adsp21062_device::pm_read32(UINT32 address)
{
	if (address >= 0x20000 && address < 0x28000)
	{
		UINT32 addr = (address & 0x7fff) * 3;

		return (UINT32)(m_internal_ram_block0[addr + 0] << 16) |
						(m_internal_ram_block0[addr + 1]);
	}
	else if (address >= 0x28000 && address < 0x40000)
	{
		// block 1 is mirrored in 0x28000...2ffff, 0x30000...0x37fff and 0x38000...3ffff
		UINT32 addr = (address & 0x7fff) * 3;

		return (UINT32)(m_internal_ram_block1[addr + 0] << 16) |
						(m_internal_ram_block1[addr + 1]);
	}
	else {
		fatalerror("SHARC: PM Bus Read32 %08X at %08X\n", address, m_pc);
	}
}

void adsp21062_device::pm_write32(UINT32 address, UINT32 data)
{
	if (address >= 0x20000 && address < 0x28000)
	{
		UINT32 addr = (address & 0x7fff) * 3;

		m_internal_ram_block0[addr + 0] = (UINT16)(data >> 16);
		m_internal_ram_block0[addr + 1] = (UINT16)(data);
		return;
	}
	else if (address >= 0x28000 && address < 0x40000)
	{
		// block 1 is mirrored in 0x28000...2ffff, 0x30000...0x37fff and 0x38000...3ffff
		UINT32 addr = (address & 0x7fff) * 3;

		m_internal_ram_block1[addr + 0] = (UINT16)(data >> 16);
		m_internal_ram_block1[addr + 1] = (UINT16)(data);
		return;
	}
	else {
		fatalerror("SHARC: PM Bus Write32 %08X, %08X at %08X\n", address, data, m_pc);
	}
}

UINT64 adsp21062_device::pm_read48(UINT32 address)
{
	if ((address >= 0x20000 && address < 0x28000))
	{
		UINT32 addr = (address & 0x7fff) * 3;

		return ((UINT64)(m_internal_ram_block0[addr + 0]) << 32) |
				((UINT64)(m_internal_ram_block0[addr + 1]) << 16) |
				((UINT64)(m_internal_ram_block0[addr + 2]) << 0);
	}
	else if (address >= 0x28000 && address < 0x40000)
	{
		// block 1 is mirrored in 0x28000...2ffff, 0x30000...0x37fff and 0x38000...3ffff
		UINT32 addr = (address & 0x7fff) * 3;

		return ((UINT64)(m_internal_ram_block1[addr + 0]) << 32) |
				((UINT64)(m_internal_ram_block1[addr + 1]) << 16) |
				((UINT64)(m_internal_ram_block1[addr + 2]) << 0);
	}
	else {
		fatalerror("SHARC: PM Bus Read48 %08X at %08X\n", address, m_pc);
	}

	return 0;
}

void adsp21062_device::pm_write48(UINT32 address, UINT64 data)
{
	if ((address >= 0x20000 && address < 0x28000))
	{
		UINT32 addr = (address & 0x7fff) * 3;

		m_internal_ram_block0[addr + 0] = (UINT16)(data >> 32);
		m_internal_ram_block0[addr + 1] = (UINT16)(data >> 16);
		m_internal_ram_block0[addr + 2] = (UINT16)(data);
		return;
	}
	else if (address >= 0x28000 && address < 0x40000)
	{
		// block 1 is mirrored in 0x28000...2ffff, 0x30000...0x37fff and 0x38000...3ffff
		UINT32 addr = (address & 0x7fff) * 3;

		m_internal_ram_block1[addr + 0] = (UINT16)(data >> 32);
		m_internal_ram_block1[addr + 1] = (UINT16)(data >> 16);
		m_internal_ram_block1[addr + 2] = (UINT16)(data);
		return;
	}
	else {
		fatalerror("SHARC: PM Bus Write48 %08X, %04X%08X at %08X\n", address, (UINT16)(data >> 32),(UINT32)data, m_pc);
	}
}

UINT32 adsp21062_device::dm_read32(UINT32 address)
{
	if (address < 0x100)
	{
		return sharc_iop_r(address);
	}
	else if (address >= 0x20000 && address < 0x28000)
	{
		UINT32 addr = (address & 0x7fff) * 2;

		return (UINT32)(m_internal_ram_block0[addr + 0] << 16) |
						(m_internal_ram_block0[addr + 1]);
	}
	else if (address >= 0x28000 && address < 0x40000)
	{
		// block 1 is mirrored in 0x28000...2ffff, 0x30000...0x37fff and 0x38000...3ffff
		UINT32 addr = (address & 0x7fff) * 2;

		return (UINT32)(m_internal_ram_block1[addr + 0] << 16) |
						(m_internal_ram_block1[addr + 1]);
	}

	// short word addressing
	else if (address >= 0x40000 && address < 0x50000)
	{
		UINT32 addr = address & 0xffff;

		UINT16 r = m_internal_ram_block0[addr ^ 1];
		if (m_mode1 & 0x4000)
		{
			// sign-extend
			return (INT32)(INT16)(r);
		}
		else
		{
			return (UINT32)(r);
		}
	}
	else if (address >= 0x50000 && address < 0x80000)
	{
		// block 1 is mirrored in 0x50000...5ffff, 0x60000...0x6ffff and 0x70000...7ffff
		UINT32 addr = address & 0xffff;

		UINT16 r = m_internal_ram_block1[addr ^ 1];
		if (m_mode1 & 0x4000)
		{
			// sign-extend
			return (INT32)(INT16)(r);
		}
		else
		{
			return (UINT32)(r);
		}
	}

	return m_data->read_dword(address << 2);
}

void adsp21062_device::dm_write32(UINT32 address, UINT32 data)
{
	if (address < 0x100)
	{
		sharc_iop_w(address, data);
		return;
	}
	else if (address >= 0x20000 && address < 0x28000)
	{
		UINT32 addr = (address & 0x7fff) * 2;

		m_internal_ram_block0[addr + 0] = (UINT16)(data >> 16);
		m_internal_ram_block0[addr + 1] = (UINT16)(data);
		return;
	}
	else if (address >= 0x28000 && address < 0x40000)
	{
		// block 1 is mirrored in 0x28000...2ffff, 0x30000...0x37fff and 0x38000...3ffff
		UINT32 addr = (address & 0x7fff) * 2;

		m_internal_ram_block1[addr + 0] = (UINT16)(data >> 16);
		m_internal_ram_block1[addr + 1] = (UINT16)(data);
		return;
	}

	// short word addressing
	else if (address >= 0x40000 && address < 0x50000)
	{
		UINT32 addr = address & 0xffff;

		m_internal_ram_block0[addr ^ 1] = data;
		return;
	}
	else if (address >= 0x50000 && address < 0x80000)
	{
		// block 1 is mirrored in 0x50000...5ffff, 0x60000...0x6ffff and 0x70000...7ffff
		UINT32 addr = address & 0xffff;

		m_internal_ram_block1[addr ^ 1] = data;
		return;
	}

	m_data->write_dword(address << 2, data);
}
