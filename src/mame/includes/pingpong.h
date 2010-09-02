/*----------- defined in video/pingpong.c -----------*/

extern UINT8 *pingpong_videoram;
extern UINT8 *pingpong_colorram;

WRITE8_HANDLER( pingpong_videoram_w );
WRITE8_HANDLER( pingpong_colorram_w );

PALETTE_INIT( pingpong );
VIDEO_START( pingpong );
VIDEO_UPDATE( pingpong );
