// license:BSD-3-Clause
// copyright-holders:David Haywood, Sylvain Glaize, Paul Priest, Olivier Galibert
#ifndef MAME_KANEKO_SUPRNOVA_H
#define MAME_KANEKO_SUPRNOVA_H

#pragma once

#include "kaneko_rlespr.h"

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
		m_tilemap_ram(*this, "tilemap%c_ram", 'a'),
		m_v3slc_ram(*this, "v3slc_ram"),
		m_pal_regs(*this, "pal_regs"),
		m_palette_ram(*this, "palette_ram"),
		m_v3t_ram(*this, "v3t_ram"),
		m_main_ram(*this, "main_ram"),
		m_cache_ram(*this, "cache_ram"),
		m_paddle(*this, "PADDLE.%c", 'A')
	{ }

	void sknsk(machine_config &config) ATTR_COLD;
	void sknsu(machine_config &config) ATTR_COLD;
	void sknsa(machine_config &config) ATTR_COLD;
	void sknsj(machine_config &config) ATTR_COLD;
	void sknse(machine_config &config) ATTR_COLD;
	void skns(machine_config &config) ATTR_COLD;

	void init_sengekis() ATTR_COLD;
	void init_cyvern() ATTR_COLD;
	void init_puzzloopa() ATTR_COLD;
	void init_teljan() ATTR_COLD;
	void init_panicstr() ATTR_COLD;
	void init_puzzloope() ATTR_COLD;
	void init_sengekij() ATTR_COLD;
	void init_puzzloopj() ATTR_COLD;
	void init_sarukani() ATTR_COLD;
	void init_gutsn() ATTR_COLD;
	void init_jjparad2() ATTR_COLD;
	void init_galpans3() ATTR_COLD;
	void init_jjparads() ATTR_COLD;
	void init_galpans2() ATTR_COLD;
	void init_galpanis() ATTR_COLD;
	void init_puzzloopu() ATTR_COLD;
	void init_senknow() ATTR_COLD;
	void init_galpani4() ATTR_COLD;
	void init_ryouran() ATTR_COLD;

	template <int P> ioport_value paddle_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	virtual void video_reset() override ATTR_COLD;

private:
	struct hit_t
	{
		u16 x1p = 0, y1p = 0, z1p = 0, x1s = 0, y1s = 0, z1s = 0;
		u16 x2p = 0, y2p = 0, z2p = 0, x2s = 0, y2s = 0, z2s = 0;
		u16 org = 0;

		u16 x1_p1 = 0, x1_p2 = 0, y1_p1 = 0, y1_p2 = 0, z1_p1 = 0, z1_p2 = 0;
		u16 x2_p1 = 0, x2_p2 = 0, y2_p1 = 0, y2_p2 = 0, z2_p1 = 0, z2_p2 = 0;
		u16 x1tox2 = 0, y1toy2 = 0, z1toz2 = 0;
		s16 x_in = 0, y_in = 0, z_in = 0;
		u16 flag = 0;

		u8 disconnect = 0;
	};

	required_device<sh7604_device> m_maincpu;
	required_device<kaneko_rle_sprites_device> m_spritegen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	required_shared_ptr<u32> m_spriteram;
	required_shared_ptr<u32> m_spc_regs;
	required_shared_ptr<u32> m_v3_regs;
	required_shared_ptr_array<u32, 2> m_tilemap_ram;
	required_shared_ptr<u32> m_v3slc_ram;
	required_shared_ptr<u32> m_pal_regs;
	required_shared_ptr<u32> m_palette_ram;
	required_shared_ptr<u32> m_v3t_ram;
	required_shared_ptr<u32> m_main_ram;
	required_shared_ptr<u32> m_cache_ram;

	optional_ioport_array<4> m_paddle;

	hit_t m_hit;
	bitmap_ind16 m_tilemap_bitmap_lower;
	bitmap_ind8 m_tilemap_bitmapflags_lower;
	bitmap_ind16 m_tilemap_bitmap_higher;
	bitmap_ind8 m_tilemap_bitmapflags_higher;
	bool m_use_spc_bright = false;
	bool m_use_v3_bright = false;
	u8 m_bright_spc_b = 0;
	u8 m_bright_spc_g = 0;
	u8 m_bright_spc_r = 0;
	u8 m_bright_spc_b_trans = 0;
	u8 m_bright_spc_g_trans = 0;
	u8 m_bright_spc_r_trans = 0;
	u8 m_bright_v3_b = 0;
	u8 m_bright_v3_g = 0;
	u8 m_bright_v3_r = 0;
	u8 m_bright_v3_b_trans = 0;
	u8 m_bright_v3_g_trans = 0;
	u8 m_bright_v3_r_trans = 0;
	bool m_spc_changed = false;
	bool m_v3_changed = false;
	bool m_palette_updated = false;
	bool m_alt_enable_background = false;
	bool m_alt_enable_sprites = false;
	tilemap_t *m_tilemap[2]{nullptr};
	u8 *m_btiles = nullptr;
	u8 m_region = 0;

	void hit_w(offs_t offset, u32 data);
	void hit2_w(u32 data);
	u32 hit_r(offs_t offset);
	void io_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void v3t_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void pal_regs_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void palette_ram_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <unsigned Which> void tilemapram_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void v3_regs_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u32 gutsn_speedup_r();
	u32 cyvern_speedup_r();
	u32 puzzloopj_speedup_r();
	u32 puzzloopa_speedup_r();
	u32 puzzloopu_speedup_r();
	u32 puzzloope_speedup_r();
	u32 senknow_speedup_r();
	u32 teljan_speedup_r();
	u32 jjparads_speedup_r();
	u32 jjparad2_speedup_r();
	u32 ryouran_speedup_r();
	u32 galpans2_speedup_r();
	u32 panicstr_speedup_r();
	u32 sengekis_speedup_r();
	u32 sengekij_speedup_r();

	DECLARE_MACHINE_RESET(sknsa);
	DECLARE_MACHINE_RESET(sknsj);
	DECLARE_MACHINE_RESET(sknsu);
	DECLARE_MACHINE_RESET(sknse);
	DECLARE_MACHINE_RESET(sknsk);

	template <unsigned Which> TILE_GET_INFO_MEMBER(get_tile_info);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);

	TIMER_DEVICE_CALLBACK_MEMBER(interrupt_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(irq);
	void draw_roz(bitmap_ind16 &bitmap, bitmap_ind8& bitmapflags, const rectangle &cliprect, tilemap_t *tmap, u32 startx, u32 starty, int incxx, int incxy, int incyx, int incyy, int wraparound, int columnscroll, u32* scrollram);
	void palette_set_brightness(offs_t offset);
	void palette_update();
	void draw_tilemap(bitmap_ind16 &bitmap, bitmap_ind8 &bitmap_flags, const rectangle &cliprect, int which);
	void hit_recalc();
	void init_drc();
	void set_drc_pcflush(u32 addr);

	void skns_map(address_map &map) ATTR_COLD;
};

#endif // MAME_KANEKO_SUPRNOVA_H
