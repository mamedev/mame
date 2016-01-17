// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Coleco Adam Internal 64KB RAM Expansion emulation

**********************************************************************/

#include "ram.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ADAM_RAM = &device_creator<adam_ram_expansion_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  adam_ram_expansion_device - constructor
//-------------------------------------------------

adam_ram_expansion_device::adam_ram_expansion_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, ADAM_RAM, "Adam 64KB RAM expansion", tag, owner, clock, "adam_ram", __FILE__),
	device_adam_expansion_slot_card_interface(mconfig, *this),
	m_ram(*this, "ram")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void adam_ram_expansion_device::device_start()
{
	m_ram.allocate(0x10000);
}


//-------------------------------------------------
//  adam_bd_r - buffered data read
//-------------------------------------------------

UINT8 adam_ram_expansion_device::adam_bd_r(address_space &space, offs_t offset, UINT8 data, int bmreq, int biorq, int aux_rom_cs, int cas1, int cas2)
{
	if (!cas2)
	{
		data = m_ram[offset];
	}

	return data;
}


//-------------------------------------------------
//  adam_bd_w - buffered data write
//-------------------------------------------------

void adam_ram_expansion_device::adam_bd_w(address_space &space, offs_t offset, UINT8 data, int bmreq, int biorq, int aux_rom_cs, int cas1, int cas2)
{
	if (!cas2)
	{
		m_ram[offset] = data;
	}
}
