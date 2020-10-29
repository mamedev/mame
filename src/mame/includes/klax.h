// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Klax hardware

*************************************************************************/
#ifndef MAME_INCLUDES_KLAX_H
#define MAME_INCLUDES_KLAX_H

#pragma once

#include "machine/timer.h"
#include "video/atarimo.h"
#include "screen.h"
#include "tilemap.h"

class klax_state : public driver_device
{
public:
	klax_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_playfield_tilemap(*this, "playfield")
		, m_mob(*this, "mob")
		, m_p1(*this, "P1")
		, m_audiocpu(*this, "audiocpu")
	{ }

	void klax(machine_config &config);
	void klax5bl(machine_config &config);

	void init_bootleg();

private:
	virtual void machine_reset() override;

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_update);

	void interrupt_ack_w(u16 data = 0);

	void latch_w(u16 data);

	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void bootleg_sound_map(address_map &map);
	void klax5bl_map(address_map &map);
	void klax_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<tilemap_device> m_playfield_tilemap;
	required_device<atari_motion_objects_device> m_mob;

	required_ioport m_p1;

	// bootleg hardware
	uint8_t audio_ram_r(offs_t offset);
	void audio_ram_w(offs_t offset, uint8_t data);

	optional_device<cpu_device> m_audiocpu;
	std::unique_ptr<uint8_t[]> m_audio_ram;

	static const atari_motion_objects_config s_mob_config;
};

#endif // MAME_INCLUDES_KLAX_H
