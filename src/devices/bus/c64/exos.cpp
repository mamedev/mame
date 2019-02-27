// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Rex ExOS cartridge emulation

**********************************************************************/

#include "emu.h"
#include "exos.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_EXOS, c64_exos_cartridge_device, "c64_exos", "C64 ExOS cartridge")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_exos_cartridge_device - constructor
//-------------------------------------------------

c64_exos_cartridge_device::c64_exos_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_EXOS, tag, owner, clock),
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

uint8_t c64_exos_cartridge_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
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
