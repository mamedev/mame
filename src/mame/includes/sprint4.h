class sprint4_state : public driver_device
{
public:
	sprint4_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
};


/*----------- defined in video/sprint4.c -----------*/

extern int sprint4_collision[4];

PALETTE_INIT( sprint4 );

VIDEO_EOF( sprint4 );
VIDEO_START( sprint4 );
VIDEO_UPDATE( sprint4 );

WRITE8_HANDLER( sprint4_video_ram_w );
