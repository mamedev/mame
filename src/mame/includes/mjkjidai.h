class mjkjidai_state : public driver_device
{
public:
	mjkjidai_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	UINT8 *spriteram1;
	UINT8 *spriteram2;
	UINT8 *spriteram3;

	int keyb;
	int nvram_init_count;
	UINT8 *nvram;
	size_t nvram_size;
};


/*----------- defined in video/mjkjidai.c -----------*/

VIDEO_START( mjkjidai );
VIDEO_UPDATE( mjkjidai );
WRITE8_HANDLER( mjkjidai_videoram_w );
WRITE8_HANDLER( mjkjidai_ctrl_w );


