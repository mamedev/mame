/*----------- defined in video/timelimt.c -----------*/

extern UINT8 *timelimt_bg_videoram;
extern size_t timelimt_bg_videoram_size;

VIDEO_START( timelimt );
PALETTE_INIT( timelimt );
VIDEO_UPDATE( timelimt );

WRITE8_HANDLER( timelimt_videoram_w );
WRITE8_HANDLER( timelimt_bg_videoram_w );
WRITE8_HANDLER( timelimt_scroll_y_w );
WRITE8_HANDLER( timelimt_scroll_x_msb_w );
WRITE8_HANDLER( timelimt_scroll_x_lsb_w );
