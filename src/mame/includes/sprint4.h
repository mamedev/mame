class sprint4_state : public driver_device
{
public:
	sprint4_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	int da_latch;
	int steer_FF1[4];
	int steer_FF2[4];
	int gear[4];
	UINT8 last_wheel[4];
	int collision[4];
	tilemap_t* playfield;
	bitmap_t* helper;
};


/*----------- defined in video/sprint4.c -----------*/

PALETTE_INIT( sprint4 );

SCREEN_EOF( sprint4 );
VIDEO_START( sprint4 );
SCREEN_UPDATE( sprint4 );

WRITE8_HANDLER( sprint4_video_ram_w );
