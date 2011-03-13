class trucocl_state : public driver_device
{
public:
	trucocl_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int cur_dac_address;
	int cur_dac_address_index;
	UINT8 *videoram;
	UINT8 *colorram;
	tilemap_t *bg_tilemap;
};


/*----------- defined in video/trucocl.c -----------*/

WRITE8_HANDLER( trucocl_videoram_w );
WRITE8_HANDLER( trucocl_colorram_w );
PALETTE_INIT( trucocl );
VIDEO_START( trucocl );
SCREEN_UPDATE( trucocl );
