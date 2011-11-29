class hyperspt_state : public driver_device
{
public:
	hyperspt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *  m_videoram;
	UINT8 *  m_colorram;
	UINT8 *  m_scroll;
	UINT8 *  m_scroll2;
	UINT8 *  m_spriteram;
	UINT8 *  m_spriteram2;
	size_t   m_spriteram_size;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	int		 m_sprites_gfx_banked;

	UINT8    m_irq_mask;
};

/*----------- defined in video/hyperspt.c -----------*/

WRITE8_HANDLER( hyperspt_videoram_w );
WRITE8_HANDLER( hyperspt_colorram_w );
WRITE8_HANDLER( hyperspt_flipscreen_w );

PALETTE_INIT( hyperspt );
VIDEO_START( hyperspt );
SCREEN_UPDATE( hyperspt );
VIDEO_START( roadf );

