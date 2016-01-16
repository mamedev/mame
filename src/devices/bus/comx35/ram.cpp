// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    COMX-35 RAM Card emulation

**********************************************************************/

#include "ram.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define RAM_SIZE    0x8000



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type COMX_RAM = &device_creator<comx_ram_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  comx_ram_device - constructor
//-------------------------------------------------

comx_ram_device::comx_ram_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, COMX_RAM, "COMX-35 RAM Card", tag, owner, clock, "comx_ram", __FILE__),
	device_comx_expansion_card_interface(mconfig, *this),
	m_ram(*this, "ram"),
	m_bank(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void comx_ram_device::device_start()
{
	m_ram.allocate(RAM_SIZE);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void comx_ram_device::device_reset()
{
}


//-------------------------------------------------
//  comx_mrd_r - memory read
//-------------------------------------------------

UINT8 comx_ram_device::comx_mrd_r(address_space &space, offs_t offset, int *extrom)
{
	UINT8 data = 0;

	if (offset >= 0xc000 && offset < 0xd000)
	{
		data = m_ram[(m_bank << 12) | (offset & 0xfff)];
	}

	return data;
}


//-------------------------------------------------
//  comx_mwr_w - memory write
//-------------------------------------------------

void comx_ram_device::comx_mwr_w(address_space &space, offs_t offset, UINT8 data)
{
	if (offset >= 0xc000 && offset < 0xd000)
	{
		m_ram[(m_bank << 12) | (offset & 0xfff)] = data;
	}
}


//-------------------------------------------------
//  comx_io_w - I/O write
//-------------------------------------------------

void comx_ram_device::comx_io_w(address_space &space, offs_t offset, UINT8 data)
{
	if (offset == 1)
	{
		m_bank = (data >> 4) & 0x03;
	}
}
