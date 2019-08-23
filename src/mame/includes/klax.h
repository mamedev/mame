// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Klax hardware

*************************************************************************/
#ifndef MAME_INCLUDES_KLAX_H
#define MAME_INCLUDES_KLAX_H

#pragma once

#include "machine/atarigen.h"
#include "video/atarimo.h"
#include "tilemap.h"

class klax_state : public atarigen_state
{
public:
	klax_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag)
		, m_playfield_tilemap(*this, "playfield")
		, m_mob(*this, "mob")
		, m_p1(*this, "P1")
	{ }

	void klax(machine_config &config);
	void klax2bl(machine_config &config);

private:
	virtual void machine_reset() override;

	virtual void scanline_update(screen_device &screen, int scanline) override;

	virtual void update_interrupts() override;
	void interrupt_ack_w(u16 data = 0);

	void klax_latch_w(u16 data);

	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void bootleg_sound_map(address_map &map);
	void klax2bl_map(address_map &map);
	void klax_map(address_map &map);

	required_device<tilemap_device> m_playfield_tilemap;
	required_device<atari_motion_objects_device> m_mob;

	required_ioport m_p1;

	static const atari_motion_objects_config s_mob_config;
};

#endif // MAME_INCLUDES_KLAX_H
