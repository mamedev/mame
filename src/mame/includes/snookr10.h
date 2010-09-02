/*----------- defined in video/snookr10.c -----------*/

extern UINT8 *snookr10_videoram;
extern UINT8 *snookr10_colorram;

WRITE8_HANDLER( snookr10_videoram_w );
WRITE8_HANDLER( snookr10_colorram_w );
PALETTE_INIT( snookr10 );
PALETTE_INIT( apple10 );
VIDEO_START( snookr10 );
VIDEO_START( apple10 );
VIDEO_UPDATE( snookr10 );

