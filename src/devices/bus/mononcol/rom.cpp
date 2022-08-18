// license:BSD-3-Clause
// copyright-holders:Fabio Priuli

#include "emu.h"
#include "rom.h"

//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(MONONCOL_ROM_PLAIN,    mononcol_rom_plain_device,    "mononcol_rom_plain",    "Monon Color ROM cartridge")

void mononcol_rom_device::device_start()
{
}

mononcol_rom_device::mononcol_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock), device_mononcol_cart_interface(mconfig, *this)
{
}

mononcol_rom_plain_device::mononcol_rom_plain_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: mononcol_rom_device(mconfig, type, tag, owner, clock)
	, m_spi(*this, "spi")
{
}

mononcol_rom_plain_device::mononcol_rom_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mononcol_rom_plain_device(mconfig, MONONCOL_ROM_PLAIN, tag, owner, clock)
{
}


void mononcol_rom_plain_device::device_add_mconfig(machine_config &config)
{
	GENERIC_SPI_FLASH(config, m_spi, 0);
}
