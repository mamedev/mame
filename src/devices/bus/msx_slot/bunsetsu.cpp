// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*
  Emulation for the bunsetsu internal firmware mapper found in a number of MSX machines
*/

#include "emu.h"
#include "bunsetsu.h"


const device_type MSX_SLOT_BUNSETSU = &device_creator<msx_slot_bunsetsu_device>;


msx_slot_bunsetsu_device::msx_slot_bunsetsu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: msx_slot_rom_device(mconfig, MSX_SLOT_BUNSETSU, "MSX Internal BUNSETSU", tag, owner, clock, "msx_slot_bunsetsu", __FILE__)
	, m_bunsetsu_region(*this, 0x20000)
	, m_bunsetsu_address(0)
{
}


void msx_slot_bunsetsu_device::device_reset()
{
	m_bunsetsu_address = 0;
}


READ8_MEMBER(msx_slot_bunsetsu_device::read)
{
	if (offset == 0xbfff)
	{
		return m_bunsetsu_region[m_bunsetsu_address++ & 0x1ffff];
	}
	return msx_slot_rom_device::read(space, offset);
}


WRITE8_MEMBER(msx_slot_bunsetsu_device::write)
{
	switch (offset)
	{
		case 0xbffc:
			m_bunsetsu_address = (m_bunsetsu_address & 0xffff00) | data;
			break;

		case 0xbffd:
			m_bunsetsu_address = (m_bunsetsu_address & 0xff00ff) | (data << 8);
			break;

		case 0xbffe:
			m_bunsetsu_address = (m_bunsetsu_address & 0x00ffff) | (data << 16);
			break;
	}
}
