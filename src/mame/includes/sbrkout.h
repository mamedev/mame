/*************************************************************************

    Atari Super Breakout hardware

*************************************************************************/

/*----------- defined in machine/sbrkout.c -----------*/

WRITE8_HANDLER( sbrkout_serve_led_w );
WRITE8_HANDLER( sbrkout_start_1_led_w );
WRITE8_HANDLER( sbrkout_start_2_led_w );
READ8_HANDLER( sbrkout_read_DIPs_r );
INTERRUPT_GEN( sbrkout_interrupt );
READ8_HANDLER( sbrkout_select1_r );
READ8_HANDLER( sbrkout_select2_r );


/*----------- defined in video/sbrkout.c -----------*/

extern UINT8 *sbrkout_horiz_ram;
extern UINT8 *sbrkout_vert_ram;

WRITE8_HANDLER( sbrkout_videoram_w );

VIDEO_START( sbrkout );
VIDEO_UPDATE( sbrkout );
