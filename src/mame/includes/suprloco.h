/*----------- defined in video/suprloco.c -----------*/

extern UINT8 *suprloco_videoram;
extern UINT8 *suprloco_scrollram;

PALETTE_INIT( suprloco );
VIDEO_START( suprloco );
VIDEO_UPDATE( suprloco );
WRITE8_HANDLER( suprloco_videoram_w );
WRITE8_HANDLER( suprloco_scrollram_w );
WRITE8_HANDLER( suprloco_control_w );
READ8_HANDLER( suprloco_control_r );
