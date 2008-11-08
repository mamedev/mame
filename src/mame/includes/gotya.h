/*----------- defined in audio/gotya.c -----------*/

WRITE8_HANDLER( gotya_soundlatch_w );


/*----------- defined in video/gotya.c -----------*/

extern UINT8 *gotya_scroll;
extern UINT8 *gotya_videoram2;

WRITE8_HANDLER( gotya_videoram_w );
WRITE8_HANDLER( gotya_colorram_w );
WRITE8_HANDLER( gotya_video_control_w );

PALETTE_INIT( gotya );
VIDEO_START( gotya );
VIDEO_UPDATE( gotya );
