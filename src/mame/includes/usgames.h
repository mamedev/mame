/*----------- defined in video/usgames.c -----------*/

extern UINT8 *usgames_videoram,*usgames_charram;

WRITE8_HANDLER( usgames_videoram_w );
WRITE8_HANDLER( usgames_charram_w );
VIDEO_START( usgames );
PALETTE_INIT( usgames );
VIDEO_UPDATE( usgames );
