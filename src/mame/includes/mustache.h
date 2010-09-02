/*----------- defined in video/mustache.c -----------*/

WRITE8_HANDLER( mustache_videoram_w );
WRITE8_HANDLER( mustache_scroll_w );
WRITE8_HANDLER( mustache_video_control_w );
VIDEO_START( mustache );
VIDEO_UPDATE( mustache );
PALETTE_INIT( mustache );
