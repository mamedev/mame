/*----------- defined in video/srumbler.c -----------*/

extern UINT8 *srumbler_backgroundram,*srumbler_foregroundram;

WRITE8_HANDLER( srumbler_background_w );
WRITE8_HANDLER( srumbler_foreground_w );
WRITE8_HANDLER( srumbler_scroll_w );
WRITE8_HANDLER( srumbler_4009_w );

VIDEO_START( srumbler );
VIDEO_UPDATE( srumbler );
VIDEO_EOF( srumbler );
