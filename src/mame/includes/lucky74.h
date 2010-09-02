/*----------- defined in video/lucky74.c -----------*/

extern UINT8 *lucky74_fg_videoram, *lucky74_fg_colorram, *lucky74_bg_videoram, *lucky74_bg_colorram;

WRITE8_HANDLER( lucky74_fg_videoram_w );
WRITE8_HANDLER( lucky74_fg_colorram_w );
WRITE8_HANDLER( lucky74_bg_videoram_w );
WRITE8_HANDLER( lucky74_bg_colorram_w );
PALETTE_INIT( lucky74 );
VIDEO_START( lucky74 );
VIDEO_UPDATE( lucky74 );
