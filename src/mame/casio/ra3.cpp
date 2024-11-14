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
DEFINE_DEVICE_TYPE(CASIO_RA5, casio_ra5_device, "casio_ra5", "Casio RA-5 RAM cartridge")
DEFINE_DEVICE_TYPE(CASIO_RA6, casio_ra6_device, "casio_ra6", "Casio RA-6 RAM cartridge")

/**************************************************************************/
casio_ram_cart_device::casio_ram_cart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, unsigned max_size)
	: device_t(mconfig, type, tag, owner, clock)
	, device_memcard_image_interface(mconfig, *this)
	, m_max_size(max_size)
{
	m_mask = max_size - 1;
	m_size = 0;

	// only power-of-two sizes are supported
	assert(!(m_max_size & m_mask));
}

/**************************************************************************/
casio_ra3_device::casio_ra3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: casio_ram_cart_device(mconfig, CASIO_RA3, tag, owner, clock, 0x1000)
{
}

/**************************************************************************/
casio_ra5_device::casio_ra5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: casio_ram_cart_device(mconfig, CASIO_RA5, tag, owner, clock, 0x2000)
{
}

/**************************************************************************/
casio_ra6_device::casio_ra6_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: casio_ram_cart_device(mconfig, CASIO_RA6, tag, owner, clock, 0x4000)
{
}

/**************************************************************************/
void casio_ram_cart_device::device_start()
{
	m_ram.resize(m_max_size, 0xff);

	save_item(NAME(m_ram));
}

/**************************************************************************/
std::pair<std::error_condition, std::string> casio_ram_cart_device::call_load()
{
	const u32 size = loaded_through_softlist() ? get_software_region_length("rom") : length();

	// size must be a power of two and at least 4kb
	if (size < 0x1000 || (size & (size - 1)))
		return std::make_pair(image_error::INVALIDLENGTH, std::string());

	// allow loading larger than the maximum size (e.g. for CZ-1 ROM carts that use oversized ROMs)
	m_size = std::min(size, m_max_size);
	m_mask = m_size - 1;

	if (loaded_through_softlist())
	{
		memcpy(m_ram.data(), get_software_region("rom"), m_size);
	}
	else
	{
		fseek(0, SEEK_SET);
		const size_t ret = fread(m_ram.data(), m_size);
		if (ret != m_size)
			return std::make_pair(std::errc::io_error, "Error reading file");
	}

	return std::make_pair(std::error_condition(), std::string());
}

/**************************************************************************/
void casio_ram_cart_device::call_unload()
{
	if (is_loaded())
	{
		fseek(0, SEEK_SET);
		fwrite(m_ram.data(), m_size);
	}
	std::fill(m_ram.begin(), m_ram.end(), 0xff);
}

/**************************************************************************/
std::pair<std::error_condition, std::string> casio_ram_cart_device::call_create(int format_type, util::option_resolution *format_options)
{
	std::fill(m_ram.begin(), m_ram.end(), 0);

	m_size = m_max_size;
	m_mask = m_max_size - 1;
	const size_t ret = fwrite(m_ram.data(), m_size);
	if (ret != m_size)
		return std::make_pair(std::errc::io_error, "Error writing file");

	return std::make_pair(std::error_condition(), std::string());
}

/**************************************************************************/
u8 casio_ram_cart_device::read(offs_t offset)
{
	return m_ram[offset & m_mask];
}

/**************************************************************************/
void casio_ram_cart_device::write(offs_t offset, u8 data)
{
	if (is_loaded())
		m_ram[offset & m_mask] = data;
}
