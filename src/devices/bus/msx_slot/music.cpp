// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "music.h"


DEFINE_DEVICE_TYPE(MSX_SLOT_MUSIC, msx_slot_music_device, "msx_slot_music", "MSX Internal MSX-MUSIC")


msx_slot_music_device::msx_slot_music_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: msx_slot_rom_device(mconfig, MSX_SLOT_MUSIC, tag, owner, clock)
	, m_ym2413(nullptr)
	, m_ym2413_tag(nullptr)
{
}


void msx_slot_music_device::device_start()
{
	msx_slot_rom_device::device_start();

	if (m_ym2413_tag == nullptr)
	{
		fatalerror("msx_slot_music_device: no YM2413 tag specified\n");
	}

	m_ym2413 = owner()->subdevice<ym2413_device>(m_ym2413_tag);

	if (m_ym2413 == nullptr)
	{
		fatalerror("msx_slot_ym2413_device: Unable to find YM2413 with tag '%s'\n", m_ym2413_tag);
	}

	// Install IO read/write handlers
	io_space().install_write_handler(0x7c, 0x7d, write8sm_delegate(FUNC(msx_slot_music_device::write_ym2413), this));
}


uint8_t msx_slot_music_device::read(offs_t offset)
{
	return msx_slot_rom_device::read(offset);
}


void msx_slot_music_device::write_ym2413(offs_t offset, uint8_t data)
{
	m_ym2413->write(offset & 1, data);
}
