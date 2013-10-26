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

const device_type Z88_32K_ROM = &device_creator<z88_32k_rom_device>;
const device_type Z88_128K_ROM = &device_creator<z88_128k_rom_device>;
const device_type Z88_256K_ROM = &device_creator<z88_256k_rom_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  z88_32k_rom_device - constructor
//-------------------------------------------------

z88_32k_rom_device::z88_32k_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, Z88_32K_ROM, "Z88 32KB ROM", tag, owner, clock, "z88_32k_rom", __FILE__),
		device_z88cart_interface( mconfig, *this )
{
}

z88_32k_rom_device::z88_32k_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
		: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_z88cart_interface( mconfig, *this )
{
}

//-------------------------------------------------
//  z88_128k_rom_device - constructor
//-------------------------------------------------

z88_128k_rom_device::z88_128k_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: z88_32k_rom_device(mconfig, Z88_128K_ROM, "Z88 128KB ROM", tag, owner, clock, "z88_128k_rom", __FILE__)
{
}

//-------------------------------------------------
//  z88_256k_rom_device - constructor
//-------------------------------------------------

z88_256k_rom_device::z88_256k_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: z88_32k_rom_device(mconfig, Z88_256K_ROM, "Z88 256KB ROM", tag, owner, clock, "z88_256k_rom", __FILE__)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void z88_32k_rom_device::device_start()
{
	m_rom = machine().memory().region_alloc(tag(), get_cart_size(), 1, ENDIANNESS_LITTLE)->base();
}

/*-------------------------------------------------
    get_cart_base
-------------------------------------------------*/

UINT8* z88_32k_rom_device::get_cart_base()
{
	return m_rom;
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

READ8_MEMBER(z88_32k_rom_device::read)
{
	return m_rom[offset & (get_cart_size() - 1)];
}
