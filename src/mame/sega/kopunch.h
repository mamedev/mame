// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

  Sega KO Punch

*************************************************************************/
#ifndef MAME_INCLUDES_KOPUNCH_H
#define MAME_INCLUDES_KOPUNCH_H

#pragma once

#include "emupal.h"
#include "tilemap.h"

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
	uint8_t sensors1_r();
	uint8_t sensors2_r();
	void lamp_w(uint8_t data);
	void coin_w(uint8_t data);
	void vram_fg_w(offs_t offset, uint8_t data);
	void vram_bg_w(offs_t offset, uint8_t data);
	void scroll_x_w(uint8_t data);
	void scroll_y_w(uint8_t data);
	void gfxbank_w(uint8_t data);

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
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	uint8_t m_gfxbank = 0U;
	uint8_t m_scrollx = 0U;
};

#endif // MAME_INCLUDES_KOPUNCH_H
