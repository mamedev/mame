// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Ross cartridge emulation

**********************************************************************/

#include "ross.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_ROSS = &device_creator<c64_ross_cartridge_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_ross_cartridge_device - constructor
//-------------------------------------------------

c64_ross_cartridge_device::c64_ross_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_ROSS, "C64 Ross cartridge", tag, owner, clock, "c64_ross", __FILE__),
	device_c64_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_ross_cartridge_device::device_start()
{
	// state saving
	save_item(NAME(m_bank));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_ross_cartridge_device::device_reset()
{
	m_exrom = 0;
	m_game = 0;

	m_bank = 0;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_ross_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml || !romh)
	{
		offs_t addr = (m_bank << 14) | (offset & 0x3fff);

		data = m_roml[addr & m_roml.mask()];
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_ross_cartridge_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1)
	{
		m_bank = 1;
	}
	else if (!io2)
	{
		m_exrom = 1;
		m_game = 1;
	}
}
