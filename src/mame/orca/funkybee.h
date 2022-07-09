// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
#ifndef MAME_INCLUDES_FUNKYBEE_H
#define MAME_INCLUDES_FUNKYBEE_H

#pragma once

#include "machine/watchdog.h"
#include "emupal.h"
#include "tilemap.h"


class funkybee_state : public driver_device
{
public:
	funkybee_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void funkybee(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;

	/* video-related */
	tilemap_t    *m_bg_tilemap = nullptr;
	int        m_gfx_bank = 0;
	uint8_t funkybee_input_port_0_r();
	DECLARE_WRITE_LINE_MEMBER(coin_counter_1_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_2_w);
	void funkybee_videoram_w(offs_t offset, uint8_t data);
	void funkybee_colorram_w(offs_t offset, uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(gfx_bank_w);
	void funkybee_scroll_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(flipscreen_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILEMAP_MAPPER_MEMBER(funkybee_tilemap_scan);
	virtual void machine_start() override;
	virtual void video_start() override;
	void funkybee_palette(palette_device &palette) const;
	uint32_t screen_update_funkybee(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_columns( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	void funkybee_map(address_map &map);
	void io_map(address_map &map);
};

#endif // MAME_INCLUDES_FUNKYBEE_H
