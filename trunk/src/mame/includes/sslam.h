class sslam_state : public driver_device
{
public:
	sslam_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	emu_timer *m_music_timer;

	int m_sound;
	int m_melody;
	int m_bar;
	int m_track;
	int m_snd_bank;

	UINT16 *m_bg_tileram;
	UINT16 *m_tx_tileram;
	UINT16 *m_md_tileram;
	UINT16 *m_spriteram;
	UINT16 *m_regs;

	UINT8 m_oki_control;
	UINT8 m_oki_command;
	UINT8 m_oki_bank;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_tx_tilemap;
	tilemap_t *m_md_tilemap;

	int m_sprites_x_offset;
};


/*----------- defined in video/sslam.c -----------*/

WRITE16_HANDLER( sslam_paletteram_w );
WRITE16_HANDLER( sslam_tx_tileram_w );
WRITE16_HANDLER( sslam_md_tileram_w );
WRITE16_HANDLER( sslam_bg_tileram_w );
WRITE16_HANDLER( powerbls_bg_tileram_w );
VIDEO_START(sslam);
VIDEO_START(powerbls);
SCREEN_UPDATE(sslam);
SCREEN_UPDATE(powerbls);
