class travrusa_state : public driver_device
{
public:
	travrusa_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *              m_videoram;
	UINT8 *              m_spriteram;
	size_t               m_spriteram_size;

	/* video-related */
	tilemap_t*             m_bg_tilemap;
	int                  m_scrollx[2];
};

/*----------- defined in video/travrusa.c -----------*/

WRITE8_HANDLER( travrusa_videoram_w );
WRITE8_HANDLER( travrusa_scroll_x_low_w );
WRITE8_HANDLER( travrusa_scroll_x_high_w );
WRITE8_HANDLER( travrusa_flipscreen_w );

PALETTE_INIT( travrusa );
PALETTE_INIT( shtrider );
VIDEO_START( travrusa );
SCREEN_UPDATE( travrusa );
