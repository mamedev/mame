/***************************************************************************
                ToaPlan game hardware from 1988-1991
                ------------------------------------
****************************************************************************/


/*----------- defined in machine/toaplan1.c -----------*/

INTERRUPT_GEN( toaplan1_interrupt );
WRITE16_HANDLER( toaplan1_intenable_w );
READ16_HANDLER ( toaplan1_shared_r );
WRITE16_HANDLER( toaplan1_shared_w );
WRITE16_HANDLER( toaplan1_reset_sound );
WRITE16_HANDLER( demonwld_dsp_addrsel_w );
READ16_HANDLER ( demonwld_dsp_r );
WRITE16_HANDLER( demonwld_dsp_w );
WRITE16_HANDLER( demonwld_dsp_bio_w );
WRITE16_HANDLER( demonwld_dsp_ctrl_w );
READ16_HANDLER ( demonwld_BIO_r );
READ16_HANDLER ( samesame_port_6_word_r );
READ16_HANDLER ( vimana_mcu_r );
WRITE16_HANDLER( vimana_mcu_w );
READ16_HANDLER ( vimana_input_port_5_word_r );

WRITE8_HANDLER( rallybik_coin_w );
WRITE8_HANDLER( toaplan1_coin_w );
WRITE16_HANDLER( samesame_coin_w );

MACHINE_RESET( toaplan1 );
MACHINE_RESET( demonwld );
MACHINE_RESET( vimana );
MACHINE_RESET( zerozone );	/* hack for ZeroWing/OutZone. See video */

extern void toaplan1_driver_savestate(void);
extern void demonwld_driver_savestate(void);
extern void vimana_driver_savestate(void);


extern int toaplan1_unk_reset_port;

extern UINT8 *toaplan1_sharedram;



/*----------- defined in video/toaplan1.c -----------*/

READ16_HANDLER ( toaplan1_frame_done_r );
WRITE16_HANDLER( toaplan1_bcu_control_w );
WRITE16_HANDLER( rallybik_bcu_flipscreen_w );
WRITE16_HANDLER( toaplan1_bcu_flipscreen_w );
WRITE16_HANDLER( toaplan1_fcu_flipscreen_w );

READ16_HANDLER ( rallybik_tileram16_r );
READ16_HANDLER ( toaplan1_tileram16_r );
WRITE16_HANDLER( toaplan1_tileram16_w );
READ16_HANDLER ( toaplan1_spriteram16_r );
WRITE16_HANDLER( toaplan1_spriteram16_w );
READ16_HANDLER ( toaplan1_spritesizeram16_r );
WRITE16_HANDLER( toaplan1_spritesizeram16_w );
READ16_HANDLER ( toaplan1_colorram1_r );
WRITE16_HANDLER( toaplan1_colorram1_w );
READ16_HANDLER ( toaplan1_colorram2_r );
WRITE16_HANDLER( toaplan1_colorram2_w );

READ16_HANDLER ( toaplan1_scroll_regs_r );
WRITE16_HANDLER( toaplan1_scroll_regs_w );
WRITE16_HANDLER( toaplan1_tile_offsets_w );
READ16_HANDLER ( toaplan1_tileram_offs_r );
WRITE16_HANDLER( toaplan1_tileram_offs_w );
READ16_HANDLER ( toaplan1_spriteram_offs_r );
WRITE16_HANDLER( toaplan1_spriteram_offs_w );

VIDEO_EOF( rallybik );
VIDEO_EOF( toaplan1 );
VIDEO_EOF( samesame );
VIDEO_START( rallybik );
VIDEO_START( toaplan1 );
VIDEO_UPDATE( rallybik );
VIDEO_UPDATE( toaplan1 );
VIDEO_UPDATE( demonwld );


extern UINT16 *toaplan1_colorram1;
extern UINT16 *toaplan1_colorram2;
extern size_t toaplan1_colorram1_size;
extern size_t toaplan1_colorram2_size;
