class stadhero_state : public driver_device
{
public:
	stadhero_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *m_pf1_data;
	tilemap_t *m_pf1_tilemap;
	int m_flipscreen;
	UINT16 *m_spriteram;
};


/*----------- defined in video/stadhero.c -----------*/

VIDEO_START( stadhero );
SCREEN_UPDATE( stadhero );

WRITE16_HANDLER( stadhero_pf1_data_w );
