class toki_state : public driver_device
{
public:
	toki_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *videoram;
	int msm5205next;
	int toggle;
	UINT16 *background1_videoram16;
	UINT16 *background2_videoram16;
	UINT16 *scrollram16;
	tilemap_t *background_layer;
	tilemap_t *foreground_layer;
	tilemap_t *text_layer;
};


/*----------- defined in video/toki.c -----------*/

VIDEO_START( toki );
SCREEN_EOF( toki );
SCREEN_EOF( tokib );
SCREEN_UPDATE( toki );
SCREEN_UPDATE( tokib );
WRITE16_HANDLER( toki_background1_videoram16_w );
WRITE16_HANDLER( toki_background2_videoram16_w );
WRITE16_HANDLER( toki_control_w );
WRITE16_HANDLER( toki_foreground_videoram16_w );
