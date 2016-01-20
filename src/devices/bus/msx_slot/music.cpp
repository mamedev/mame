// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "music.h"


const device_type MSX_SLOT_MUSIC = &device_creator<msx_slot_music_device>;


msx_slot_music_device::msx_slot_music_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: msx_slot_rom_device(mconfig, MSX_SLOT_MUSIC, "MSX Internal MSX-MUSIC", tag, owner, clock, "msx_slot_music", __FILE__)
	, m_ym2413(nullptr)
{
}


void msx_slot_music_device::device_start()
{
	msx_slot_rom_device::device_start();

	if (m_ym2413_tag.empty())
	{
		fatalerror("msx_slot_music_device: no YM2413 tag specified\n");
	}

	m_ym2413 = owner()->subdevice<ym2413_device>(m_ym2413_tag);

	if (m_ym2413 == nullptr)
	{
		fatalerror("msx_slot_ym2413_device: Unable to find YM2413 with tag '%s'\n", m_ym2413_tag.c_str());
	}

	// Install IO read/write handlers
	address_space &space = machine().device<cpu_device>("maincpu")->space(AS_IO);
	space.install_write_handler(0x7c, 0x7d, write8_delegate(FUNC(msx_slot_music_device::write_ym2413), this));
}


READ8_MEMBER(msx_slot_music_device::read)
{
	return msx_slot_rom_device::read(space, offset);
}


WRITE8_MEMBER(msx_slot_music_device::write_ym2413)
{
	m_ym2413->write(space, offset & 1, data);
}
