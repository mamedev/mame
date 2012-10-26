/**********************************************************************

    GLA 24K RAM cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "cbm2_24k.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type CBM2_24K = &device_creator<cbm2_24k_cartridge_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cbm2_24k_cartridge_device - constructor
//-------------------------------------------------

cbm2_24k_cartridge_device::cbm2_24k_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, CBM2_24K, "24K RAM/ROM cartridge", tag, owner, clock),
	device_cbm2_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cbm2_24k_cartridge_device::device_start()
{
	cbm2_ram_pointer(machine(), 0x6000);
}


//-------------------------------------------------
//  cbm2_bd_r - cartridge data read
//-------------------------------------------------

UINT8 cbm2_24k_cartridge_device::cbm2_bd_r(address_space &space, offs_t offset, UINT8 data, int csbank1, int csbank2, int csbank3)
{
	if (!csbank1)
	{
		data = m_ram[offset];
	}
	else if (!csbank2)
	{
		data = m_ram[0x2000 | offset];
	}
	else if (!csbank3)
	{
		data = m_ram[0x4000 | offset];
	}

	return data;
}


//-------------------------------------------------
//  cbm2_bd_w - cartridge data write
//-------------------------------------------------

void cbm2_24k_cartridge_device::cbm2_bd_w(address_space &space, offs_t offset, UINT8 data, int csbank1, int csbank2, int csbank3)
{
	if (!csbank1)
	{
		m_ram[offset] = data;
	}
	else if (!csbank2)
	{
		m_ram[0x2000 | offset] = data;
	}
	else if (!csbank3)
	{
		m_ram[0x4000 | offset] = data;
	}
}
