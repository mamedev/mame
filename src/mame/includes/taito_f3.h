// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#include "sound/okim6295.h"

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
	enum
	{
		TIMER_F3_INTERRUPT3
	};

	taito_f3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_f3_ram(*this,"f3_ram"),
		m_paletteram32(*this, "paletteram"),
		m_input(*this, "IN"),
		m_dial(*this, "DIAL"),
		m_eepromin(*this, "EEPROMIN")

	{ }

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	optional_shared_ptr<UINT32> m_f3_ram;
	optional_shared_ptr<UINT32> m_paletteram32;
	optional_ioport_array<6> m_input;
	optional_ioport_array<2> m_dial;
	optional_ioport m_eepromin;


	UINT16 *m_videoram;
	UINT16 *m_spriteram;
	UINT16 *m_f3_vram;
	UINT16 *m_f3_line_ram;
	UINT16 *m_f3_pf_data;
	UINT16 *m_f3_pivot_ram;

	UINT32 m_coin_word[2];
	int m_f3_game;
	tilemap_t *m_pf1_tilemap;
	tilemap_t *m_pf2_tilemap;
	tilemap_t *m_pf3_tilemap;
	tilemap_t *m_pf4_tilemap;
	tilemap_t *m_pf5_tilemap;
	tilemap_t *m_pf6_tilemap;
	tilemap_t *m_pf7_tilemap;
	tilemap_t *m_pf8_tilemap;
	tilemap_t *m_pixel_layer;
	tilemap_t *m_vram_layer;
	UINT16 *m_spriteram16_buffered;
	UINT16 m_f3_control_0[8];
	UINT16 m_f3_control_1[8];
	int m_flipscreen;
	UINT8 m_sprite_extra_planes;
	UINT8 m_sprite_pen_mask;
	UINT16 *m_f3_pf_data_1;
	UINT16 *m_f3_pf_data_2;
	UINT16 *m_f3_pf_data_3;
	UINT16 *m_f3_pf_data_4;
	UINT16 *m_f3_pf_data_5;
	UINT16 *m_f3_pf_data_6;
	UINT16 *m_f3_pf_data_7;
	UINT16 *m_f3_pf_data_8;
	int m_f3_skip_this_frame;
	int m_sprite_lag;
	UINT8 m_sprite_pri_usage;
	bitmap_ind8 m_pri_alp_bitmap;
	int m_f3_alpha_level_2as;
	int m_f3_alpha_level_2ad;
	int m_f3_alpha_level_3as;
	int m_f3_alpha_level_3ad;
	int m_f3_alpha_level_2bs;
	int m_f3_alpha_level_2bd;
	int m_f3_alpha_level_3bs;
	int m_f3_alpha_level_3bd;
	int m_alpha_level_last;
	int m_width_mask;
	int m_twidth_mask;
	int m_twidth_mask_bit;
	UINT8 *m_tile_opaque_sp;
	UINT8 *m_tile_opaque_pf[8];
	UINT8 m_add_sat[256][256];
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
	UINT32 m_dval;
	UINT8 m_pval;
	UINT8 m_tval;
	UINT8 m_pdest_2a;
	UINT8 m_pdest_2b;
	int m_tr_2a;
	int m_tr_2b;
	UINT8 m_pdest_3a;
	UINT8 m_pdest_3b;
	int m_tr_3a;
	int m_tr_3b;
	UINT16 *m_src0;
	UINT16 *m_src_s0;
	UINT16 *m_src_e0;
	UINT16 m_clip_al0;
	UINT16 m_clip_ar0;
	UINT16 m_clip_bl0;
	UINT16 m_clip_br0;
	UINT8 *m_tsrc0;
	UINT8 *m_tsrc_s0;
	UINT32 m_x_count0;
	UINT32 m_x_zoom0;
	UINT16 *m_src1;
	UINT16 *m_src_s1;
	UINT16 *m_src_e1;
	UINT16 m_clip_al1;
	UINT16 m_clip_ar1;
	UINT16 m_clip_bl1;
	UINT16 m_clip_br1;
	UINT8 *m_tsrc1;
	UINT8 *m_tsrc_s1;
	UINT32 m_x_count1;
	UINT32 m_x_zoom1;
	UINT16 *m_src2;
	UINT16 *m_src_s2;
	UINT16 *m_src_e2;
	UINT16 m_clip_al2;
	UINT16 m_clip_ar2;
	UINT16 m_clip_bl2;
	UINT16 m_clip_br2;
	UINT8 *m_tsrc2;
	UINT8 *m_tsrc_s2;
	UINT32 m_x_count2;
	UINT32 m_x_zoom2;
	UINT16 *m_src3;
	UINT16 *m_src_s3;
	UINT16 *m_src_e3;
	UINT16 m_clip_al3;
	UINT16 m_clip_ar3;
	UINT16 m_clip_bl3;
	UINT16 m_clip_br3;
	UINT8 *m_tsrc3;
	UINT8 *m_tsrc_s3;
	UINT32 m_x_count3;
	UINT32 m_x_zoom3;
	UINT16 *m_src4;
	UINT16 *m_src_s4;
	UINT16 *m_src_e4;
	UINT16 m_clip_al4;
	UINT16 m_clip_ar4;
	UINT16 m_clip_bl4;
	UINT16 m_clip_br4;
	UINT8 *m_tsrc4;
	UINT8 *m_tsrc_s4;
	UINT32 m_x_count4;
	UINT32 m_x_zoom4;
	struct tempsprite *m_spritelist;
	const struct tempsprite *m_sprite_end;
	struct f3_playfield_line_inf *m_pf_line_inf;
	struct f3_spritealpha_line_inf *m_sa_line_inf;
	const struct F3config *m_f3_game_config;
	int (taito_f3_state::*m_dpix_n[8][16])(UINT32 s_pix);
	int (taito_f3_state::**m_dpix_lp[5])(UINT32 s_pix);
	int (taito_f3_state::**m_dpix_sp[9])(UINT32 s_pix);

	DECLARE_READ32_MEMBER(f3_control_r);
	DECLARE_WRITE32_MEMBER(f3_control_w);
	DECLARE_WRITE32_MEMBER(f3_sound_reset_0_w);
	DECLARE_WRITE32_MEMBER(f3_sound_reset_1_w);
	DECLARE_WRITE32_MEMBER(f3_sound_bankswitch_w);
	DECLARE_WRITE16_MEMBER(f3_unk_w);
	DECLARE_READ32_MEMBER(bubsympb_oki_r);
	DECLARE_WRITE32_MEMBER(bubsympb_oki_w);
	DECLARE_READ16_MEMBER(f3_pf_data_r);
	DECLARE_WRITE16_MEMBER(f3_pf_data_w);
	DECLARE_WRITE16_MEMBER(f3_control_0_w);
	DECLARE_WRITE16_MEMBER(f3_control_1_w);
	DECLARE_READ16_MEMBER(f3_spriteram_r);
	DECLARE_WRITE16_MEMBER(f3_spriteram_w);
	DECLARE_READ16_MEMBER(f3_videoram_r);
	DECLARE_WRITE16_MEMBER(f3_videoram_w);
	DECLARE_READ16_MEMBER(f3_vram_r);
	DECLARE_WRITE16_MEMBER(f3_vram_w);
	DECLARE_READ16_MEMBER(f3_pivot_r);
	DECLARE_WRITE16_MEMBER(f3_pivot_w);
	DECLARE_READ16_MEMBER(f3_lineram_r);
	DECLARE_WRITE16_MEMBER(f3_lineram_w);
	DECLARE_WRITE32_MEMBER(f3_palette_24bit_w);
	DECLARE_CUSTOM_INPUT_MEMBER(f3_analog_r);
	DECLARE_CUSTOM_INPUT_MEMBER(f3_coin_r);
	DECLARE_CUSTOM_INPUT_MEMBER(eeprom_read);
	DECLARE_DRIVER_INIT(commandw);
	DECLARE_DRIVER_INIT(pbobble2);
	DECLARE_DRIVER_INIT(puchicar);
	DECLARE_DRIVER_INIT(intcup94);
	DECLARE_DRIVER_INIT(landmakr);
	DECLARE_DRIVER_INIT(twinqix);
	DECLARE_DRIVER_INIT(elvactr);
	DECLARE_DRIVER_INIT(arabianm);
	DECLARE_DRIVER_INIT(bubsympb);
	DECLARE_DRIVER_INIT(ktiger2);
	DECLARE_DRIVER_INIT(lightbr);
	DECLARE_DRIVER_INIT(gekirido);
	DECLARE_DRIVER_INIT(arkretrn);
	DECLARE_DRIVER_INIT(kirameki);
	DECLARE_DRIVER_INIT(qtheater);
	DECLARE_DRIVER_INIT(popnpop);
	DECLARE_DRIVER_INIT(spcinvdj);
	DECLARE_DRIVER_INIT(pbobbl2p);
	DECLARE_DRIVER_INIT(landmkrp);
	DECLARE_DRIVER_INIT(bubblem);
	DECLARE_DRIVER_INIT(ridingf);
	DECLARE_DRIVER_INIT(gseeker);
	DECLARE_DRIVER_INIT(bubsymph);
	DECLARE_DRIVER_INIT(hthero95);
	DECLARE_DRIVER_INIT(gunlock);
	DECLARE_DRIVER_INIT(pbobble4);
	DECLARE_DRIVER_INIT(dariusg);
	DECLARE_DRIVER_INIT(recalh);
	DECLARE_DRIVER_INIT(kaiserkn);
	DECLARE_DRIVER_INIT(spcinv95);
	DECLARE_DRIVER_INIT(trstaroj);
	DECLARE_DRIVER_INIT(ringrage);
	DECLARE_DRIVER_INIT(cupfinal);
	DECLARE_DRIVER_INIT(quizhuhu);
	DECLARE_DRIVER_INIT(pbobble3);
	DECLARE_DRIVER_INIT(cleopatr);
	DECLARE_DRIVER_INIT(scfinals);
	DECLARE_DRIVER_INIT(pbobbl2x);
	TILE_GET_INFO_MEMBER(get_tile_info1);
	TILE_GET_INFO_MEMBER(get_tile_info2);
	TILE_GET_INFO_MEMBER(get_tile_info3);
	TILE_GET_INFO_MEMBER(get_tile_info4);
	TILE_GET_INFO_MEMBER(get_tile_info5);
	TILE_GET_INFO_MEMBER(get_tile_info6);
	TILE_GET_INFO_MEMBER(get_tile_info7);
	TILE_GET_INFO_MEMBER(get_tile_info8);
	TILE_GET_INFO_MEMBER(get_tile_info_vram);
	TILE_GET_INFO_MEMBER(get_tile_info_pixel);
	DECLARE_MACHINE_START(f3);
	DECLARE_MACHINE_RESET(f3);
	DECLARE_VIDEO_START(f3);
	UINT32 screen_update_f3(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_eof_f3(screen_device &screen, bool state);
	INTERRUPT_GEN_MEMBER(f3_interrupt2);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	inline void get_tile_info(tile_data &tileinfo, int tile_index, UINT16 *gfx_base);
	inline void f3_drawgfx(bitmap_rgb32 &dest_bmp,const rectangle &clip,gfx_element *gfx,int code,int color,int flipx,int flipy,int sx,int sy,UINT8 pri_dst);
	inline void f3_drawgfxzoom(bitmap_rgb32 &dest_bmp,const rectangle &clip,gfx_element *gfx,int code,int color,int flipx,int flipy,int sx,int sy,int scalex,int scaley,UINT8 pri_dst);
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void get_sprite_info(const UINT16 *spriteram16_ptr);
	void print_debug_info(bitmap_rgb32 &bitmap);
	inline void f3_alpha_set_level();
	inline void f3_alpha_blend32_s(int alphas, UINT32 s);
	inline void f3_alpha_blend32_d(int alphas, UINT32 s);
	inline void f3_alpha_blend_1_1(UINT32 s);
	inline void f3_alpha_blend_1_2(UINT32 s);
	inline void f3_alpha_blend_1_4(UINT32 s);
	inline void f3_alpha_blend_1_5(UINT32 s);
	inline void f3_alpha_blend_1_6(UINT32 s);
	inline void f3_alpha_blend_1_8(UINT32 s);
	inline void f3_alpha_blend_1_9(UINT32 s);
	inline void f3_alpha_blend_1_a(UINT32 s);
	inline void f3_alpha_blend_2a_0(UINT32 s);
	inline void f3_alpha_blend_2a_4(UINT32 s);
	inline void f3_alpha_blend_2a_8(UINT32 s);
	inline void f3_alpha_blend_2b_0(UINT32 s);
	inline void f3_alpha_blend_2b_4(UINT32 s);
	inline void f3_alpha_blend_2b_8(UINT32 s);
	inline void f3_alpha_blend_3a_0(UINT32 s);
	inline void f3_alpha_blend_3a_1(UINT32 s);
	inline void f3_alpha_blend_3a_2(UINT32 s);
	inline void f3_alpha_blend_3b_0(UINT32 s);
	inline void f3_alpha_blend_3b_1(UINT32 s);
	inline void f3_alpha_blend_3b_2(UINT32 s);
	int dpix_1_noalpha(UINT32 s_pix);
	int dpix_ret1(UINT32 s_pix);
	int dpix_ret0(UINT32 s_pix);
	int dpix_1_1(UINT32 s_pix);
	int dpix_1_2(UINT32 s_pix);
	int dpix_1_4(UINT32 s_pix);
	int dpix_1_5(UINT32 s_pix);
	int dpix_1_6(UINT32 s_pix);
	int dpix_1_8(UINT32 s_pix);
	int dpix_1_9(UINT32 s_pix);
	int dpix_1_a(UINT32 s_pix);
	int dpix_2a_0(UINT32 s_pix);
	int dpix_2a_4(UINT32 s_pix);
	int dpix_2a_8(UINT32 s_pix);
	int dpix_3a_0(UINT32 s_pix);
	int dpix_3a_1(UINT32 s_pix);
	int dpix_3a_2(UINT32 s_pix);
	int dpix_2b_0(UINT32 s_pix);
	int dpix_2b_4(UINT32 s_pix);
	int dpix_2b_8(UINT32 s_pix);
	int dpix_3b_0(UINT32 s_pix);
	int dpix_3b_1(UINT32 s_pix);
	int dpix_3b_2(UINT32 s_pix);
	int dpix_2_0(UINT32 s_pix);
	int dpix_2_4(UINT32 s_pix);
	int dpix_2_8(UINT32 s_pix);
	int dpix_3_0(UINT32 s_pix);
	int dpix_3_1(UINT32 s_pix);
	int dpix_3_2(UINT32 s_pix);
	inline void dpix_1_sprite(UINT32 s_pix);
	inline void dpix_bg(UINT32 bgcolor);
	void init_alpha_blend_func();
	inline void draw_scanlines(bitmap_rgb32 &bitmap, int xsize, INT16 *draw_line_num, const struct f3_playfield_line_inf **line_t, const int *sprite, UINT32 orient, int skip_layer_num);
	void visible_tile_check(struct f3_playfield_line_inf *line_t, int line, UINT32 x_index_fx,UINT32 y_index, UINT16 *f3_pf_data_n);
	void calculate_clip(int y, UINT16 pri, UINT32* clip0, UINT32* clip1, int* line_enable);
	void get_spritealphaclip_info();
	void get_line_ram_info(tilemap_t *tmap, int sx, int sy, int pos, UINT16 *f3_pf_data_n);
	void get_vram_info(tilemap_t *vram_tilemap, tilemap_t *pixel_tilemap, int sx, int sy);
	void scanline_draw(bitmap_rgb32 &bitmap, const rectangle &cliprect);
};
