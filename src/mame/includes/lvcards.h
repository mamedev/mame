/*----------- defined in video/lvcards.c -----------*/

extern UINT8 *lvcards_videoram;
extern UINT8 *lvcards_colorram;

WRITE8_HANDLER( lvcards_videoram_w );
WRITE8_HANDLER( lvcards_colorram_w );

PALETTE_INIT( lvcards );
PALETTE_INIT( ponttehk );
VIDEO_START( lvcards );
VIDEO_UPDATE( lvcards );
