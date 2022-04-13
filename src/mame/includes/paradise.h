// license:BSD-3-Clause
// copyright-holders:Luca Elia
#ifndef MAME_INCLUDES_PARADISE_H
#define MAME_INCLUDES_PARADISE_H

#pragma once

#include "sound/okim6295.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class paradise_state : public driver_device
{
public:
	paradise_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_oki2(*this, "oki2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_vram_0(*this, "vram_0"),
		m_vram_1(*this, "vram_1"),
		m_vram_2(*this, "vram_2"),
		m_videoram(*this, "videoram"),
		m_paletteram(*this, "paletteram"),
		m_spriteram(*this, "spriteram")
	{ }

	void penkyi(machine_config &config);
	void tgtball(machine_config &config);
	void paradise(machine_config &config);
	void madball(machine_config &config);
	void torus(machine_config &config);
	void penky(machine_config &config);

	void init_torus();
	void init_paradise();
	void init_tgtball();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<okim6295_device> m_oki2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_vram_0;
	required_shared_ptr<uint8_t> m_vram_1;
	required_shared_ptr<uint8_t> m_vram_2;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_paletteram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t *m_tilemap_0 = nullptr;
	tilemap_t *m_tilemap_1 = nullptr;
	tilemap_t *m_tilemap_2 = nullptr;

	bitmap_ind16 m_tmpbitmap;
	uint8_t m_palbank = 0;
	uint8_t m_priority = 0;
	uint8_t m_pixbank = 0;
	int m_sprite_inc = 0;
	int m_irq_count = 0;

	// common
	void rombank_w(uint8_t data);
	void flipscreen_w(uint8_t data);
	void palette_w(offs_t offset, uint8_t data);
	void palbank_w(uint8_t data);
	void vram_0_w(offs_t offset, uint8_t data);
	void vram_1_w(offs_t offset, uint8_t data);
	void vram_2_w(offs_t offset, uint8_t data);
	void pixmap_w(offs_t offset, uint8_t data);
	void priority_w(uint8_t data);

	// paradise specific
	void paradise_okibank_w(uint8_t data);

	// torus specific
	void torus_coin_counter_w(uint8_t data);

	// tgtball specific
	void tgtball_flipscreen_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_tile_info_0);
	TILE_GET_INFO_MEMBER(get_tile_info_1);
	TILE_GET_INFO_MEMBER(get_tile_info_2);

	uint32_t screen_update_paradise(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_torus(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_madball(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(irq);

	void update_pix_palbank();
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void base_map(address_map &map);
	void paradise_io_map(address_map &map);
	void paradise_map(address_map &map);
	void tgtball_map(address_map &map);
	void torus_io_map(address_map &map);
	void torus_map(address_map &map);
};

#endif // MAME_INCLUDES_PARADISE_H
