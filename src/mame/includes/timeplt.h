/***************************************************************************

	Time Pilot
	
***************************************************************************/

/*----------- defined in video/timeplt.c -----------*/

READ8_HANDLER( timeplt_scanline_r );
WRITE8_HANDLER( timeplt_videoram_w );
WRITE8_HANDLER( timeplt_colorram_w );
WRITE8_HANDLER( timeplt_flipscreen_w );
VIDEO_START( timeplt );
PALETTE_INIT( timeplt );
VIDEO_UPDATE( timeplt );


