class mjkjidai_state : public driver_device
{
public:
	mjkjidai_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_nvram(*this, "nvram"),
		m_spriteram1(*this, "spriteram1"),
		m_spriteram2(*this, "spriteram2"),
		m_spriteram3(*this, "spriteram3"),
		m_videoram(*this, "videoram"){ }

	required_shared_ptr<UINT8> m_nvram;
	required_shared_ptr<UINT8> m_spriteram1;
	required_shared_ptr<UINT8> m_spriteram2;
	required_shared_ptr<UINT8> m_spriteram3;
	required_shared_ptr<UINT8> m_videoram;

	int m_keyb;
	int m_nvram_init_count;
	int m_display_enable;
	tilemap_t *m_bg_tilemap;

	UINT8 m_nmi_mask;
	DECLARE_READ8_MEMBER(keyboard_r);
	DECLARE_WRITE8_MEMBER(keyboard_select_w);
	DECLARE_WRITE8_MEMBER(mjkjidai_videoram_w);
	DECLARE_WRITE8_MEMBER(mjkjidai_ctrl_w);
	DECLARE_WRITE8_MEMBER(adpcm_w);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void video_start();
	UINT32 screen_update_mjkjidai(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defined in video/mjkjidai.c -----------*/





