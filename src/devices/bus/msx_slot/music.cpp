// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "music.h"


DEFINE_DEVICE_TYPE(MSX_SLOT_MUSIC, msx_slot_music_device, "msx_slot_music", "MSX Internal MSX-MUSIC")


msx_slot_music_device::msx_slot_music_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
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
	io_space().install_write_handler(0x7c, 0x7d, write8sm_delegate(*m_ym2413, FUNC(ym2413_device::write)));
}
