class strnskil_state : public driver_device
{
public:
	strnskil_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_subcpu(*this,"sub")
		{ }

	UINT8 *m_videoram;
	UINT8 *m_xscroll;
	UINT8 m_scrl_ctrl;
	tilemap_t *m_bg_tilemap;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
	UINT8 m_irq_source;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	DECLARE_READ8_MEMBER(strnskil_d800_r);
	DECLARE_READ8_MEMBER(pettanp_protection_r);
	DECLARE_READ8_MEMBER(banbam_protection_r);
	DECLARE_WRITE8_MEMBER(protection_w);
	DECLARE_WRITE8_MEMBER(strnskil_videoram_w);
	DECLARE_WRITE8_MEMBER(strnskil_scrl_ctrl_w);
};


/*----------- defined in video/strnskil.c -----------*/


PALETTE_INIT( strnskil );
VIDEO_START( strnskil );
SCREEN_UPDATE_IND16( strnskil );
