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

const device_type Z88_1024K_FLASH =  &device_creator<z88_1024k_flash_device>;

//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( z88_flash )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT(z88_flash)
	MCFG_INTEL_E28F008SA_ADD(FLASH_TAG)
MACHINE_CONFIG_END


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  z88_1024k_flash_device - constructor
//-------------------------------------------------

z88_1024k_flash_device::z88_1024k_flash_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, Z88_1024K_FLASH, "Z88 1024KB Flash", tag, owner, clock, "z88_1024k_flash", __FILE__),
		device_z88cart_interface( mconfig, *this ),
		m_flash(*this, FLASH_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void z88_1024k_flash_device::device_start()
{
}


//-------------------------------------------------
//  device_mconfig_additions
//-------------------------------------------------

machine_config_constructor z88_1024k_flash_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( z88_flash );
}

/*-------------------------------------------------
    get_cart_base
-------------------------------------------------*/

UINT8* z88_1024k_flash_device::get_cart_base()
{
	return (UINT8*)m_flash->space().get_read_ptr(0);
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
