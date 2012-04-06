class m90_state : public driver_device
{
public:
	m90_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_video_data;
	UINT16 *m_spriteram;
	size_t m_spriteram_size;
	UINT16 m_video_control_data[8];
	tilemap_t *m_pf1_layer;
	tilemap_t *m_pf2_layer;
	tilemap_t *m_pf1_wide_layer;
	tilemap_t *m_pf2_wide_layer;
	UINT8 m_last_pf1;
	UINT8 m_last_pf2;
	device_t *m_audio;
	DECLARE_WRITE16_MEMBER(m90_coincounter_w);
	DECLARE_WRITE16_MEMBER(quizf1_bankswitch_w);
	DECLARE_WRITE16_MEMBER(dynablsb_sound_command_w);
	DECLARE_WRITE16_MEMBER(unknown_w);
	DECLARE_WRITE16_MEMBER(m90_video_control_w);
	DECLARE_WRITE16_MEMBER(m90_video_w);
};


/*----------- defined in video/m90.c -----------*/

VIDEO_START( m90 );
VIDEO_START( dynablsb );
VIDEO_START( bomblord );
SCREEN_UPDATE_IND16( m90 );
SCREEN_UPDATE_IND16( dynablsb );
SCREEN_UPDATE_IND16( bomblord );
