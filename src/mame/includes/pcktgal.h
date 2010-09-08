class pcktgal_state : public driver_device
{
public:
	pcktgal_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
};


/*----------- defined in video/pcktgal.c -----------*/

WRITE8_HANDLER( pcktgal_videoram_w );
WRITE8_HANDLER( pcktgal_flipscreen_w );

PALETTE_INIT( pcktgal );
VIDEO_START( pcktgal );
VIDEO_UPDATE( pcktgal );
