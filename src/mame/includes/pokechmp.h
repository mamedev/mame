class pokechmp_state : public driver_device
{
public:
	pokechmp_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	tilemap_t *bg_tilemap;
};


/*----------- defined in video/pokechmp.c -----------*/

WRITE8_HANDLER( pokechmp_videoram_w );
WRITE8_HANDLER( pokechmp_flipscreen_w );

VIDEO_START( pokechmp );
SCREEN_UPDATE( pokechmp );
