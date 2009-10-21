/*************************************************************************

    Incredible Technologies/Strata system
    (8-bit blitter variant)

**************************************************************************/

/*----------- defined in drivers/itech8.c -----------*/

void itech8_update_interrupts(running_machine *machine, int periodic, int tms34061, int blitter);


/*----------- defined in machine/slikshot.c -----------*/

READ8_HANDLER( slikz80_port_r );
WRITE8_HANDLER( slikz80_port_w );

READ8_HANDLER( slikshot_z80_r );
READ8_HANDLER( slikshot_z80_control_r );
WRITE8_HANDLER( slikshot_z80_control_w );

VIDEO_START( slikshot );
VIDEO_UPDATE( slikshot );


/*----------- defined in video/itech8.c -----------*/

extern UINT8 *itech8_grom_bank;

VIDEO_START( itech8 );

WRITE8_HANDLER( itech8_page_w );

WRITE8_HANDLER( itech8_palette_w );

READ8_HANDLER( itech8_blitter_r );
WRITE8_HANDLER( itech8_blitter_w );

WRITE8_HANDLER( itech8_tms34061_w );
READ8_HANDLER( itech8_tms34061_r );

WRITE8_HANDLER( grmatch_palette_w );
WRITE8_HANDLER( grmatch_xscroll_w );
TIMER_DEVICE_CALLBACK( grmatch_palette_update );

VIDEO_UPDATE( itech8_2layer );
VIDEO_UPDATE( itech8_grmatch );
VIDEO_UPDATE( itech8_2page );
VIDEO_UPDATE( itech8_2page_large );
