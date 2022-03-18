// license:BSD-3-Clause
// copyright-holders:Carlos A. Lozano, Uki
/***************************************************************************

    Aeroboto

***************************************************************************/
#ifndef MAME_INCLUDES_AEROBOTO_H
#define MAME_INCLUDES_AEROBOTO_H

#pragma once

#include "emupal.h"
#include "tilemap.h"

class aeroboto_state : public driver_device
{
public:
	aeroboto_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_mainram(*this, "mainram"),
		m_videoram(*this, "videoram"),
		m_hscroll(*this, "hscroll"),
		m_tilecolor(*this, "tilecolor"),
		m_spriteram(*this, "spriteram"),
		m_vscroll(*this, "vscroll"),
		m_starx(*this, "starx"),
		m_stary(*this, "stary"),
		m_bgcolor(*this, "bgcolor"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_mainram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_hscroll;
	required_shared_ptr<uint8_t> m_tilecolor;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_vscroll;
	required_shared_ptr<uint8_t> m_starx;
	required_shared_ptr<uint8_t> m_stary;
	required_shared_ptr<uint8_t> m_bgcolor;

	/* stars layout */
	uint8_t * m_stars_rom = 0U;
	int     m_stars_length = 0;

	/* video-related */
	tilemap_t *m_bg_tilemap = 0;
	int     m_charbank = 0;
	int     m_starsoff = 0;
	int     m_sx = 0;
	int     m_sy = 0;
	uint8_t   m_ox = 0U;
	uint8_t   m_oy = 0U;

	/* misc */
	int m_count = 0;
	int m_disable_irq = 0;
	uint8_t aeroboto_201_r();
	uint8_t aeroboto_irq_ack_r();
	uint8_t aeroboto_2973_r();
	void aeroboto_1a2_w(uint8_t data);
	uint8_t aeroboto_in0_r();
	void aeroboto_3000_w(uint8_t data);
	void aeroboto_videoram_w(offs_t offset, uint8_t data);
	void aeroboto_tilecolor_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_aeroboto(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	void formatz(machine_config &config);
	void main_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_AEROBOTO_H
