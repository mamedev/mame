// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Zaxxon/Super Zaxxon cartridge emulation

**********************************************************************/

#include "zaxxon.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_ZAXXON = &device_creator<c64_zaxxon_cartridge_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_zaxxon_cartridge_device - constructor
//-------------------------------------------------

c64_zaxxon_cartridge_device::c64_zaxxon_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_ZAXXON, "C64 Zaxxon cartridge", tag, owner, clock, "c64_zaxxon", __FILE__),
	device_c64_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_zaxxon_cartridge_device::device_start()
{
	// state saving
	save_item(NAME(m_bank));
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_zaxxon_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml)
	{
		data = m_roml[offset & 0xfff];

		m_bank = BIT(offset, 12);
	}
	else if (!romh)
	{
		offs_t addr = (m_bank << 13) | (offset & 0x1fff);
		data = m_romh[addr];
	}

	return data;
}
