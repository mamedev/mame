// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi, David Haywood
#ifndef MAME_INCLUDES_GOMOKU_H
#define MAME_INCLUDES_GOMOKU_H

#pragma once

#include "emupal.h"
#include "screen.h"

class gomoku_state : public driver_device
{
public:
	gomoku_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_bgram(*this, "bgram"),
		m_inputs(*this, {"IN0", "IN1", "DSW", "UNUSED0", "UNUSED1", "UNUSED2", "UNUSED3", "UNUSED4"}),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen")
	{ }

	void gomoku(machine_config &config);

private:
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_bgram;
	int m_flipscreen;
	int m_bg_dispsw;
	tilemap_t *m_fg_tilemap;
	bitmap_ind16 m_bg_bitmap;
	optional_ioport_array<8> m_inputs;
	DECLARE_READ8_MEMBER(input_port_r);
	DECLARE_WRITE8_MEMBER(gomoku_videoram_w);
	DECLARE_WRITE8_MEMBER(gomoku_colorram_w);
	DECLARE_WRITE8_MEMBER(gomoku_bgram_w);
	DECLARE_WRITE_LINE_MEMBER(flipscreen_w);
	DECLARE_WRITE_LINE_MEMBER(bg_dispsw_w);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void video_start() override;
	void gomoku_palette(palette_device &palette) const;
	uint32_t screen_update_gomoku(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	void gomoku_map(address_map &map);
};

#endif // MAME_INCLUDES_GOMOKU_H
