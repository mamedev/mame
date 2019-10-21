// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*
  Emulation for the bunsetsu internal firmware mapper found in a number of MSX machines
*/

#include "emu.h"
#include "bunsetsu.h"


DEFINE_DEVICE_TYPE(MSX_SLOT_BUNSETSU, msx_slot_bunsetsu_device, "msx_slot_bunsetsu", "MSX Internal BUNSETSU")


msx_slot_bunsetsu_device::msx_slot_bunsetsu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: msx_slot_rom_device(mconfig, MSX_SLOT_BUNSETSU, tag, owner, clock)
	, m_bunsetsu_region(*this, finder_base::DUMMY_TAG, 0x20000)
	, m_bunsetsu_address(0)
{
}


void msx_slot_bunsetsu_device::device_reset()
{
	m_bunsetsu_address = 0;
}


uint8_t msx_slot_bunsetsu_device::read(offs_t offset)
{
	if (offset == 0xbfff)
	{
		return m_bunsetsu_region[m_bunsetsu_address++ & 0x1ffff];
	}
	return msx_slot_rom_device::read(offset);
}


void msx_slot_bunsetsu_device::write(offs_t offset, uint8_t data)
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
