class rpunch_state : public driver_device
{
public:
	rpunch_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_bitmapram(*this, "bitmapram"),
		m_spriteram(*this, "spriteram"){ }

	required_shared_ptr<UINT16> m_videoram;
	UINT8 m_sound_data;
	UINT8 m_sound_busy;
	UINT8 m_ym2151_irq;
	UINT8 m_upd_rom_bank;
	required_shared_ptr<UINT16> m_bitmapram;
	int m_sprite_palette;
	tilemap_t *m_background[2];
	UINT16 m_videoflags;
	UINT8 m_crtc_register;
	emu_timer *m_crtc_timer;
	UINT8 m_bins;
	UINT8 m_gins;
	required_shared_ptr<UINT16> m_spriteram;
	DECLARE_WRITE16_MEMBER(sound_command_w);
	DECLARE_READ8_MEMBER(sound_command_r);
	DECLARE_READ16_MEMBER(sound_busy_r);
	DECLARE_WRITE16_MEMBER(rpunch_videoram_w);
	DECLARE_WRITE16_MEMBER(rpunch_videoreg_w);
	DECLARE_WRITE16_MEMBER(rpunch_scrollreg_w);
	DECLARE_WRITE16_MEMBER(rpunch_crtc_data_w);
	DECLARE_WRITE16_MEMBER(rpunch_crtc_register_w);
	DECLARE_WRITE16_MEMBER(rpunch_ins_w);
	DECLARE_CUSTOM_INPUT_MEMBER(hi_bits_r);
	DECLARE_WRITE8_MEMBER(upd_control_w);
	DECLARE_WRITE8_MEMBER(upd_data_w);
};


/*----------- defined in video/rpunch.c -----------*/

VIDEO_START( rpunch );
SCREEN_UPDATE_IND16( rpunch );

