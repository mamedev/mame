class mystwarr_state : public konamigx_state
{
public:
	mystwarr_state(const machine_config &mconfig, device_type type, const char *tag)
		: konamigx_state(mconfig, type, tag),
		m_gx_workram(*this,"gx_workram"),
		m_spriteram(*this,"spriteram"),
		m_maincpu(*this,"maincpu")
		{ }

	required_shared_ptr<UINT16> m_gx_workram;
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
	optional_shared_ptr<UINT16> m_spriteram;

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
	TILE_GET_INFO_MEMBER(get_gai_936_tile_info);
	TILE_GET_INFO_MEMBER(get_ult_936_tile_info);
	DECLARE_MACHINE_START(mystwarr);
	DECLARE_MACHINE_RESET(mystwarr);
	DECLARE_VIDEO_START(mystwarr);
	DECLARE_MACHINE_RESET(viostorm);
	DECLARE_VIDEO_START(viostorm);
	DECLARE_MACHINE_RESET(metamrph);
	DECLARE_VIDEO_START(metamrph);
	DECLARE_MACHINE_RESET(dadandrn);
	DECLARE_VIDEO_START(dadandrn);
	DECLARE_MACHINE_RESET(gaiapols);
	DECLARE_VIDEO_START(gaiapols);
	DECLARE_MACHINE_RESET(martchmp);
	DECLARE_VIDEO_START(martchmp);
	UINT32 screen_update_mystwarr(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_metamrph(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_dadandrn(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_martchmp(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(ddd_interrupt);
};
