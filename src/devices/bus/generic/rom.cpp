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

const device_type GENERIC_ROM_PLAIN  = &device_creator<generic_rom_plain_device>;
const device_type GENERIC_ROM_LINEAR = &device_creator<generic_rom_linear_device>;
const device_type GENERIC_ROMRAM_PLAIN = &device_creator<generic_romram_plain_device>;


generic_rom_device::generic_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_generic_cart_interface(mconfig, *this)
{
}

generic_rom_plain_device::generic_rom_plain_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source)
					: generic_rom_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

generic_rom_plain_device::generic_rom_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
					: generic_rom_device(mconfig, GENERIC_ROM_PLAIN, "Generic ROM (plain mapping)", tag, owner, clock, "generic_rom_plain", __FILE__)
{
}

generic_rom_linear_device::generic_rom_linear_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
					: generic_rom_device(mconfig, GENERIC_ROM_LINEAR, "Generic ROM (linear mapping)", tag, owner, clock, "generic_rom_linear", __FILE__)
{
}

generic_romram_plain_device::generic_romram_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
					: generic_rom_plain_device(mconfig, GENERIC_ROMRAM_PLAIN, "Generic ROM + RAM (plain mapping)", tag, owner, clock, "generic_romram_plain", __FILE__)
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

uint8_t generic_rom_plain_device::read_rom(address_space &space, offs_t offset, uint8_t mem_mask)
{
	if (offset < m_rom_size)
		return m_rom[offset];
	else
		return 0xff;
}

uint16_t generic_rom_plain_device::read16_rom(address_space &space, offs_t offset, uint16_t mem_mask)
{
	uint16_t *ROM = (uint16_t *)m_rom;
	if (offset < m_rom_size/2)
		return ROM[offset];
	else
		return 0xffff;
}

uint32_t generic_rom_plain_device::read32_rom(address_space &space, offs_t offset, uint32_t mem_mask)
{
	uint32_t *ROM = (uint32_t *)m_rom;
	if (offset < m_rom_size/4)
		return ROM[offset];
	else
		return 0xffffffff;
}


uint8_t generic_rom_linear_device::read_rom(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return m_rom[offset % m_rom_size];
}

uint16_t generic_rom_linear_device::read16_rom(address_space &space, offs_t offset, uint16_t mem_mask)
{
	uint16_t *ROM = (uint16_t *)m_rom;
	return ROM[offset % (m_rom_size/2)];
}

uint32_t generic_rom_linear_device::read32_rom(address_space &space, offs_t offset, uint32_t mem_mask)
{
	uint32_t *ROM = (uint32_t *)m_rom;
	return ROM[offset % (m_rom_size/4)];
}


uint8_t generic_romram_plain_device::read_ram(address_space &space, offs_t offset, uint8_t mem_mask)
{
	if (offset < m_ram.size())
		return m_ram[offset];
	else
		return 0xff;
}

void generic_romram_plain_device::write_ram(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	if (offset < m_ram.size())
		m_ram[offset] = data;
}
