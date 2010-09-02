/*----------- defined in video/xain.c -----------*/

extern UINT8 *xain_charram, *xain_bgram0, *xain_bgram1, xain_pri;

VIDEO_UPDATE( xain );
VIDEO_START( xain );
WRITE8_HANDLER( xain_scrollxP0_w );
WRITE8_HANDLER( xain_scrollyP0_w );
WRITE8_HANDLER( xain_scrollxP1_w );
WRITE8_HANDLER( xain_scrollyP1_w );
WRITE8_HANDLER( xain_charram_w );
WRITE8_HANDLER( xain_bgram0_w );
WRITE8_HANDLER( xain_bgram1_w );
WRITE8_HANDLER( xain_flipscreen_w );
