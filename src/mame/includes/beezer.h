/*----------- defined in machine/beezer.c -----------*/

DRIVER_INIT( beezer );
WRITE8_HANDLER( beezer_bankswitch_w );


/*----------- defined in video/beezer.c -----------*/

INTERRUPT_GEN( beezer_interrupt );
VIDEO_UPDATE( beezer );
WRITE8_HANDLER( beezer_map_w );
READ8_HANDLER( beezer_line_r );
