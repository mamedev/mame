class snookr10_state : public driver_device
{
public:
	snookr10_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"){ }

	int m_outportl;
	int m_outporth;
	int m_bit0;
	int m_bit1;
	int m_bit2;
	int m_bit3;
	int m_bit4;
	int m_bit5;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	tilemap_t *m_bg_tilemap;
	DECLARE_READ8_MEMBER(dsw_port_1_r);
	DECLARE_WRITE8_MEMBER(output_port_0_w);
	DECLARE_WRITE8_MEMBER(output_port_1_w);
	DECLARE_WRITE8_MEMBER(snookr10_videoram_w);
	DECLARE_WRITE8_MEMBER(snookr10_colorram_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(apple10_get_bg_tile_info);
	virtual void video_start();
	virtual void palette_init();
	DECLARE_VIDEO_START(apple10);
	DECLARE_PALETTE_INIT(apple10);
	UINT32 screen_update_snookr10(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defined in video/snookr10.c -----------*/







