/*************************************************************************

    Atari Escape hardware

*************************************************************************/

/*----------- defined in video/eprom.c -----------*/

VIDEO_START( eprom );
VIDEO_UPDATE( eprom );

VIDEO_START( guts );
VIDEO_UPDATE( guts );

void eprom_scanline_update(const device_config *screen, int scanline);
