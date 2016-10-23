// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 GCE Vectrex cart emulation

 TODO:
   - better understand how much SRAM is expected to be present by the homebrew using
     this cart type and use a RAM array instead of the ROM region for writes

 ***********************************************************************************************************/


#include "emu.h"
#include "rom.h"


//-------------------------------------------------
//  vectrex_rom_device - constructor
//-------------------------------------------------

const device_type VECTREX_ROM_STD = &device_creator<vectrex_rom_device>;
const device_type VECTREX_ROM_64K = &device_creator<vectrex_rom64k_device>;
const device_type VECTREX_ROM_SRAM = &device_creator<vectrex_sram_device>;


vectrex_rom_device::vectrex_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_vectrex_cart_interface(mconfig, *this)
{
}

vectrex_rom_device::vectrex_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
					: device_t(mconfig, VECTREX_ROM_STD, "Vectrex Standard Carts", tag, owner, clock, "vectrex_rom", __FILE__),
						device_vectrex_cart_interface(mconfig, *this)
{
}

vectrex_rom64k_device::vectrex_rom64k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
					: vectrex_rom_device(mconfig, VECTREX_ROM_64K, "Vectrex Carts w/ Bankswitch", tag, owner, clock, "vectrex_64k", __FILE__), m_bank(0)
				{
}

vectrex_sram_device::vectrex_sram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
					: vectrex_rom_device(mconfig, VECTREX_ROM_SRAM, "Vectrex Carts w/ SRAM", tag, owner, clock, "vectrex_sram", __FILE__)
{
}


void vectrex_rom64k_device::device_start()
{
	save_item(NAME(m_bank));
}

void vectrex_rom64k_device::device_reset()
{
	m_bank = 0;
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

uint8_t vectrex_rom_device::read_rom(address_space &space, offs_t offset, uint8_t mem_mask)
{
	if (offset < m_rom_size)
		return m_rom[offset];
	else
		return 0xff;
}


uint8_t vectrex_rom64k_device::read_rom(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return m_rom[(offset + m_bank * 0x8000) & (m_rom_size - 1)];
}

void vectrex_rom64k_device::write_bank(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_bank = data >> 6;
}

void vectrex_sram_device::write_ram(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_rom[offset & (m_rom_size - 1)] = data;
}
