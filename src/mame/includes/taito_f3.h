// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#include "audio/taito_en.h"
#include "machine/watchdog.h"
#include "sound/okim6295.h"
#include "screen.h"

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
		m_taito_en(*this, "taito_en"),
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
	optional_device<taito_en_device> m_taito_en;
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

	emu_timer *m_interrupt3_timer;
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
	virtual void machine_start() override;
	DECLARE_MACHINE_RESET(f3);
	DECLARE_VIDEO_START(f3);
	uint32_t screen_update_f3(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_f3);
	INTERRUPT_GEN_MEMBER(f3_interrupt2);

	void f3_eeprom(machine_config &config);
	void f3(machine_config &config);
	void f3_224a(machine_config &config);
	void bubsympb(machine_config &config);
	void f3_224b(machine_config &config);
	void f3_224c(machine_config &config);
	void f3_224b_eeprom(machine_config &config);
	void bubsympb_map(address_map &map);
	void f3_map(address_map &map);
protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void device_post_load(void) override;

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
