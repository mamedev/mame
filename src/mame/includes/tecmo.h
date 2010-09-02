/*----------- defined in video/tecmo.c -----------*/

extern int tecmo_video_type;
extern UINT8 *tecmo_txvideoram,*tecmo_fgvideoram,*tecmo_bgvideoram;

WRITE8_HANDLER( tecmo_txvideoram_w );
WRITE8_HANDLER( tecmo_fgvideoram_w );
WRITE8_HANDLER( tecmo_bgvideoram_w );
WRITE8_HANDLER( tecmo_fgscroll_w );
WRITE8_HANDLER( tecmo_bgscroll_w );
WRITE8_HANDLER( tecmo_flipscreen_w );

VIDEO_START( tecmo );
VIDEO_UPDATE( tecmo );
