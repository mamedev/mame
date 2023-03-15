// license:BSD-3-Clause
// copyright-holders:Fabio Priuli

#include "emu.h"
#include "rom.h"

//-------------------------------------------------
//  device type definitions
//-------------------------------------------------

DEFINE_DEVICE_TYPE(MONONCOL_ROM_PLAIN,    mononcol_rom_plain_device,    "mononcol_rom_plain",    "Monon Color ROM cartridge")


//-------------------------------------------------
//  constructor
//-------------------------------------------------

mononcol_rom_plain_device::mononcol_rom_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mononcol_rom_plain_device(mconfig, MONONCOL_ROM_PLAIN, tag, owner, clock)
{
}

mononcol_rom_plain_device::mononcol_rom_plain_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_mononcol_cart_interface(mconfig, *this)
	, m_spi(*this, "spi")
{
}


void mononcol_rom_plain_device::device_add_mconfig(machine_config &config)
{
	GENERIC_SPI_FLASH(config, m_spi, 0);
}

void mononcol_rom_plain_device::device_start()
{
}
