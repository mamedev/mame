/*----------- defined in video/pcktgal.c -----------*/

WRITE8_HANDLER( pcktgal_videoram_w );
WRITE8_HANDLER( pcktgal_flipscreen_w );

PALETTE_INIT( pcktgal );
VIDEO_START( pcktgal );
VIDEO_UPDATE( pcktgal );
