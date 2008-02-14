/*************************************************************************

    Atari Crystal Castles hardware

*************************************************************************/

/*----------- defined in drivers/ccastles.c -----------*/

extern int ccastles_vblank_start;
extern int ccastles_vblank_end;

/*----------- defined in video/ccastles.c -----------*/


VIDEO_START( ccastles );
VIDEO_UPDATE( ccastles );

WRITE8_HANDLER( ccastles_hscroll_w );
WRITE8_HANDLER( ccastles_vscroll_w );
WRITE8_HANDLER( ccastles_video_control_w );

WRITE8_HANDLER( ccastles_paletteram_w );
WRITE8_HANDLER( ccastles_videoram_w );

READ8_HANDLER( ccastles_bitmode_r );
WRITE8_HANDLER( ccastles_bitmode_w );
WRITE8_HANDLER( ccastles_bitmode_addr_w );
