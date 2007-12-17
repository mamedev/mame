/*************************************************************************

    Atari Centipede hardware

*************************************************************************/

/*----------- defined in video/centiped.c -----------*/

extern UINT8 centiped_flipscreen, *bullsdrt_tiles_bankram;

PALETTE_INIT( centiped );
PALETTE_INIT( milliped );
PALETTE_INIT( warlords );

VIDEO_START( centiped );
VIDEO_START( milliped );
VIDEO_START( warlords );
VIDEO_START( bullsdrt );

VIDEO_UPDATE( centiped );
VIDEO_UPDATE( milliped );
VIDEO_UPDATE( warlords );
VIDEO_UPDATE( bullsdrt );

WRITE8_HANDLER( centiped_paletteram_w );
WRITE8_HANDLER( milliped_paletteram_w );

WRITE8_HANDLER( centiped_videoram_w );
WRITE8_HANDLER( centiped_flip_screen_w );
WRITE8_HANDLER( bullsdrt_tilesbank_w );
WRITE8_HANDLER( bullsdrt_sprites_bank_w );

