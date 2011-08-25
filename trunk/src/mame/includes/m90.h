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
};


/*----------- defined in video/m90.c -----------*/

VIDEO_START( m90 );
VIDEO_START( dynablsb );
VIDEO_START( bomblord );
SCREEN_UPDATE( m90 );
SCREEN_UPDATE( dynablsb );
SCREEN_UPDATE( bomblord );
WRITE16_HANDLER( m90_video_w );
WRITE16_HANDLER( m90_video_control_w );
