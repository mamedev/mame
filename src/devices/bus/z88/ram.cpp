// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    z88.c

    Z88 RAM cartridges emulation

***************************************************************************/

#include "emu.h"
#include "ram.h"

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type Z88_32K_RAM =  &device_creator<z88_32k_ram_device>;
const device_type Z88_128K_RAM = &device_creator<z88_128k_ram_device>;
const device_type Z88_512K_RAM = &device_creator<z88_512k_ram_device>;
const device_type Z88_1024K_RAM = &device_creator<z88_1024k_ram_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  z88_32k_ram_device - constructor
//-------------------------------------------------

z88_32k_ram_device::z88_32k_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, Z88_32K_RAM, "Z88 32KB RAM", tag, owner, clock, "z88_32k_ram", __FILE__),
		device_z88cart_interface( mconfig, *this ), m_ram(nullptr)
	{
}

z88_32k_ram_device::z88_32k_ram_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
		: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_z88cart_interface( mconfig, *this ), m_ram(nullptr)
	{
}

//-------------------------------------------------
//  z88_128k_ram_device - constructor
//-------------------------------------------------

z88_128k_ram_device::z88_128k_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: z88_32k_ram_device(mconfig, Z88_128K_RAM, "Z88 128KB RAM", tag, owner, clock, "z88_128k_ram", __FILE__)
{
}

//-------------------------------------------------
//  z88_512k_ram_device - constructor
//-------------------------------------------------

z88_512k_ram_device::z88_512k_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: z88_32k_ram_device(mconfig, Z88_512K_RAM, "Z88 512KB RAM", tag, owner, clock, "z88_512k_ram", __FILE__)
{
}

//-------------------------------------------------
//  z88_1024k_ram_device - constructor
//-------------------------------------------------

z88_1024k_ram_device::z88_1024k_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: z88_32k_ram_device(mconfig, Z88_1024K_RAM, "Z88 1024KB RAM", tag, owner, clock, "z88_1024k_ram", __FILE__)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void z88_32k_ram_device::device_start()
{
	m_ram = machine().memory().region_alloc(tag(), get_cart_size(), 1, ENDIANNESS_LITTLE)->base();
	memset(m_ram, 0, get_cart_size());
}

/*-------------------------------------------------
    get_cart_base
-------------------------------------------------*/

UINT8* z88_32k_ram_device::get_cart_base()
{
	return m_ram;
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

READ8_MEMBER(z88_32k_ram_device::read)
{
	return m_ram[offset & (get_cart_size() - 1)];
}

/*-------------------------------------------------
    write
-------------------------------------------------*/

WRITE8_MEMBER(z88_32k_ram_device::write)
{
	m_ram[offset & (get_cart_size() - 1)] = data;
}
