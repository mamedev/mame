/*----------- defined in video/portrait.c -----------*/

extern UINT8 *portrait_bgvideoram,*portrait_fgvideoram;
extern int portrait_scroll;

PALETTE_INIT( portrait );
VIDEO_START( portrait );
VIDEO_UPDATE( portrait );
WRITE8_HANDLER( portrait_bgvideo_write );
WRITE8_HANDLER( portrait_fgvideo_write );
