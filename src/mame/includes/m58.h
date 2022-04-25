// license:BSD-3-Clause
// copyright-holders:Lee Taylor
// thanks-to:John Clegg
/****************************************************************************

    Irem M58 hardware

****************************************************************************/
#ifndef MAME_INCLUDES_M58_H
#define MAME_INCLUDES_M58_H

#pragma once

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class m58_state : public driver_device
{
public:
	m58_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_scroll_x_low(*this, "scroll_x_low"),
		m_scroll_x_high(*this, "scroll_x_high"),
		m_scroll_y_low(*this, "scroll_y_low"),
		m_score_panel_disabled(*this, "score_disable")
	{ }

	void yard(machine_config &config);

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_scroll_x_low;
	required_shared_ptr<uint8_t> m_scroll_x_high;
	required_shared_ptr<uint8_t> m_scroll_y_low;
	required_shared_ptr<uint8_t> m_score_panel_disabled;

	/* video-related */
	tilemap_t* m_bg_tilemap = nullptr;
	bitmap_ind16 m_scroll_panel_bitmap;

	void videoram_w(offs_t offset, uint8_t data);
	void scroll_panel_w(offs_t offset, uint8_t data);
	void flipscreen_w(uint8_t data);

	virtual void video_start() override;
	void m58_palette(palette_device &palette) const;

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILEMAP_MAPPER_MEMBER(tilemap_scan_rows);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_panel( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void yard_map(address_map &map);
};

#endif // MAME_INCLUDES_M58_H
