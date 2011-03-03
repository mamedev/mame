class raiden_state : public driver_device
{
public:
	raiden_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *videoram;
	UINT16 *shared_ram;
	UINT16 *back_data;
	UINT16 *fore_data;
	UINT16 *scroll_ram;
	tilemap_t *bg_layer;
	tilemap_t *fg_layer;
	tilemap_t *tx_layer;
	int flipscreen;
	int alternate;
};


/*----------- defined in video/raiden.c -----------*/

WRITE16_HANDLER( raiden_background_w );
WRITE16_HANDLER( raiden_foreground_w );
WRITE16_HANDLER( raiden_text_w );
VIDEO_START( raiden );
VIDEO_START( raidena );
WRITE16_HANDLER( raiden_control_w );
WRITE16_HANDLER( raidena_control_w );
SCREEN_UPDATE( raiden );
