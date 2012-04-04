class argus_state : public driver_device
{
public:
	argus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_paletteram;
	UINT8 *m_txram;
	UINT8 *m_bg0_scrollx;
	UINT8 *m_bg0_scrolly;
	UINT8 *m_bg1ram;
	UINT8 *m_bg1_scrollx;
	UINT8 *m_bg1_scrolly;
	UINT8 *m_butasan_bg1ram;
	UINT8 *m_dummy_bg0ram;
	UINT8 *m_butasan_txram;
	UINT8 *m_butasan_bg0ram;
	UINT8 *m_butasan_bg0backram;
	UINT8 *m_butasan_txbackram;
	UINT8 *m_butasan_pagedram[2];
	UINT8 m_butasan_page_latch;
	tilemap_t *m_tx_tilemap;
	tilemap_t *m_bg0_tilemap;
	tilemap_t *m_bg1_tilemap;
	UINT8 m_bg_status;
	UINT8 m_butasan_bg1_status;
	UINT8 m_flipscreen;
	UINT16 m_palette_intensity;
	int m_lowbitscroll;
	int m_prvscrollx;
	UINT8 m_valtric_mosaic;
	bitmap_rgb32 m_mosaicbitmap;
	UINT8 m_valtric_unknown;
	UINT8 m_butasan_unknown;
	int m_mosaic;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
	DECLARE_WRITE8_MEMBER(argus_bankselect_w);
};


/*----------- defined in video/argus.c -----------*/

VIDEO_START( argus );
VIDEO_START( valtric );
VIDEO_START( butasan );
VIDEO_RESET( argus );
VIDEO_RESET( valtric );
VIDEO_RESET( butasan );
SCREEN_UPDATE_RGB32( argus );
SCREEN_UPDATE_RGB32( valtric );
SCREEN_UPDATE_RGB32( butasan );

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
