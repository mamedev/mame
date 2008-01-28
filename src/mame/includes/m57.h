/*************************************************************************

	Irem M57 hardware

*************************************************************************/

/*----------- defined in video/m57.c -----------*/

extern UINT8 *m57_scroll;

WRITE8_HANDLER( m57_videoram_w );
WRITE8_HANDLER( m57_flipscreen_w );

PALETTE_INIT( m57 );
VIDEO_START( m57 );
VIDEO_UPDATE( m57 );

