class calomega_state : public driver_device
{
public:
	calomega_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 tx_line;
	UINT8 rx_line;
	int s903_mux_data;
	int s905_mux_data;
	UINT8 *videoram;
	UINT8 *colorram;
	tilemap_t *bg_tilemap;
};


/*----------- defined in video/calomega.c -----------*/

WRITE8_HANDLER( calomega_videoram_w );
WRITE8_HANDLER( calomega_colorram_w );
PALETTE_INIT( calomega );
VIDEO_START( calomega );
SCREEN_UPDATE( calomega );
