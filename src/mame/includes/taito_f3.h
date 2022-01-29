// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#ifndef MAME_INCLUDES_TAITO_F3_H
#define MAME_INCLUDES_TAITO_F3_H

#pragma once

#include "audio/taito_en.h"
#include "machine/eepromser.h"
#include "machine/watchdog.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

struct F3config;

/* This it the best way to allow game specific kludges until the system is fully understood */
enum {
	/* Early F3 class games, these are not cartridge games and system features may be different */
	RINGRAGE=0, /* D21 */
	ARABIANM,   /* D29 */
	RIDINGF,    /* D34 */
	GSEEKER,    /* D40 */
	TRSTAR,     /* D53 */
	GUNLOCK,    /* D66 */
	TWINQIX,
	UNDRFIRE,   /* D67 - Heavily modified F3 hardware (different memory map) */
	SCFINALS,
	LIGHTBR,    /* D69 */

	/* D77 - F3 motherboard proms, all following games are 'F3 package system' */
	/* D78 I CUP */
	KAISERKN,   /* D84 */
	DARIUSG,    /* D87 */
	BUBSYMPH,   /* D90 */
	SPCINVDX,   /* D93 */
	HTHERO95,   /* D94 */
	QTHEATER,   /* D95 */
	EACTION2,   /* E02 */
	SPCINV95,   /* E06 */
	QUIZHUHU,   /* E08 */
	PBOBBLE2,   /* E10 */
	GEKIRIDO,   /* E11 */
	KTIGER2,    /* E15 */
	BUBBLEM,    /* E21 */
	CLEOPATR,   /* E28 */
	PBOBBLE3,   /* E29 */
	ARKRETRN,   /* E36 */
	KIRAMEKI,   /* E44 */
	PUCHICAR,   /* E46 */
	PBOBBLE4,   /* E49 */
	POPNPOP,    /* E51 */
	LANDMAKR,   /* E61 */
	RECALH,     /* prototype */
	COMMANDW,   /* prototype */
	TMDRILL
};

class taito_f3_state : public driver_device
{
public:
	taito_f3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_eeprom(*this, "eeprom"),
		m_textram(*this, "textram", 0x2000, ENDIANNESS_BIG),
		m_spriteram(*this, "spriteram", 0x10000, ENDIANNESS_BIG),
		m_charram(*this, "charram", 0x2000, ENDIANNESS_BIG),
		m_line_ram(*this, "line_ram", 0x10000, ENDIANNESS_BIG),
		m_pf_ram(*this, "pf_ram", 0xc000, ENDIANNESS_BIG),
		m_pivot_ram(*this, "pivot_ram", 0x10000, ENDIANNESS_BIG),
		m_input(*this, "IN.%u", 0),
		m_dial(*this, "DIAL.%u", 0),
		m_eepromin(*this, "EEPROMIN"),
		m_eepromout(*this, "EEPROMOUT"),
		m_taito_en(*this, "taito_en"),
		m_oki(*this, "oki"),
		m_paletteram32(*this, "paletteram"),
		m_okibank(*this, "okibank")
	{ }

	void f3(machine_config &config);
	void f3_224a(machine_config &config);
	void bubsympb(machine_config &config);
	void f3_224b(machine_config &config);
	void f3_224c(machine_config &config);

	void init_commandw();
	void init_pbobble2();
	void init_puchicar();
	void init_intcup94();
	void init_landmakr();
	void init_twinqix();
	void init_elvactr();
	void init_arabianm();
	void init_bubsympb();
	void init_ktiger2();
	void init_lightbr();
	void init_gekirido();
	void init_arkretrn();
	void init_kirameki();
	void init_qtheater();
	void init_popnpop();
	void init_spcinvdj();
	void init_pbobbl2p();
	void init_landmkrp();
	void init_bubblem();
	void init_ridingf();
	void init_gseeker();
	void init_bubsymph();
	void init_hthero95();
	void init_gunlock();
	void init_pbobble4();
	void init_dariusg();
	void init_recalh();
	void init_kaiserkn();
	void init_spcinv95();
	void init_trstaroj();
	void init_ringrage();
	void init_cupfinal();
	void init_quizhuhu();
	void init_pbobble3();
	void init_cleopatr();
	void init_scfinals();
	void init_pbobbl2x();

	template <int Num> DECLARE_CUSTOM_INPUT_MEMBER(f3_analog_r);
	template <int Num> DECLARE_CUSTOM_INPUT_MEMBER(f3_coin_r);
	DECLARE_CUSTOM_INPUT_MEMBER(eeprom_read);

protected:
	enum
	{
		TIMER_F3_INTERRUPT3
	};

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;
	virtual void device_post_load(void) override;

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	required_device<cpu_device> m_maincpu;
	optional_device<watchdog_timer_device> m_watchdog;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<eeprom_serial_base_device> m_eeprom;

	memory_share_creator<u16> m_textram;
	memory_share_creator<u16> m_spriteram;
	memory_share_creator<u16> m_charram;
	memory_share_creator<u16> m_line_ram;
	memory_share_creator<u16> m_pf_ram;
	memory_share_creator<u16> m_pivot_ram;

	optional_ioport_array<6> m_input;
	optional_ioport_array<2> m_dial;
	optional_ioport m_eepromin;
	optional_ioport m_eepromout;

	emu_timer *m_interrupt3_timer;
	u32 m_coin_word[2];
	std::unique_ptr<u8[]> m_decoded_gfx4;
	std::unique_ptr<u8[]> m_decoded_gfx5;

	struct tempsprite
	{
		int code, color;
		int flipx, flipy;
		int x, y;
		int zoomx, zoomy;
		int pri;
	};

	struct f3_playfield_line_inf
	{
		int alpha_mode[256];
		int pri[256];

		/* use for draw_scanlines */
		u16 *src[256], *src_s[256], *src_e[256];
		u8 *tsrc[256], *tsrc_s[256];
		int x_count[256];
		u32 x_zoom[256];
		u32 clip0[256];
		u32 clip1[256];
	};

	struct f3_spritealpha_line_inf
	{
		u16 alpha_level[256];
		u16 spri[256];
		u16 sprite_alpha[256];
		u32 sprite_clip0[256];
		u32 sprite_clip1[256];
		s16 clip0_l[256];
		s16 clip0_r[256];
		s16 clip1_l[256];
		s16 clip1_r[256];
	};

	int m_game;
	tilemap_t *m_tilemap[8];
	tilemap_t *m_pixel_layer;
	tilemap_t *m_vram_layer;
	std::unique_ptr<u16[]> m_spriteram16_buffered;
	u16 m_control_0[8];
	u16 m_control_1[8];
	int m_flipscreen;
	u8 m_sprite_extra_planes;
	u8 m_sprite_pen_mask;
	u16 *m_pf_data[8];
	int m_sprite_lag;
	u8 m_sprite_pri_usage;
	bitmap_ind8 m_pri_alp_bitmap;
	int m_alpha_level_2as;
	int m_alpha_level_2ad;
	int m_alpha_level_3as;
	int m_alpha_level_3ad;
	int m_alpha_level_2bs;
	int m_alpha_level_2bd;
	int m_alpha_level_3bs;
	int m_alpha_level_3bd;
	int m_alpha_level_last;
	int m_width_mask;
	int m_twidth_mask;
	int m_twidth_mask_bit;
	std::unique_ptr<u8[]> m_tile_opaque_sp;
	std::unique_ptr<u8[]> m_tile_opaque_pf[8];
	u8 m_add_sat[256][256];
	int m_alpha_s_1_1;
	int m_alpha_s_1_2;
	int m_alpha_s_1_4;
	int m_alpha_s_1_5;
	int m_alpha_s_1_6;
	int m_alpha_s_1_8;
	int m_alpha_s_1_9;
	int m_alpha_s_1_a;
	int m_alpha_s_2a_0;
	int m_alpha_s_2a_4;
	int m_alpha_s_2a_8;
	int m_alpha_s_2b_0;
	int m_alpha_s_2b_4;
	int m_alpha_s_2b_8;
	int m_alpha_s_3a_0;
	int m_alpha_s_3a_1;
	int m_alpha_s_3a_2;
	int m_alpha_s_3b_0;
	int m_alpha_s_3b_1;
	int m_alpha_s_3b_2;
	u32 m_dval;
	u8 m_pval;
	u8 m_tval;
	u8 m_pdest_2a;
	u8 m_pdest_2b;
	int m_tr_2a;
	int m_tr_2b;
	u8 m_pdest_3a;
	u8 m_pdest_3b;
	int m_tr_3a;
	int m_tr_3b;
	u16 *m_src[5];
	u16 *m_src_s[5];
	u16 *m_src_e[5];
	u16 m_clip_al[5];
	u16 m_clip_ar[5];
	u16 m_clip_bl[5];
	u16 m_clip_br[5];
	u8 *m_tsrc[5];
	u8 *m_tsrc_s[5];
	u32 m_x_count[5];
	u32 m_x_zoom[5];
	std::unique_ptr<tempsprite[]> m_spritelist;
	const tempsprite *m_sprite_end;
	std::unique_ptr<f3_playfield_line_inf[]> m_pf_line_inf;
	std::unique_ptr<f3_spritealpha_line_inf[]> m_sa_line_inf;
	const F3config *m_game_config;
	int (taito_f3_state::*m_dpix_n[8][16])(u32 s_pix);
	int (taito_f3_state::**m_dpix_lp[5])(u32 s_pix);
	int (taito_f3_state::**m_dpix_sp[9])(u32 s_pix);

	u16 pf_ram_r(offs_t offset);
	void pf_ram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void control_0_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void control_1_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 spriteram_r(offs_t offset);
	void spriteram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 textram_r(offs_t offset);
	void textram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 charram_r(offs_t offset);
	void charram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 pivot_r(offs_t offset);
	void pivot_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 lineram_r(offs_t offset);
	void lineram_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	template<unsigned Layer> TILE_GET_INFO_MEMBER(get_tile_info);
	TILE_GET_INFO_MEMBER(get_tile_info_text);
	TILE_GET_INFO_MEMBER(get_tile_info_pixel);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank);

	void bubsympb_map(address_map &map);
	void f3_map(address_map &map);

	void tile_decode();

	inline void f3_drawgfx(bitmap_rgb32 &dest_bmp, const rectangle &clip, gfx_element *gfx, int code, int color, int flipx, int flipy, int sx, int sy, u8 pri_dst);
	inline void f3_drawgfxzoom(bitmap_rgb32 &dest_bmp, const rectangle &clip, gfx_element *gfx, int code, int color, int flipx, int flipy, int sx, int sy, int scalex, int scaley, u8 pri_dst);
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void get_sprite_info(const u16 *spriteram16_ptr);
	void print_debug_info(bitmap_rgb32 &bitmap);
	inline void alpha_set_level();
	inline void alpha_blend32_s(int alphas, u32 s);
	inline void alpha_blend32_d(int alphas, u32 s);
	inline void alpha_blend_1_1(u32 s);
	inline void alpha_blend_1_2(u32 s);
	inline void alpha_blend_1_4(u32 s);
	inline void alpha_blend_1_5(u32 s);
	inline void alpha_blend_1_6(u32 s);
	inline void alpha_blend_1_8(u32 s);
	inline void alpha_blend_1_9(u32 s);
	inline void alpha_blend_1_a(u32 s);
	inline void alpha_blend_2a_0(u32 s);
	inline void alpha_blend_2a_4(u32 s);
	inline void alpha_blend_2a_8(u32 s);
	inline void alpha_blend_2b_0(u32 s);
	inline void alpha_blend_2b_4(u32 s);
	inline void alpha_blend_2b_8(u32 s);
	inline void alpha_blend_3a_0(u32 s);
	inline void alpha_blend_3a_1(u32 s);
	inline void alpha_blend_3a_2(u32 s);
	inline void alpha_blend_3b_0(u32 s);
	inline void alpha_blend_3b_1(u32 s);
	inline void alpha_blend_3b_2(u32 s);
	int dpix_1_noalpha(u32 s_pix);
	int dpix_ret1(u32 s_pix);
	int dpix_ret0(u32 s_pix);
	int dpix_1_1(u32 s_pix);
	int dpix_1_2(u32 s_pix);
	int dpix_1_4(u32 s_pix);
	int dpix_1_5(u32 s_pix);
	int dpix_1_6(u32 s_pix);
	int dpix_1_8(u32 s_pix);
	int dpix_1_9(u32 s_pix);
	int dpix_1_a(u32 s_pix);
	int dpix_2a_0(u32 s_pix);
	int dpix_2a_4(u32 s_pix);
	int dpix_2a_8(u32 s_pix);
	int dpix_3a_0(u32 s_pix);
	int dpix_3a_1(u32 s_pix);
	int dpix_3a_2(u32 s_pix);
	int dpix_2b_0(u32 s_pix);
	int dpix_2b_4(u32 s_pix);
	int dpix_2b_8(u32 s_pix);
	int dpix_3b_0(u32 s_pix);
	int dpix_3b_1(u32 s_pix);
	int dpix_3b_2(u32 s_pix);
	int dpix_2_0(u32 s_pix);
	int dpix_2_4(u32 s_pix);
	int dpix_2_8(u32 s_pix);
	int dpix_3_0(u32 s_pix);
	int dpix_3_1(u32 s_pix);
	int dpix_3_2(u32 s_pix);
	inline void dpix_1_sprite(u32 s_pix);
	inline void dpix_bg(u32 bgcolor);
	void init_alpha_blend_func();
	inline void draw_scanlines(bitmap_rgb32 &bitmap, int xsize, s16 *draw_line_num, const f3_playfield_line_inf **line_t, const int *sprite, u32 orient, int skip_layer_num);
	void visible_tile_check(f3_playfield_line_inf *line_t, int line, u32 x_index_fx, u32 y_index, u16 *pf_data_n);
	void calculate_clip(int y, u16 pri, u32* clip0, u32* clip1, int *line_enable);
	void get_spritealphaclip_info();
	void get_line_ram_info(tilemap_t *tmap, int sx, int sy, int pos, u16 *pf_data_n);
	void get_vram_info(tilemap_t *vram_tilemap, tilemap_t *pixel_tilemap, int sx, int sy);
	void scanline_draw(bitmap_rgb32 &bitmap, const rectangle &cliprect);

private:
	optional_device<taito_en_device> m_taito_en;
	optional_device<okim6295_device> m_oki;

	optional_shared_ptr<u32> m_paletteram32;
	optional_memory_bank m_okibank;

	void bubsympb_oki_w(u8 data);
	u32 f3_control_r(offs_t offset);
	void f3_control_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void f3_unk_w(offs_t offset, u16 data);
	void sound_reset_0_w(u32 data);
	void sound_reset_1_w(u32 data);
	void sound_bankswitch_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void palette_24bit_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	INTERRUPT_GEN_MEMBER(interrupt2);

	void bubsympb_oki_map(address_map &map);
};

#endif // MAME_INCLUDES_TAITO_F3_H
