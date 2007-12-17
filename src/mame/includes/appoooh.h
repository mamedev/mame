/* ----------- defined in video/appoooh.c -----------*/

extern UINT8 *appoooh_fg_videoram,*appoooh_fg_colorram;
extern UINT8 *appoooh_bg_videoram,*appoooh_bg_colorram;
WRITE8_HANDLER( appoooh_fg_videoram_w );
WRITE8_HANDLER( appoooh_fg_colorram_w );
WRITE8_HANDLER( appoooh_bg_videoram_w );
WRITE8_HANDLER( appoooh_bg_colorram_w );
PALETTE_INIT( appoooh );
PALETTE_INIT( robowres );
WRITE8_HANDLER( appoooh_scroll_w );
WRITE8_HANDLER( appoooh_out_w );
VIDEO_START( appoooh );
VIDEO_UPDATE( appoooh );
VIDEO_UPDATE( robowres );

