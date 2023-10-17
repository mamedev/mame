// license:BSD-3-Clause
// copyright-holders:Phil Bennett
/***********************************************************************************************************

    Casio Loopy cart emulation

 ***********************************************************************************************************/


#include "emu.h"
#include "rom.h"


//-------------------------------------------------
//  casloopy_rom_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(CASLOOPY_ROM_STD,     casloopy_rom_device,    "casloopy_rom",    "Casio Loopy Standard Cart")
DEFINE_DEVICE_TYPE(CASLOOPY_ROM_ADPCM,   casloopy_adpcm_device,  "casloopy_adpcm",  "Casio Loopy ADPCM Cart")


casloopy_rom_device::casloopy_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock), device_casloopy_cart_interface(mconfig, *this)
{
}

casloopy_rom_device::casloopy_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: casloopy_rom_device(mconfig, CASLOOPY_ROM_STD, tag, owner, clock)
{
}

casloopy_adpcm_device::casloopy_adpcm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: casloopy_rom_device(mconfig, CASLOOPY_ROM_ADPCM, tag, owner, clock)
{
}

void casloopy_adpcm_device::device_add_mconfig(machine_config &config)
{
	// TODO: Add support for the MSM6653A ADPCM chip, or samples
	// and route to the speakers
}


/*-------------------------------------------------
 read accessors
 -------------------------------------------------*/

uint16_t casloopy_rom_device::read_rom(offs_t offset)
{
	if (offset < m_rom_size)
		return m_rom[offset];
	else
		return 0xffff;
}

uint8_t casloopy_rom_device::read_ram(offs_t offset)
{
	return m_nvram[offset & (m_nvram.size() - 1)];
}


/*-------------------------------------------------
 write accessors
 -------------------------------------------------*/

void casloopy_rom_device::write_ram(offs_t offset, u8 data)
{
	m_nvram[offset & (m_nvram.size() - 1)] = data;
}

