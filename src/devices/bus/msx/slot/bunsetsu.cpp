// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*
  Emulation for the bunsetsu internal firmware mapper found in a number of MSX machines
*/

#include "emu.h"
#include "bunsetsu.h"


DEFINE_DEVICE_TYPE(MSX_SLOT_BUNSETSU, msx_slot_bunsetsu_device, "msx_slot_bunsetsu", "MSX Internal BUNSETSU")


msx_slot_bunsetsu_device::msx_slot_bunsetsu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_rom_device(mconfig, MSX_SLOT_BUNSETSU, tag, owner, clock)
	, m_bunsetsu_region(*this, finder_base::DUMMY_TAG)
	, m_bunsetsu_address(0)
{
}

void msx_slot_bunsetsu_device::device_start()
{
	msx_slot_rom_device::device_start();

	save_item(NAME(m_bunsetsu_address));

	page(2)->install_read_handler(0xbfff, 0xbfff, read8smo_delegate(*this, FUNC(msx_slot_bunsetsu_device::buns_read)));
	page(2)->install_write_handler(0xbffc, 0xbffe, write8sm_delegate(*this, FUNC(msx_slot_bunsetsu_device::buns_write)));
}

void msx_slot_bunsetsu_device::device_reset()
{
	m_bunsetsu_address = 0;
}

u8 msx_slot_bunsetsu_device::buns_read()
{
	u8 data = m_bunsetsu_region[m_bunsetsu_address & 0x1ffff];

	if (!machine().side_effects_disabled())
		m_bunsetsu_address++;

	return data;
}

void msx_slot_bunsetsu_device::buns_write(offs_t offset, u8 data)
{
	switch (offset)
	{
		case 0:
			m_bunsetsu_address = (m_bunsetsu_address & 0xffff00) | data;
			break;

		case 1:
			m_bunsetsu_address = (m_bunsetsu_address & 0xff00ff) | (data << 8);
			break;

		case 2:
			m_bunsetsu_address = (m_bunsetsu_address & 0x00ffff) | (data << 16);
			break;
	}
}
