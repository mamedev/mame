class m90_state : public driver_device
{
public:
	m90_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT32 m_bankaddress;
	UINT16 *m_video_data;
	UINT16 *m_spriteram;
	size_t m_spriteram_size;
	UINT16 m_video_control_data[8];
	tilemap_t *m_pf1_layer;
	tilemap_t *m_pf2_layer;
	tilemap_t *m_pf1_wide_layer;
	tilemap_t *m_pf2_wide_layer;
	int m_last_pf1;
	int m_last_pf2;
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
