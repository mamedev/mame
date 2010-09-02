/*----------- defined in video/calomega.c -----------*/

extern UINT8 *calomega_videoram;
extern UINT8 *calomega_colorram;
WRITE8_HANDLER( calomega_videoram_w );
WRITE8_HANDLER( calomega_colorram_w );
PALETTE_INIT( calomega );
VIDEO_START( calomega );
VIDEO_UPDATE( calomega );
