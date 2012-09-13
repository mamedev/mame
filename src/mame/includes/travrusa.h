class travrusa_state : public driver_device
{
public:
	travrusa_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t*             m_bg_tilemap;
	int                  m_scrollx[2];
	DECLARE_WRITE8_MEMBER(travrusa_videoram_w);
	DECLARE_WRITE8_MEMBER(travrusa_scroll_x_low_w);
	DECLARE_WRITE8_MEMBER(travrusa_scroll_x_high_w);
	DECLARE_WRITE8_MEMBER(travrusa_flipscreen_w);
	DECLARE_DRIVER_INIT(shtridra);
	DECLARE_DRIVER_INIT(motorace);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	DECLARE_PALETTE_INIT(shtrider);
};

/*----------- defined in video/travrusa.c -----------*/





SCREEN_UPDATE_IND16( travrusa );
