// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_NAMCO_MAPPY_H
#define MAME_NAMCO_MAPPY_H

#pragma once

#include "namcoio.h"
#include "sound/namco.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class mappy_state : public driver_device
{
public:
	mappy_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_namcoio(*this, "namcoio_%u", 1),
		m_namco_15xx(*this, "namco"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_leds(*this, "led%u", 0U)
	{ }

	void mappy_common(machine_config &config);
	void mappy(machine_config &config);
	void motos(machine_config &config);
	void grobda(machine_config &config);
	void digdug2(machine_config &config);
	void pacnpal(machine_config &config);
	void superpac_common(machine_config &config);
	void superpac(machine_config &config);
	void todruaga(machine_config &config);

	void init_digdug2();

protected:
	virtual void machine_start() override ATTR_COLD;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device_array<namcoio_device, 2> m_namcoio;
	required_device<namco_15xx_device> m_namco_15xx;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	output_finder<2> m_leds;

	tilemap_t *m_bg_tilemap = nullptr;
	bitmap_ind16 m_sprite_bitmap;

	uint8_t m_scroll = 0;

	uint8_t m_main_irq_mask = 0;
	uint8_t m_sub_irq_mask = 0;

	emu_timer *m_namcoio_run_timer[2]{};

	void main_int_on_w(int state);
	void sub_int_on_w(int state);
	void superpac_videoram_w(offs_t offset, uint8_t data);
	void mappy_videoram_w(offs_t offset, uint8_t data);
	void flipscreen_w(uint8_t data);
	uint8_t flipscreen_r();
	void mappy_scroll_w(offs_t offset, uint8_t data);
	void out_lamps(uint8_t data);
	TILEMAP_MAPPER_MEMBER(superpac_tilemap_scan);
	TILEMAP_MAPPER_MEMBER(mappy_tilemap_scan);
	TILE_GET_INFO_MEMBER(superpac_get_tile_info);
	TILE_GET_INFO_MEMBER(mappy_get_tile_info);
	template<uint8_t Chip> TIMER_CALLBACK_MEMBER(namcoio_run_timer);

	DECLARE_VIDEO_START(superpac);
	void superpac_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(mappy);
	void mappy_palette(palette_device &palette) const;
	uint32_t screen_update_superpac(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_mappy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(int state);
	void mappy_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t *spriteram_base);

	void mappy_main_map(address_map &map) ATTR_COLD;
	void superpac_main_map(address_map &map) ATTR_COLD;
	void superpac_sub_map(address_map &map) ATTR_COLD;
};

class phozon_state : public mappy_state
{
public:
	phozon_state(const machine_config &mconfig, device_type type, const char *tag) :
		mappy_state(mconfig, type, tag),
		m_subcpu2(*this, "sub2")
	{ }

	void phozon(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_subcpu2;

	uint8_t m_sub2_irq_mask = 0;

	void sub2_int_on_w(int state);

	TILE_GET_INFO_MEMBER(get_tile_info);

	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void phozon_vblank_irq(int state);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t *spriteram_base);

	void main_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;
	void sub2_map(address_map &map) ATTR_COLD;
};

#endif // MAME_NAMCO_MAPPY_H
