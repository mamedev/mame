// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Rex ExOS cartridge emulation

**********************************************************************/

#include "exos.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_EXOS = &device_creator<c64_exos_cartridge_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_exos_cartridge_device - constructor
//-------------------------------------------------

c64_exos_cartridge_device::c64_exos_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_EXOS, "C64 ExOS cartridge", tag, owner, clock, "c64_exos", __FILE__),
	device_c64_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_exos_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_exos_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!romh)
	{
		data = m_romh[offset & 0x1fff];
	}

	return data;
}


//-------------------------------------------------
//  c64_game_r - GAME read
//-------------------------------------------------

int c64_exos_cartridge_device::c64_game_r(offs_t offset, int sphi2, int ba, int rw)
{
	return !(ba & rw & ((offset & 0xe000) == 0xe000) & m_slot->hiram());
}
