/*----------- defined in video/terracre.c -----------*/

extern UINT16 *amazon_videoram;

PALETTE_INIT( amazon );
WRITE16_HANDLER( amazon_background_w );
WRITE16_HANDLER( amazon_foreground_w );
WRITE16_HANDLER( amazon_scrolly_w );
WRITE16_HANDLER( amazon_scrollx_w );
WRITE16_HANDLER( amazon_flipscreen_w );
VIDEO_START( amazon );
VIDEO_UPDATE( amazon );
