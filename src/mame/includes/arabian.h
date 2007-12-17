/***************************************************************************

    Sun Electronics Arabian hardware

    driver by Dan Boris

***************************************************************************/

/*----------- defined in video/arabian.c -----------*/

extern UINT8 arabian_video_control;
extern UINT8 arabian_flip_screen;

PALETTE_INIT( arabian );
VIDEO_START( arabian );
VIDEO_UPDATE( arabian );

WRITE8_HANDLER( arabian_blitter_w );
WRITE8_HANDLER( arabian_videoram_w );
