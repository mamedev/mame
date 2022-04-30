// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Xybots hardware

*************************************************************************/
#ifndef MAME_INCLUDES_XYBOTS_H
#define MAME_INCLUDES_XYBOTS_H

#pragma once

#include "machine/slapstic.h"
#include "audio/atarijsa.h"
#include "video/atarimo.h"
#include "screen.h"
#include "tilemap.h"

class xybots_state : public driver_device
{
public:
	xybots_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_slapstic(*this, "slapstic"),
		m_slapstic_bank(*this, "slapstic_bank"),
		m_jsa(*this, "jsa"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_playfield_tilemap(*this, "playfield"),
		m_alpha_tilemap(*this, "alpha"),
		m_mob(*this, "mob")
	{ }

	void xybots(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	void video_int_ack_w(uint16_t data = 0);
	uint16_t special_port1_r();
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	uint32_t screen_update_xybots(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<atari_slapstic_device> m_slapstic;
	required_memory_bank m_slapstic_bank;
	required_device<atari_jsa_i_device> m_jsa;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_device<atari_motion_objects_device> m_mob;

	uint16_t          m_h256 = 0;

	static const atari_motion_objects_config s_mob_config;
};

#endif // MAME_INCLUDES_XYBOTS_H
