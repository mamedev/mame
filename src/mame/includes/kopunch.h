// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

  Sega KO Punch

*************************************************************************/
#ifndef MAME_INCLUDES_KOPUNCH_H
#define MAME_INCLUDES_KOPUNCH_H

#pragma once

#include "emupal.h"

class kopunch_state : public driver_device
{
public:
	kopunch_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_vram_fg(*this, "vram_fg")
		, m_vram_bg(*this, "vram_bg")
		, m_lamp(*this, "lamp0")
	{ }

	void kopunch(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(left_coin_inserted);
	DECLARE_INPUT_CHANGED_MEMBER(right_coin_inserted);

private:
	DECLARE_READ8_MEMBER(sensors1_r);
	DECLARE_READ8_MEMBER(sensors2_r);
	DECLARE_WRITE8_MEMBER(lamp_w);
	DECLARE_WRITE8_MEMBER(coin_w);
	DECLARE_WRITE8_MEMBER(vram_fg_w);
	DECLARE_WRITE8_MEMBER(vram_bg_w);
	DECLARE_WRITE8_MEMBER(scroll_x_w);
	DECLARE_WRITE8_MEMBER(scroll_y_w);
	DECLARE_WRITE8_MEMBER(gfxbank_w);

	INTERRUPT_GEN_MEMBER(vblank_interrupt);

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void kopunch_palette(palette_device &palette) const;
	uint32_t screen_update_kopunch(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void kopunch_io_map(address_map &map);
	void kopunch_map(address_map &map);

	virtual void machine_start() override;
	virtual void video_start() override;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_vram_fg;
	required_shared_ptr<uint8_t> m_vram_bg;

	output_finder<> m_lamp;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	uint8_t m_gfxbank;
	uint8_t m_scrollx;
};

#endif // MAME_INCLUDES_KOPUNCH_H
