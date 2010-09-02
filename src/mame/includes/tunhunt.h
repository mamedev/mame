class tunhunt_state : public driver_device
{
public:
	tunhunt_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 control;
	UINT8 *workram;
	UINT8 *spriteram;
	UINT8 *videoram;
	tilemap_t *fg_tilemap;
};


/*----------- defined in video/tunhunt.c -----------*/

WRITE8_HANDLER( tunhunt_videoram_w );

PALETTE_INIT( tunhunt );
VIDEO_START( tunhunt );
VIDEO_UPDATE( tunhunt );
