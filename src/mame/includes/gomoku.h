// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi, David Haywood
#ifndef MAME_INCLUDES_GOMOKU_H
#define MAME_INCLUDES_GOMOKU_H

#pragma once

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

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
		m_screen(*this, "screen"),
		m_bg_x(*this, "bg_x"),
		m_bg_y(*this, "bg_y"),
		m_bg_d(*this, "bg_d")
	{ }

	void gomoku(machine_config &config);

protected:
	virtual void video_start() override;

private:
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_bgram;
	optional_ioport_array<8> m_inputs;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_region_ptr<uint8_t> m_bg_x;
	required_region_ptr<uint8_t> m_bg_y;
	required_region_ptr<uint8_t> m_bg_d;

	bool m_flipscreen = false;
	bool m_bg_dispsw = false;
	tilemap_t *m_fg_tilemap = nullptr;
	bitmap_ind16 m_bg_bitmap{};

	uint8_t input_port_r(offs_t offset);
	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	void flipscreen_w(int state);
	void bg_dispsw_w(int state);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void prg_map(address_map &map);
};

#endif // MAME_INCLUDES_GOMOKU_H
