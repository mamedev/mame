// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/*************************************************************************

    Ghosts'n Goblins

*************************************************************************/
#ifndef MAME_INCLUDES_GNG_H
#define MAME_INCLUDES_GNG_H

#pragma once

#include "sound/ymopn.h"
#include "video/bufsprite.h"
#include "emupal.h"
#include "tilemap.h"

class gng_state : public driver_device
{
public:
	gng_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_maincpu(*this, "maincpu"),
		m_ym(*this, "ym%u", 1),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void gng(machine_config &config);
	void diamond(machine_config &config);

private:
	void gng_bankswitch_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(ym_reset_w);
	uint8_t diamond_hack_r();
	void gng_fgvideoram_w(offs_t offset, uint8_t data);
	void gng_bgvideoram_w(offs_t offset, uint8_t data);
	void gng_bgscrollx_w(offs_t offset, uint8_t data);
	void gng_bgscrolly_w(offs_t offset, uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(flipscreen_w);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_gng(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void diamond_map(address_map &map);
	void gng_map(address_map &map);
	void sound_map(address_map &map);

	/* memory pointers */
	required_device<buffered_spriteram8_device> m_spriteram;
	required_shared_ptr<uint8_t> m_fgvideoram;
	required_shared_ptr<uint8_t> m_bgvideoram;

	/* video-related */
	tilemap_t    *m_bg_tilemap = nullptr;
	tilemap_t    *m_fg_tilemap = nullptr;
	uint8_t      m_scrollx[2]{};
	uint8_t      m_scrolly[2]{};

	required_device<cpu_device> m_maincpu;
	required_device_array<ym2203_device, 2> m_ym;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

#endif // MAME_INCLUDES_GNG_H
