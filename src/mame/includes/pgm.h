
typedef struct _pgm_state pgm_state;
struct _pgm_state
{
	/* memory pointers */
//  UINT16 *      mainram;  // currently this is also used by nvram handler
	UINT16 *      bg_videoram;
	UINT16 *      tx_videoram;
	UINT16 *      videoregs;
	UINT16 *      rowscrollram;
	UINT16 *      videoram;
	UINT8  *      z80_mainram;
	UINT32 *      arm7_shareram;
	UINT32 *      svg_shareram[2];	//for 5585G MACHINE
	UINT16 *      sharedprotram;		// killbld & olds
	UINT8  *      sprite_a_region;
	size_t        sprite_a_region_size;
	UINT16 *      spritebufferram; // buffered spriteram
//  UINT16 *      paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t       *bg_tilemap, *tx_tilemap;
	UINT16        *sprite_temp_render;
	bitmap_t      *tmppgmbitmap;

	/* misc */
	// kov2
	UINT32        kov2_latchdata_68k_w;
	UINT32        kov2_latchdata_arm_w;
	// kovsh
	UINT16        kovsh_highlatch_arm_w, kovsh_lowlatch_arm_w;
	UINT16        kovsh_highlatch_68k_w, kovsh_lowlatch_68k_w;
	UINT32        kovsh_counter;
	// svg
	int           svg_ram_sel;
	// killbld & olds
	int           kb_cmd;
	int           kb_reg;
	int           kb_ptr;
	UINT32        kb_regs[0x10];
	UINT16        olds_bs, olds_cmd3;
	// pstars
	UINT16        pstars_key;
	UINT16        pstars_int[2];
	UINT32        pstars_regs[16];
	UINT32        pstars_val;
	UINT16        pstar_e7, pstar_b1, pstar_ce;
	UINT16        pstar_ram[3];
	// ASIC 3 (oriental legends protection)
	UINT8         asic3_reg, asic3_latch[3], asic3_x, asic3_y, asic3_z, asic3_h1, asic3_h2;
	UINT16        asic3_hold;
	// ASIC28
	UINT16        asic28_key;
	UINT16        asic28_regs[10];
	UINT16        asic_params[256];
	UINT16        asic28_rcnt;
	UINT32        eoregs[16];

	/* calendar */
	UINT8        cal_val, cal_mask, cal_com, cal_cnt;
	mame_system_time systime;

	/* devices */
	running_device *soundcpu;
	running_device *prot;
	running_device *ics;
};

extern UINT16 *pgm_mainram;	// used by nvram handler, we cannot move it to driver data struct

/*----------- defined in machine/pgmcrypt.c -----------*/

void pgm_kov_decrypt(running_machine *machine);
void pgm_kovsh_decrypt(running_machine *machine);
void pgm_kov2_decrypt(running_machine *machine);
void pgm_kov2p_decrypt(running_machine *machine);
void pgm_mm_decrypt(running_machine *machine);
void pgm_dw2_decrypt(running_machine *machine);
void pgm_djlzz_decrypt(running_machine *machine);
void pgm_dw3_decrypt(running_machine *machine);
void pgm_killbld_decrypt(running_machine *machine);
void pgm_pstar_decrypt(running_machine *machine);
void pgm_puzzli2_decrypt(running_machine *machine);
void pgm_theglad_decrypt(running_machine *machine);
void pgm_ddp2_decrypt(running_machine *machine);
void pgm_dfront_decrypt(running_machine *machine);
void pgm_oldsplus_decrypt(running_machine *machine);
void pgm_kovshp_decrypt(running_machine *machine);
void pgm_killbldp_decrypt(running_machine *machine);
void pgm_svg_decrypt(running_machine *machine);


/*----------- defined in machine/pgmprot.c -----------*/

READ16_HANDLER( pstars_protram_r );
READ16_HANDLER( pstars_r );
WRITE16_HANDLER( pstars_w );

READ16_HANDLER( pgm_asic3_r );
WRITE16_HANDLER( pgm_asic3_w );
WRITE16_HANDLER( pgm_asic3_reg_w );

READ16_HANDLER( sango_protram_r );
READ16_HANDLER( asic28_r );
WRITE16_HANDLER( asic28_w );

READ16_HANDLER( dw2_d80000_r );


/*----------- defined in video/pgm.c -----------*/

WRITE16_HANDLER( pgm_tx_videoram_w );
WRITE16_HANDLER( pgm_bg_videoram_w );

VIDEO_START( pgm );
VIDEO_EOF( pgm );
VIDEO_UPDATE( pgm );
