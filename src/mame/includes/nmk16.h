class nmk16_state : public driver_device
{
public:
	nmk16_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int mask[4*2];
	UINT16* mainram;
	UINT16 *nmk_bgvideoram0;
	UINT16 *nmk_bgvideoram1;
	UINT16 *nmk_bgvideoram2;
	UINT16 *nmk_bgvideoram3;
	UINT16 *nmk_fgvideoram;
	UINT16 *nmk_txvideoram;
	UINT16 *gunnail_scrollram;
	UINT16 *gunnail_scrollramy;
	UINT16 *afega_scroll_0;
	UINT16 *afega_scroll_1;
	int simple_scroll;
	int redraw_bitmap;
	UINT16 *spriteram_old;
	UINT16 *spriteram_old2;
	int bgbank;
	int videoshift;
	int bioship_background_bank;
	UINT8 bioship_scroll[4];
	tilemap_t *bg_tilemap0;
	tilemap_t *bg_tilemap1;
	tilemap_t *bg_tilemap2;
	tilemap_t *bg_tilemap3;
	tilemap_t *tx_tilemap;
	tilemap_t *fg_tilemap;
	bitmap_t *background_bitmap;
	int mustang_bg_xscroll;
	UINT8 scroll[4];
	UINT8 scroll_2[4];
	UINT16 vscroll[4];
	UINT8 *spriteram;
	size_t spriteram_size;
	int prot_count;
	UINT8 input_pressed;
	UINT8 start_helper;
	UINT8 coin_count[2];
	UINT8 coin_count_frac[2];
};


/*----------- defined in video/nmk16.c -----------*/

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
