// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Kingsoft cartridge emulation

**********************************************************************/

#include "kingsoft.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_KINGSOFT = &device_creator<c64_kingsoft_cartridge_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_kingsoft_cartridge_device - constructor
//-------------------------------------------------

c64_kingsoft_cartridge_device::c64_kingsoft_cartridge_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_KINGSOFT, "C64 Kingsoft cartridge", tag, owner, clock, "c64_kingsoft", __FILE__),
	device_c64_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_kingsoft_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_kingsoft_cartridge_device::device_reset()
{
	m_exrom = 0;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_kingsoft_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml)
	{
		data = m_roml[offset & 0x1fff];
	}
	else if (!romh)
	{
		data = m_romh[(m_exrom << 13) | (offset & 0x1fff)];
	}
	else if (!io1)
	{
		m_exrom = 0;
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_kingsoft_cartridge_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1)
	{
		m_exrom = 1;
	}
}


//-------------------------------------------------
//  c64_game_r - GAME read
//-------------------------------------------------

int c64_kingsoft_cartridge_device::c64_game_r(offs_t offset, int sphi2, int ba, int rw)
{
	return m_exrom & ((ba & rw & ((offset >= 0x8000 && offset < 0xc000) || (offset >= 0xe000))) ? 0 : 1);
}
