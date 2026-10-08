// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "rom.h"

//-------------------------------------------------
//  device type definitions
//-------------------------------------------------

DEFINE_DEVICE_TYPE(C2COLOR_ROM_PLAIN,    c2color_rom_plain_device,    "c2color_rom_plain",    "Monon Color ROM cartridge")


//-------------------------------------------------
//  constructor
//-------------------------------------------------

c2color_rom_plain_device::c2color_rom_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: c2color_rom_plain_device(mconfig, C2COLOR_ROM_PLAIN, tag, owner, clock)
{
}

c2color_rom_plain_device::c2color_rom_plain_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_c2color_cart_interface(mconfig, *this)
	, m_spi(*this, "spi")
{
}


void c2color_rom_plain_device::device_add_mconfig(machine_config &config)
{
	GENERIC_SPI_FLASH(config, m_spi);
}

void c2color_rom_plain_device::device_start()
{
}
