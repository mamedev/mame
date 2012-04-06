class thedeep_state : public driver_device
{
public:
	thedeep_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu")
		{ }

	UINT8 *m_spriteram;
	size_t m_spriteram_size;
	int m_nmi_enable;
	UINT8 m_protection_command;
	UINT8 m_protection_data;
	int m_protection_index;
	int m_protection_irq;
	int m_rombank;
	UINT8 *m_vram_0;
	UINT8 *m_vram_1;
	UINT8 *m_scroll;
	UINT8 *m_scroll2;
	tilemap_t *m_tilemap_0;
	tilemap_t *m_tilemap_1;
	UINT8 m_mcu_p3_reg;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_mcu;
	DECLARE_WRITE8_MEMBER(thedeep_nmi_w);
	DECLARE_WRITE8_MEMBER(thedeep_sound_w);
	DECLARE_WRITE8_MEMBER(thedeep_protection_w);
	DECLARE_READ8_MEMBER(thedeep_e004_r);
	DECLARE_READ8_MEMBER(thedeep_protection_r);
	DECLARE_WRITE8_MEMBER(thedeep_e100_w);
	DECLARE_WRITE8_MEMBER(thedeep_p1_w);
	DECLARE_READ8_MEMBER(thedeep_from_main_r);
	DECLARE_WRITE8_MEMBER(thedeep_to_main_w);
	DECLARE_WRITE8_MEMBER(thedeep_p3_w);
	DECLARE_READ8_MEMBER(thedeep_p0_r);
	DECLARE_WRITE8_MEMBER(thedeep_vram_0_w);
	DECLARE_WRITE8_MEMBER(thedeep_vram_1_w);
};


/*----------- defined in video/thedeep.c -----------*/


PALETTE_INIT( thedeep );
VIDEO_START( thedeep );
SCREEN_UPDATE_IND16( thedeep );

