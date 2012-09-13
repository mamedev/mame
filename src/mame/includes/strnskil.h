class strnskil_state : public driver_device
{
public:
	strnskil_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_subcpu(*this,"sub"),
		m_videoram(*this, "videoram"),
		m_xscroll(*this, "xscroll"),
		m_spriteram(*this, "spriteram"){ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_xscroll;
	required_shared_ptr<UINT8> m_spriteram;

	UINT8 m_scrl_ctrl;
	tilemap_t *m_bg_tilemap;
	UINT8 m_irq_source;

	DECLARE_READ8_MEMBER(strnskil_d800_r);
	DECLARE_READ8_MEMBER(pettanp_protection_r);
	DECLARE_READ8_MEMBER(banbam_protection_r);
	DECLARE_WRITE8_MEMBER(protection_w);
	DECLARE_WRITE8_MEMBER(strnskil_videoram_w);
	DECLARE_WRITE8_MEMBER(strnskil_scrl_ctrl_w);
	DECLARE_DRIVER_INIT(banbam);
	DECLARE_DRIVER_INIT(pettanp);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start();
	virtual void palette_init();
};


/*----------- defined in video/strnskil.c -----------*/




SCREEN_UPDATE_IND16( strnskil );
