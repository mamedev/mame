class xorworld_state : public driver_device
{
public:
	xorworld_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *videoram;
};


/*----------- defined in video/xorworld.c -----------*/

WRITE16_HANDLER( xorworld_videoram16_w );

PALETTE_INIT( xorworld );
VIDEO_START( xorworld );
VIDEO_UPDATE( xorworld );
