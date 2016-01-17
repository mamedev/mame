// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    VizaWrite 64 cartridge emulation

**********************************************************************/

#include "simons_basic.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_SIMONS_BASIC = &device_creator<c64_simons_basic_cartridge_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_simons_basic_cartridge_device - constructor
//-------------------------------------------------

c64_simons_basic_cartridge_device::c64_simons_basic_cartridge_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_SIMONS_BASIC, "Simons' BASIC", tag, owner, clock, "c64_simons_basic", __FILE__),
	device_c64_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_simons_basic_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_simons_basic_cartridge_device::device_reset()
{
	m_game = 1;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_simons_basic_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml)
	{
		data = m_roml[offset & 0x1fff];
	}
	else if (!romh)
	{
		data = m_romh[offset & 0x1fff];
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_simons_basic_cartridge_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1)
	{
		m_game = !BIT(data, 0);
	}
}
