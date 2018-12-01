// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Vindicators hardware

*************************************************************************/
#ifndef MAME_INCLUDES_VINDICTR_H
#define MAME_INCLUDES_VINDICTR_H

#pragma once

#include "machine/atarigen.h"
#include "audio/atarijsa.h"
#include "video/atarimo.h"
#include "emupal.h"

class vindictr_state : public atarigen_state
{
public:
	vindictr_state(const machine_config &mconfig, device_type type, const char *tag) :
		atarigen_state(mconfig, type, tag),
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
	virtual void update_interrupts() override;
	virtual void scanline_update(screen_device &screen, int scanline) override;
	DECLARE_READ16_MEMBER(port1_r);
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	uint32_t screen_update_vindictr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE16_MEMBER( vindictr_paletteram_w );

	static const atari_motion_objects_config s_mob_config;
	void main_map(address_map &map);

	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_device<atari_motion_objects_device> m_mob;
	required_device<atari_jsa_i_device> m_jsa;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint16_t> m_paletteram;

	uint8_t           m_playfield_tile_bank;
	uint16_t          m_playfield_xscroll;
	uint16_t          m_playfield_yscroll;
};

#endif // MAME_INCLUDES_VINDICTR_H
