/*----------- defined in machine/docastle.c -----------*/

READ8_HANDLER( docastle_shared0_r );
READ8_HANDLER( docastle_shared1_r );
WRITE8_HANDLER( docastle_shared0_w );
WRITE8_HANDLER( docastle_shared1_w );
WRITE8_HANDLER( docastle_nmitrigger_w );


/*----------- defined in video/docastle.c -----------*/

WRITE8_HANDLER( docastle_videoram_w );
WRITE8_HANDLER( docastle_colorram_w );
READ8_HANDLER( docastle_flipscreen_off_r );
READ8_HANDLER( docastle_flipscreen_on_r );
WRITE8_HANDLER( docastle_flipscreen_off_w );
WRITE8_HANDLER( docastle_flipscreen_on_w );

PALETTE_INIT( docastle );
VIDEO_START( docastle );
VIDEO_START( dorunrun );
VIDEO_UPDATE( docastle );

