/*----------- defined in video/speedbal.c -----------*/

extern UINT8 *speedbal_background_videoram;
extern UINT8 *speedbal_foreground_videoram;

VIDEO_START( speedbal );
VIDEO_UPDATE( speedbal );
WRITE8_HANDLER( speedbal_foreground_videoram_w );
WRITE8_HANDLER( speedbal_background_videoram_w );
