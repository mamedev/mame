/*----------- defined in video/dec8.c -----------*/

extern UINT8 *dec8_pf0_data,*dec8_pf1_data,*dec8_row;

PALETTE_INIT( ghostb );
VIDEO_UPDATE( cobracom );
VIDEO_UPDATE( ghostb );
VIDEO_UPDATE( srdarwin );
VIDEO_UPDATE( gondo );
VIDEO_UPDATE( garyoret );
VIDEO_UPDATE( lastmiss );
VIDEO_UPDATE( shackled );
VIDEO_UPDATE( oscar );
VIDEO_START( cobracom );
VIDEO_START( oscar );
VIDEO_START( ghostb );
VIDEO_START( lastmiss );
VIDEO_START( shackled );
VIDEO_START( srdarwin );
VIDEO_START( gondo );
VIDEO_START( garyoret );

WRITE8_HANDLER( dec8_bac06_0_w );
WRITE8_HANDLER( dec8_bac06_1_w );
WRITE8_HANDLER( dec8_pf0_data_w );
WRITE8_HANDLER( dec8_pf1_data_w );
READ8_HANDLER( dec8_pf0_data_r );
READ8_HANDLER( dec8_pf1_data_r );
WRITE8_HANDLER( srdarwin_videoram_w );
WRITE8_HANDLER( dec8_scroll2_w );
WRITE8_HANDLER( srdarwin_control_w );
WRITE8_HANDLER( gondo_scroll_w );
WRITE8_HANDLER( shackled_control_w );
WRITE8_HANDLER( lastmiss_control_w );
WRITE8_HANDLER( lastmiss_scrollx_w );
WRITE8_HANDLER( lastmiss_scrolly_w );
WRITE8_HANDLER( dec8_videoram_w );
