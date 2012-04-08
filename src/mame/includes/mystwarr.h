class mystwarr_state : public konamigx_state
{
public:
	mystwarr_state(const machine_config &mconfig, device_type type, const char *tag)
		: konamigx_state(mconfig, type, tag),
		m_maincpu(*this,"maincpu")
		{ }

	UINT16 *m_gx_workram;
	UINT8 m_mw_irq_control;
	int m_cur_sound_region;
	int m_layer_colorbase[6];
	int m_oinprion;
	int m_cbparam;
	int m_sprite_colorbase;
	int m_sub1_colorbase;
	int m_last_psac_colorbase;
	int m_gametype;
	int m_roz_enable;
	int m_roz_rombank;
	tilemap_t *m_ult_936_tilemap;
	UINT16 m_clip;
	UINT16 *m_spriteram;

	required_device<cpu_device> m_maincpu;
	DECLARE_READ16_MEMBER(eeprom_r);
	DECLARE_WRITE16_MEMBER(mweeprom_w);
	DECLARE_READ16_MEMBER(dddeeprom_r);
	DECLARE_WRITE16_MEMBER(mmeeprom_w);
	DECLARE_WRITE16_MEMBER(sound_cmd1_w);
	DECLARE_WRITE16_MEMBER(sound_cmd1_msb_w);
	DECLARE_WRITE16_MEMBER(sound_cmd2_w);
	DECLARE_WRITE16_MEMBER(sound_cmd2_msb_w);
	DECLARE_WRITE16_MEMBER(sound_irq_w);
	DECLARE_READ16_MEMBER(sound_status_r);
	DECLARE_READ16_MEMBER(sound_status_msb_r);
	DECLARE_WRITE16_MEMBER(irq_ack_w);
	DECLARE_READ16_MEMBER(K053247_scattered_word_r);
	DECLARE_WRITE16_MEMBER(K053247_scattered_word_w);
	DECLARE_READ16_MEMBER(K053247_martchmp_word_r);
	DECLARE_WRITE16_MEMBER(K053247_martchmp_word_w);
	DECLARE_READ16_MEMBER(mccontrol_r);
	DECLARE_WRITE16_MEMBER(mccontrol_w);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);

	DECLARE_WRITE16_MEMBER(ddd_053936_enable_w);
	DECLARE_WRITE16_MEMBER(ddd_053936_clip_w);
	DECLARE_READ16_MEMBER(gai_053936_tilerom_0_r);
	DECLARE_READ16_MEMBER(ddd_053936_tilerom_0_r);
	DECLARE_READ16_MEMBER(ddd_053936_tilerom_1_r);
	DECLARE_READ16_MEMBER(gai_053936_tilerom_2_r);
	DECLARE_READ16_MEMBER(ddd_053936_tilerom_2_r);
};


/*----------- defined in video/mystwarr.c -----------*/

VIDEO_START( gaiapols );
VIDEO_START( dadandrn );
VIDEO_START( viostorm );
VIDEO_START( metamrph );
VIDEO_START( martchmp );
VIDEO_START( mystwarr );
SCREEN_UPDATE_RGB32( dadandrn );
SCREEN_UPDATE_RGB32( mystwarr );
SCREEN_UPDATE_RGB32( metamrph );
SCREEN_UPDATE_RGB32( martchmp );

