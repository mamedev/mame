class mappy_state : public driver_device
{
public:
	mappy_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 *m_spriteram;
	tilemap_t *m_bg_tilemap;
	bitmap_t *m_sprite_bitmap;

	UINT8 m_scroll;
	int m_mux;

	UINT8 m_main_irq_mask;
	UINT8 m_sub_irq_mask;
	UINT8 m_sub2_irq_mask;
};


/*----------- defined in video/mappy.c -----------*/

VIDEO_START( phozon );
PALETTE_INIT( phozon );
SCREEN_UPDATE( phozon );

PALETTE_INIT( superpac );
PALETTE_INIT( mappy );
VIDEO_START( superpac );
VIDEO_START( mappy );
SCREEN_UPDATE( superpac );
SCREEN_UPDATE( mappy );
WRITE8_HANDLER( superpac_videoram_w );
WRITE8_HANDLER( mappy_videoram_w );
WRITE8_HANDLER( mappy_scroll_w );
READ8_HANDLER( superpac_flipscreen_r );
WRITE8_HANDLER( superpac_flipscreen_w );
