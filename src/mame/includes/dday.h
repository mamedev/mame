// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/*************************************************************************

    D-Day

*************************************************************************/
#ifndef MAME_INCLUDES_DDAY_H
#define MAME_INCLUDES_DDAY_H

#pragma once

#include "emupal.h"
#include "screen.h"
#include "sound/ay8910.h"
#include "tilemap.h"


class dday_state : public driver_device
{
public:
	dday_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_textvideoram(*this, "textvideoram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_ay1(*this, "ay1")
	{ }

	void dday(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_textvideoram;
	required_shared_ptr<uint8_t> m_fgvideoram;
	required_shared_ptr<uint8_t> m_bgvideoram;
	required_shared_ptr<uint8_t> m_colorram;

	/* video-related */
	tilemap_t        *m_fg_tilemap = nullptr;
	tilemap_t        *m_bg_tilemap = nullptr;
	tilemap_t        *m_text_tilemap = nullptr;
	tilemap_t        *m_sl_tilemap = nullptr;
	bitmap_ind16 m_main_bitmap = 0;
	int            m_control = 0;
	int            m_sl_image = 0;
	int            m_sl_enable = 0;
	int            m_timer_value = 0;
	emu_timer *m_countdown_timer = nullptr;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<ay8912_device> m_ay1;

	uint8_t dday_countdown_timer_r();
	void dday_bgvideoram_w(offs_t offset, uint8_t data);
	void dday_fgvideoram_w(offs_t offset, uint8_t data);
	void dday_textvideoram_w(offs_t offset, uint8_t data);
	void dday_colorram_w(offs_t offset, uint8_t data);
	uint8_t dday_colorram_r(offs_t offset);
	void dday_sl_control_w(uint8_t data);
	void dday_control_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_text_tile_info);
	TILE_GET_INFO_MEMBER(get_sl_tile_info);
	void dday_palette(palette_device &palette) const;
	uint32_t screen_update_dday(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(countdown_timer_callback);
	void start_countdown_timer();
	void dday_map(address_map &map);
};

#endif // MAME_INCLUDES_DDAY_H
