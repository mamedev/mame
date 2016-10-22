// license:BSD-3-Clause
// copyright-holders:Ville Linde
/* SHARC memory operations */

uint32_t adsp21062_device::pm_read32(uint32_t address)
{
	if (address >= 0x20000 && address < 0x28000)
	{
		uint32_t addr = (address & 0x7fff) * 3;

		return (uint32_t)(m_internal_ram_block0[addr + 0] << 16) |
						(m_internal_ram_block0[addr + 1]);
	}
	else if (address >= 0x28000 && address < 0x40000)
	{
		// block 1 is mirrored in 0x28000...2ffff, 0x30000...0x37fff and 0x38000...3ffff
		uint32_t addr = (address & 0x7fff) * 3;

		return (uint32_t)(m_internal_ram_block1[addr + 0] << 16) |
						(m_internal_ram_block1[addr + 1]);
	}
	else {
		fatalerror("SHARC: PM Bus Read32 %08X at %08X\n", address, m_core->pc);
	}
}

void adsp21062_device::pm_write32(uint32_t address, uint32_t data)
{
	if (address >= 0x20000 && address < 0x28000)
	{
		uint32_t addr = (address & 0x7fff) * 3;

		m_internal_ram_block0[addr + 0] = (uint16_t)(data >> 16);
		m_internal_ram_block0[addr + 1] = (uint16_t)(data);
		return;
	}
	else if (address >= 0x28000 && address < 0x40000)
	{
		// block 1 is mirrored in 0x28000...2ffff, 0x30000...0x37fff and 0x38000...3ffff
		uint32_t addr = (address & 0x7fff) * 3;

		m_internal_ram_block1[addr + 0] = (uint16_t)(data >> 16);
		m_internal_ram_block1[addr + 1] = (uint16_t)(data);
		return;
	}
	else {
		fatalerror("SHARC: PM Bus Write32 %08X, %08X at %08X\n", address, data, m_core->pc);
	}
}

uint64_t adsp21062_device::pm_read48(uint32_t address)
{
	if ((address >= 0x20000 && address < 0x28000))
	{
		uint32_t addr = (address & 0x7fff) * 3;

		return ((uint64_t)(m_internal_ram_block0[addr + 0]) << 32) |
				((uint64_t)(m_internal_ram_block0[addr + 1]) << 16) |
				((uint64_t)(m_internal_ram_block0[addr + 2]) << 0);
	}
	else if (address >= 0x28000 && address < 0x40000)
	{
		// block 1 is mirrored in 0x28000...2ffff, 0x30000...0x37fff and 0x38000...3ffff
		uint32_t addr = (address & 0x7fff) * 3;

		return ((uint64_t)(m_internal_ram_block1[addr + 0]) << 32) |
				((uint64_t)(m_internal_ram_block1[addr + 1]) << 16) |
				((uint64_t)(m_internal_ram_block1[addr + 2]) << 0);
	}
	else {
		fatalerror("SHARC: PM Bus Read48 %08X at %08X\n", address, m_core->pc);
	}

	return 0;
}

void adsp21062_device::pm_write48(uint32_t address, uint64_t data)
{
	if ((address >= 0x20000 && address < 0x28000))
	{
		uint32_t addr = (address & 0x7fff) * 3;

		m_internal_ram_block0[addr + 0] = (uint16_t)(data >> 32);
		m_internal_ram_block0[addr + 1] = (uint16_t)(data >> 16);
		m_internal_ram_block0[addr + 2] = (uint16_t)(data);
		return;
	}
	else if (address >= 0x28000 && address < 0x40000)
	{
		// block 1 is mirrored in 0x28000...2ffff, 0x30000...0x37fff and 0x38000...3ffff
		uint32_t addr = (address & 0x7fff) * 3;

		m_internal_ram_block1[addr + 0] = (uint16_t)(data >> 32);
		m_internal_ram_block1[addr + 1] = (uint16_t)(data >> 16);
		m_internal_ram_block1[addr + 2] = (uint16_t)(data);
		return;
	}
	else {
		fatalerror("SHARC: PM Bus Write48 %08X, %04X%08X at %08X\n", address, (uint16_t)(data >> 32),(uint32_t)data, m_core->pc);
	}
}

uint32_t adsp21062_device::dm_read32(uint32_t address)
{
	if (address < 0x100)
	{
		return sharc_iop_r(address);
	}
	else if (address >= 0x20000 && address < 0x28000)
	{
		uint32_t addr = (address & 0x7fff) * 2;

		return (uint32_t)(m_internal_ram_block0[addr + 0] << 16) |
						(m_internal_ram_block0[addr + 1]);
	}
	else if (address >= 0x28000 && address < 0x40000)
	{
		// block 1 is mirrored in 0x28000...2ffff, 0x30000...0x37fff and 0x38000...3ffff
		uint32_t addr = (address & 0x7fff) * 2;

		return (uint32_t)(m_internal_ram_block1[addr + 0] << 16) |
						(m_internal_ram_block1[addr + 1]);
	}

	// short word addressing
	else if (address >= 0x40000 && address < 0x50000)
	{
		uint32_t addr = address & 0xffff;

		uint16_t r = m_internal_ram_block0[addr ^ 1];
		if (m_core->mode1 & 0x4000)
		{
			// sign-extend
			return (int32_t)(int16_t)(r);
		}
		else
		{
			return (uint32_t)(r);
		}
	}
	else if (address >= 0x50000 && address < 0x80000)
	{
		// block 1 is mirrored in 0x50000...5ffff, 0x60000...0x6ffff and 0x70000...7ffff
		uint32_t addr = address & 0xffff;

		uint16_t r = m_internal_ram_block1[addr ^ 1];
		if (m_core->mode1 & 0x4000)
		{
			// sign-extend
			return (int32_t)(int16_t)(r);
		}
		else
		{
			return (uint32_t)(r);
		}
	}

	return m_data->read_dword(address << 2);
}

void adsp21062_device::dm_write32(uint32_t address, uint32_t data)
{
	if (address < 0x100)
	{
		sharc_iop_w(address, data);
		return;
	}
	else if (address >= 0x20000 && address < 0x28000)
	{
		uint32_t addr = (address & 0x7fff) * 2;

		m_internal_ram_block0[addr + 0] = (uint16_t)(data >> 16);
		m_internal_ram_block0[addr + 1] = (uint16_t)(data);
		return;
	}
	else if (address >= 0x28000 && address < 0x40000)
	{
		// block 1 is mirrored in 0x28000...2ffff, 0x30000...0x37fff and 0x38000...3ffff
		uint32_t addr = (address & 0x7fff) * 2;

		m_internal_ram_block1[addr + 0] = (uint16_t)(data >> 16);
		m_internal_ram_block1[addr + 1] = (uint16_t)(data);
		return;
	}

	// short word addressing
	else if (address >= 0x40000 && address < 0x50000)
	{
		uint32_t addr = address & 0xffff;

		m_internal_ram_block0[addr ^ 1] = data;
		return;
	}
	else if (address >= 0x50000 && address < 0x80000)
	{
		// block 1 is mirrored in 0x50000...5ffff, 0x60000...0x6ffff and 0x70000...7ffff
		uint32_t addr = address & 0xffff;

		m_internal_ram_block1[addr ^ 1] = data;
		return;
	}

	m_data->write_dword(address << 2, data);
}
