/*************************************************************************

    Atari Cloud 9 (prototype) hardware

*************************************************************************/

/*----------- defined in drivers/cloud9.c -----------*/

extern int cloud9_vblank_start;
extern int cloud9_vblank_end;


/*----------- defined in video/cloud9.c -----------*/

VIDEO_START( cloud9 );
VIDEO_UPDATE( cloud9 );

WRITE8_HANDLER( cloud9_video_control_w );

WRITE8_HANDLER( cloud9_paletteram_w );
WRITE8_HANDLER( cloud9_videoram_w );

READ8_HANDLER( cloud9_bitmode_r );
WRITE8_HANDLER( cloud9_bitmode_w );
WRITE8_HANDLER( cloud9_bitmode_addr_w );
