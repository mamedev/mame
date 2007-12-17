/*************************************************************************

    Atari Ultra Tank hardware

*************************************************************************/


/*----------- defined in video/ultratnk.c -----------*/

extern int ultratnk_collision[4];

PALETTE_INIT( ultratnk );
VIDEO_START( ultratnk );
VIDEO_UPDATE( ultratnk );
VIDEO_EOF( ultratnk );

WRITE8_HANDLER( ultratnk_video_ram_w );
