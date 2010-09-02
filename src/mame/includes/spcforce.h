class spcforce_state : public driver_device
{
public:
	spcforce_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *scrollram;
	UINT8 *videoram;
	UINT8 *colorram;

	int sn76496_latch;
	int sn76496_select;
};


/*----------- defined in video/spcforce.c -----------*/

WRITE8_HANDLER( spcforce_flip_screen_w );
VIDEO_UPDATE( spcforce );
