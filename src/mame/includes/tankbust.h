/*----------- defined in video/tankbust.c -----------*/

extern UINT8 *tankbust_txtram;
extern UINT8 *tankbust_videoram;
extern UINT8 *tankbust_colorram;

VIDEO_START( tankbust );
VIDEO_UPDATE( tankbust );

WRITE8_HANDLER( tankbust_background_videoram_w );
READ8_HANDLER( tankbust_background_videoram_r );
WRITE8_HANDLER( tankbust_background_colorram_w );
READ8_HANDLER( tankbust_background_colorram_r );
WRITE8_HANDLER( tankbust_txtram_w );
READ8_HANDLER( tankbust_txtram_r );

WRITE8_HANDLER( tankbust_xscroll_w );
WRITE8_HANDLER( tankbust_yscroll_w );


