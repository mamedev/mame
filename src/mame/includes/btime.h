/*----------- defined in machine/btime.c -----------*/

READ8_HANDLER( mmonkey_protection_r );
WRITE8_HANDLER( mmonkey_protection_w );


/*----------- defined in video/btime.c -----------*/

extern UINT8 *btime_videoram;
extern size_t btime_videoram_size;
extern UINT8 *btime_colorram;
extern UINT8 *lnc_charbank;
extern UINT8 *bnj_backgroundram;
extern size_t bnj_backgroundram_size;
extern UINT8 *zoar_scrollram;
extern UINT8 *deco_charram;

PALETTE_INIT( btime );
PALETTE_INIT( lnc );

MACHINE_RESET( lnc );

VIDEO_START( btime );
VIDEO_START( bnj );

VIDEO_UPDATE( btime );
VIDEO_UPDATE( cookrace );
VIDEO_UPDATE( bnj );
VIDEO_UPDATE( lnc );
VIDEO_UPDATE( zoar );
VIDEO_UPDATE( disco );
VIDEO_UPDATE( eggs );

WRITE8_HANDLER( btime_paletteram_w );
WRITE8_HANDLER( bnj_background_w );
WRITE8_HANDLER( bnj_scroll1_w );
WRITE8_HANDLER( bnj_scroll2_w );
READ8_HANDLER( btime_mirrorvideoram_r );
WRITE8_HANDLER( btime_mirrorvideoram_w );
READ8_HANDLER( btime_mirrorcolorram_r );
WRITE8_HANDLER( btime_mirrorcolorram_w );
WRITE8_HANDLER( lnc_videoram_w );
WRITE8_HANDLER( lnc_mirrorvideoram_w );
WRITE8_HANDLER( deco_charram_w );

WRITE8_HANDLER( zoar_video_control_w );
WRITE8_HANDLER( btime_video_control_w );
WRITE8_HANDLER( bnj_video_control_w );
WRITE8_HANDLER( disco_video_control_w );
