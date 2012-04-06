class m58_state : public driver_device
{
public:
	m58_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

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
	bitmap_ind16             *m_scroll_panel_bitmap;
	DECLARE_WRITE8_MEMBER(yard_videoram_w);
	DECLARE_WRITE8_MEMBER(yard_scroll_panel_w);
	DECLARE_WRITE8_MEMBER(yard_flipscreen_w);
};

/*----------- defined in video/m58.c -----------*/


PALETTE_INIT( yard );
VIDEO_START( yard );
SCREEN_UPDATE_IND16( yard );
