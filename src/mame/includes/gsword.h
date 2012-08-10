class gsword_state : public driver_device
{
public:
	gsword_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spritetile_ram(*this, "spritetile_ram"),
		m_spritexy_ram(*this, "spritexy_ram"),
		m_spriteattrib_ram(*this, "spriteattram"),
		m_videoram(*this, "videoram"),
		m_cpu2_ram(*this, "cpu2_ram"){ }

	required_shared_ptr<UINT8> m_spritetile_ram;
	required_shared_ptr<UINT8> m_spritexy_ram;
	required_shared_ptr<UINT8> m_spriteattrib_ram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_cpu2_ram;

	int m_coins;
	int m_fake8910_0;
	int m_fake8910_1;
	int m_nmi_enable;
	int m_protect_hack;
	int m_charbank;
	int m_charpalbank;
	int m_flipscreen;
	tilemap_t *m_bg_tilemap;

	DECLARE_WRITE8_MEMBER(gsword_videoram_w);
	DECLARE_WRITE8_MEMBER(gsword_charbank_w);
	DECLARE_WRITE8_MEMBER(gsword_videoctrl_w);
	DECLARE_WRITE8_MEMBER(gsword_scroll_w);
	DECLARE_READ8_MEMBER(gsword_hack_r);
	DECLARE_WRITE8_MEMBER(adpcm_soundcommand_w);
	DECLARE_WRITE8_MEMBER(gsword_nmi_set_w);
	DECLARE_WRITE8_MEMBER(gsword_AY8910_control_port_0_w);
	DECLARE_WRITE8_MEMBER(gsword_AY8910_control_port_1_w);
	DECLARE_READ8_MEMBER(gsword_fake_0_r);
	DECLARE_READ8_MEMBER(gsword_fake_1_r);
	DECLARE_WRITE8_MEMBER(gsword_adpcm_data_w);
	DECLARE_DRIVER_INIT(gsword);
	DECLARE_DRIVER_INIT(gsword2);
};


/*----------- defined in video/gsword.c -----------*/


PALETTE_INIT( josvolly );
PALETTE_INIT( gsword );
VIDEO_START( gsword );
SCREEN_UPDATE_IND16( gsword );
