/*----------- defined in drivers/pgm.c -----------*/

extern UINT16 *pgm_mainram, *pgm_bg_videoram, *pgm_tx_videoram, *pgm_videoregs, *pgm_rowscrollram;

extern UINT8 *pgm_sprite_a_region;
extern size_t	pgm_sprite_a_region_allocate;


/*----------- defined in machine/pgmcrypt.c -----------*/


void pgm_kov_decrypt(void);
void pgm_kovsh_decrypt(void);
void pgm_kov2_decrypt(void);
void pgm_mm_decrypt(void);
void pgm_dw2_decrypt(void);
void pgm_djlzz_decrypt(void);
void pgm_dw3_decrypt(void);
void pgm_killbld_decrypt(void);
void pgm_pstar_decrypt(void);
void pgm_puzzli2_decrypt(void);


/*----------- defined in machine/pgmprot.c -----------*/

READ16_HANDLER (PSTARS_protram_r);
READ16_HANDLER (PSTARS_r16);

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
