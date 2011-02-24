class argus_state : public driver_device
{
public:
	argus_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *paletteram;
	UINT8 *txram;
	UINT8 *bg0_scrollx;
	UINT8 *bg0_scrolly;
	UINT8 *bg1ram;
	UINT8 *bg1_scrollx;
	UINT8 *bg1_scrolly;
	UINT8 *butasan_bg1ram;
	UINT8 *dummy_bg0ram;
	UINT8 *butasan_txram;
	UINT8 *butasan_bg0ram;
	UINT8 *butasan_bg0backram;
	UINT8 *butasan_txbackram;
	UINT8 *butasan_pagedram[2];
	UINT8 butasan_page_latch;
	tilemap_t *tx_tilemap;
	tilemap_t *bg0_tilemap;
	tilemap_t *bg1_tilemap;
	UINT8 bg_status;
	UINT8 butasan_bg1_status;
	UINT8 flipscreen;
	UINT16 palette_intensity;
	int lowbitscroll;
	int prvscrollx;
	UINT8 valtric_mosaic;
	bitmap_t *mosaicbitmap;
	UINT8 valtric_unknown;
	UINT8 butasan_unknown;
	int mosaic;
};


/*----------- defined in video/argus.c -----------*/

VIDEO_START( argus );
VIDEO_START( valtric );
VIDEO_START( butasan );
VIDEO_RESET( argus );
VIDEO_RESET( valtric );
VIDEO_RESET( butasan );
SCREEN_UPDATE( argus );
SCREEN_UPDATE( valtric );
SCREEN_UPDATE( butasan );

READ8_HANDLER( argus_txram_r );
READ8_HANDLER( argus_bg1ram_r );
READ8_HANDLER( argus_paletteram_r );

WRITE8_HANDLER( argus_txram_w );
WRITE8_HANDLER( argus_bg1ram_w );
WRITE8_HANDLER( argus_bg_status_w );
WRITE8_HANDLER( argus_flipscreen_w );
WRITE8_HANDLER( argus_paletteram_w );

WRITE8_HANDLER( valtric_bg_status_w );
WRITE8_HANDLER( valtric_paletteram_w );
WRITE8_HANDLER( valtric_mosaic_w );
WRITE8_HANDLER( valtric_unknown_w );

READ8_HANDLER( butasan_pagedram_r );
READ8_HANDLER( butasan_bg1ram_r );
WRITE8_HANDLER( butasan_pageselect_w );
WRITE8_HANDLER( butasan_pagedram_w );
WRITE8_HANDLER( butasan_bg1ram_w );
WRITE8_HANDLER( butasan_bg0_status_w );
WRITE8_HANDLER( butasan_paletteram_w );
WRITE8_HANDLER( butasan_bg1_status_w );
WRITE8_HANDLER( butasan_unknown_w );
