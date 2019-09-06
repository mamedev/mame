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

DEFINE_DEVICE_TYPE(VECTREX_ROM_STD,  vectrex_rom_device,    "vectrex_rom",  "Vectrex Standard Carts")
DEFINE_DEVICE_TYPE(VECTREX_ROM_64K,  vectrex_rom64k_device, "vectrex_64k",  "Vectrex Carts w/Bankswitch")
DEFINE_DEVICE_TYPE(VECTREX_ROM_SRAM, vectrex_sram_device,   "vectrex_sram", "Vectrex Carts w/SRAM")


vectrex_rom_device::vectrex_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock), device_vectrex_cart_interface(mconfig, *this)
{
}

vectrex_rom_device::vectrex_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vectrex_rom_device(mconfig, VECTREX_ROM_STD, tag, owner, clock)
{
}

vectrex_rom64k_device::vectrex_rom64k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vectrex_rom_device(mconfig, VECTREX_ROM_64K, tag, owner, clock), m_bank(0)
{
}

vectrex_sram_device::vectrex_sram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vectrex_rom_device(mconfig, VECTREX_ROM_SRAM, tag, owner, clock)
{
}


void vectrex_rom64k_device::device_start()
{
	save_item(NAME(m_bank));
}

void vectrex_rom64k_device::device_reset()
{
	// Resetting to 1 instead of 0 fixes 64KiB cartridges that don't have a workaround
	// for the fact that MAME does not currently emulate the pull-up resistor on the 6522's PB6 line.
	// TODO: correctly emulate PB6 pull-up behavior (line should be high whenever PB6 set to input).
	m_bank = 1;
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
