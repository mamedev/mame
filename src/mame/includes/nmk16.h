/*----------- defined in drivers/nmk16.c -----------*/

extern UINT16* nmk16_mainram;


/*----------- defined in video/nmk16.c -----------*/

extern UINT16 *nmk_bgvideoram0;
extern UINT16 *nmk_bgvideoram1;
extern UINT16 *nmk_bgvideoram2;
extern UINT16 *nmk_bgvideoram3;

extern UINT16 *nmk_fgvideoram,*nmk_txvideoram;
extern UINT16 *gunnail_scrollram, *gunnail_scrollramy;
extern UINT16 *afega_scroll_0;
extern UINT16 *afega_scroll_1;


WRITE16_HANDLER( nmk_bgvideoram0_w );
WRITE16_HANDLER( nmk_bgvideoram1_w );
WRITE16_HANDLER( nmk_bgvideoram2_w );
WRITE16_HANDLER( nmk_bgvideoram3_w );

WRITE16_HANDLER( nmk_fgvideoram_w );
WRITE16_HANDLER( nmk_txvideoram_w );
WRITE16_HANDLER( nmk_scroll_w );
WRITE16_HANDLER( nmk_scroll_2_w );
WRITE16_HANDLER( nmk_flipscreen_w );
WRITE16_HANDLER( nmk_tilebank_w );
WRITE16_HANDLER( bioship_scroll_w );
WRITE16_HANDLER( bioship_bank_w );
WRITE16_HANDLER( mustang_scroll_w );
WRITE16_HANDLER( bioshipbg_scroll_w );
WRITE16_HANDLER( vandyke_scroll_w );
WRITE16_HANDLER( vandykeb_scroll_w );
WRITE16_HANDLER( manybloc_scroll_w );

VIDEO_START( macross );
SCREEN_UPDATE( manybloc );
VIDEO_START( gunnail );
VIDEO_START( macross2 );
VIDEO_START( raphero );
VIDEO_START( bjtwin );
VIDEO_START( bioship );
VIDEO_START( strahl );
SCREEN_UPDATE( bioship );
SCREEN_UPDATE( strahl );
SCREEN_UPDATE( macross );
SCREEN_UPDATE( gunnail );
SCREEN_UPDATE( bjtwin );
SCREEN_UPDATE( tharrier );
SCREEN_UPDATE( hachamf );
SCREEN_UPDATE( tdragon );
SCREEN_EOF( nmk );
SCREEN_EOF( strahl );

VIDEO_START( afega );
VIDEO_START( grdnstrm );
VIDEO_START( firehawk );
SCREEN_UPDATE( afega );
SCREEN_UPDATE( redhawkb );
SCREEN_UPDATE(redhawki );
SCREEN_UPDATE( bubl2000 );
SCREEN_UPDATE( firehawk );
