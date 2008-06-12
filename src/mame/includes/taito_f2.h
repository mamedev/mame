/*----------- defined in video/taito_f2.c -----------*/

extern UINT16 *f2_sprite_extension;
extern size_t f2_spriteext_size;

VIDEO_START( taitof2_default );
VIDEO_START( taitof2_quiz );
VIDEO_START( taitof2_finalb );
VIDEO_START( taitof2_megab );
VIDEO_START( taitof2_solfigtr );
VIDEO_START( taitof2_koshien );
VIDEO_START( taitof2_driftout );
VIDEO_START( taitof2_dondokod );
VIDEO_START( taitof2_thundfox );
VIDEO_START( taitof2_growl );
VIDEO_START( taitof2_yuyugogo );
VIDEO_START( taitof2_mjnquest );
VIDEO_START( taitof2_footchmp );
VIDEO_START( taitof2_hthero );
VIDEO_START( taitof2_ssi );
VIDEO_START( taitof2_gunfront );
VIDEO_START( taitof2_ninjak );
VIDEO_START( taitof2_pulirula );
VIDEO_START( taitof2_metalb );
VIDEO_START( taitof2_qzchikyu );
VIDEO_START( taitof2_yesnoj );
VIDEO_START( taitof2_deadconx );
VIDEO_START( taitof2_deadconj );
VIDEO_START( taitof2_dinorex );
VIDEO_EOF( taitof2_no_buffer );
VIDEO_EOF( taitof2_full_buffer_delayed );
VIDEO_EOF( taitof2_partial_buffer_delayed );
VIDEO_EOF( taitof2_partial_buffer_delayed_thundfox );
VIDEO_EOF( taitof2_partial_buffer_delayed_qzchikyu );

VIDEO_UPDATE( taitof2 );
VIDEO_UPDATE( taitof2_pri );
VIDEO_UPDATE( taitof2_pri_roz );
VIDEO_UPDATE( ssi );
VIDEO_UPDATE( thundfox );
VIDEO_UPDATE( deadconx );
VIDEO_UPDATE( metalb );
VIDEO_UPDATE( yesnoj );

WRITE16_HANDLER( taitof2_spritebank_w );
READ16_HANDLER ( koshien_spritebank_r );
WRITE16_HANDLER( koshien_spritebank_w );
WRITE16_HANDLER( taitof2_sprite_extension_w );

/*----------- defined in machine/cchip.c -----------*/

extern UINT16 *cchip2_ram;
READ16_HANDLER ( cchip2_word_r );
WRITE16_HANDLER( cchip2_word_w );

MACHINE_RESET( cchip1 );
READ16_HANDLER( cchip1_ctrl_r );
READ16_HANDLER( cchip1_ram_r );
WRITE16_HANDLER( cchip1_ctrl_w );
WRITE16_HANDLER( cchip1_bank_w );
WRITE16_HANDLER( cchip1_ram_w );
