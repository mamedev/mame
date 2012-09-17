class ampoker2_state : public driver_device
{
public:
	ampoker2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"){ }

	required_shared_ptr<UINT8> m_videoram;
	tilemap_t *m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(ampoker2_port30_w);
	DECLARE_WRITE8_MEMBER(ampoker2_port31_w);
	DECLARE_WRITE8_MEMBER(ampoker2_port32_w);
	DECLARE_WRITE8_MEMBER(ampoker2_port33_w);
	DECLARE_WRITE8_MEMBER(ampoker2_port34_w);
	DECLARE_WRITE8_MEMBER(ampoker2_port35_w);
	DECLARE_WRITE8_MEMBER(ampoker2_port36_w);
	DECLARE_WRITE8_MEMBER(ampoker2_watchdog_reset_w);
	DECLARE_WRITE8_MEMBER(ampoker2_videoram_w);
	DECLARE_DRIVER_INIT(rabbitpk);
	DECLARE_DRIVER_INIT(piccolop);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(s2k_get_bg_tile_info);
	virtual void video_start();
	virtual void palette_init();
	DECLARE_VIDEO_START(sigma2k);
	UINT32 screen_update_ampoker2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defined in video/ampoker2.c -----------*/





