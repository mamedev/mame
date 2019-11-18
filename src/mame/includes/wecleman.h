// license:BSD-3-Clause
// copyright-holders:Luca Elia
#ifndef MAME_INCLUDES_WECLEMAN_H
#define MAME_INCLUDES_WECLEMAN_H

#pragma once

#include "machine/timer.h"
#include "sound/k007232.h"
#include "video/k051316.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class wecleman_state : public driver_device
{
public:
	wecleman_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_videostatus(*this, "videostatus")
		, m_protection_ram(*this, "protection_ram")
		, m_blitter_regs(*this, "blitter_regs")
		, m_pageram(*this, "pageram")
		, m_txtram(*this, "txtram")
		, m_spriteram(*this, "spriteram")
		, m_roadram(*this, "roadram")
		, m_generic_paletteram_16(*this, "paletteram")
		, m_sprite_region(*this, "sprites")
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_subcpu(*this, "sub")
		, m_k051316(*this, "k051316_%u", 1)
		, m_k007232(*this, "k007232_%u", 1)
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
		, m_led(*this, "led%u", 0U)
	{ }

	void hotchase(machine_config &config);
	void wecleman(machine_config &config);

	void init_wecleman();
	void init_hotchase();

	DECLARE_READ_LINE_MEMBER(hotchase_sound_status_r);

private:
	enum
	{
		WECLEMAN_ID = 0,
		HOTCHASE_ID
	};

	optional_shared_ptr<uint16_t> m_videostatus;
	optional_shared_ptr<uint16_t> m_protection_ram;
	required_shared_ptr<uint16_t> m_blitter_regs;
	optional_shared_ptr<uint16_t> m_pageram;
	optional_shared_ptr<uint16_t> m_txtram;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_roadram;
	required_shared_ptr<uint16_t> m_generic_paletteram_16;

	required_region_ptr<uint8_t> m_sprite_region;

	int m_multiply_reg[2];
	int m_spr_color_offs;
	int m_prot_state;
	int m_selected_ip;
	int m_irqctrl;
	int m_bgpage[4];
	int m_fgpage[4];
	const int *m_gfx_bank;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_txt_tilemap;
	int *m_spr_idx_list;
	int *m_spr_pri_list;
	int *m_t32x32pm;
	int m_gameid;
	int m_spr_offsx;
	int m_spr_offsy;
	int m_spr_count;
	uint16_t *m_rgb_half;
	int m_cloud_blend;
	int m_cloud_ds;
	int m_cloud_visible;
	int m_sound_hw_type;
	bool m_hotchase_sound_hs;
	pen_t m_black_pen;

	DECLARE_READ16_MEMBER(wecleman_protection_r);
	DECLARE_WRITE16_MEMBER(wecleman_protection_w);
	DECLARE_WRITE16_MEMBER(irqctrl_w);
	DECLARE_WRITE16_MEMBER(selected_ip_w);
	uint8_t selected_ip_r();
	DECLARE_WRITE16_MEMBER(blitter_w);
	DECLARE_READ8_MEMBER(multiply_r);
	DECLARE_WRITE8_MEMBER(multiply_w);
	DECLARE_WRITE8_MEMBER(hotchase_sound_control_w);
	DECLARE_WRITE16_MEMBER(wecleman_txtram_w);
	DECLARE_WRITE16_MEMBER(wecleman_pageram_w);
	DECLARE_WRITE16_MEMBER(wecleman_videostatus_w);
	DECLARE_WRITE16_MEMBER(hotchase_paletteram16_SBGRBBBBGGGGRRRR_word_w);
	DECLARE_WRITE16_MEMBER(wecleman_paletteram16_SSSSBBBBGGGGRRRR_word_w);
	DECLARE_WRITE8_MEMBER(wecleman_K00723216_bank_w);
	DECLARE_WRITE8_MEMBER(wecleman_volume_callback);
	template<int Chip> DECLARE_READ8_MEMBER(hotchase_k007232_r);
	template<int Chip> DECLARE_WRITE8_MEMBER(hotchase_k007232_w);

	TILE_GET_INFO_MEMBER(wecleman_get_txt_tile_info);
	TILE_GET_INFO_MEMBER(wecleman_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(wecleman_get_fg_tile_info);

	DECLARE_MACHINE_START(wecleman);
	DECLARE_MACHINE_RESET(wecleman);
	DECLARE_VIDEO_START(wecleman);

	DECLARE_MACHINE_START(hotchase);
	DECLARE_MACHINE_RESET(hotchase);
	DECLARE_VIDEO_START(hotchase);

	uint32_t screen_update_wecleman(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_hotchase(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(hotchase_sound_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(wecleman_scanline);
	TIMER_DEVICE_CALLBACK_MEMBER(hotchase_scanline);
	void draw_cloud(bitmap_rgb32 &bitmap,gfx_element *gfx,uint16_t *tm_base,int x0,int y0,int xcount,int ycount,int scrollx,int scrolly,int tmw_l2,int tmh_l2,int alpha,int pal_offset);
	void wecleman_unpack_sprites();
	void bitswap(uint8_t *src,size_t len,int _14,int _13,int _12,int _11,int _10,int _f,int _e,int _d,int _c,int _b,int _a,int _9,int _8,int _7,int _6,int _5,int _4,int _3,int _2,int _1,int _0);
	void hotchase_sprite_decode( int num16_banks, int bank_size );
	void get_sprite_info();
	void sortsprite(int *idx_array, int *key_array, int size);
	void wecleman_draw_road(bitmap_rgb32 &bitmap, const rectangle &cliprect, int priority);
	void hotchase_draw_road(bitmap_ind16 &bitmap, const rectangle &cliprect);
	K051316_CB_MEMBER(hotchase_zoom_callback_1);
	K051316_CB_MEMBER(hotchase_zoom_callback_2);

	DECLARE_WRITE8_MEMBER(hotchase_sound_hs_w);

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	optional_device_array<k051316_device, 2> m_k051316;
	optional_device_array<k007232_device, 3> m_k007232;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	output_finder<1> m_led;

	void hotchase_map(address_map &map);
	void hotchase_sound_map(address_map &map);
	void hotchase_sub_map(address_map &map);
	void wecleman_map(address_map &map);
	void wecleman_sound_map(address_map &map);
	void wecleman_sub_map(address_map &map);

	struct sprite_t
	{
		sprite_t() { }

		uint8_t *pen_data = nullptr;    /* points to top left corner of tile data */
		int line_offset = 0;

		const pen_t *pal_data = nullptr;
		rgb_t pal_base;

		int x_offset = 0, y_offset = 0;
		int tile_width = 0, tile_height = 0;
		int total_width = 0, total_height = 0;  /* in screen coordinates */
		int x = 0, y = 0;
		int shadow_mode = 0, flags = 0;
	};

	template<class _BitmapClass> void do_blit_zoom32(_BitmapClass &bitmap, const rectangle &cliprect, const sprite_t &sprite);
	template<class _BitmapClass> void sprite_draw(_BitmapClass &bitmap, const rectangle &cliprect);

	std::unique_ptr<sprite_t []> m_sprite_list;
	sprite_t **m_spr_ptr_list;
};

#endif // MAME_INCLUDES_WECLEMAN_H
