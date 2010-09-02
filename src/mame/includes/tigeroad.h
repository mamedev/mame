/*----------- defined in video/tigeroad.c -----------*/

WRITE16_HANDLER( tigeroad_videoram_w );
WRITE16_HANDLER( tigeroad_videoctrl_w );
WRITE16_HANDLER( tigeroad_scroll_w );
VIDEO_START( tigeroad );
VIDEO_UPDATE( tigeroad );
VIDEO_EOF( tigeroad );
