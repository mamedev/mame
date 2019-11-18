// license:BSD-3-Clause
// copyright-holders:Paul Leaman
/*************************************************************************

    Gun.Smoke

*************************************************************************/
#ifndef MAME_INCLUDES_GUNSMOKE_H
#define MAME_INCLUDES_GUNSMOKE_H

#pragma once

#include "emupal.h"
#include "tilemap.h"

class gunsmoke_state : public driver_device
{
public:
	gunsmoke_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_scrollx(*this, "scrollx"),
		m_scrolly(*this, "scrolly"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void gunsmoke(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_scrollx;
	required_shared_ptr<uint8_t> m_scrolly;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	tilemap_t    *m_fg_tilemap;
	uint8_t      m_chon;
	uint8_t      m_objon;
	uint8_t      m_bgon;
	uint8_t      m_sprite3bank;
	DECLARE_READ8_MEMBER(gunsmoke_protection_r);
	DECLARE_WRITE8_MEMBER(gunsmoke_videoram_w);
	DECLARE_WRITE8_MEMBER(gunsmoke_colorram_w);
	DECLARE_WRITE8_MEMBER(gunsmoke_c804_w);
	DECLARE_WRITE8_MEMBER(gunsmoke_d806_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void gunsmoke_palette(palette_device &palette) const;
	uint32_t screen_update_gunsmoke(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	void gunsmoke_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_GUNSMOKE_H
