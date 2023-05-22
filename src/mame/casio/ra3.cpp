// license:BSD-3-Clause
// copyright-holders:Devin Acker
/*********************************************************************
    Casio CZ-series RAM cartridges
*********************************************************************/

#include "emu.h"
#include "ra3.h"

#include "emuopts.h"

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
	save_item(NAME(m_ram));
}

/**************************************************************************/
std::pair<std::error_condition, std::string> casio_ra3_device::call_load()
{
	if (length() != 0x1000)
		return std::make_pair(image_error::INVALIDLENGTH, std::string());

	fseek(0, SEEK_SET);
	size_t ret = fread(m_ram, 0x1000);
	if (ret != 0x1000)
		return std::make_pair(image_error::UNSPECIFIED, "Error reading file");

	return std::make_pair(std::error_condition(), std::string());
}

/**************************************************************************/
void casio_ra3_device::call_unload()
{
	fseek(0, SEEK_SET);
	fwrite(m_ram, 0x1000);
}

/**************************************************************************/
std::pair<std::error_condition, std::string> casio_ra3_device::call_create(int format_type, util::option_resolution *format_options)
{
	memset(m_ram, 0, 0x1000);

	size_t const ret = fwrite(m_ram, 0x1000);
	if (ret != 0x1000)
		return std::make_pair(image_error::UNSPECIFIED, "Error writing file");

	return std::make_pair(std::error_condition(), std::string());
}

/**************************************************************************/
u8 casio_ra3_device::read(offs_t offset)
{
	return is_loaded() ? m_ram[offset & 0xfff] : 0xff;
}

/**************************************************************************/
void casio_ra3_device::write(offs_t offset, u8 data)
{
	if (is_loaded())
		m_ram[offset & 0xfff] = data;
}
