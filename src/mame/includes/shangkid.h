class shangkid_state : public driver_device
{
public:
	shangkid_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_videoreg(*this, "videoreg"){ }

	required_shared_ptr<UINT8> m_videoram;
	optional_shared_ptr<UINT8> m_spriteram;
	UINT8 m_bbx_sound_enable;
	UINT8 m_sound_latch;
	optional_shared_ptr<UINT8> m_videoreg;
	int m_gfx_type;
	tilemap_t *m_background;
	DECLARE_WRITE8_MEMBER(shangkid_maincpu_bank_w);
	DECLARE_WRITE8_MEMBER(shangkid_bbx_enable_w);
	DECLARE_WRITE8_MEMBER(shangkid_cpu_reset_w);
	DECLARE_WRITE8_MEMBER(shangkid_sound_enable_w);
	DECLARE_READ8_MEMBER(shangkid_soundlatch_r);
	DECLARE_WRITE8_MEMBER(shangkid_videoram_w);
	DECLARE_WRITE8_MEMBER(chinhero_ay8910_porta_w);
	DECLARE_WRITE8_MEMBER(shangkid_ay8910_porta_w);
	DECLARE_WRITE8_MEMBER(ay8910_portb_w);
};


/*----------- defined in video/shangkid.c -----------*/

VIDEO_START( shangkid );
SCREEN_UPDATE_IND16( shangkid );

PALETTE_INIT( dynamski );
SCREEN_UPDATE_IND16( dynamski );

