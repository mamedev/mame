// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Wang PC Network card emulation

**********************************************************************/

#include "lic.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define OPTION_ID       0x30



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type WANGPC_LIC = &device_creator<wangpc_lic_device>;


//-------------------------------------------------
//  ROM( wangpc_lic )
//-------------------------------------------------

ROM_START( wangpc_lic )
	ROM_REGION( 0x1000, "network", 0 )
	ROM_LOAD( "7025.l22", 0x0000, 0x1000, CRC(487e5f04) SHA1(81e52e70e0c6e34715119b121ec19a7758cd6772) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *wangpc_lic_device::device_rom_region() const
{
	return ROM_NAME( wangpc_lic );
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( wangpc_lic )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( wangpc_lic )
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor wangpc_lic_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( wangpc_lic );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  wangpc_lic_device - constructor
//-------------------------------------------------

wangpc_lic_device::wangpc_lic_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, WANGPC_LIC, "Wang PC-PM070", tag, owner, clock, "wangpc_lic", __FILE__),
	device_wangpcbus_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void wangpc_lic_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void wangpc_lic_device::device_reset()
{
}


//-------------------------------------------------
//  wangpcbus_mrdc_r - memory read
//-------------------------------------------------

UINT16 wangpc_lic_device::wangpcbus_mrdc_r(address_space &space, offs_t offset, UINT16 mem_mask)
{
	UINT16 data = 0xffff;

	return data;
}


//-------------------------------------------------
//  wangpcbus_amwc_w - memory write
//-------------------------------------------------

void wangpc_lic_device::wangpcbus_amwc_w(address_space &space, offs_t offset, UINT16 mem_mask, UINT16 data)
{
}


//-------------------------------------------------
//  wangpcbus_iorc_r - I/O read
//-------------------------------------------------

UINT16 wangpc_lic_device::wangpcbus_iorc_r(address_space &space, offs_t offset, UINT16 mem_mask)
{
	UINT16 data = 0xffff;

	if (sad(offset))
	{
		switch (offset & 0x7f)
		{
		case 0xfe/2:
			data = 0xff00 | OPTION_ID;
			break;
		}
	}

	return data;
}


//-------------------------------------------------
//  wangpcbus_aiowc_w - I/O write
//-------------------------------------------------

void wangpc_lic_device::wangpcbus_aiowc_w(address_space &space, offs_t offset, UINT16 mem_mask, UINT16 data)
{
	if (sad(offset))
	{
		switch (offset & 0x7f)
		{
		case 0xfc/2:
			device_reset();
			break;
		}
	}
}
