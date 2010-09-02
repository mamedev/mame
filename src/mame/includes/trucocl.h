/*----------- defined in video/trucocl.c -----------*/

extern UINT8 *trucocl_videoram;
extern UINT8 *trucocl_colorram;

WRITE8_HANDLER( trucocl_videoram_w );
WRITE8_HANDLER( trucocl_colorram_w );
PALETTE_INIT( trucocl );
VIDEO_START( trucocl );
VIDEO_UPDATE( trucocl );
