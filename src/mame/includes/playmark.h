/*----------- defined in video/playmark.c -----------*/

extern UINT16 *bigtwin_bgvideoram;
extern UINT16 *wbeachvl_videoram1,*wbeachvl_videoram2,*wbeachvl_videoram3;
extern UINT16 *wbeachvl_rowscroll;

VIDEO_START( bigtwin );
VIDEO_START( wbeachvl );
VIDEO_START( excelsr );
VIDEO_START( hotmind );
VIDEO_START( hrdtimes );
WRITE16_HANDLER( wbeachvl_txvideoram_w );
WRITE16_HANDLER( wbeachvl_fgvideoram_w );
WRITE16_HANDLER( wbeachvl_bgvideoram_w );
WRITE16_HANDLER( hrdtimes_txvideoram_w );
WRITE16_HANDLER( hrdtimes_fgvideoram_w );
WRITE16_HANDLER( hrdtimes_bgvideoram_w );
WRITE16_HANDLER( bigtwin_paletteram_w );
WRITE16_HANDLER( bigtwin_scroll_w );
WRITE16_HANDLER( wbeachvl_scroll_w );
WRITE16_HANDLER( excelsr_scroll_w );
WRITE16_HANDLER( hrdtimes_scroll_w );
VIDEO_UPDATE( bigtwin );
VIDEO_UPDATE( wbeachvl );
VIDEO_UPDATE( excelsr );
VIDEO_UPDATE( hrdtimes );
