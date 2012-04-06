class speedatk_state : public driver_device
{
public:
	speedatk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 *m_colorram;
	UINT8 m_crtc_vreg[0x100];
	UINT8 m_crtc_index;
	UINT8 m_flip_scr;

	UINT8 m_mux_data;
	UINT8 m_km_status;
	UINT8 m_coin_settings;
	UINT8 m_coin_impulse;
	DECLARE_READ8_MEMBER(key_matrix_r);
	DECLARE_WRITE8_MEMBER(key_matrix_w);
	DECLARE_READ8_MEMBER(key_matrix_status_r);
	DECLARE_WRITE8_MEMBER(key_matrix_status_w);
	DECLARE_WRITE8_MEMBER(speedatk_videoram_w);
	DECLARE_WRITE8_MEMBER(speedatk_colorram_w);
	DECLARE_WRITE8_MEMBER(speedatk_6845_w);
};


/*----------- defined in video/speedatk.c -----------*/

PALETTE_INIT( speedatk );
VIDEO_START( speedatk );
SCREEN_UPDATE_IND16( speedatk );
