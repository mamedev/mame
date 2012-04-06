/* This it the best way to allow game specific kludges until the system is fully understood */
enum {
	/* Early F3 class games, these are not cartridge games and system features may be different */
	RINGRAGE=0,	/* D21 */
	ARABIANM,	/* D29 */
	RIDINGF,	/* D34 */
	GSEEKER,	/* D40 */
	TRSTAR,		/* D53 */
	GUNLOCK,	/* D66 */
	TWINQIX,
	UNDRFIRE,	/* D67 - Heavily modified F3 hardware (different memory map) */
	SCFINALS,
	LIGHTBR,	/* D69 */

	/* D77 - F3 motherboard proms, all following games are 'F3 package system' */
	/* D78 I CUP */
	KAISERKN,	/* D84 */
	DARIUSG,	/* D87 */
	BUBSYMPH,	/* D90 */
	SPCINVDX,	/* D93 */
	HTHERO95,	/* D94 */
	QTHEATER,	/* D95 */
	EACTION2,	/* E02 */
	SPCINV95,	/* E06 */
	QUIZHUHU,	/* E08 */
	PBOBBLE2,	/* E10 */
	GEKIRIDO,	/* E11 */
	KTIGER2,	/* E15 */
	BUBBLEM,	/* E21 */
	CLEOPATR,	/* E28 */
	PBOBBLE3,	/* E29 */
	ARKRETRN,	/* E36 */
	KIRAMEKI,	/* E44 */
	PUCHICAR,	/* E46 */
	PBOBBLE4,	/* E49 */
	POPNPOP,	/* E51 */
	LANDMAKR,	/* E61 */
	RECALH,		/* prototype */
	COMMANDW,	/* prototype */
	TMDRILL
};

class taito_f3_state : public driver_device
{
public:
	taito_f3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_videoram;
	UINT16 *m_spriteram;
//  size_t m_spriteram_size;
	UINT32 m_coin_word[2];
	UINT32 *m_f3_ram;
	int m_f3_game;
	UINT16 *m_f3_vram;
	UINT16 *m_f3_line_ram;
	UINT16 *m_f3_pf_data;
	UINT16 *m_f3_pivot_ram;
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
	int (*m_dpix_n[8][16])(taito_f3_state *state, UINT32 s_pix);
	int (**m_dpix_lp[5])(taito_f3_state *state, UINT32 s_pix);
	int (**m_dpix_sp[9])(taito_f3_state *state, UINT32 s_pix);
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
};


/*----------- defined in video/taito_f3.c -----------*/

VIDEO_START( f3 );
SCREEN_UPDATE_RGB32( f3 );
SCREEN_VBLANK( f3 );


