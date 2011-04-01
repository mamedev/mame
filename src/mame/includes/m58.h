class m58_state : public driver_device
{
public:
	m58_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *              m_videoram;
	UINT8 *              m_spriteram;
	size_t               m_spriteram_size;

	/* video-related */
	tilemap_t*             m_bg_tilemap;

	UINT8                *m_yard_scroll_x_low;
	UINT8                *m_yard_scroll_x_high;
	UINT8                *m_yard_scroll_y_low;
	UINT8                *m_yard_score_panel_disabled;
	bitmap_t             *m_scroll_panel_bitmap;
};

/*----------- defined in video/m58.c -----------*/

WRITE8_HANDLER( yard_videoram_w );
WRITE8_HANDLER( yard_scroll_panel_w );
WRITE8_HANDLER( yard_flipscreen_w );

PALETTE_INIT( yard );
VIDEO_START( yard );
SCREEN_UPDATE( yard );
