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

extern PALETTE_INIT( snk_3bpp_shadow );
extern PALETTE_INIT( snk_4bpp_shadow );

extern VIDEO_START( snk );
extern VIDEO_START( sgladiat );

extern VIDEO_UPDATE( tnk3 );
extern VIDEO_UPDATE( ikari );
extern VIDEO_UPDATE( tdfever );
extern VIDEO_UPDATE( gwar );
extern VIDEO_UPDATE( sgladiat );

void tnk3_draw_text( running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int bank, UINT8 *source );
void tnk3_draw_status( running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int bank, UINT8 *source );

// note: compare tdfever which does blinking in software with tdfeverj which does it in hardware
extern int snk_blink_parity;


/*----------- defined in drivers/hal21.c -----------*/

extern PALETTE_INIT( aso );
