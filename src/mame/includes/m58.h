/*************************************************************************

    Irem M58 hardware

*************************************************************************/

/*----------- defined in video/m58.c -----------*/

extern UINT8 *yard_scroll_x_low;
extern UINT8 *yard_scroll_x_high;
extern UINT8 *yard_scroll_y_low;
extern UINT8 *yard_score_panel_disabled;

WRITE8_HANDLER( yard_videoram_w );
WRITE8_HANDLER( yard_scroll_panel_w );
WRITE8_HANDLER( yard_flipscreen_w );

PALETTE_INIT( yard );
VIDEO_START( yard );
VIDEO_UPDATE( yard );
