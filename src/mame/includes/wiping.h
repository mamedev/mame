class wiping_state : public driver_device
{
public:
	wiping_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *sharedram1;
	UINT8 *sharedram2;
	UINT8 *videoram;
	UINT8 *colorram;
	int flipscreen;
	UINT8 *soundregs;
};


/*----------- defined in video/wiping.c -----------*/

WRITE8_HANDLER( wiping_flipscreen_w );
PALETTE_INIT( wiping );
SCREEN_UPDATE( wiping );

