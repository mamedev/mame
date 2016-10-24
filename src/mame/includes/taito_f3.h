// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#include "machine/watchdog.h"
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
		m_audiocpu(*this, "taito_en:audiocpu"),
		m_watchdog(*this, "watchdog"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_f3_ram(*this,"f3_ram"),
		m_paletteram32(*this, "paletteram"),
		m_input(*this, "IN.%u", 0),
		m_dial(*this, "DIAL.%u", 0),
		m_eepromin(*this, "EEPROMIN")

	{ }

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<watchdog_timer_device> m_watchdog;
	optional_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	optional_shared_ptr<uint32_t> m_f3_ram;
	optional_shared_ptr<uint32_t> m_paletteram32;
	optional_ioport_array<6> m_input;
	optional_ioport_array<2> m_dial;
	optional_ioport m_eepromin;


	std::unique_ptr<uint16_t[]> m_videoram;
	std::unique_ptr<uint16_t[]> m_spriteram;
	std::unique_ptr<uint16_t[]> m_f3_vram;
	std::unique_ptr<uint16_t[]> m_f3_line_ram;
	std::unique_ptr<uint16_t[]> m_f3_pf_data;
	std::unique_ptr<uint16_t[]> m_f3_pivot_ram;

	uint32_t m_coin_word[2];
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
	std::unique_ptr<uint16_t[]> m_spriteram16_buffered;
	uint16_t m_f3_control_0[8];
	uint16_t m_f3_control_1[8];
	int m_flipscreen;
	uint8_t m_sprite_extra_planes;
	uint8_t m_sprite_pen_mask;
	uint16_t *m_f3_pf_data_1;
	uint16_t *m_f3_pf_data_2;
	uint16_t *m_f3_pf_data_3;
	uint16_t *m_f3_pf_data_4;
	uint16_t *m_f3_pf_data_5;
	uint16_t *m_f3_pf_data_6;
	uint16_t *m_f3_pf_data_7;
	uint16_t *m_f3_pf_data_8;
	int m_f3_skip_this_frame;
	int m_sprite_lag;
	uint8_t m_sprite_pri_usage;
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
	std::unique_ptr<uint8_t[]> m_tile_opaque_sp;
	std::unique_ptr<uint8_t[]> m_tile_opaque_pf[8];
	uint8_t m_add_sat[256][256];
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
	uint32_t m_dval;
	uint8_t m_pval;
	uint8_t m_tval;
	uint8_t m_pdest_2a;
	uint8_t m_pdest_2b;
	int m_tr_2a;
	int m_tr_2b;
	uint8_t m_pdest_3a;
	uint8_t m_pdest_3b;
	int m_tr_3a;
	int m_tr_3b;
	uint16_t *m_src0;
	uint16_t *m_src_s0;
	uint16_t *m_src_e0;
	uint16_t m_clip_al0;
	uint16_t m_clip_ar0;
	uint16_t m_clip_bl0;
	uint16_t m_clip_br0;
	uint8_t *m_tsrc0;
	uint8_t *m_tsrc_s0;
	uint32_t m_x_count0;
	uint32_t m_x_zoom0;
	uint16_t *m_src1;
	uint16_t *m_src_s1;
	uint16_t *m_src_e1;
	uint16_t m_clip_al1;
	uint16_t m_clip_ar1;
	uint16_t m_clip_bl1;
	uint16_t m_clip_br1;
	uint8_t *m_tsrc1;
	uint8_t *m_tsrc_s1;
	uint32_t m_x_count1;
	uint32_t m_x_zoom1;
	uint16_t *m_src2;
	uint16_t *m_src_s2;
	uint16_t *m_src_e2;
	uint16_t m_clip_al2;
	uint16_t m_clip_ar2;
	uint16_t m_clip_bl2;
	uint16_t m_clip_br2;
	uint8_t *m_tsrc2;
	uint8_t *m_tsrc_s2;
	uint32_t m_x_count2;
	uint32_t m_x_zoom2;
	uint16_t *m_src3;
	uint16_t *m_src_s3;
	uint16_t *m_src_e3;
	uint16_t m_clip_al3;
	uint16_t m_clip_ar3;
	uint16_t m_clip_bl3;
	uint16_t m_clip_br3;
	uint8_t *m_tsrc3;
	uint8_t *m_tsrc_s3;
	uint32_t m_x_count3;
	uint32_t m_x_zoom3;
	uint16_t *m_src4;
	uint16_t *m_src_s4;
	uint16_t *m_src_e4;
	uint16_t m_clip_al4;
	uint16_t m_clip_ar4;
	uint16_t m_clip_bl4;
	uint16_t m_clip_br4;
	uint8_t *m_tsrc4;
	uint8_t *m_tsrc_s4;
	uint32_t m_x_count4;
	uint32_t m_x_zoom4;
	struct tempsprite *m_spritelist;
	const struct tempsprite *m_sprite_end;
	struct f3_playfield_line_inf *m_pf_line_inf;
	struct f3_spritealpha_line_inf *m_sa_line_inf;
	const struct F3config *m_f3_game_config;
	int (taito_f3_state::*m_dpix_n[8][16])(uint32_t s_pix);
	int (taito_f3_state::**m_dpix_lp[5])(uint32_t s_pix);
	int (taito_f3_state::**m_dpix_sp[9])(uint32_t s_pix);

	uint32_t f3_control_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void f3_control_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void f3_sound_reset_0_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void f3_sound_reset_1_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void f3_sound_bankswitch_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void f3_unk_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint32_t bubsympb_oki_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void bubsympb_oki_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint16_t f3_pf_data_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void f3_pf_data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void f3_control_0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void f3_control_1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t f3_spriteram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void f3_spriteram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t f3_videoram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void f3_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t f3_vram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void f3_vram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t f3_pivot_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void f3_pivot_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t f3_lineram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void f3_lineram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void f3_palette_24bit_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	ioport_value f3_analog_r(ioport_field &field, void *param);
	ioport_value f3_coin_r(ioport_field &field, void *param);
	ioport_value eeprom_read(ioport_field &field, void *param);
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
	void get_tile_info1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info2(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info3(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info4(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info5(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info6(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info7(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info8(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_vram(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_pixel(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_f3();
	void machine_reset_f3();
	void video_start_f3();
	uint32_t screen_update_f3(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_eof_f3(screen_device &screen, bool state);
	void f3_interrupt2(device_t &device);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	inline void get_tile_info(tile_data &tileinfo, int tile_index, uint16_t *gfx_base);
	inline void f3_drawgfx(bitmap_rgb32 &dest_bmp,const rectangle &clip,gfx_element *gfx,int code,int color,int flipx,int flipy,int sx,int sy,uint8_t pri_dst);
	inline void f3_drawgfxzoom(bitmap_rgb32 &dest_bmp,const rectangle &clip,gfx_element *gfx,int code,int color,int flipx,int flipy,int sx,int sy,int scalex,int scaley,uint8_t pri_dst);
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void get_sprite_info(const uint16_t *spriteram16_ptr);
	void print_debug_info(bitmap_rgb32 &bitmap);
	inline void f3_alpha_set_level();
	inline void f3_alpha_blend32_s(int alphas, uint32_t s);
	inline void f3_alpha_blend32_d(int alphas, uint32_t s);
	inline void f3_alpha_blend_1_1(uint32_t s);
	inline void f3_alpha_blend_1_2(uint32_t s);
	inline void f3_alpha_blend_1_4(uint32_t s);
	inline void f3_alpha_blend_1_5(uint32_t s);
	inline void f3_alpha_blend_1_6(uint32_t s);
	inline void f3_alpha_blend_1_8(uint32_t s);
	inline void f3_alpha_blend_1_9(uint32_t s);
	inline void f3_alpha_blend_1_a(uint32_t s);
	inline void f3_alpha_blend_2a_0(uint32_t s);
	inline void f3_alpha_blend_2a_4(uint32_t s);
	inline void f3_alpha_blend_2a_8(uint32_t s);
	inline void f3_alpha_blend_2b_0(uint32_t s);
	inline void f3_alpha_blend_2b_4(uint32_t s);
	inline void f3_alpha_blend_2b_8(uint32_t s);
	inline void f3_alpha_blend_3a_0(uint32_t s);
	inline void f3_alpha_blend_3a_1(uint32_t s);
	inline void f3_alpha_blend_3a_2(uint32_t s);
	inline void f3_alpha_blend_3b_0(uint32_t s);
	inline void f3_alpha_blend_3b_1(uint32_t s);
	inline void f3_alpha_blend_3b_2(uint32_t s);
	int dpix_1_noalpha(uint32_t s_pix);
	int dpix_ret1(uint32_t s_pix);
	int dpix_ret0(uint32_t s_pix);
	int dpix_1_1(uint32_t s_pix);
	int dpix_1_2(uint32_t s_pix);
	int dpix_1_4(uint32_t s_pix);
	int dpix_1_5(uint32_t s_pix);
	int dpix_1_6(uint32_t s_pix);
	int dpix_1_8(uint32_t s_pix);
	int dpix_1_9(uint32_t s_pix);
	int dpix_1_a(uint32_t s_pix);
	int dpix_2a_0(uint32_t s_pix);
	int dpix_2a_4(uint32_t s_pix);
	int dpix_2a_8(uint32_t s_pix);
	int dpix_3a_0(uint32_t s_pix);
	int dpix_3a_1(uint32_t s_pix);
	int dpix_3a_2(uint32_t s_pix);
	int dpix_2b_0(uint32_t s_pix);
	int dpix_2b_4(uint32_t s_pix);
	int dpix_2b_8(uint32_t s_pix);
	int dpix_3b_0(uint32_t s_pix);
	int dpix_3b_1(uint32_t s_pix);
	int dpix_3b_2(uint32_t s_pix);
	int dpix_2_0(uint32_t s_pix);
	int dpix_2_4(uint32_t s_pix);
	int dpix_2_8(uint32_t s_pix);
	int dpix_3_0(uint32_t s_pix);
	int dpix_3_1(uint32_t s_pix);
	int dpix_3_2(uint32_t s_pix);
	inline void dpix_1_sprite(uint32_t s_pix);
	inline void dpix_bg(uint32_t bgcolor);
	void init_alpha_blend_func();
	inline void draw_scanlines(bitmap_rgb32 &bitmap, int xsize, int16_t *draw_line_num, const struct f3_playfield_line_inf **line_t, const int *sprite, uint32_t orient, int skip_layer_num);
	void visible_tile_check(struct f3_playfield_line_inf *line_t, int line, uint32_t x_index_fx,uint32_t y_index, uint16_t *f3_pf_data_n);
	void calculate_clip(int y, uint16_t pri, uint32_t* clip0, uint32_t* clip1, int* line_enable);
	void get_spritealphaclip_info();
	void get_line_ram_info(tilemap_t *tmap, int sx, int sy, int pos, uint16_t *f3_pf_data_n);
	void get_vram_info(tilemap_t *vram_tilemap, tilemap_t *pixel_tilemap, int sx, int sy);
	void scanline_draw(bitmap_rgb32 &bitmap, const rectangle &cliprect);
};
