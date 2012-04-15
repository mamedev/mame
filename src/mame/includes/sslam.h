class sslam_state : public driver_device
{
public:
	sslam_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_bg_tileram(*this, "bg_tileram"),
		m_md_tileram(*this, "md_tileram"),
		m_tx_tileram(*this, "tx_tileram"),
		m_regs(*this, "regs"),
		m_spriteram(*this, "spriteram"){ }

	emu_timer *m_music_timer;

	int m_sound;
	int m_melody;
	int m_bar;
	int m_track;
	int m_snd_bank;

	required_shared_ptr<UINT16> m_bg_tileram;
	required_shared_ptr<UINT16> m_md_tileram;
	required_shared_ptr<UINT16> m_tx_tileram;
	required_shared_ptr<UINT16> m_regs;
	required_shared_ptr<UINT16> m_spriteram;

	UINT8 m_oki_control;
	UINT8 m_oki_command;
	UINT8 m_oki_bank;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_tx_tilemap;
	tilemap_t *m_md_tilemap;

	int m_sprites_x_offset;
	DECLARE_WRITE16_MEMBER(powerbls_sound_w);
	DECLARE_READ8_MEMBER(playmark_snd_command_r);
	DECLARE_WRITE8_MEMBER(playmark_oki_w);
	DECLARE_WRITE8_MEMBER(playmark_snd_control_w);
	DECLARE_WRITE16_MEMBER(sslam_paletteram_w);
	DECLARE_WRITE16_MEMBER(sslam_tx_tileram_w);
	DECLARE_WRITE16_MEMBER(sslam_md_tileram_w);
	DECLARE_WRITE16_MEMBER(sslam_bg_tileram_w);
	DECLARE_WRITE16_MEMBER(powerbls_bg_tileram_w);
};


/*----------- defined in video/sslam.c -----------*/

VIDEO_START(sslam);
VIDEO_START(powerbls);
SCREEN_UPDATE_IND16(sslam);
SCREEN_UPDATE_IND16(powerbls);
