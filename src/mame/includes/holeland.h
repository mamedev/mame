// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer
/*************************************************************************

    Hole Land

*************************************************************************/
#ifndef MAME_INCLUDES_HOLELAND_H
#define MAME_INCLUDES_HOLELAND_H

#pragma once

#include "machine/74259.h"
#include "emupal.h"
#include "tilemap.h"

class holeland_state : public driver_device
{
public:
	holeland_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_latch(*this, "latch"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram")
	{ }

	void crzrally(machine_config &config);
	void holeland(machine_config &config);

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<ls259_device> m_latch;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t    *m_bg_tilemap = nullptr;
	int        m_palette_offset = 0;

	DECLARE_WRITE_LINE_MEMBER(coin_counter_w);

	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	void pal_offs_w(uint8_t data);
	void scroll_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(flipscreen_x_w);
	DECLARE_WRITE_LINE_MEMBER(flipscreen_y_w);

	TILE_GET_INFO_MEMBER(holeland_get_tile_info);
	TILE_GET_INFO_MEMBER(crzrally_get_tile_info);

	DECLARE_VIDEO_START(holeland);
	DECLARE_VIDEO_START(crzrally);

	uint32_t screen_update_holeland(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_crzrally(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void holeland_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void crzrally_draw_sprites( bitmap_ind16 &bitmap,const rectangle &cliprect );
	void crzrally_map(address_map &map);
	void holeland_map(address_map &map);
	void io_map(address_map &map);
};

#endif // MAME_INCLUDES_HOLELAND_H
