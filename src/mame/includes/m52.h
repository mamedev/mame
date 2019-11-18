// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_INCLUDES_M52_H
#define MAME_INCLUDES_M52_H

#pragma once

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class m52_state : public driver_device
{
public:
	m52_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_sp_gfxdecode(*this, "sp_gfxdecode"),
		m_tx_gfxdecode(*this, "tx_gfxdecode"),
		m_bg_gfxdecode(*this, "bg_gfxdecode"),
		m_sp_palette(*this, "sp_palette"),
		m_tx_palette(*this, "tx_palette"),
		m_bg_palette(*this, "bg_palette")
	{ }

	void m52(machine_config &config);

	DECLARE_WRITE8_MEMBER(m52_videoram_w);
	DECLARE_WRITE8_MEMBER(m52_colorram_w);
	DECLARE_READ8_MEMBER(m52_protection_r);

protected:
	virtual void machine_reset() override;
	virtual void video_start() override;
	virtual DECLARE_WRITE8_MEMBER(m52_scroll_w);

	/* board mod changes? */
	int m_spritelimit;
	bool m_do_bg_fills;

	tilemap_t*             m_tx_tilemap;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	optional_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	uint8_t                m_bg1xpos;
	uint8_t                m_bg1ypos;
	uint8_t                m_bg2xpos;
	uint8_t                m_bg2ypos;
	uint8_t                m_bgcontrol;

	required_device<gfxdecode_device> m_sp_gfxdecode;
	required_device<gfxdecode_device> m_tx_gfxdecode;
	required_device<gfxdecode_device> m_bg_gfxdecode;
	required_device<palette_device> m_sp_palette;
	required_device<palette_device> m_tx_palette;
	required_device<palette_device> m_bg_palette;

	DECLARE_WRITE8_MEMBER(m52_bg1ypos_w);
	DECLARE_WRITE8_MEMBER(m52_bg1xpos_w);
	DECLARE_WRITE8_MEMBER(m52_bg2xpos_w);
	DECLARE_WRITE8_MEMBER(m52_bg2ypos_w);
	DECLARE_WRITE8_MEMBER(m52_bgcontrol_w);
	DECLARE_WRITE8_MEMBER(m52_flipscreen_w);
	TILE_GET_INFO_MEMBER(get_tile_info);
	void init_palette();
	template <size_t N, size_t O, size_t P>
	void init_sprite_palette(const int *resistances_3, const int *resistances_2, double (&weights_r)[N], double (&weights_g)[O], double (&weights_b)[P], double scale);
	uint32_t screen_update_m52(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect, int xpos, int ypos, int image);
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, int initoffs);

	void main_map(address_map &map);
	void main_portmap(address_map &map);
};

class m52_alpha1v_state : public m52_state
{
public:
	m52_alpha1v_state(const machine_config &mconfig, device_type type, const char *tag)
		: m52_state(mconfig, type, tag)
	{ }

	void alpha1v(machine_config &config);

	void alpha1v_map(address_map &map);

protected:
	virtual void video_start() override;
	virtual DECLARE_WRITE8_MEMBER(m52_scroll_w) override;
	DECLARE_WRITE8_MEMBER(alpha1v_flipscreen_w);

};

#endif // MAME_INCLUDES_M52_H
