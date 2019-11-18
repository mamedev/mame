// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Xybots hardware

*************************************************************************/
#ifndef MAME_INCLUDES_XYBOTS_H
#define MAME_INCLUDES_XYBOTS_H

#pragma once

#include "machine/atarigen.h"
#include "audio/atarijsa.h"
#include "video/atarimo.h"
#include "tilemap.h"

class xybots_state : public atarigen_state
{
public:
	xybots_state(const machine_config &mconfig, device_type type, const char *tag) :
		atarigen_state(mconfig, type, tag),
		m_jsa(*this, "jsa"),
		m_playfield_tilemap(*this, "playfield"),
		m_alpha_tilemap(*this, "alpha"),
		m_mob(*this, "mob")
	{ }

	void xybots(machine_config &config);

	void init_xybots();

private:
	virtual void update_interrupts() override;
	DECLARE_READ16_MEMBER(special_port1_r);
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	uint32_t screen_update_xybots(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map);

	required_device<atari_jsa_i_device> m_jsa;
	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_device<atari_motion_objects_device> m_mob;

	uint16_t          m_h256;

	static const atari_motion_objects_config s_mob_config;
};

#endif // MAME_INCLUDES_XYBOTS_H
