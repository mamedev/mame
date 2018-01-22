// license:BSD-3-Clause
// copyright-holders:Ville Linde
/* SHARC memory operations */

uint32_t adsp21062_device::pm_read32(uint32_t address)
{
	return m_program->read_dword(address);
}

void adsp21062_device::pm_write32(uint32_t address, uint32_t data)
{
	m_program->write_dword(address, data);
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
