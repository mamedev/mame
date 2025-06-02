// license:BSD-3-Clause
// copyright-holders:Ville Linde
/* SHARC memory operations */

// When PM bus is used to transfer 32-bit data, it is aligned to the upper 32 bits of the bus
uint32_t adsp21062_device::pm_read32(uint32_t address)
{
	return m_program->read_qword(address) >> 16;
}

void adsp21062_device::pm_write32(uint32_t address, uint32_t data)
{
	u64 res = ((u64)data << 16) | (m_program->read_qword(address) & 0xffff);
	m_program->write_qword(address, res);
}

uint64_t adsp21062_device::pm_read48(uint32_t address)
{
	return m_program->read_qword(address);
}

void adsp21062_device::pm_write48(uint32_t address, uint64_t data)
{
	m_program->write_qword(address, data);
}

uint32_t adsp21062_device::dm_read32(uint32_t address)
{
	return m_data->read_dword(address);
}

void adsp21062_device::dm_write32(uint32_t address, uint32_t data)
{
	m_data->write_dword(address, data);
}
