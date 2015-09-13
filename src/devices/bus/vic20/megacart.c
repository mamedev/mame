// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Mega-Cart cartridge emulation

**********************************************************************/

#include "megacart.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type VIC20_MEGACART = &device_creator<vic20_megacart_device>;


//-------------------------------------------------
//  MACHINE_DRIVER( vic20_megacart )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( vic20_megacart )

MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor vic20_megacart_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( vic20_megacart );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic20_megacart_device - constructor
//-------------------------------------------------

vic20_megacart_device::vic20_megacart_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, VIC20_MEGACART, "Mega-Cart", tag, owner, clock, "megacart", __FILE__),
		device_vic20_expansion_card_interface(mconfig, *this),
		device_nvram_interface(mconfig, *this),
		m_nvram_en(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic20_megacart_device::device_start()
{
	m_nvram.allocate(0x2000);

	// state saving
	save_item(NAME(m_nvram_en));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vic20_megacart_device::device_reset()
{
}


//-------------------------------------------------
//  vic20_cd_r - cartridge data read
//-------------------------------------------------

UINT8 vic20_megacart_device::vic20_cd_r(address_space &space, offs_t offset, UINT8 data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3)
{
	if (!ram1 || !ram2 || !ram3 || !io2 || !io3)
	{
		if (m_nvram_en)
		{
			data = m_nvram[offset & 0x1fff];
		}
	}
	else if (!blk1 || !blk2 || !blk3)
	{
	}
	else if (!blk5)
	{
	}

	return data;
}


//-------------------------------------------------
//  vic20_cd_w - cartridge data write
//-------------------------------------------------

void vic20_megacart_device::vic20_cd_w(address_space &space, offs_t offset, UINT8 data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3)
{
	if (!ram1 || !ram2 || !ram3 || !io2)
	{
		if (m_nvram_en)
		{
			m_nvram[offset & 0x1fff] = data;
		}
	}
	else if (!blk1 || !blk2 || !blk3)
	{
	}
	else if (!blk5)
	{
	}
	else if (!io3)
	{
		if (m_nvram_en)
		{
			m_nvram[offset & 0x1fff] = data;
		}
	}
}
