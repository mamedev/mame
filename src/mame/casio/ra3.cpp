// license:BSD-3-Clause
// copyright-holders:Devin Acker
/*********************************************************************
    Casio CZ-series RAM cartridges
*********************************************************************/

#include "emu.h"
#include "ra3.h"

#include "emuopts.h"

#include <algorithm>

// device type definition
DEFINE_DEVICE_TYPE(CASIO_RA3, casio_ra3_device, "casio_ra3", "Casio RA-3 RAM cartridge")

/**************************************************************************/
casio_ra3_device::casio_ra3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CASIO_RA3, tag, owner, clock)
	, device_memcard_image_interface(mconfig, *this)
{
}

/**************************************************************************/
void casio_ra3_device::device_start()
{
	m_ram.resize(0x1000, 0xff);

	save_item(NAME(m_ram));
}

/**************************************************************************/
std::pair<std::error_condition, std::string> casio_ra3_device::call_load()
{
	const size_t size = m_ram.size();
	if (length() != size)
		return std::make_pair(image_error::INVALIDLENGTH, std::string());

	fseek(0, SEEK_SET);
	const size_t ret = fread(m_ram.data(), size);
	if (ret != size)
		return std::make_pair(std::errc::io_error, "Error reading file");

	return std::make_pair(std::error_condition(), std::string());
}

/**************************************************************************/
void casio_ra3_device::call_unload()
{
	fseek(0, SEEK_SET);
	fwrite(m_ram.data(), m_ram.size());
	std::fill(m_ram.begin(), m_ram.end(), 0xff);
}

/**************************************************************************/
std::pair<std::error_condition, std::string> casio_ra3_device::call_create(int format_type, util::option_resolution *format_options)
{
	std::fill(m_ram.begin(), m_ram.end(), 0);

	const size_t size = m_ram.size();
	const size_t ret = fwrite(m_ram.data(), size);
	if (ret != size)
		return std::make_pair(std::errc::io_error, "Error writing file");

	return std::make_pair(std::error_condition(), std::string());
}

/**************************************************************************/
u8 casio_ra3_device::read(offs_t offset)
{
	return m_ram[offset & 0xfff];
}

/**************************************************************************/
void casio_ra3_device::write(offs_t offset, u8 data)
{
	if (is_loaded())
		m_ram[offset & 0xfff] = data;
}
