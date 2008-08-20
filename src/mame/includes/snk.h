/*************************************************************************

    various SNK triple Z80 games

*************************************************************************/

/*----------- defined in drivers/snk.c -----------*/

extern UINT8 *snk_rambase;

extern INTERRUPT_GEN( snk_irq_AB );
extern INTERRUPT_GEN( snk_irq_BA );

extern READ8_HANDLER ( snk_cpuA_nmi_trigger_r );
extern WRITE8_HANDLER( snk_cpuA_nmi_ack_w );

extern READ8_HANDLER ( snk_cpuB_nmi_trigger_r );
extern WRITE8_HANDLER( snk_cpuB_nmi_ack_w );

extern int snk_gamegroup;
extern int snk_sound_busy_bit;
extern int snk_irq_delay;


/*----------- defined in video/snk.c -----------*/

extern PALETTE_INIT( tnk3 );

extern VIDEO_START( tnk3 );
extern VIDEO_START( ikari );
extern VIDEO_START( snk );
extern VIDEO_START( snk_4bpp_shadow );
extern VIDEO_START( sgladiat );

extern VIDEO_UPDATE( tnk3 );
extern VIDEO_UPDATE( ikari );
extern VIDEO_UPDATE( tdfever );
extern VIDEO_UPDATE( gwar );
extern VIDEO_UPDATE( sgladiat );

extern WRITE8_HANDLER( tnk3_videoattrs_w );
extern WRITE8_HANDLER( tnk3_sp_scrolly_w );
extern WRITE8_HANDLER( tnk3_sp_scrollx_w );
extern WRITE8_HANDLER( tnk3_bg_scrolly_w );
extern WRITE8_HANDLER( tnk3_bg_scrollx_w );

extern UINT8 *tnk3_fg_videoram;
extern UINT8 *tnk3_bg_videoram;
extern WRITE8_HANDLER( tnk3_fg_videoram_w );
extern WRITE8_HANDLER( tnk3_bg_videoram_w );

/*----------- defined in drivers/hal21.c -----------*/

extern PALETTE_INIT( aso );
