/*----------- defined in drivers/pgm.c -----------*/

extern UINT16 *pgm_mainram, *pgm_bg_videoram, *pgm_tx_videoram, *pgm_videoregs, *pgm_rowscrollram;

extern UINT8 *pgm_sprite_a_region;
extern size_t	pgm_sprite_a_region_allocate;


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
void pgm_svg_decrypt(running_machine *machine);
void pgm_killbldp_decrypt(running_machine *machine);
void pgm_kovh_decrypt(running_machine *machine);
void pgm_oldss_decrypt(running_machine *machine);

/*----------- defined in machine/pgmprot.c -----------*/

READ16_HANDLER (PSTARS_protram_r);
READ16_HANDLER ( PSTARS_r16 );
WRITE16_HANDLER( PSTARS_w16 );

READ16_HANDLER( pgm_asic3_r );
WRITE16_HANDLER( pgm_asic3_w );
WRITE16_HANDLER( pgm_asic3_reg_w );

READ16_HANDLER (sango_protram_r);
READ16_HANDLER (ASIC28_r16);
WRITE16_HANDLER (ASIC28_w16);

READ16_HANDLER (dw2_d80000_r );


/*----------- defined in machine/pgmy2ks.c -----------*/

extern const UINT32 pgmy2ks[0x3c00];


/*----------- defined in video/pgm.c -----------*/


WRITE16_HANDLER( pgm_tx_videoram_w );
WRITE16_HANDLER( pgm_bg_videoram_w );
VIDEO_START( pgm );
VIDEO_EOF( pgm );
VIDEO_UPDATE( pgm );
