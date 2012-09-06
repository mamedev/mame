class m58_state : public driver_device
{
public:
	m58_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_yard_scroll_x_low(*this, "scroll_x_low"),
		m_yard_scroll_x_high(*this, "scroll_x_high"),
		m_yard_scroll_y_low(*this, "scroll_y_low"),
		m_yard_score_panel_disabled(*this, "score_disable"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t*             m_bg_tilemap;

	required_shared_ptr<UINT8> m_yard_scroll_x_low;
	required_shared_ptr<UINT8> m_yard_scroll_x_high;
	required_shared_ptr<UINT8> m_yard_scroll_y_low;
	required_shared_ptr<UINT8> m_yard_score_panel_disabled;
	bitmap_ind16             *m_scroll_panel_bitmap;
	DECLARE_WRITE8_MEMBER(yard_videoram_w);
	DECLARE_WRITE8_MEMBER(yard_scroll_panel_w);
	DECLARE_WRITE8_MEMBER(yard_flipscreen_w);
	DECLARE_DRIVER_INIT(yard85);
	TILE_GET_INFO_MEMBER(yard_get_bg_tile_info);
	TILEMAP_MAPPER_MEMBER(yard_tilemap_scan_rows);
};

/*----------- defined in video/m58.c -----------*/


PALETTE_INIT( yard );
VIDEO_START( yard );
SCREEN_UPDATE_IND16( yard );
