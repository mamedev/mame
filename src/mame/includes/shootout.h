class shootout_state : public driver_device
{
public:
	shootout_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	tilemap_t *background;
	tilemap_t *foreground;
	UINT8 *spriteram;
	UINT8 *videoram;
	UINT8 *textram;
};


/*----------- defined in video/shootout.c -----------*/

WRITE8_HANDLER( shootout_videoram_w );
WRITE8_HANDLER( shootout_textram_w );

PALETTE_INIT( shootout );
VIDEO_START( shootout );
VIDEO_UPDATE( shootout );
VIDEO_UPDATE( shootouj );
