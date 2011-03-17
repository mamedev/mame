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
	COMMANDW	/* prototype */
};

class taito_f3_state : public driver_device
{
public:
	taito_f3_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT32 *videoram;
	UINT32 *spriteram;
	size_t spriteram_size;
	UINT32 coin_word[2];
	UINT32 *f3_ram;
	int f3_game;
	UINT32 *f3_vram;
	UINT32 *f3_line_ram;
	UINT32 *f3_pf_data;
	UINT32 *f3_pivot_ram;
	tilemap_t *pf1_tilemap;
	tilemap_t *pf2_tilemap;
	tilemap_t *pf3_tilemap;
	tilemap_t *pf4_tilemap;
	tilemap_t *pixel_layer;
	tilemap_t *vram_layer;
	UINT32 *spriteram32_buffered;
	UINT32 f3_control_0[8];
	UINT32 f3_control_1[8];
	int flipscreen;
	UINT8 sprite_extra_planes;
	UINT8 sprite_pen_mask;
	UINT32 *f3_pf_data_1;
	UINT32 *f3_pf_data_2;
	UINT32 *f3_pf_data_3;
	UINT32 *f3_pf_data_4;
	int f3_skip_this_frame;
	int sprite_lag;
	UINT8 sprite_pri_usage;
	bitmap_t *pri_alp_bitmap;
	int f3_alpha_level_2as;
	int f3_alpha_level_2ad;
	int f3_alpha_level_3as;
	int f3_alpha_level_3ad;
	int f3_alpha_level_2bs;
	int f3_alpha_level_2bd;
	int f3_alpha_level_3bs;
	int f3_alpha_level_3bd;
	int alpha_level_last;
	int width_mask;
	int twidth_mask;
	int twidth_mask_bit;
	UINT8 *tile_opaque_sp;
	UINT8 *tile_opaque_pf[4];
	UINT8 add_sat[256][256];
	int alpha_s_1_1;
	int alpha_s_1_2;
	int alpha_s_1_4;
	int alpha_s_1_5;
	int alpha_s_1_6;
	int alpha_s_1_8;
	int alpha_s_1_9;
	int alpha_s_1_a;
	int alpha_s_2a_0;
	int alpha_s_2a_4;
	int alpha_s_2a_8;
	int alpha_s_2b_0;
	int alpha_s_2b_4;
	int alpha_s_2b_8;
	int alpha_s_3a_0;
	int alpha_s_3a_1;
	int alpha_s_3a_2;
	int alpha_s_3b_0;
	int alpha_s_3b_1;
	int alpha_s_3b_2;
	UINT32 dval;
	UINT8 pval;
	UINT8 tval;
	UINT8 pdest_2a;
	UINT8 pdest_2b;
	int tr_2a;
	int tr_2b;
	UINT8 pdest_3a;
	UINT8 pdest_3b;
	int tr_3a;
	int tr_3b;
	UINT16 *src0;
	UINT16 *src_s0;
	UINT16 *src_e0;
	UINT16 clip_al0;
	UINT16 clip_ar0;
	UINT16 clip_bl0;
	UINT16 clip_br0;
	UINT8 *tsrc0;
	UINT8 *tsrc_s0;
	UINT32 x_count0;
	UINT32 x_zoom0;
	UINT16 *src1;
	UINT16 *src_s1;
	UINT16 *src_e1;
	UINT16 clip_al1;
	UINT16 clip_ar1;
	UINT16 clip_bl1;
	UINT16 clip_br1;
	UINT8 *tsrc1;
	UINT8 *tsrc_s1;
	UINT32 x_count1;
	UINT32 x_zoom1;
	UINT16 *src2;
	UINT16 *src_s2;
	UINT16 *src_e2;
	UINT16 clip_al2;
	UINT16 clip_ar2;
	UINT16 clip_bl2;
	UINT16 clip_br2;
	UINT8 *tsrc2;
	UINT8 *tsrc_s2;
	UINT32 x_count2;
	UINT32 x_zoom2;
	UINT16 *src3;
	UINT16 *src_s3;
	UINT16 *src_e3;
	UINT16 clip_al3;
	UINT16 clip_ar3;
	UINT16 clip_bl3;
	UINT16 clip_br3;
	UINT8 *tsrc3;
	UINT8 *tsrc_s3;
	UINT32 x_count3;
	UINT32 x_zoom3;
	UINT16 *src4;
	UINT16 *src_s4;
	UINT16 *src_e4;
	UINT16 clip_al4;
	UINT16 clip_ar4;
	UINT16 clip_bl4;
	UINT16 clip_br4;
	UINT8 *tsrc4;
	UINT8 *tsrc_s4;
	UINT32 x_count4;
	UINT32 x_zoom4;
	struct tempsprite *spritelist;
	const struct tempsprite *sprite_end;
	struct f3_playfield_line_inf *pf_line_inf;
	struct f3_spritealpha_line_inf *sa_line_inf;
	const struct F3config *f3_game_config;
	int (*dpix_n[8][16])(taito_f3_state *state, UINT32 s_pix);
	int (**dpix_lp[5])(taito_f3_state *state, UINT32 s_pix);
	int (**dpix_sp[9])(taito_f3_state *state, UINT32 s_pix);
};


/*----------- defined in video/taito_f3.c -----------*/

VIDEO_START( f3 );
SCREEN_UPDATE( f3 );
SCREEN_EOF( f3 );

WRITE32_HANDLER( f3_control_0_w );
WRITE32_HANDLER( f3_control_1_w );
WRITE32_HANDLER( f3_palette_24bit_w );
WRITE32_HANDLER( f3_pf_data_w );
WRITE32_HANDLER( f3_vram_w );
WRITE32_HANDLER( f3_pivot_w );
WRITE32_HANDLER( f3_lineram_w );
WRITE32_HANDLER( f3_videoram_w );


