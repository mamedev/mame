/*************************************************************************

    Irem M52 hardware

*************************************************************************/

/*----------- defined in video/m52.c -----------*/

READ8_HANDLER( m52_protection_r );
WRITE8_HANDLER( m52_scroll_w );
WRITE8_HANDLER( m52_bg1xpos_w );
WRITE8_HANDLER( m52_bg1ypos_w );
WRITE8_HANDLER( m52_bg2xpos_w );
WRITE8_HANDLER( m52_bg2ypos_w );
WRITE8_HANDLER( m52_bgcontrol_w );
WRITE8_HANDLER( m52_flipscreen_w );
WRITE8_HANDLER( alpha1v_flipscreen_w );
WRITE8_HANDLER( m52_videoram_w );
WRITE8_HANDLER( m52_colorram_w );

PALETTE_INIT( m52 );
VIDEO_START( m52 );
VIDEO_UPDATE( m52 );


