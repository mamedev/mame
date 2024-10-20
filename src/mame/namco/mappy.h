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
		m_subcpu2(*this, "sub2"),
		m_namcoio(*this, "namcoio_%u", 1),
		m_namco_15xx(*this, "namco"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_leds(*this, "led%u", 0U)
	{ }

	void mappy_common(machine_config &config);
	void mappy(machine_config &config);
	void phozon(machine_config &config);
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

private:
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	optional_device<cpu_device> m_subcpu2;
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
	uint8_t m_sub2_irq_mask = 0;

	emu_timer *m_namcoio_run_timer[2]{};

	void int_on_w(int state);
	void int_on_2_w(int state);
	void int_on_3_w(int state);
	void superpac_videoram_w(offs_t offset, uint8_t data);
	void mappy_videoram_w(offs_t offset, uint8_t data);
	void superpac_flipscreen_w(uint8_t data);
	uint8_t superpac_flipscreen_r();
	void mappy_scroll_w(offs_t offset, uint8_t data);
	void out_lamps(uint8_t data);
	TILEMAP_MAPPER_MEMBER(superpac_tilemap_scan);
	TILEMAP_MAPPER_MEMBER(mappy_tilemap_scan);
	TILE_GET_INFO_MEMBER(superpac_get_tile_info);
	TILE_GET_INFO_MEMBER(phozon_get_tile_info);
	TILE_GET_INFO_MEMBER(mappy_get_tile_info);
	template<uint8_t Chip> TIMER_CALLBACK_MEMBER(namcoio_run_timer);

	DECLARE_VIDEO_START(superpac);
	void superpac_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(phozon);
	void phozon_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(mappy);
	void mappy_palette(palette_device &palette) const;
	uint32_t screen_update_superpac(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_phozon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_mappy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(int state);
	void mappy_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t *spriteram_base);
	void phozon_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t *spriteram_base);

	void mappy_cpu1_map(address_map &map) ATTR_COLD;
	void mappy_cpu2_map(address_map &map) ATTR_COLD;
	void phozon_cpu1_map(address_map &map) ATTR_COLD;
	void phozon_cpu2_map(address_map &map) ATTR_COLD;
	void phozon_cpu3_map(address_map &map) ATTR_COLD;
	void superpac_cpu1_map(address_map &map) ATTR_COLD;
	void superpac_cpu2_map(address_map &map) ATTR_COLD;
};

#endif // MAME_NAMCO_MAPPY_H
