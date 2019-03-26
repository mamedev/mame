// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 Generic ROM emulation (for carts and ROM sockets)

 This offers generic access to a ROM

 generic_rom_plain  : returns 0xff when the system reads beyond the end of the ROM
 generic_rom_linear : maps linearly the ROM in the accessed area (i.e., read offset is masked with (ROM size - 1) )

 generic_romram_plain  : allows support for carts always containing ROM + RAM (e.g. X07)


 TODO:
   - possibly support linear mapping when non-power of 2 ROMs are mapped

 ***********************************************************************************************************/


#include "emu.h"
#include "rom.h"

//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(GENERIC_ROM_PLAIN,    generic_rom_plain_device,    "generic_rom_plain",    "Generic ROM (plain mapping)")
DEFINE_DEVICE_TYPE(GENERIC_ROM_LINEAR,   generic_rom_linear_device,   "generic_rom_linear",   "Generic ROM (linear mapping)")
DEFINE_DEVICE_TYPE(GENERIC_ROMRAM_PLAIN, generic_romram_plain_device, "generic_romram_plain", "Generic ROM + RAM (plain mapping)")


generic_rom_device::generic_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock), device_generic_cart_interface(mconfig, *this)
{
}

generic_rom_plain_device::generic_rom_plain_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: generic_rom_device(mconfig, type, tag, owner, clock)
{
}

generic_rom_plain_device::generic_rom_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: generic_rom_plain_device(mconfig, GENERIC_ROM_PLAIN, tag, owner, clock)
{
}

generic_rom_linear_device::generic_rom_linear_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: generic_rom_device(mconfig, GENERIC_ROM_LINEAR, tag, owner, clock)
{
}

generic_romram_plain_device::generic_romram_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: generic_rom_plain_device(mconfig, GENERIC_ROMRAM_PLAIN, tag, owner, clock)
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

uint8_t generic_rom_plain_device::read_rom(offs_t offset)
{
	if (offset < m_rom_size)
		return m_rom[offset];
	else
		return 0xff;
}

uint16_t generic_rom_plain_device::read16_rom(offs_t offset, uint16_t mem_mask)
{
	uint16_t *ROM = (uint16_t *)m_rom;
	if (offset < m_rom_size/2)
		return ROM[offset];
	else
		return 0xffff;
}

uint32_t generic_rom_plain_device::read32_rom(offs_t offset, uint32_t mem_mask)
{
	uint32_t *ROM = (uint32_t *)m_rom;
	if (offset < m_rom_size/4)
		return ROM[offset];
	else
		return 0xffffffff;
}


uint8_t generic_rom_linear_device::read_rom(offs_t offset)
{
	return m_rom[offset % m_rom_size];
}

uint16_t generic_rom_linear_device::read16_rom(offs_t offset, uint16_t mem_mask)
{
	uint16_t *ROM = (uint16_t *)m_rom;
	return ROM[offset % (m_rom_size/2)];
}

uint32_t generic_rom_linear_device::read32_rom(offs_t offset, uint32_t mem_mask)
{
	uint32_t *ROM = (uint32_t *)m_rom;
	return ROM[offset % (m_rom_size/4)];
}


uint8_t generic_romram_plain_device::read_ram(offs_t offset)
{
	if (offset < m_ram.size())
		return m_ram[offset];
	else
		return 0xff;
}

void generic_romram_plain_device::write_ram(offs_t offset, uint8_t data)
{
	if (offset < m_ram.size())
		m_ram[offset] = data;
}
