/*----------- defined in machine/gaplus.c -----------*/

extern UINT8 *gaplus_customio_3;
WRITE8_HANDLER( gaplus_customio_3_w );
READ8_HANDLER( gaplus_customio_3_r );


/*----------- defined in video/gaplus.c -----------*/

extern UINT8 *gaplus_videoram;
extern UINT8 *gaplus_spriteram;
READ8_HANDLER( gaplus_videoram_r );
WRITE8_HANDLER( gaplus_videoram_w );
WRITE8_HANDLER( gaplus_starfield_control_w );
VIDEO_START( gaplus );
PALETTE_INIT( gaplus );
VIDEO_UPDATE( gaplus );
VIDEO_EOF( gaplus );	/* update starfields */
