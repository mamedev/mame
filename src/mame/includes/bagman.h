/*----------- defined in machine/bagman.c -----------*/

READ8_HANDLER( bagman_pal16r6_r );
MACHINE_RESET( bagman );
WRITE8_HANDLER( bagman_pal16r6_w );


/*----------- defined in video/bagman.c -----------*/

extern UINT8 *bagman_videoram;
extern UINT8 *bagman_colorram;
extern UINT8 *bagman_video_enable;

WRITE8_HANDLER( bagman_videoram_w );
WRITE8_HANDLER( bagman_colorram_w );
WRITE8_HANDLER( bagman_flipscreen_w );

PALETTE_INIT( bagman );
VIDEO_START( bagman );
VIDEO_UPDATE( bagman );
