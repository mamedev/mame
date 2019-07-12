// license:BSD-3-Clause
// copyright-holders:David Haywood, Paul Priest
#ifndef MAME_INCLUDES_ONESHOT_H
#define MAME_INCLUDES_ONESHOT_H

#pragma once

#include "sound/okim6295.h"
#include "emupal.h"

class oneshot_state : public driver_device
{
public:
	oneshot_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_mid_videoram(*this, "mid_videoram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_scroll(*this, "scroll"),
		m_io_dsw1(*this, "DSW1"),
		m_io_lightgun_x(*this, "LIGHT%u_X", 0U),
		m_io_lightgun_y(*this, "LIGHT%u_Y", 0U),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void maddonna(machine_config &config);
	void komocomo(machine_config &config);
	void oneshot(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	/* memory pointers */
	required_shared_ptr<u16> m_spriteram;
	required_shared_ptr<u16> m_bg_videoram;
	required_shared_ptr<u16> m_mid_videoram;
	required_shared_ptr<u16> m_fg_videoram;
	required_shared_ptr<u16> m_scroll;

	optional_ioport m_io_dsw1;
	optional_ioport_array<2> m_io_lightgun_x;
	optional_ioport_array<2> m_io_lightgun_y;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_mid_tilemap;
	tilemap_t  *m_fg_tilemap;

	/* misc */
	int m_gun_x_p1;
	int m_gun_y_p1;
	int m_gun_x_p2;
	int m_gun_y_p2;
	int m_gun_x_shift;
	int m_p1_wobble;
	int m_p2_wobble;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	u16 oneshot_in0_word_r();
	u16 oneshot_gun_x_p1_r();
	u16 oneshot_gun_y_p1_r();
	u16 oneshot_gun_x_p2_r();
	u16 oneshot_gun_y_p2_r();
	void bg_videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void mid_videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void fg_videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void soundbank_w(u8 data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_mid_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	u32 screen_update_oneshot(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_maddonna(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_komocomo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_crosshairs();
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void mem_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_ONESHOT_H
