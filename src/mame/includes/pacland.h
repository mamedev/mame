// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#ifndef MAME_INCLUDES_PACLAND_H
#define MAME_INCLUDES_PACLAND_H

#pragma once

#include "cpu/m6800/m6801.h"
#include "sound/namco.h"
#include "emupal.h"
#include "screen.h"

class pacland_state : public driver_device
{
public:
	pacland_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu"),
		m_cus30(*this, "namco"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_videoram2(*this, "videoram2"),
		m_spriteram(*this, "spriteram"),
		m_color_prom(*this, "proms"),
		m_leds(*this, "led%u", 0U)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<hd63701_cpu_device> m_mcu;
	required_device<namco_cus30_device> m_cus30;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_videoram2;
	required_shared_ptr<uint8_t> m_spriteram;
	required_region_ptr<uint8_t> m_color_prom;

	output_finder<2> m_leds;

	uint8_t m_palette_bank;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	bitmap_ind16 m_fg_bitmap;
	bitmap_ind16 m_sprite_bitmap;
	std::unique_ptr<uint32_t[]> m_transmask[3];
	uint16_t m_scroll0;
	uint16_t m_scroll1;
	uint8_t m_main_irq_mask;
	uint8_t m_mcu_irq_mask;

	DECLARE_WRITE8_MEMBER(subreset_w);
	DECLARE_WRITE8_MEMBER(flipscreen_w);
	DECLARE_READ8_MEMBER(input_r);
	DECLARE_WRITE8_MEMBER(coin_w);
	DECLARE_WRITE8_MEMBER(led_w);
	DECLARE_WRITE8_MEMBER(irq_1_ctrl_w);
	DECLARE_WRITE8_MEMBER(irq_2_ctrl_w);
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(videoram2_w);
	DECLARE_WRITE8_MEMBER(scroll0_w);
	DECLARE_WRITE8_MEMBER(scroll1_w);
	DECLARE_WRITE8_MEMBER(bankswitch_w);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	virtual void machine_start() override;
	virtual void video_start() override;
	void pacland_palette(palette_device &palette);

	DECLARE_WRITE_LINE_MEMBER(vblank_irq);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void switch_palette();
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flip, int whichmask);
	void draw_fg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority );
	void pacland(machine_config &config);
	void main_map(address_map &map);
	void mcu_map(address_map &map);
};

#endif // MAME_INCLUDES_PACLAND_H
