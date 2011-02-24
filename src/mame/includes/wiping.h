/*----------- defined in video/wiping.c -----------*/

extern UINT8 *wiping_videoram;
extern UINT8 *wiping_colorram;

WRITE8_HANDLER( wiping_flipscreen_w );
PALETTE_INIT( wiping );
SCREEN_UPDATE( wiping );

