class shangkid_state : public driver_device
{
public:
	shangkid_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 *m_spriteram;
	UINT8 m_bbx_sound_enable;
	UINT8 m_sound_latch;
	UINT8 *m_videoreg;
	int m_gfx_type;
	tilemap_t *m_background;
	DECLARE_WRITE8_MEMBER(shangkid_maincpu_bank_w);
	DECLARE_WRITE8_MEMBER(shangkid_bbx_enable_w);
	DECLARE_WRITE8_MEMBER(shangkid_cpu_reset_w);
	DECLARE_WRITE8_MEMBER(shangkid_sound_enable_w);
	DECLARE_READ8_MEMBER(shangkid_soundlatch_r);
	DECLARE_WRITE8_MEMBER(shangkid_videoram_w);
};


/*----------- defined in video/shangkid.c -----------*/

VIDEO_START( shangkid );
SCREEN_UPDATE_IND16( shangkid );

PALETTE_INIT( dynamski );
SCREEN_UPDATE_IND16( dynamski );

