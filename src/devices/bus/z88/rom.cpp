// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    rom.c

    Z88 ROM cartridges emulation

***************************************************************************/

#include "emu.h"
#include "rom.h"

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(Z88_32K_ROM,  z88_32k_rom_device,  "z88_32k_rom",  "Z88 32KB ROM")
DEFINE_DEVICE_TYPE(Z88_128K_ROM, z88_128k_rom_device, "z88_128k_rom", "Z88 128KB ROM")
DEFINE_DEVICE_TYPE(Z88_256K_ROM, z88_256k_rom_device, "z88_256k_rom", "Z88 256KB ROM")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  z88_32k_rom_device - constructor
//-------------------------------------------------

z88_32k_rom_device::z88_32k_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z88_32k_rom_device(mconfig, Z88_32K_ROM, tag, owner, clock)
{
}

z88_32k_rom_device::z88_32k_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
	, device_z88cart_interface(mconfig, *this)
	, m_rom(nullptr)
	, m_vpp_state(0)
	, m_modified(false)
{
}

//-------------------------------------------------
//  z88_128k_rom_device - constructor
//-------------------------------------------------

z88_128k_rom_device::z88_128k_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z88_32k_rom_device(mconfig, Z88_128K_ROM, tag, owner, clock)
{
}

//-------------------------------------------------
//  z88_256k_rom_device - constructor
//-------------------------------------------------

z88_256k_rom_device::z88_256k_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z88_32k_rom_device(mconfig, Z88_256K_ROM, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void z88_32k_rom_device::device_start()
{
	m_rom = machine().memory().region_alloc(tag(), get_cart_size(), 1, ENDIANNESS_LITTLE)->base();
	std::fill_n(m_rom, get_cart_size(), 0xff);

	save_item(NAME(m_vpp_state));
	save_item(NAME(m_modified));
}

/*-------------------------------------------------
    get_cart_base
-------------------------------------------------*/

uint8_t* z88_32k_rom_device::get_cart_base()
{
	return m_rom;
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

uint8_t z88_32k_rom_device::read(offs_t offset)
{
	return m_rom[offset & (get_cart_size() - 1)];
}

/*-------------------------------------------------
    write
-------------------------------------------------*/

void z88_32k_rom_device::write(offs_t offset, uint8_t data)
{
	if (m_vpp_state)
	{
		const uint32_t offset_mask = get_cart_size() - 1;
		if (m_rom[offset & offset_mask] & ~data)
			m_modified = true;

		m_rom[offset & offset_mask] &= data;
	}
}

