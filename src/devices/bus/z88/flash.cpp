// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    flash.c

    Z88 Flash cartridges emulation

***************************************************************************/

#include "emu.h"
#include "flash.h"


//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define FLASH_TAG   "flash"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(Z88_1024K_FLASH, z88_1024k_flash_device, "z88_1024k_flash", "Z88 1024KB Flash")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  z88_1024k_flash_device - constructor
//-------------------------------------------------

z88_1024k_flash_device::z88_1024k_flash_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, Z88_1024K_FLASH, tag, owner, clock)
	, device_z88cart_interface(mconfig, *this)
	, m_flash(*this, FLASH_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void z88_1024k_flash_device::device_start()
{
}


//-------------------------------------------------
//  device_add_mconfig
//-------------------------------------------------

void z88_1024k_flash_device::device_add_mconfig(machine_config &config)
{
	INTEL_E28F008SA(config, FLASH_TAG);
}

/*-------------------------------------------------
    get_cart_base
-------------------------------------------------*/

uint8_t* z88_1024k_flash_device::get_cart_base()
{
	return m_flash->base();
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

READ8_MEMBER(z88_1024k_flash_device::read)
{
	return m_flash->read(offset & (get_cart_size() - 1));
}

/*-------------------------------------------------
    write
-------------------------------------------------*/

WRITE8_MEMBER(z88_1024k_flash_device::write)
{
	m_flash->write(offset & (get_cart_size() - 1), data);
}
