class trucocl_state : public driver_device
{
public:
	trucocl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"){ }

	int m_cur_dac_address;
	int m_cur_dac_address_index;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	tilemap_t *m_bg_tilemap;

	UINT8 m_irq_mask;
	DECLARE_WRITE8_MEMBER(irq_enable_w);
	DECLARE_WRITE8_MEMBER(trucocl_videoram_w);
	DECLARE_WRITE8_MEMBER(trucocl_colorram_w);
	DECLARE_WRITE8_MEMBER(audio_dac_w);
	DECLARE_DRIVER_INIT(trucocl);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_trucocl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defined in video/trucocl.c -----------*/




