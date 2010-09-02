/*----------- defined in video/funworld.c -----------*/

extern UINT8* funworld_videoram;
extern UINT8* funworld_colorram;

WRITE8_HANDLER( funworld_videoram_w );
WRITE8_HANDLER( funworld_colorram_w );
PALETTE_INIT( funworld );
VIDEO_START( funworld );
VIDEO_START( magicrd2 );
VIDEO_UPDATE( funworld );
