/*----------- defined in video/wc90b.c -----------*/

extern UINT8 *wc90b_fgvideoram,*wc90b_bgvideoram,*wc90b_txvideoram;

extern UINT8 *wc90b_scroll1x;
extern UINT8 *wc90b_scroll2x;

extern UINT8 *wc90b_scroll1y;
extern UINT8 *wc90b_scroll2y;

extern UINT8 *wc90b_scroll_x_lo;

VIDEO_START( wc90b );
VIDEO_UPDATE( wc90b );

WRITE8_HANDLER( wc90b_bgvideoram_w );
WRITE8_HANDLER( wc90b_fgvideoram_w );
WRITE8_HANDLER( wc90b_txvideoram_w );
