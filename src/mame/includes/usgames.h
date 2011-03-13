class usgames_state : public driver_device
{
public:
	usgames_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	UINT8 *charram;
	tilemap_t *tilemap;
};


/*----------- defined in video/usgames.c -----------*/

WRITE8_HANDLER( usgames_videoram_w );
WRITE8_HANDLER( usgames_charram_w );
VIDEO_START( usgames );
PALETTE_INIT( usgames );
SCREEN_UPDATE( usgames );
