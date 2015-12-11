// license:BSD-3-Clause
// copyright-holders:David Haywood, Sylvain Glaize, Paul Priest, Olivier Galibert

#include "cpu/sh2/sh2.h"


struct hit_t
{
	UINT16 x1p, y1p, z1p, x1s, y1s, z1s;
	UINT16 x2p, y2p, z2p, x2s, y2s, z2s;
	UINT16 org;

	UINT16 x1_p1, x1_p2, y1_p1, y1_p2, z1_p1, z1_p2;
	UINT16 x2_p1, x2_p2, y2_p1, y2_p2, z2_p1, z2_p2;
	UINT16 x1tox2, y1toy2, z1toz2;
	INT16 x_in, y_in, z_in;
	UINT16 flag;

	UINT8 disconnect;
};


class skns_state : public driver_device
{
public:
	skns_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_spritegen(*this, "spritegen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
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
		m_cache_ram(*this, "cache_ram") { }

	required_device<sh2_device> m_maincpu;
	required_device<sknsspr_device> m_spritegen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT32> m_spriteram;
	required_shared_ptr<UINT32> m_spc_regs;
	required_shared_ptr<UINT32> m_v3_regs;
	required_shared_ptr<UINT32> m_tilemapA_ram;
	required_shared_ptr<UINT32> m_tilemapB_ram;
	required_shared_ptr<UINT32> m_v3slc_ram;
	required_shared_ptr<UINT32> m_pal_regs;
	required_shared_ptr<UINT32> m_palette_ram;
	required_shared_ptr<UINT32> m_v3t_ram;
	required_shared_ptr<UINT32> m_main_ram;
	required_shared_ptr<UINT32> m_cache_ram;

	hit_t m_hit;
	UINT32 m_timer_0_temp[4];
	bitmap_ind16 m_sprite_bitmap;
	bitmap_ind16 m_tilemap_bitmap_lower;
	bitmap_ind8 m_tilemap_bitmapflags_lower;
	bitmap_ind16 m_tilemap_bitmap_higher;
	bitmap_ind8 m_tilemap_bitmapflags_higher;
	int m_depthA;
	int m_depthB;
	int m_use_spc_bright;
	int m_use_v3_bright;
	UINT8 m_bright_spc_b;
	UINT8 m_bright_spc_g;
	UINT8 m_bright_spc_r;
	UINT8 m_bright_spc_b_trans;
	UINT8 m_bright_spc_g_trans;
	UINT8 m_bright_spc_r_trans;
	UINT8 m_bright_v3_b;
	UINT8 m_bright_v3_g;
	UINT8 m_bright_v3_r;
	UINT8 m_bright_v3_b_trans;
	UINT8 m_bright_v3_g_trans;
	UINT8 m_bright_v3_r_trans;
	int m_spc_changed;
	int m_v3_changed;
	int m_palette_updated;
	int m_alt_enable_background;
	int m_alt_enable_sprites;
	tilemap_t *m_tilemap_A;
	tilemap_t *m_tilemap_B;

	UINT8 m_region;

	DECLARE_WRITE32_MEMBER(skns_hit_w);
	DECLARE_WRITE32_MEMBER(skns_hit2_w);
	DECLARE_READ32_MEMBER(skns_hit_r);
	DECLARE_WRITE32_MEMBER(skns_io_w);
	DECLARE_WRITE32_MEMBER(skns_v3t_w);
	DECLARE_READ32_MEMBER(gutsn_speedup_r);
	DECLARE_READ32_MEMBER(cyvern_speedup_r);
	DECLARE_READ32_MEMBER(puzzloopj_speedup_r);
	DECLARE_READ32_MEMBER(puzzloopa_speedup_r);
	DECLARE_READ32_MEMBER(puzzloopu_speedup_r);
	DECLARE_READ32_MEMBER(puzzloope_speedup_r);
	DECLARE_READ32_MEMBER(senknow_speedup_r);
	DECLARE_READ32_MEMBER(teljan_speedup_r);
	DECLARE_READ32_MEMBER(jjparads_speedup_r);
	DECLARE_READ32_MEMBER(jjparad2_speedup_r);
	DECLARE_READ32_MEMBER(ryouran_speedup_r);
	DECLARE_READ32_MEMBER(galpans2_speedup_r);
	DECLARE_READ32_MEMBER(panicstr_speedup_r);
	DECLARE_READ32_MEMBER(sengekis_speedup_r);
	DECLARE_READ32_MEMBER(sengekij_speedup_r);
	DECLARE_WRITE32_MEMBER(skns_pal_regs_w);
	DECLARE_WRITE32_MEMBER(skns_palette_ram_w);
	DECLARE_WRITE32_MEMBER(skns_tilemapA_w);
	DECLARE_WRITE32_MEMBER(skns_tilemapB_w);
	DECLARE_WRITE32_MEMBER(skns_v3_regs_w);
	DECLARE_CUSTOM_INPUT_MEMBER(paddle_r);
	DECLARE_DRIVER_INIT(sengekis);
	DECLARE_DRIVER_INIT(cyvern);
	DECLARE_DRIVER_INIT(puzzloopa);
	DECLARE_DRIVER_INIT(teljan);
	DECLARE_DRIVER_INIT(panicstr);
	DECLARE_DRIVER_INIT(puzzloope);
	DECLARE_DRIVER_INIT(sengekij);
	DECLARE_DRIVER_INIT(puzzloopj);
	DECLARE_DRIVER_INIT(sarukani);
	DECLARE_DRIVER_INIT(gutsn);
	DECLARE_DRIVER_INIT(jjparad2);
	DECLARE_DRIVER_INIT(galpans3);
	DECLARE_DRIVER_INIT(jjparads);
	DECLARE_DRIVER_INIT(galpans2);
	DECLARE_DRIVER_INIT(galpanis);
	DECLARE_DRIVER_INIT(puzzloopu);
	DECLARE_DRIVER_INIT(senknow);
	DECLARE_DRIVER_INIT(galpani4);
	DECLARE_DRIVER_INIT(ryouran);
	TILE_GET_INFO_MEMBER(get_tilemap_A_tile_info);
	TILE_GET_INFO_MEMBER(get_tilemap_B_tile_info);
	virtual void machine_reset() override;
	virtual void video_start() override;
	virtual void video_reset() override;
	DECLARE_MACHINE_RESET(sknsa);
	DECLARE_MACHINE_RESET(sknsj);
	DECLARE_MACHINE_RESET(sknsu);
	DECLARE_MACHINE_RESET(sknse);
	DECLARE_MACHINE_RESET(sknsk);
	UINT32 screen_update_skns(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_eof_skns(screen_device &screen, bool state);
	TIMER_DEVICE_CALLBACK_MEMBER(interrupt_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(skns_irq);
	void suprnova_draw_roz(bitmap_ind16 &bitmap, bitmap_ind8& bitmapflags, const rectangle &cliprect, tilemap_t *tmap, UINT32 startx, UINT32 starty, int incxx, int incxy, int incyx, int incyy, int wraparound, int columnscroll, UINT32* scrollram);
	void palette_set_rgb_brightness (int offset, UINT8 brightness_r, UINT8 brightness_g, UINT8 brightness_b);
	void palette_update();
	void supernova_draw_a( bitmap_ind16 &bitmap, bitmap_ind8 &bitmap_flags, const rectangle &cliprect, int tran );
	void supernova_draw_b( bitmap_ind16 &bitmap, bitmap_ind8 &bitmap_flags, const rectangle &cliprect, int tran );
	void hit_recalc();
	void init_skns();
	void set_drc_pcflush(UINT32 addr);
};
