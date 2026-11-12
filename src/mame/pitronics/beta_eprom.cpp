// license:BSD-3-Clause
// copyright-holders:Curt Coder
#include "emu.h"
#include "beta_eprom.h"


DEFINE_DEVICE_TYPE(BETA_EPROM, beta_eprom_device, "beta_eprom", "Beta EPROM socket")

beta_eprom_device::beta_eprom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, BETA_EPROM, tag, owner, clock),
	device_memcard_image_interface(mconfig, *this),
	m_rom(0x800, 0xff)
{
}

void beta_eprom_device::device_start()
{
	save_item(NAME(m_rom));
}

std::pair<std::error_condition, std::string> beta_eprom_device::call_load()
{
	if (length() != 0x800)
		return std::make_pair(image_error::INVALIDLENGTH, "Unsupported image size (only 2K images are supported)");

	if (fread(&m_rom[0], 0x800) != 0x800)
		return std::make_pair(image_error::UNSPECIFIED, std::string());

	return std::make_pair(std::error_condition(), std::string());
}

void beta_eprom_device::call_unload()
{
	if (!is_readonly())
	{
		fseek(0, SEEK_SET);
		fwrite(&m_rom[0], 0x800);
	}
}
