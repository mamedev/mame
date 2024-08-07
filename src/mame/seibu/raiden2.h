// license:LGPL-2.1+
// copyright-holders:Olivier Galibert, Angelo Salese, David Haywood, Tomasz Slanina
#ifndef MAME_SEIBU_RAIDEN2_H
#define MAME_SEIBU_RAIDEN2_H

#pragma once

#include "seibu_crtc.h"
#include "seibucop.h"
#include "sei25x_rise1x_spr.h"

#include "seibusound.h"

#include "video/bufsprite.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

#include <algorithm>

class raiden2_state : public driver_device
{
public:
	raiden2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_spriteram(*this, "spriteram")
		, m_maincpu(*this, "maincpu")
		, m_seibu_sound(*this, "seibu_sound")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
		, m_spritegen(*this, "spritegen")
		, m_mainbank(*this, "mainbank")
		, m_raiden2cop(*this, "raiden2cop")

		, m_bg_bank(0)
		, m_fg_bank(0)
		, m_mid_bank(0)
		, m_tx_bank(0)
		, m_tilemap_enable(0)

		, m_sprite_prot_x(0)
		, m_sprite_prot_y(0)
		, m_dst1(0)
		, m_cop_spr_maxx(0)
		, m_cop_spr_off(0)

		, m_prg_bank(0)
		, m_cop_bank(0)
	{
		std::fill(std::begin(m_sprite_prot_src_addr), std::end(m_sprite_prot_src_addr), 0);
		std::fill(std::begin(m_scrollvals), std::end(m_scrollvals), 0);
	}

	void raidendx(machine_config &config);
	void xsedae(machine_config &config);
	void zeroteam(machine_config &config);
	void raiden2(machine_config &config);

	void init_raidendx();
	void init_xsedae();
	void init_zeroteam();
	void init_raiden2();

protected:
	void sprite_prot_x_w(u16 data);
	void sprite_prot_y_w(u16 data);
	void sprite_prot_src_seg_w(u16 data);
	void sprite_prot_src_w(address_space &space, u16 data);
	u16 sprite_prot_src_seg_r();
	u16 sprite_prot_dst1_r();
	u16 sprite_prot_maxx_r();
	u16 sprite_prot_off_r();
	void sprite_prot_dst1_w(u16 data);
	void sprite_prot_maxx_w(u16 data);
	void sprite_prot_off_w(u16 data);

	INTERRUPT_GEN_MEMBER(interrupt);
	void common_save_state();
	virtual void video_start() override;

	void tilemap_enable_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void tile_scroll_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void background_w(offs_t offset, u16 data);
	void foreground_w(offs_t offset, u16 data);
	void midground_w(offs_t offset, u16 data);
	void text_w(offs_t offset, u16 data);
	void m_videoram_private_w(offs_t offset, uint16_t data);

	void bank_reset(int bgbank, int fgbank, int midbank, int txbank);

	DECLARE_GFXDECODE_MEMBER(gfx_raiden2);
	DECLARE_GFXDECODE_MEMBER(gfx_raiden2_spr);
	TILE_GET_INFO_MEMBER(get_back_tile_info);
	TILE_GET_INFO_MEMBER(get_mid_tile_info);
	TILE_GET_INFO_MEMBER(get_fore_tile_info);
	TILE_GET_INFO_MEMBER(get_text_tile_info);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void blend_layer(bitmap_rgb32 &bitmap, const rectangle &cliprect, bitmap_ind16 &source, int layer);
	void tilemap_draw_and_blend(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, tilemap_t *tilemap);

	void init_blending(const u16 *table);

	void zeroteam_sound_map(address_map &map);

	// devices
	required_device<buffered_spriteram16_device> m_spriteram;
	required_device<cpu_device> m_maincpu;
	optional_device<seibu_sound_device> m_seibu_sound;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<sei25x_rise1x_device> m_spritegen;
	optional_memory_bank m_mainbank;
	optional_device<raiden2cop_device> m_raiden2cop;

	// video related
	static u16 const raiden_blended_colors[];
	static u16 const xsedae_blended_colors[];
	static u16 const zeroteam_blended_colors[];

	bool m_blend_active[0x800]{}; // cfg

	tilemap_t *m_background_layer = nullptr;
	tilemap_t *m_midground_layer = nullptr;
	tilemap_t *m_foreground_layer = nullptr;
	tilemap_t *m_text_layer = nullptr;

	std::unique_ptr<u16[]> m_back_data;
	std::unique_ptr<u16[]> m_fore_data;
	std::unique_ptr<u16[]> m_mid_data;
	std::unique_ptr<u16[]> m_text_data; // private buffers, allocated in init
	std::unique_ptr<u16[]> m_palette_data;

	u32 m_bg_bank, m_fg_bank, m_mid_bank, m_tx_bank;
	u16 m_tilemap_enable;

	u16 m_scrollvals[6];

	const int *m_cur_spri = nullptr; // cfg

	bitmap_ind16 m_tile_bitmap;

	// protection related
	u16 m_sprite_prot_x, m_sprite_prot_y, m_dst1, m_cop_spr_maxx, m_cop_spr_off;
	u16 m_sprite_prot_src_addr[2];

private:
	void raiden2_bank_w(u8 data);
	void tile_bank_01_w(u8 data);
	u16 cop_tile_bank_2_r();
	void cop_tile_bank_2_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void raidendx_cop_bank_2_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void sprcpt_val_1_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void sprcpt_val_2_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void sprcpt_data_1_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void sprcpt_data_2_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void sprcpt_data_3_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void sprcpt_data_4_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void sprcpt_adr_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void sprcpt_flags_1_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void sprcpt_flags_2_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	virtual void machine_start() override;
	DECLARE_MACHINE_RESET(raiden2);
	DECLARE_MACHINE_RESET(zeroteam);
	DECLARE_MACHINE_RESET(xsedae);
	DECLARE_MACHINE_RESET(raidendx);

	void combine32(u32 *val, offs_t offset, u16 data, u16 mem_mask);
	void sprcpt_init();
	void raiden2_cop_mem(address_map &map);
	void raiden2_mem(address_map &map);
	void raiden2_sound_map(address_map &map);
	void raidendx_mem(address_map &map);
	void xsedae_mem(address_map &map);
	void zeroteam_mem(address_map &map);

	// misc
	u8 m_prg_bank;
	u16 m_cop_bank;

	// protection related
	u32 m_sprcpt_adr = 0, m_sprcpt_idx = 0;

	u32 m_sprcpt_val[2]{}, m_sprcpt_flags1 = 0;
	u16 m_sprcpt_flags2 = 0;
	u32 m_sprcpt_data_1[0x100]{}, m_sprcpt_data_2[0x40]{}, m_sprcpt_data_3[6]{}, m_sprcpt_data_4[4]{};

};

#endif // MAME_SEIBU_RAIDEN2_H
