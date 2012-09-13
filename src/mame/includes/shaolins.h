class shaolins_state : public driver_device
{
public:
	shaolins_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"){ }

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_videoram;
	int m_palettebank;

	tilemap_t *m_bg_tilemap;
	UINT8 m_nmi_enable;

	DECLARE_WRITE8_MEMBER(shaolins_videoram_w);
	DECLARE_WRITE8_MEMBER(shaolins_colorram_w);
	DECLARE_WRITE8_MEMBER(shaolins_palettebank_w);
	DECLARE_WRITE8_MEMBER(shaolins_scroll_w);
	DECLARE_WRITE8_MEMBER(shaolins_nmi_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start();
	virtual void palette_init();
};


/*----------- defined in video/sauro.c -----------*/




SCREEN_UPDATE_IND16( shaolins );
