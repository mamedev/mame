class seicross_state : public driver_device
{
public:
	seicross_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_row_scroll(*this, "row_scroll"),
		m_spriteram2(*this, "spriteram2"),
		m_colorram(*this, "colorram"),
		m_nvram(*this, "nvram"){ }

	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_row_scroll;
	required_shared_ptr<UINT8> m_spriteram2;
	required_shared_ptr<UINT8> m_colorram;
	optional_shared_ptr<UINT8> m_nvram;

	UINT8 m_portb;
	tilemap_t *m_bg_tilemap;

	UINT8 m_irq_mask;
	DECLARE_WRITE8_MEMBER(seicross_videoram_w);
	DECLARE_WRITE8_MEMBER(seicross_colorram_w);
	DECLARE_READ8_MEMBER(friskyt_portB_r);
	DECLARE_WRITE8_MEMBER(friskyt_portB_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_seicross(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defined in video/seicross.c -----------*/





