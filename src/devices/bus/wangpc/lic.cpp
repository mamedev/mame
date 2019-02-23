// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Wang PC Network card emulation

**********************************************************************/

#include "emu.h"
#include "lic.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define OPTION_ID       0x30



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(WANGPC_LIC, wangpc_lic_device, "wangpc_lic", "Wang PC-PM070 Local Interconnect")


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

const tiny_rom_entry *wangpc_lic_device::device_rom_region() const
{
	return ROM_NAME( wangpc_lic );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void wangpc_lic_device::device_add_mconfig(machine_config &config)
{
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  wangpc_lic_device - constructor
//-------------------------------------------------

wangpc_lic_device::wangpc_lic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, WANGPC_LIC, tag, owner, clock),
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

uint16_t wangpc_lic_device::wangpcbus_mrdc_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0xffff;

	return data;
}


//-------------------------------------------------
//  wangpcbus_amwc_w - memory write
//-------------------------------------------------

void wangpc_lic_device::wangpcbus_amwc_w(address_space &space, offs_t offset, uint16_t mem_mask, uint16_t data)
{
}


//-------------------------------------------------
//  wangpcbus_iorc_r - I/O read
//-------------------------------------------------

uint16_t wangpc_lic_device::wangpcbus_iorc_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0xffff;

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

void wangpc_lic_device::wangpcbus_aiowc_w(address_space &space, offs_t offset, uint16_t mem_mask, uint16_t data)
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
