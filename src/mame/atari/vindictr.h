// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Vindicators hardware

*************************************************************************/
#ifndef MAME_INCLUDES_VINDICTR_H
#define MAME_INCLUDES_VINDICTR_H

#pragma once

#include "machine/timer.h"
#include "atarijsa.h"
#include "atarimo.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class vindictr_state : public driver_device
{
public:
	vindictr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_playfield_tilemap(*this, "playfield"),
		m_alpha_tilemap(*this, "alpha"),
		m_mob(*this, "mob"),
		m_jsa(*this, "jsa"),
		m_palette(*this, "palette"),
		m_paletteram(*this, "paletteram")
	{ }

	void vindictr(machine_config &config);

	void init_vindictr();

private:
	virtual void machine_reset() override;
	virtual void video_start() override;
	void scanline_interrupt();
	void scanline_int_ack_w(uint16_t data);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_update);
	uint16_t port1_r();
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	uint32_t screen_update_vindictr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vindictr_paletteram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	static const atari_motion_objects_config s_mob_config;
	void main_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_device<atari_motion_objects_device> m_mob;
	required_device<atari_jsa_i_device> m_jsa;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint16_t> m_paletteram;

	uint8_t           m_playfield_tile_bank = 0;
	uint16_t          m_playfield_xscroll = 0;
	uint16_t          m_playfield_yscroll = 0;
};

#endif // MAME_INCLUDES_VINDICTR_H
