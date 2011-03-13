class speedbal_state : public driver_device
{
public:
	speedbal_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *background_videoram;
	UINT8 *foreground_videoram;
	tilemap_t *bg_tilemap;
	tilemap_t *fg_tilemap;
	UINT8 *spriteram;
	size_t spriteram_size;
};


/*----------- defined in video/speedbal.c -----------*/

VIDEO_START( speedbal );
SCREEN_UPDATE( speedbal );
WRITE8_HANDLER( speedbal_foreground_videoram_w );
WRITE8_HANDLER( speedbal_background_videoram_w );
