class timelimt_state : public driver_device
{
public:
	timelimt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_spriteram(*this, "spriteram"){ }

	required_shared_ptr<UINT8> m_videoram;
	int m_nmi_enabled;
	required_shared_ptr<UINT8> m_bg_videoram;
	int m_scrollx;
	int m_scrolly;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	required_shared_ptr<UINT8> m_spriteram;
	DECLARE_WRITE8_MEMBER(nmi_enable_w);
	DECLARE_WRITE8_MEMBER(sound_reset_w);
	DECLARE_WRITE8_MEMBER(timelimt_videoram_w);
	DECLARE_WRITE8_MEMBER(timelimt_bg_videoram_w);
	DECLARE_WRITE8_MEMBER(timelimt_scroll_x_lsb_w);
	DECLARE_WRITE8_MEMBER(timelimt_scroll_x_msb_w);
	DECLARE_WRITE8_MEMBER(timelimt_scroll_y_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_timelimt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
