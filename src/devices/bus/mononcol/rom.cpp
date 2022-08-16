// license:BSD-3-Clause
// copyright-holders:Fabio Priuli

#include "emu.h"
#include "rom.h"

//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(MONONCOL_ROM_PLAIN,    mononcol_rom_plain_device,    "mononcol_rom_plain",    "Monon Color ROM cartridge")


mononcol_rom_device::mononcol_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock), device_mononcol_cart_interface(mconfig, *this)
{
}

mononcol_rom_plain_device::mononcol_rom_plain_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: mononcol_rom_device(mconfig, type, tag, owner, clock)
{
}

mononcol_rom_plain_device::mononcol_rom_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mononcol_rom_plain_device(mconfig, MONONCOL_ROM_PLAIN, tag, owner, clock)
{
}

/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

uint8_t mononcol_rom_plain_device::read_rom(offs_t offset)
{
	if (offset < m_rom_size)
		return m_rom[offset];
	else
		return 0xff;
}

uint16_t mononcol_rom_plain_device::read16_rom(offs_t offset, uint16_t mem_mask)
{
	uint16_t *ROM = (uint16_t *)m_rom;
	if (offset < m_rom_size/2)
		return ROM[offset];
	else
		return 0xffff;
}

uint32_t mononcol_rom_plain_device::read32_rom(offs_t offset, uint32_t mem_mask)
{
	uint32_t *ROM = (uint32_t *)m_rom;
	if (offset < m_rom_size/4)
		return ROM[offset];
	else
		return 0xffffffff;
}
