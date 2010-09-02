/*----------- defined in video/sprint4.c -----------*/

extern int sprint4_collision[4];

PALETTE_INIT( sprint4 );

VIDEO_EOF( sprint4 );
VIDEO_START( sprint4 );
VIDEO_UPDATE( sprint4 );

WRITE8_HANDLER( sprint4_video_ram_w );
