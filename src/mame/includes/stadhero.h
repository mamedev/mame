class stadhero_state : public driver_device
{
public:
	stadhero_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *pf1_data;
	UINT16 *pf2_control_0;
	UINT16 *pf2_control_1;
	UINT16 *pf2_data;
	tilemap_t *pf1_tilemap;
	tilemap_t *pf2_tilemap;
	int flipscreen;
};


/*----------- defined in video/stadhero.c -----------*/

VIDEO_START( stadhero );
VIDEO_UPDATE( stadhero );

WRITE16_HANDLER( stadhero_pf1_data_w );
READ16_HANDLER( stadhero_pf2_data_r );
WRITE16_HANDLER( stadhero_pf2_data_w );
