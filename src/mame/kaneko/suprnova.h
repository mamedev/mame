// license:BSD-3-Clause
// copyright-holders:David Haywood, Sylvain Glaize, Paul Priest, Olivier Galibert
#ifndef MAME_KANEKO_SUPRNOVA_H
#define MAME_KANEKO_SUPRNOVA_H

#pragma once

#include "sknsspr.h"

#include "cpu/sh/sh7604.h"
#include "machine/timer.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


class skns_state : public driver_device
{
public:
	skns_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_spritegen(*this, "spritegen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_spriteram(*this,"spriteram"),
		m_spc_regs(*this, "spc_regs"),
		m_v3_regs(*this, "v3_regs"),
		m_tilemapA_ram(*this, "tilemapa_ram"),
		m_tilemapB_ram(*this, "tilemapb_ram"),
		m_v3slc_ram(*this, "v3slc_ram"),
		m_pal_regs(*this, "pal_regs"),
		m_palette_ram(*this, "palette_ram"),
		m_v3t_ram(*this, "v3t_ram"),
		m_main_ram(*this, "main_ram"),
		m_cache_ram(*this, "cache_ram"),
		m_paddle(*this, "Paddle.%c", 'A')
	{ }

	void sknsk(machine_config &config);
	void sknsu(machine_config &config);
	void sknsa(machine_config &config);
	void sknsj(machine_config &config);
	void sknse(machine_config &config);
	void skns(machine_config &config);

	void init_sengekis();
	void init_cyvern();
	void init_puzzloopa();
	void init_teljan();
	void init_panicstr();
	void init_puzzloope();
	void init_sengekij();
	void init_puzzloopj();
	void init_sarukani();
	void init_gutsn();
	void init_jjparad2();
	void init_galpans3();
	void init_jjparads();
	void init_galpans2();
	void init_galpanis();
	void init_puzzloopu();
	void init_senknow();
	void init_galpani4();
	void init_ryouran();

	template <int P> ioport_value paddle_r();

private:
	struct hit_t
	{
		uint16_t x1p = 0, y1p = 0, z1p = 0, x1s = 0, y1s = 0, z1s = 0;
		uint16_t x2p = 0, y2p = 0, z2p = 0, x2s = 0, y2s = 0, z2s = 0;
		uint16_t org = 0;

		uint16_t x1_p1 = 0, x1_p2 = 0, y1_p1 = 0, y1_p2 = 0, z1_p1 = 0, z1_p2 = 0;
		uint16_t x2_p1 = 0, x2_p2 = 0, y2_p1 = 0, y2_p2 = 0, z2_p1 = 0, z2_p2 = 0;
		uint16_t x1tox2 = 0, y1toy2 = 0, z1toz2 = 0;
		int16_t x_in = 0, y_in = 0, z_in = 0;
		uint16_t flag = 0;

		uint8_t disconnect = 0;
	};

	required_device<sh7604_device> m_maincpu;
	required_device<sknsspr_device> m_spritegen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	required_shared_ptr<uint32_t> m_spriteram;
	required_shared_ptr<uint32_t> m_spc_regs;
	required_shared_ptr<uint32_t> m_v3_regs;
	required_shared_ptr<uint32_t> m_tilemapA_ram;
	required_shared_ptr<uint32_t> m_tilemapB_ram;
	required_shared_ptr<uint32_t> m_v3slc_ram;
	required_shared_ptr<uint32_t> m_pal_regs;
	required_shared_ptr<uint32_t> m_palette_ram;
	required_shared_ptr<uint32_t> m_v3t_ram;
	required_shared_ptr<uint32_t> m_main_ram;
	required_shared_ptr<uint32_t> m_cache_ram;

	optional_ioport_array<4> m_paddle;

	hit_t m_hit;
	bitmap_ind16 m_sprite_bitmap;
	bitmap_ind16 m_tilemap_bitmap_lower;
	bitmap_ind8 m_tilemap_bitmapflags_lower;
	bitmap_ind16 m_tilemap_bitmap_higher;
	bitmap_ind8 m_tilemap_bitmapflags_higher;
	int m_depthA = 0;
	int m_depthB = 0;
	int m_use_spc_bright = 0;
	int m_use_v3_bright = 0;
	uint8_t m_bright_spc_b = 0;
	uint8_t m_bright_spc_g = 0;
	uint8_t m_bright_spc_r = 0;
	uint8_t m_bright_spc_b_trans = 0;
	uint8_t m_bright_spc_g_trans = 0;
	uint8_t m_bright_spc_r_trans = 0;
	uint8_t m_bright_v3_b = 0;
	uint8_t m_bright_v3_g = 0;
	uint8_t m_bright_v3_r = 0;
	uint8_t m_bright_v3_b_trans = 0;
	uint8_t m_bright_v3_g_trans = 0;
	uint8_t m_bright_v3_r_trans = 0;
	int m_spc_changed = 0;
	int m_v3_changed = 0;
	int m_palette_updated = 0;
	int m_alt_enable_background = 0;
	int m_alt_enable_sprites = 0;
	tilemap_t *m_tilemap_A = nullptr;
	tilemap_t *m_tilemap_B = nullptr;
	uint8_t *m_btiles = nullptr;
	uint8_t m_region = 0;

	void hit_w(offs_t offset, uint32_t data);
	void hit2_w(uint32_t data);
	uint32_t hit_r(offs_t offset);
	void io_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void v3t_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void pal_regs_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void palette_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void tilemapA_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void tilemapB_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void v3_regs_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t gutsn_speedup_r();
	uint32_t cyvern_speedup_r();
	uint32_t puzzloopj_speedup_r();
	uint32_t puzzloopa_speedup_r();
	uint32_t puzzloopu_speedup_r();
	uint32_t puzzloope_speedup_r();
	uint32_t senknow_speedup_r();
	uint32_t teljan_speedup_r();
	uint32_t jjparads_speedup_r();
	uint32_t jjparad2_speedup_r();
	uint32_t ryouran_speedup_r();
	uint32_t galpans2_speedup_r();
	uint32_t panicstr_speedup_r();
	uint32_t sengekis_speedup_r();
	uint32_t sengekij_speedup_r();

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	virtual void video_reset() override ATTR_COLD;
	DECLARE_MACHINE_RESET(sknsa);
	DECLARE_MACHINE_RESET(sknsj);
	DECLARE_MACHINE_RESET(sknsu);
	DECLARE_MACHINE_RESET(sknse);
	DECLARE_MACHINE_RESET(sknsk);

	TILE_GET_INFO_MEMBER(get_tilemap_A_tile_info);
	TILE_GET_INFO_MEMBER(get_tilemap_B_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);

	TIMER_DEVICE_CALLBACK_MEMBER(interrupt_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(irq);
	void draw_roz(bitmap_ind16 &bitmap, bitmap_ind8& bitmapflags, const rectangle &cliprect, tilemap_t *tmap, uint32_t startx, uint32_t starty, int incxx, int incxy, int incyx, int incyy, int wraparound, int columnscroll, uint32_t* scrollram);
	void palette_set_rgb_brightness (int offset, uint8_t brightness_r, uint8_t brightness_g, uint8_t brightness_b);
	void palette_update();
	void draw_a( bitmap_ind16 &bitmap, bitmap_ind8 &bitmap_flags, const rectangle &cliprect, int tran );
	void draw_b( bitmap_ind16 &bitmap, bitmap_ind8 &bitmap_flags, const rectangle &cliprect, int tran );
	void hit_recalc();
	void init_drc();
	void set_drc_pcflush(uint32_t addr);

	void skns_map(address_map &map) ATTR_COLD;
};

#endif // MAME_KANEKO_SUPRNOVA_H
