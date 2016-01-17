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


vectrex_rom_device::vectrex_rom_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_vectrex_cart_interface(mconfig, *this)
{
}

vectrex_rom_device::vectrex_rom_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, VECTREX_ROM_STD, "Vectrex Standard Carts", tag, owner, clock, "vectrex_rom", __FILE__),
						device_vectrex_cart_interface(mconfig, *this)
{
}

vectrex_rom64k_device::vectrex_rom64k_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: vectrex_rom_device(mconfig, VECTREX_ROM_64K, "Vectrex Carts w/ Bankswitch", tag, owner, clock, "vectrex_64k", __FILE__), m_bank(0)
				{
}

vectrex_sram_device::vectrex_sram_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
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

READ8_MEMBER(vectrex_rom_device::read_rom)
{
	if (offset < m_rom_size)
		return m_rom[offset];
	else
		return 0xff;
}


READ8_MEMBER(vectrex_rom64k_device::read_rom)
{
	return m_rom[(offset + m_bank * 0x8000) & (m_rom_size - 1)];
}

WRITE8_MEMBER(vectrex_rom64k_device::write_bank)
{
	m_bank = data >> 6;
}

WRITE8_MEMBER(vectrex_sram_device::write_ram)
{
	m_rom[offset & (m_rom_size - 1)] = data;
}
