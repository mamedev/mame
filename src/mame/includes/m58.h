class m58_state : public driver_device
{
public:
	m58_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_yard_scroll_x_low(*this, "scroll_x_low"),
		m_yard_scroll_x_high(*this, "scroll_x_high"),
		m_yard_scroll_y_low(*this, "scroll_y_low"),
		m_yard_score_panel_disabled(*this, "score_disable"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

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
	virtual void video_start();
	DECLARE_PALETTE_INIT(m58);
	UINT32 screen_update_yard(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_panel( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};
