class truco_state : public driver_device
{
public:
	truco_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
};


/*----------- defined in video/truco.c -----------*/

VIDEO_UPDATE( truco );
PALETTE_INIT( truco );
